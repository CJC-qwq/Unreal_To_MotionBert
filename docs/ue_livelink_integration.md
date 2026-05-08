# MotionBERT -> Unreal Live Link Integration

This document records the current first-pass integration path between MotionBERT realtime output and the Unreal Engine 5.6 project under `unreal/MotionBERT_UE`.

## Current Status

The project now contains a custom UE plugin:

- `unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink`

What is already implemented:

1. A custom Live Link source plugin for UE 5.6.
2. A custom Live Link source factory entry that appears in the Live Link panel.
3. A UDP listener inside the UE plugin.
4. A MotionBERT realtime sender that can emit Live Link-compatible JSON packets over UDP.
5. A verified UE 5.6 editor build that compiles the plugin successfully.
6. A native `UMotionBERTLiveLinkRetargetAsset` class that can rebuild a basic Manny-compatible pose from the MotionBERT 17-joint stream.

## UE Plugin

Plugin descriptor:

- [MotionBERTLiveLink.uplugin](</d:/Git/Github/MotionBERT/unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink/MotionBERTLiveLink.uplugin>)

Core source files:

- [MotionBERTLiveLinkSource.h](</d:/Git/Github/MotionBERT/unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink/Source/MotionBERTLiveLink/Public/MotionBERTLiveLinkSource.h>)
- [MotionBERTLiveLinkSource.cpp](</d:/Git/Github/MotionBERT/unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink/Source/MotionBERTLiveLink/Private/MotionBERTLiveLinkSource.cpp>)
- [MotionBERTLiveLinkSourceFactory.h](</d:/Git/Github/MotionBERT/unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink/Source/MotionBERTLiveLink/Public/MotionBERTLiveLinkSourceFactory.h>)
- [MotionBERTLiveLinkSourceFactory.cpp](</d:/Git/Github/MotionBERT/unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink/Source/MotionBERTLiveLink/Private/MotionBERTLiveLinkSourceFactory.cpp>)
- [MotionBERTLiveLinkRetargetAsset.h](</d:/Git/Github/MotionBERT/unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink/Source/MotionBERTLiveLink/Public/MotionBERTLiveLinkRetargetAsset.h>)
- [MotionBERTLiveLinkRetargetAsset.cpp](</d:/Git/Github/MotionBERT/unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink/Source/MotionBERTLiveLink/Private/MotionBERTLiveLinkRetargetAsset.cpp>)

The source publishes:

- `ULiveLinkAnimationRole`
- default subject name: `MotionBERTPose`
- default skeleton: H36M-style 17-joint hierarchy

The retarget asset reconstructs a first-pass mannequin pose by:

1. rebuilding source component-space joints from the incoming local transforms
2. mapping MotionBERT joints onto the UE mannequin hierarchy
3. preserving the mannequin reference bone lengths
4. solving a subset of pelvis, spine, neck, arm, and leg rotations from joint directions

## Direct Preview Path

The project now also contains a native mocap preview runtime path that does not require a custom Anim Blueprint asset.

Preview runtime files:

- [MotionBERTMocapPreviewCharacter.h](</d:/Git/Github/MotionBERT/unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERTMocapPreviewCharacter.h>)
- [MotionBERTMocapPreviewCharacter.cpp](</d:/Git/Github/MotionBERT/unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERTMocapPreviewCharacter.cpp>)
- [MotionBERTMocapPreviewGameMode.h](</d:/Git/Github/MotionBERT/unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERTMocapPreviewGameMode.h>)
- [MotionBERTMocapPreviewGameMode.cpp](</d:/Git/Github/MotionBERT/unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERTMocapPreviewGameMode.cpp>)

What it does:

1. spawns a dedicated preview character as the default pawn
2. assigns `SKM_Manny_Simple` directly in native code
3. uses the official `ULiveLinkInstance` as the animation instance
4. sets the Live Link subject to `MotionBERTPose`
5. sets the retarget class to `UMotionBERTLiveLinkRetargetAsset`
6. switches the project default game mode to `MotionBERTMocapPreviewGameMode`

This means the shortest current workflow is:

1. open the UE 5.6 project
2. open the default Third Person map
3. make sure the MotionBERT Live Link source is receiving packets
4. press Play

At that point the spawned preview character should already be wired to the MotionBERT Live Link subject.

## MotionBERT Sender

The realtime sender lives in:

- [realtime_motionbert.py](</d:/Git/Github/MotionBERT/realtime_motionbert.py>)

The batch entry exposes sender settings in:

- [run_motionbert_realtime.bat](</d:/Git/Github/MotionBERT/run_motionbert_realtime.bat>)
- [run_motionbert_offline_livelink.bat](</d:/Git/Github/MotionBERT/run_motionbert_offline_livelink.bat>)

Offline replay utility:

- [replay_offline_to_ue.py](</d:/Git/Github/MotionBERT/replay_offline_to_ue.py>)

Relevant runtime arguments:

- `--ue_livelink_host`
- `--ue_livelink_port`
- `--ue_livelink_subject`
- `--ue_livelink_scale`

## Offline Debug Path

For retarget debugging, the repository now supports replaying the original offline MotionBERT output into Unreal through the same Live Link subject.

Recommended debug flow:

1. Run the original offline pipeline and generate:
   - `outputs/X3D.npy`
   - `outputs/X3D.mp4`
2. Open UE and keep the `MotionBERT Live Link` source listening on port `7001`.
3. Run:

```cmd
run_motionbert_offline_livelink.bat
```

What this does:

1. loads `X3D.npy`
2. replays each `[17, 3]` frame toward UE
3. uses the same subject name `MotionBERTPose`
4. optionally derives playback FPS from the original source video

Why this path is useful:

1. it removes webcam, capture-card, and realtime AlphaPose jitter from the test
2. it gives a deterministic 3D sequence for repeated retarget comparisons
3. it makes it easier to tell whether a problem lives in:
   - MotionBERT 3D output itself
   - MotionBERT-to-UE coordinate mapping
   - UE retarget logic

Batch variables:

- `UE_LIVELINK_HOST`
- `UE_LIVELINK_PORT`
- `UE_LIVELINK_SUBJECT`
- `UE_LIVELINK_SCALE`

## Current Packet Format

The current sender emits one UTF-8 JSON UDP packet per frame:

```json
{
  "subject_name": "MotionBERTPose",
  "timestamp": 1777896218.5769525,
  "bone_names": ["pelvis", "right_hip", "..."],
  "bone_parents": [-1, 0, 1, "..."],
  "transforms": [
    {
      "translation": [x, y, z],
      "rotation": [0.0, 0.0, 0.0, 1.0],
      "scale": [1.0, 1.0, 1.0]
    }
  ]
}
```

Notes:

- `translation` is currently parent-relative.
- `rotation` is currently identity.
- `scale` is currently unit scale.
- MotionBERT positions are mapped into UE coordinates and multiplied by `UE_LIVELINK_SCALE`.

## Retarget Status

The transport path and the first-pass mannequin retarget path are now both implemented.

That means:

1. MotionBERT can stream a Live Link animation subject toward UE.
2. UE can ingest the subject through the custom UDP Live Link source.
3. `UMotionBERTLiveLinkRetargetAsset` can synthesize a basic mannequin pose from the MotionBERT 17-joint layout.

This is a practical first pass, not a polished production retarget. Current limits:

- finger animation is not generated
- foot roll / toe articulation is not generated
- pelvis translation is locked by default
- shoulder and torso twist are approximate because MotionBERT provides sparse joints, not a full UE-ready rig

## UE Wiring

The default Third Person setup currently uses these project assets:

- `/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter`
- `/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode`
- `/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple`
- `/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed`

To preview MotionBERT through Live Link on the default character manually:

1. Open the UE 5.6 project.
2. Enable the `Live Link` window and add a `MotionBERT Live Link` source.
3. Keep the source port aligned with `UE_LIVELINK_PORT`, default `7001`.
4. Open the animation blueprint currently used by the character, or create a dedicated mocap preview AnimBP for Manny or Quinn.
5. Add a `Live Link Pose` node in the Anim Graph.
6. Set `Live Link Subject Name` to `MotionBERTPose` or the subject name you configured on the Python side.
7. Set `Retarget Asset` on the node to `MotionBERTLiveLinkRetargetAsset`.
8. Compile the AnimBP and assign it to the character mesh, typically through `BP_ThirdPersonCharacter`.

For a cleaner preview path, it is usually better to duplicate the current character blueprint and animation blueprint into a dedicated mocap preview pair instead of replacing gameplay assets directly.

For this repository, the native preview character path above is now the faster option because it avoids a custom AnimBP asset and is already compiled into the project.

## Quick Test Path

1. Open the UE 5.6 project.
2. Open the Live Link panel.
3. Add the `MotionBERT Live Link` source.
4. Keep the default port `7001`.
5. Run MotionBERT realtime with:

```cmd
set UE_LIVELINK_HOST=127.0.0.1
set UE_LIVELINK_PORT=7001
run_motionbert_realtime.bat
```

If the source is receiving packets, the subject should appear in Live Link even before final Manny retargeting is completed.

With the current preview runtime in place, pressing Play in the default map is enough to spawn the mocap preview character once the Live Link source is active.
