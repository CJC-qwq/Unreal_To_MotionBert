import argparse
import math
import os
import sys
import time
import threading
from collections import deque
from pathlib import Path
from types import SimpleNamespace

import cv2
import numpy as np
import torch

ROOT_DIR = Path(__file__).resolve().parent
ALPHAPOSE_DIR = ROOT_DIR / ".external" / "AlphaPose"

os.environ.setdefault("MPLCONFIGDIR", str(ROOT_DIR / "alpha_cache" / "matplotlib"))
os.environ.setdefault("TORCH_HOME", str(ROOT_DIR / "alpha_cache" / "torch"))

if str(ALPHAPOSE_DIR) not in sys.path:
    sys.path.insert(0, str(ALPHAPOSE_DIR))
if str(ROOT_DIR) not in sys.path:
    sys.path.insert(0, str(ROOT_DIR))

from alphapose.models import builder
from alphapose.utils.config import update_config
from alphapose.utils.pPose_nms import pose_nms
from alphapose.utils.presets import SimpleTransform, SimpleTransform3DSMPL
from alphapose.utils.transforms import flip, flip_heatmap, get_func_heatmap_to_coord
from alphapose.utils.vis import getTime
from lib.utils.learning import load_backbone, load_pretrained_weights
from lib.utils.tools import get_config
from lib.utils.ue_livelink import UnrealLiveLinkSender
from lib.utils.utils_data import crop_scale, flip_data


SKELETON_EDGES = [
    (0, 1), (1, 2), (2, 3),
    (0, 4), (4, 5), (5, 6),
    (0, 7), (7, 8), (8, 9),
    (8, 11), (8, 14),
    (9, 10), (11, 12), (12, 13),
    (14, 15), (15, 16),
]

LEFT_JOINTS = {4, 5, 6, 11, 12, 13}
RIGHT_JOINTS = {1, 2, 3, 14, 15, 16}

def halpe2h36m(x):
    num_frames, _, dims = x.shape
    y = np.zeros((num_frames, 17, dims), dtype=x.dtype)
    y[:, 0, :] = x[:, 19, :]
    y[:, 1, :] = x[:, 12, :]
    y[:, 2, :] = x[:, 14, :]
    y[:, 3, :] = x[:, 16, :]
    y[:, 4, :] = x[:, 11, :]
    y[:, 5, :] = x[:, 13, :]
    y[:, 6, :] = x[:, 15, :]
    y[:, 7, :] = (x[:, 18, :] + x[:, 19, :]) * 0.5
    y[:, 8, :] = x[:, 18, :]
    y[:, 9, :] = x[:, 0, :]
    y[:, 10, :] = x[:, 17, :]
    y[:, 11, :] = x[:, 5, :]
    y[:, 12, :] = x[:, 7, :]
    y[:, 13, :] = x[:, 9, :]
    y[:, 14, :] = x[:, 6, :]
    y[:, 15, :] = x[:, 8, :]
    y[:, 16, :] = x[:, 10, :]
    return y


def parse_args():
    parser = argparse.ArgumentParser(description="Realtime MotionBERT 3D pose demo")
    parser.add_argument("--source", default="0", help="Camera index like 0 or a video file path")
    parser.add_argument("--camera_width", type=int, default=1280, help="Requested camera width")
    parser.add_argument("--camera_height", type=int, default=720, help="Requested camera height")
    parser.add_argument("--camera_fps", type=int, default=30, help="Requested camera FPS")
    parser.add_argument("--max_frames", type=int, default=0, help="Stop after N frames, 0 means no limit")
    parser.add_argument("--min_frames", type=int, default=27, help="Minimum buffered frames before 3D inference")
    parser.add_argument("--clip_len", type=int, default=243, help="Max sequence length for MotionBERT")
    parser.add_argument("--focus", type=int, default=None, help="Optional tracked person id to focus on")
    parser.add_argument("--display_scale", type=float, default=1.0, help="Resize display window output")
    parser.add_argument("--no_display", action="store_true", help="Run inference without opening a window")
    parser.add_argument("--save_video", type=str, default="", help="Optional path to save side-by-side output")
    parser.add_argument("--show_fps", action="store_true", help="Overlay smoothed FPS")
    parser.add_argument("--motion_smooth", type=float, default=0.0, help="EMA smoothing for 3D pose. 0 disables, higher is smoother but adds lag")
    parser.add_argument("--infer_every_n", type=int, default=1, help="Run 3D inference every N frames after warmup. 1 means every frame")
    parser.add_argument("--ue_livelink_host", type=str, default="", help="Optional UDP host for Unreal Live Link streaming")
    parser.add_argument("--ue_livelink_port", type=int, default=7001, help="UDP port for Unreal Live Link streaming")
    parser.add_argument("--ue_livelink_subject", type=str, default="MotionBERTPose", help="Live Link subject name")
    parser.add_argument("--ue_livelink_scale", type=float, default=100.0, help="Scale factor when converting MotionBERT units to Unreal centimeters")
    parser.add_argument("--ue_send_fps", type=float, default=60.0, help="Fixed UE packet send rate using the latest available 3D pose")
    parser.add_argument("--ue_interpolate", action="store_true", help="Resample irregular 3D updates into smoother fixed-rate UE output")
    parser.add_argument("--ue_buffer_ms", type=float, default=45.0, help="Interpolation buffer in milliseconds for UE output resampling")

    parser.add_argument("--config", type=str, default="configs/pose3d/MB_ft_h36m_global_lite.yaml")
    parser.add_argument("--evaluate", type=str, default="checkpoint/pose3d/FT_MB_lite_MB_ft_h36m_global_lite/best_epoch.bin")
    parser.add_argument("--alphapose_cfg", type=str, default=".external/AlphaPose/configs/halpe_26/resnet/256x192_res50_lr1e-3_1x.yaml")
    parser.add_argument("--alphapose_ckpt", type=str, default=".external/AlphaPose/pretrained_models/halpe26_fast_res50_256x192.pth")
    parser.add_argument("--detector", type=str, default="yolox-x")
    parser.add_argument("--gpus", type=str, default="0", help="GPU ids, use -1 for CPU")
    parser.add_argument("--flip", action="store_true", help="Enable AlphaPose flip testing")
    parser.add_argument("--vis_fast", action="store_true", help="Use AlphaPose fast 2D drawing")
    parser.add_argument("--detbatch", type=int, default=1)
    parser.add_argument("--posebatch", type=int, default=1)
    parser.add_argument("--detfile", type=str, default="")
    parser.add_argument("--qsize", type=int, default=128)
    parser.add_argument("--save_img", action="store_true")
    parser.add_argument("--save_video_alpha", action="store_true")
    parser.add_argument("--pose_flow", action="store_true")
    parser.add_argument("--pose_track", action="store_true")
    parser.add_argument("--format", type=str, default="coco")
    parser.add_argument("--eval_alpha", action="store_true")
    parser.add_argument("--profile", action="store_true")
    parser.add_argument("--showbox", action="store_true")
    parser.add_argument("--debug", action="store_true")
    parser.add_argument("--min_box_area", type=int, default=0)
    return parser.parse_args()


def resolve_path(path_str):
    path = Path(path_str)
    if path.is_absolute():
        return path
    return ROOT_DIR / path


def parse_gpu_args(gpus):
    if gpus.strip() == "-1" or not torch.cuda.is_available():
        return [-1], torch.device("cpu")
    gpu_ids = [int(item.strip()) for item in gpus.split(",") if item.strip()]
    if not gpu_ids:
        gpu_ids = [0]
    return gpu_ids, torch.device(f"cuda:{gpu_ids[0]}")


def build_detector(opt):
    detector_name = opt.detector.lower()
    if detector_name == "yolox":
        detector_name = "yolox-x"
        opt.detector = detector_name

    if "yolox" in detector_name:
        from detector.yolox_api import YOLOXDetector
        from detector.yolox_cfg import cfg as yolox_cfg

        yolox_cfg.MODEL_NAME = detector_name
        yolox_cfg.MODEL_WEIGHTS = str(ALPHAPOSE_DIR / "detector" / "yolox" / "data" / f"{detector_name.replace('-', '_')}.pth")
        return YOLOXDetector(yolox_cfg, opt)

    raise NotImplementedError(f"Detector '{opt.detector}' is not supported in realtime mode yet.")


class DetectionLoader:
    def __init__(self, detector, cfg, opt):
        self.cfg = cfg
        self.opt = opt
        self.device = opt.device
        self.detector = detector

        self._input_size = cfg.DATA_PRESET.IMAGE_SIZE
        self._output_size = cfg.DATA_PRESET.HEATMAP_SIZE
        self._sigma = cfg.DATA_PRESET.SIGMA

        if cfg.DATA_PRESET.TYPE == "simple":
            pose_dataset = builder.retrieve_dataset(self.cfg.DATASET.TRAIN)
            self.transformation = SimpleTransform(
                pose_dataset,
                scale_factor=0,
                input_size=self._input_size,
                output_size=self._output_size,
                rot=0,
                sigma=self._sigma,
                train=False,
                add_dpg=False,
                gpu_device=self.device,
            )
        elif cfg.DATA_PRESET.TYPE == "simple_smpl":
            from easydict import EasyDict as edict

            dummy_set = edict(
                {
                    "joint_pairs_17": None,
                    "joint_pairs_24": None,
                    "joint_pairs_29": None,
                    "bbox_3d_shape": (2.2, 2.2, 2.2),
                }
            )
            self.transformation = SimpleTransform3DSMPL(
                dummy_set,
                scale_factor=cfg.DATASET.SCALE_FACTOR,
                color_factor=cfg.DATASET.COLOR_FACTOR,
                occlusion=cfg.DATASET.OCCLUSION,
                input_size=cfg.MODEL.IMAGE_SIZE,
                output_size=cfg.MODEL.HEATMAP_SIZE,
                depth_dim=cfg.MODEL.EXTRA.DEPTH_DIM,
                bbox_3d_shape=(2.2, 2.2, 2.2),
                rot=cfg.DATASET.ROT_FACTOR,
                sigma=cfg.MODEL.EXTRA.SIGMA,
                train=False,
                add_dpg=False,
                gpu_device=self.device,
                loss_type=cfg.LOSS["TYPE"],
            )
        else:
            raise ValueError(f"Unsupported AlphaPose preset type: {cfg.DATA_PRESET.TYPE}")

        self.image = (None, None, None, None)
        self.det = (None, None, None, None, None, None, None)
        self.pose = (None, None, None, None, None, None, None)

    def process(self, im_name, image):
        self.image_preprocess(im_name, image)
        self.image_detection()
        self.image_postprocess()
        return self

    def image_preprocess(self, im_name, image):
        img = self.detector.image_preprocess(image)
        if isinstance(img, np.ndarray):
            img = torch.from_numpy(img)
        if img.dim() == 3:
            img = img.unsqueeze(0)
        orig_img = image
        im_dim = orig_img.shape[1], orig_img.shape[0]
        im_name = os.path.basename(im_name)

        with torch.no_grad():
            im_dim = torch.FloatTensor(im_dim).repeat(1, 2)

        self.image = (img, orig_img, im_name, im_dim)

    def image_detection(self):
        imgs, orig_imgs, im_names, im_dim_list = self.image
        if imgs is None:
            self.det = (None, None, None, None, None, None, None)
            return

        with torch.no_grad():
            dets = self.detector.images_detection(imgs, im_dim_list)
            if isinstance(dets, int) or dets.shape[0] == 0:
                self.det = (orig_imgs, im_names, None, None, None, None, None)
                return
            if isinstance(dets, np.ndarray):
                dets = torch.from_numpy(dets)
            dets = dets.cpu()
            boxes = dets[:, 1:5]
            scores = dets[:, 5:6]
            ids = torch.zeros(scores.shape)

        boxes = boxes[dets[:, 0] == 0]
        if isinstance(boxes, int) or boxes.shape[0] == 0:
            self.det = (orig_imgs, im_names, None, None, None, None, None)
            return

        inps = torch.zeros(boxes.size(0), 3, *self._input_size)
        cropped_boxes = torch.zeros(boxes.size(0), 4)
        self.det = (
            orig_imgs,
            im_names,
            boxes,
            scores[dets[:, 0] == 0],
            ids[dets[:, 0] == 0],
            inps,
            cropped_boxes,
        )

    def image_postprocess(self):
        with torch.no_grad():
            orig_img, im_name, boxes, scores, ids, inps, cropped_boxes = self.det
            if orig_img is None:
                self.pose = (None, None, None, None, None, None, None)
                return
            if boxes is None or boxes.nelement() == 0:
                self.pose = (None, orig_img, im_name, boxes, scores, ids, None)
                return

            for index, box in enumerate(boxes):
                inps[index], cropped_box = self.transformation.test_transform(orig_img, box)
                cropped_boxes[index] = torch.FloatTensor(cropped_box)

            self.pose = (inps, orig_img, im_name, boxes, scores, ids, cropped_boxes)

    def read(self):
        return self.pose


class DataWriter:
    def __init__(self, cfg, opt):
        self.cfg = cfg
        self.opt = opt
        self.eval_joints = list(range(cfg.DATA_PRESET.NUM_JOINTS))
        self.heatmap_to_coord = get_func_heatmap_to_coord(cfg)
        self.item = (None, None, None, None, None, None, None)
        self.orig_img = None

        loss_type = self.cfg.DATA_PRESET.get("LOSS_TYPE", "MSELoss")
        num_joints = self.cfg.DATA_PRESET.NUM_JOINTS
        if loss_type == "MSELoss":
            self.vis_thres = [0.4] * num_joints
        elif "JointRegression" in loss_type:
            self.vis_thres = [0.05] * num_joints
        elif loss_type == "Combined":
            hand_face_num = 42 if num_joints == 68 else 110
            self.vis_thres = [0.4] * (num_joints - hand_face_num) + [0.05] * hand_face_num
        else:
            self.vis_thres = [0.4] * num_joints

        self.use_heatmap_loss = self.cfg.DATA_PRESET.get("LOSS_TYPE", "MSELoss") == "MSELoss"
        if self.opt.vis_fast:
            from alphapose.utils.vis import vis_frame_fast as vis_frame
        else:
            from alphapose.utils.vis import vis_frame
        self.vis_frame = vis_frame

    def save(self, boxes, scores, ids, hm_data, cropped_boxes, orig_img, im_name):
        self.item = (boxes, scores, ids, hm_data, cropped_boxes, orig_img, im_name)

    def start(self):
        return self.update()

    def update(self):
        norm_type = self.cfg.LOSS.get("NORM_TYPE", None)
        hm_size = self.cfg.DATA_PRESET.HEATMAP_SIZE
        boxes, scores, ids, hm_data, cropped_boxes, orig_img, im_name = self.item
        if orig_img is None:
            return None

        orig_img = np.array(orig_img, dtype=np.uint8)[:, :, ::-1]
        self.orig_img = orig_img

        if boxes is None or len(boxes) == 0:
            return None

        assert hm_data.dim() == 4
        if hm_data.size()[1] == 136:
            self.eval_joints = list(range(136))
        elif hm_data.size()[1] == 26:
            self.eval_joints = list(range(26))
        elif hm_data.size()[1] == 133:
            self.eval_joints = list(range(133))

        pose_coords = []
        pose_scores = []
        for index in range(hm_data.shape[0]):
            bbox = cropped_boxes[index].tolist()
            if isinstance(self.heatmap_to_coord, list):
                pose_coords_body_foot, pose_scores_body_foot = self.heatmap_to_coord[0](
                    hm_data[index][self.eval_joints[:-110]],
                    bbox,
                    hm_shape=hm_size,
                    norm_type=norm_type,
                )
                pose_coords_face_hand, pose_scores_face_hand = self.heatmap_to_coord[1](
                    hm_data[index][self.eval_joints[-110:]],
                    bbox,
                    hm_shape=hm_size,
                    norm_type=norm_type,
                )
                pose_coord = np.concatenate((pose_coords_body_foot, pose_coords_face_hand), axis=0)
                pose_score = np.concatenate((pose_scores_body_foot, pose_scores_face_hand), axis=0)
            else:
                pose_coord, pose_score = self.heatmap_to_coord(
                    hm_data[index][self.eval_joints], bbox, hm_shape=hm_size, norm_type=norm_type
                )
            pose_coords.append(torch.from_numpy(pose_coord).unsqueeze(0))
            pose_scores.append(torch.from_numpy(pose_score).unsqueeze(0))

        preds_img = torch.cat(pose_coords)
        preds_scores = torch.cat(pose_scores)
        boxes, scores, ids, preds_img, preds_scores, _ = pose_nms(
            boxes,
            scores,
            ids,
            preds_img,
            preds_scores,
            self.opt.min_box_area,
            use_heatmap_loss=self.use_heatmap_loss,
        )

        result = []
        for index in range(len(scores)):
            result.append(
                {
                    "keypoints": preds_img[index],
                    "kp_score": preds_scores[index],
                    "proposal_score": torch.mean(preds_scores[index]) + scores[index] + 1.25 * max(preds_scores[index]),
                    "idx": ids[index],
                    "bbox": [
                        boxes[index][0],
                        boxes[index][1],
                        boxes[index][2] - boxes[index][0],
                        boxes[index][3] - boxes[index][1],
                    ],
                }
            )
        return {"imgname": im_name, "result": result}


class AlphaPoseSingleFrame:
    def __init__(self, args):
        self.args = args
        self.cfg = update_config(str(resolve_path(args.alphapose_cfg)))
        self.pose_model = builder.build_sppe(self.cfg.MODEL, preset_cfg=self.cfg.DATA_PRESET)

        print(f"Loading AlphaPose checkpoint {resolve_path(args.alphapose_ckpt)}")
        self.pose_model.load_state_dict(torch.load(resolve_path(args.alphapose_ckpt), map_location=args.device))
        self.pose_dataset = builder.retrieve_dataset(self.cfg.DATASET.TRAIN)
        self.pose_model.to(args.device)
        self.pose_model.eval()

        self.det_loader = DetectionLoader(build_detector(self.args), self.cfg, self.args)
        self.writer = DataWriter(self.cfg, self.args)

    def process(self, im_name, image_rgb):
        runtime_profile = {"dt": [], "pt": [], "pn": []}
        pose = None
        start_time = getTime()
        with torch.no_grad():
            inps, orig_img, im_name, boxes, scores, ids, cropped_boxes = self.det_loader.process(im_name, image_rgb).read()
            if orig_img is None:
                raise RuntimeError("No image was provided to AlphaPose")

            if boxes is None or boxes.nelement() == 0:
                if self.args.profile:
                    ckpt_time, det_time = getTime(start_time)
                    runtime_profile["dt"].append(det_time)
                self.writer.save(None, None, None, None, None, orig_img, im_name)
                pose = self.writer.start()
            else:
                if self.args.profile:
                    ckpt_time, det_time = getTime(start_time)
                    runtime_profile["dt"].append(det_time)
                inps = inps.to(self.args.device)
                if self.args.flip:
                    inps = torch.cat((inps, flip(inps)))
                hm = self.pose_model(inps)
                if self.args.flip:
                    hm_flip = flip_heatmap(hm[int(len(hm) / 2):], self.pose_dataset.joint_pairs, shift=True)
                    hm = (hm[0:int(len(hm) / 2)] + hm_flip) / 2
                if self.args.profile:
                    ckpt_time, pose_time = getTime(ckpt_time)
                    runtime_profile["pt"].append(pose_time)
                hm = hm.cpu()
                self.writer.save(boxes, scores, ids, hm, cropped_boxes, orig_img, im_name)
                pose = self.writer.start()
                if self.args.profile:
                    _, post_time = getTime(ckpt_time)
                    runtime_profile["pn"].append(post_time)

        if self.args.profile:
            print(
                "det time: {dt:.4f} | pose time: {pt:.4f} | post processing: {pn:.4f}".format(
                    dt=np.mean(runtime_profile["dt"]) if runtime_profile["dt"] else 0.0,
                    pt=np.mean(runtime_profile["pt"]) if runtime_profile["pt"] else 0.0,
                    pn=np.mean(runtime_profile["pn"]) if runtime_profile["pn"] else 0.0,
                )
            )
        return pose

    def get_bgr_image(self):
        return self.writer.orig_img

    def draw_pose(self, image_bgr, pose):
        if pose is not None:
            return self.writer.vis_frame(image_bgr, pose, self.writer.opt, self.writer.vis_thres)
        return image_bgr


class MotionBERTRealtime:
    def __init__(self, args):
        self.args = get_config(args.config)
        self.device = args.device
        self.model = load_backbone(self.args)
        self.model = self.model.to(self.device)

        checkpoint_path = resolve_path(args.evaluate)
        print(f"Loading MotionBERT checkpoint {checkpoint_path}")
        checkpoint = torch.load(checkpoint_path, map_location="cpu", weights_only=False)
        self.model = load_pretrained_weights(self.model, checkpoint["model_pos"])
        self.model.eval()

    def infer(self, motion_clip):
        clip = crop_scale(motion_clip, [1, 1]).astype(np.float32)
        batch_input = torch.from_numpy(clip).unsqueeze(0).to(self.device)

        with torch.no_grad():
            if self.args.no_conf:
                batch_input = batch_input[:, :, :, :2]
            if self.args.flip:
                batch_input_flip = flip_data(batch_input)
                predicted_3d_pos_1 = self.model(batch_input)
                predicted_3d_pos_flip = self.model(batch_input_flip)
                predicted_3d_pos_2 = flip_data(predicted_3d_pos_flip)
                predicted_3d_pos = (predicted_3d_pos_1 + predicted_3d_pos_2) / 2.0
            else:
                predicted_3d_pos = self.model(batch_input)

            if self.args.rootrel:
                predicted_3d_pos[:, :, 0, :] = 0
            else:
                predicted_3d_pos[:, 0, 0, 2] = 0
            if self.args.gt_2d:
                predicted_3d_pos[..., :2] = batch_input[..., :2]
        return predicted_3d_pos.squeeze(0).cpu().numpy()


def select_person(pose_result, focus_id=None):
    if not pose_result or not pose_result.get("result"):
        return None

    candidates = pose_result["result"]
    if focus_id is not None:
        for item in candidates:
            idx_val = item.get("idx")
            if idx_val is None:
                continue
            idx_num = int(idx_val.item()) if hasattr(idx_val, "item") else int(idx_val)
            if idx_num == focus_id:
                return item

    def score_key(item):
        bbox = item.get("bbox", [0, 0, 0, 0])
        area = float(bbox[2]) * float(bbox[3])
        proposal = item.get("proposal_score", 0.0)
        proposal = float(proposal.item()) if hasattr(proposal, "item") else float(proposal)
        return proposal, area

    return max(candidates, key=score_key)


def person_to_h36m(person_result):
    keypoints = person_result["keypoints"]
    kp_score = person_result["kp_score"]

    keypoints = keypoints.detach().cpu().numpy() if hasattr(keypoints, "detach") else np.asarray(keypoints)
    kp_score = kp_score.detach().cpu().numpy() if hasattr(kp_score, "detach") else np.asarray(kp_score)

    if kp_score.ndim == 1:
        kp_score = kp_score[:, None]

    halpe = np.concatenate([keypoints[:, :2], kp_score[:, :1]], axis=1).astype(np.float32)
    return halpe2h36m(halpe[None, ...])[0]


def empty_pose_frame():
    return np.zeros((17, 3), dtype=np.float32)


def make_rotation(yaw_deg=25.0, pitch_deg=-12.0):
    yaw = math.radians(yaw_deg)
    pitch = math.radians(pitch_deg)
    yaw_m = np.array(
        [
            [math.cos(yaw), 0.0, math.sin(yaw)],
            [0.0, 1.0, 0.0],
            [-math.sin(yaw), 0.0, math.cos(yaw)],
        ],
        dtype=np.float32,
    )
    pitch_m = np.array(
        [
            [1.0, 0.0, 0.0],
            [0.0, math.cos(pitch), -math.sin(pitch)],
            [0.0, math.sin(pitch), math.cos(pitch)],
        ],
        dtype=np.float32,
    )
    return pitch_m @ yaw_m


def render_3d_pose(joints_3d, canvas_size, fps_text="", state_text=""):
    width, height = canvas_size
    canvas = np.full((height, width, 3), 18, dtype=np.uint8)

    if joints_3d is None:
        put_status(canvas, "Waiting for 3D pose", fps_text, state_text)
        return canvas

    points = joints_3d.copy().astype(np.float32)
    points -= points[0]
    points[:, 2] *= -1.0
    rotated = points @ make_rotation().T

    scale = max(np.ptp(rotated[:, 0]), np.ptp(rotated[:, 1]), 1e-4)
    zoom = min(width, height) * 0.34 / scale

    depth = rotated[:, 2]
    perspective = 1.0 / (1.0 + np.clip(depth, -0.6, 1.4) * 0.35)
    proj_x = rotated[:, 0] * zoom * perspective + width * 0.5
    proj_y = rotated[:, 1] * zoom * perspective + height * 0.62

    for joint_a, joint_b in SKELETON_EDGES:
        point_a = (int(proj_x[joint_a]), int(proj_y[joint_a]))
        point_b = (int(proj_x[joint_b]), int(proj_y[joint_b]))
        color = (80, 200, 255)
        if joint_a in LEFT_JOINTS or joint_b in LEFT_JOINTS:
            color = (60, 190, 120)
        elif joint_a in RIGHT_JOINTS or joint_b in RIGHT_JOINTS:
            color = (255, 160, 70)
        cv2.line(canvas, point_a, point_b, color, 2, cv2.LINE_AA)

    for joint_idx, (x_coord, y_coord) in enumerate(zip(proj_x, proj_y)):
        color = (240, 240, 240)
        if joint_idx in LEFT_JOINTS:
            color = (80, 220, 140)
        elif joint_idx in RIGHT_JOINTS:
            color = (255, 185, 90)
        cv2.circle(canvas, (int(x_coord), int(y_coord)), 4, color, -1, cv2.LINE_AA)

    put_status(canvas, "MotionBERT 3D", fps_text, state_text)
    return canvas


def put_status(canvas, title, fps_text="", state_text=""):
    cv2.putText(canvas, title, (18, 34), cv2.FONT_HERSHEY_SIMPLEX, 0.85, (235, 235, 235), 2, cv2.LINE_AA)
    if fps_text:
        cv2.putText(canvas, fps_text, (18, 64), cv2.FONT_HERSHEY_SIMPLEX, 0.65, (180, 180, 180), 2, cv2.LINE_AA)
    if state_text:
        cv2.putText(canvas, state_text, (18, 94), cv2.FONT_HERSHEY_SIMPLEX, 0.65, (120, 210, 255), 2, cv2.LINE_AA)


def make_capture(source, args):
    if source.isdigit():
        capture = cv2.VideoCapture(int(source), cv2.CAP_DSHOW)
        capture.set(cv2.CAP_PROP_FRAME_WIDTH, args.camera_width)
        capture.set(cv2.CAP_PROP_FRAME_HEIGHT, args.camera_height)
        capture.set(cv2.CAP_PROP_FPS, args.camera_fps)
        return capture, f"camera:{source}"
    capture = cv2.VideoCapture(source)
    return capture, source


class SharedLatestFrame:
    def __init__(self):
        self.lock = threading.Lock()
        self.frame_bgr = None
        self.frame_index = -1
        self.ended = False

    def update(self, frame_bgr, frame_index):
        with self.lock:
            self.frame_bgr = frame_bgr.copy()
            self.frame_index = frame_index

    def mark_ended(self):
        with self.lock:
            self.ended = True

    def get(self):
        with self.lock:
            frame_bgr = None if self.frame_bgr is None else self.frame_bgr.copy()
            return frame_bgr, self.frame_index, self.ended


class SharedLatestResult:
    def __init__(self):
        self.lock = threading.Lock()
        self.combined = None
        self.processed_index = -1
        self.state_text = "Waiting for frames"

    def update(self, combined, processed_index, state_text):
        with self.lock:
            self.combined = combined.copy()
            self.processed_index = processed_index
            self.state_text = state_text

    def get(self):
        with self.lock:
            combined = None if self.combined is None else self.combined.copy()
            return combined, self.processed_index, self.state_text


class SharedPoseBuffer:
    def __init__(self):
        self.lock = threading.Lock()
        self.samples = deque(maxlen=8)

    def update(self, pose, timestamp=None):
        sample_time = time.time() if timestamp is None else float(timestamp)
        with self.lock:
            if pose is None:
                return
            self.samples.append((sample_time, pose.copy()))

    def latest(self):
        with self.lock:
            if not self.samples:
                return None, 0.0
            timestamp, pose = self.samples[-1]
            return pose.copy(), timestamp

    def sample(self, target_time):
        with self.lock:
            if not self.samples:
                return None, 0.0
            if len(self.samples) == 1:
                timestamp, pose = self.samples[0]
                return pose.copy(), timestamp

            samples = list(self.samples)

        first_time, first_pose = samples[0]
        if target_time <= first_time:
            return first_pose.copy(), first_time

        last_time, last_pose = samples[-1]
        if target_time >= last_time:
            return last_pose.copy(), last_time

        for (time_a, pose_a), (time_b, pose_b) in zip(samples[:-1], samples[1:]):
            if time_a <= target_time <= time_b:
                interval = max(time_b - time_a, 1e-6)
                alpha = float(np.clip((target_time - time_a) / interval, 0.0, 1.0))
                blended = pose_a * (1.0 - alpha) + pose_b * alpha
                return blended.astype(np.float32, copy=False), target_time

        return last_pose.copy(), last_time


def main():
    cli_args = parse_args()
    gpu_ids, device = parse_gpu_args(cli_args.gpus)
    cli_args.gpus = gpu_ids
    cli_args.device = device
    cli_args.tracking = cli_args.pose_track or cli_args.pose_flow or cli_args.detector == "tracker"

    if cli_args.min_frames > cli_args.clip_len:
        raise ValueError("--min_frames cannot be larger than --clip_len")
    if cli_args.infer_every_n < 1:
        raise ValueError("--infer_every_n must be at least 1")
    if not 0.0 <= cli_args.motion_smooth < 1.0:
        raise ValueError("--motion_smooth must be in [0, 1)")
    if cli_args.ue_send_fps <= 0:
        raise ValueError("--ue_send_fps must be > 0")
    if cli_args.ue_buffer_ms < 0:
        raise ValueError("--ue_buffer_ms must be >= 0")

    alpha_cfg = resolve_path(cli_args.alphapose_cfg)
    alpha_ckpt = resolve_path(cli_args.alphapose_ckpt)
    motion_cfg = resolve_path(cli_args.config)
    motion_ckpt = resolve_path(cli_args.evaluate)
    for required_path in [alpha_cfg, alpha_ckpt, motion_cfg, motion_ckpt]:
        if not required_path.exists():
            raise FileNotFoundError(f"Required file not found: {required_path}")

    capture, source_name = make_capture(cli_args.source, cli_args)
    if not capture.isOpened():
        raise RuntimeError(f"Unable to open source: {source_name}")
    shared_frame = SharedLatestFrame()
    shared_result = SharedLatestResult()
    shared_pose = SharedPoseBuffer()
    stop_event = threading.Event()

    if cli_args.save_video:
        save_path = resolve_path(cli_args.save_video)
        save_path.parent.mkdir(parents=True, exist_ok=True)
    else:
        save_path = None
    need_visual_output = (not cli_args.no_display) or (save_path is not None)

    def capture_loop():
        frame_index = 0
        try:
            while not stop_event.is_set():
                ok, frame_bgr = capture.read()
                if not ok:
                    print("[INFO] Input stream ended or frame capture failed.")
                    break
                shared_frame.update(frame_bgr, frame_index)
                frame_index += 1
                if cli_args.max_frames and frame_index >= cli_args.max_frames:
                    print(f"[INFO] Reached max frame limit: {cli_args.max_frames}")
                    break
        finally:
            shared_frame.mark_ended()
            stop_event.set()

    def process_loop():
        alpha_pose = AlphaPoseSingleFrame(cli_args)
        motionbert = MotionBERTRealtime(cli_args)
        sequence_buffer = deque(maxlen=cli_args.clip_len)
        fps_history = deque(maxlen=20)
        writer = None
        last_prediction = None
        last_raw_prediction = None
        last_processed_index = -1

        try:
            while True:
                frame_bgr, frame_index, ended = shared_frame.get()
                if frame_bgr is None:
                    if ended or stop_event.is_set():
                        break
                    time.sleep(0.002)
                    continue
                if frame_index <= last_processed_index:
                    if ended or stop_event.is_set():
                        break
                    time.sleep(0.001)
                    continue

                frame_rgb = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB)
                pose_result = alpha_pose.process(f"frame_{frame_index:06d}.jpg", frame_rgb)

                person = select_person(pose_result, cli_args.focus)
                if person is not None:
                    sequence_buffer.append(person_to_h36m(person))
                else:
                    sequence_buffer.append(empty_pose_frame())

                loop_start = time.time()
                state_text = f"Buffer {len(sequence_buffer)}/{cli_args.clip_len}"
                if len(sequence_buffer) >= cli_args.min_frames:
                    if ((frame_index - cli_args.min_frames) % cli_args.infer_every_n) == 0 or last_prediction is None:
                        motion_clip = np.stack(sequence_buffer, axis=0)
                        prediction = motionbert.infer(motion_clip)
                        current_prediction = prediction[-1]
                        if cli_args.motion_smooth > 0.0 and last_raw_prediction is not None:
                            current_prediction = (
                                last_raw_prediction * cli_args.motion_smooth
                                + current_prediction * (1.0 - cli_args.motion_smooth)
                            )
                        last_prediction = current_prediction
                        last_raw_prediction = current_prediction.copy()
                        shared_pose.update(last_prediction, timestamp=time.time())
                    state_text = (
                        f"3D active {len(sequence_buffer)} frames"
                        f" | infer/{cli_args.infer_every_n}"
                        f" | smooth {cli_args.motion_smooth:.2f}"
                        f" | ue_buffer {cli_args.ue_buffer_ms:.0f}ms"
                        f" | src {frame_index}"
                    )
                else:
                    state_text = f"Warming up {len(sequence_buffer)}/{cli_args.min_frames}"

                elapsed = max(time.time() - loop_start, 1e-6)
                fps_history.append(1.0 / elapsed)
                fps_text = ""
                if cli_args.show_fps:
                    fps_text = f"FPS {np.mean(fps_history):.1f}"

                if need_visual_output:
                    display_left = alpha_pose.get_bgr_image()
                    if display_left is None:
                        display_left = frame_bgr.copy()
                    display_left = alpha_pose.draw_pose(display_left.copy(), pose_result)
                    right_panel = render_3d_pose(last_prediction, (display_left.shape[1], display_left.shape[0]), fps_text, state_text)
                    combined = np.hstack([display_left, right_panel])

                    if cli_args.display_scale != 1.0:
                        combined = cv2.resize(
                            combined,
                            None,
                            fx=cli_args.display_scale,
                            fy=cli_args.display_scale,
                            interpolation=cv2.INTER_AREA,
                        )

                    if save_path is not None:
                        if writer is None:
                            save_fps = capture.get(cv2.CAP_PROP_FPS)
                            if not save_fps or save_fps <= 1:
                                save_fps = 30.0
                            writer = cv2.VideoWriter(
                                str(save_path),
                                cv2.VideoWriter_fourcc(*"mp4v"),
                                float(save_fps),
                                (combined.shape[1], combined.shape[0]),
                            )
                        writer.write(combined)

                    shared_result.update(combined, frame_index, state_text)
                last_processed_index = frame_index
        finally:
            if writer is not None:
                writer.release()
            stop_event.set()

    def livelink_loop():
        if not cli_args.ue_livelink_host:
            return
        livelink_sender = UnrealLiveLinkSender(
            cli_args.ue_livelink_host,
            cli_args.ue_livelink_port,
            cli_args.ue_livelink_subject,
            cli_args.ue_livelink_scale,
        )
        send_interval = 1.0 / cli_args.ue_send_fps
        buffer_seconds = cli_args.ue_buffer_ms / 1000.0
        try:
            while not stop_event.is_set():
                send_start = time.perf_counter()
                if cli_args.ue_interpolate:
                    pose, _ = shared_pose.sample(time.time() - buffer_seconds)
                else:
                    pose, _ = shared_pose.latest()
                if pose is not None:
                    livelink_sender.send(pose)
                elapsed = time.perf_counter() - send_start
                time.sleep(max(0.0, send_interval - elapsed))
        finally:
            livelink_sender.close()

    capture_thread = threading.Thread(target=capture_loop, name="capture_loop", daemon=True)
    process_thread = threading.Thread(target=process_loop, name="process_loop", daemon=True)
    livelink_thread = None
    if cli_args.ue_livelink_host:
        livelink_thread = threading.Thread(target=livelink_loop, name="livelink_loop", daemon=True)

    print(f"[INFO] Realtime source: {source_name}")
    print(f"[INFO] Press Q or ESC to quit.")
    capture_thread.start()
    process_thread.start()
    if livelink_thread is not None:
        livelink_thread.start()

    try:
        while True:
            if stop_event.is_set() and not process_thread.is_alive():
                break

            combined, processed_index, state_text = shared_result.get()
            if not cli_args.no_display:
                if combined is not None:
                    cv2.imshow("MotionBERT Realtime 3D", combined)
                key = cv2.waitKey(1) & 0xFF
                if key in (27, ord("q"), ord("Q")):
                    print("[INFO] Exit requested by user.")
                    stop_event.set()
                    break
            else:
                time.sleep(0.01)
    finally:
        stop_event.set()
        capture_thread.join(timeout=2.0)
        process_thread.join(timeout=2.0)
        if livelink_thread is not None:
            livelink_thread.join(timeout=2.0)
        capture.release()
        if not cli_args.no_display:
            cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
