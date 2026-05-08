import argparse
import time
from pathlib import Path

import imageio
import numpy as np

from lib.utils.ue_livelink import UnrealLiveLinkSender


ROOT_DIR = Path(__file__).resolve().parent


def parse_args():
    parser = argparse.ArgumentParser(description="Replay offline MotionBERT X3D.npy data to Unreal Live Link")
    parser.add_argument("--npy_path", type=str, required=True, help="Path to MotionBERT X3D.npy")
    parser.add_argument("--video_path", type=str, default="", help="Optional source video path for FPS auto-detection")
    parser.add_argument("--fps", type=float, default=0.0, help="Playback FPS. If 0, try reading from video, else fallback to 25")
    parser.add_argument("--start_frame", type=int, default=0, help="Start frame index")
    parser.add_argument("--end_frame", type=int, default=-1, help="End frame index (inclusive), -1 means last frame")
    parser.add_argument("--loop", action="store_true", help="Loop playback until interrupted")
    parser.add_argument("--ue_livelink_host", type=str, default="127.0.0.1", help="UDP host for Unreal Live Link")
    parser.add_argument("--ue_livelink_port", type=int, default=7001, help="UDP port for Unreal Live Link")
    parser.add_argument("--ue_livelink_subject", type=str, default="MotionBERTPose", help="Live Link subject name")
    parser.add_argument("--ue_livelink_scale", type=float, default=100.0, help="Scale factor when converting MotionBERT units to Unreal centimeters")
    return parser.parse_args()


def resolve_path(path_str):
    path = Path(path_str)
    if path.is_absolute():
        return path
    return ROOT_DIR / path


def detect_fps(video_path, fallback_fps):
    if not video_path:
        return fallback_fps
    video_path = resolve_path(video_path)
    if not video_path.exists():
        print(f"[WARN] Video path not found for FPS detection: {video_path}")
        return fallback_fps
    try:
        reader = imageio.get_reader(str(video_path), "ffmpeg")
        fps_value = float(reader.get_meta_data().get("fps", 0.0))
        reader.close()
        if fps_value > 0:
            return fps_value
    except Exception as exc:
        print(f"[WARN] Failed to detect FPS from {video_path}: {exc}")
    return fallback_fps


def main():
    args = parse_args()
    npy_path = resolve_path(args.npy_path)
    if not npy_path.exists():
        raise FileNotFoundError(f"X3D numpy file not found: {npy_path}")

    motion = np.load(npy_path)
    if motion.ndim != 3 or motion.shape[1:] != (17, 3):
        raise ValueError(f"Expected [T,17,3] motion array, got shape {motion.shape}")

    total_frames = motion.shape[0]
    start_frame = max(0, args.start_frame)
    end_frame = total_frames - 1 if args.end_frame < 0 else min(args.end_frame, total_frames - 1)
    if start_frame > end_frame:
        raise ValueError(f"Invalid frame range: start={start_frame}, end={end_frame}")

    fps_value = args.fps if args.fps > 0 else detect_fps(args.video_path, 25.0)
    frame_duration = 1.0 / max(fps_value, 1e-6)

    sender = UnrealLiveLinkSender(
        args.ue_livelink_host,
        args.ue_livelink_port,
        args.ue_livelink_subject,
        args.ue_livelink_scale,
    )

    print(f"[INFO] Replaying offline MotionBERT data: {npy_path}")
    print(f"[INFO] Frames: {start_frame}..{end_frame} / {total_frames - 1}")
    print(f"[INFO] FPS: {fps_value:.3f}")
    print(f"[INFO] UE Live Link: {args.ue_livelink_host}:{args.ue_livelink_port} subject={args.ue_livelink_subject}")
    print("[INFO] Press Ctrl+C to stop.")

    try:
        while True:
            for frame_index in range(start_frame, end_frame + 1):
                send_start = time.perf_counter()
                sender.send(motion[frame_index])
                elapsed = time.perf_counter() - send_start
                time.sleep(max(0.0, frame_duration - elapsed))
            if not args.loop:
                break
    except KeyboardInterrupt:
        print("\n[INFO] Offline replay stopped by user.")
    finally:
        sender.close()


if __name__ == "__main__":
    main()
