import json
import socket
import time


LIVELINK_BONE_NAMES = [
    "pelvis",
    "right_hip",
    "right_knee",
    "right_ankle",
    "left_hip",
    "left_knee",
    "left_ankle",
    "spine",
    "thorax",
    "nose",
    "head",
    "left_shoulder",
    "left_elbow",
    "left_wrist",
    "right_shoulder",
    "right_elbow",
    "right_wrist",
]

LIVELINK_BONE_PARENTS = [-1, 0, 1, 2, 0, 4, 5, 0, 7, 8, 9, 8, 11, 12, 8, 14, 15]


def motionbert_to_ue_vector(joint_xyz, scale):
    x_coord, y_coord, z_coord = joint_xyz
    return [float(z_coord * scale), float(x_coord * scale), float(-y_coord * scale)]


def joints_to_livelink_transforms(joints_3d, scale):
    ue_global = [motionbert_to_ue_vector(joint_xyz, scale) for joint_xyz in joints_3d]
    transforms = []
    for joint_index, parent_index in enumerate(LIVELINK_BONE_PARENTS):
        if parent_index < 0:
            local_translation = ue_global[joint_index]
        else:
            parent_translation = ue_global[parent_index]
            joint_translation = ue_global[joint_index]
            local_translation = [
                joint_translation[0] - parent_translation[0],
                joint_translation[1] - parent_translation[1],
                joint_translation[2] - parent_translation[2],
            ]
        transforms.append(
            {
                "translation": local_translation,
                "rotation": [0.0, 0.0, 0.0, 1.0],
                "scale": [1.0, 1.0, 1.0],
            }
        )
    return transforms


def joints_to_livelink_local_positions(joints_3d, scale):
    ue_global = [motionbert_to_ue_vector(joint_xyz, scale) for joint_xyz in joints_3d]
    local_positions = []
    for joint_index, parent_index in enumerate(LIVELINK_BONE_PARENTS):
        if parent_index < 0:
            local_positions.append(ue_global[joint_index])
        else:
            parent_translation = ue_global[parent_index]
            joint_translation = ue_global[joint_index]
            local_positions.append(
                [
                    joint_translation[0] - parent_translation[0],
                    joint_translation[1] - parent_translation[1],
                    joint_translation[2] - parent_translation[2],
                ]
            )
    return local_positions




class UnrealLiveLinkSender:
    def __init__(self, host, port, subject_name, scale):
        self.host = host
        self.port = port
        self.subject_name = subject_name
        self.scale = scale
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def send(self, joints_3d, timestamp=None, include_skeleton=False, compact=True):
        if compact:
            payload = {
                "subject_name": self.subject_name,
                "positions": joints_to_livelink_local_positions(joints_3d, self.scale),
            }
            if include_skeleton:
                payload["bone_names"] = LIVELINK_BONE_NAMES
                payload["bone_parents"] = LIVELINK_BONE_PARENTS
            if timestamp is not None:
                payload["timestamp"] = float(timestamp)
        else:
            payload = {
                "subject_name": self.subject_name,
                "bone_names": LIVELINK_BONE_NAMES,
                "bone_parents": LIVELINK_BONE_PARENTS,
                "transforms": joints_to_livelink_transforms(joints_3d, self.scale),
            }
            if timestamp is not None:
                payload["timestamp"] = float(timestamp)
        self.socket.sendto(json.dumps(payload, separators=(",", ":")).encode("utf-8"), (self.host, self.port))

    def close(self):
        self.socket.close()
