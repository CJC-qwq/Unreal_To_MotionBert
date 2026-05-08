# Unreal To MotionBERT

This repository is a working Windows-focused MotionBERT project adapted for:

- realtime 3D pose inference
- Unreal Engine Live Link preview
- local training and checkpoint testing
- one-click environment rebuild on new machines

It is no longer just the upstream official MotionBERT code drop. It now contains:

- MotionBERT runtime and training scripts
- AlphaPose integration
- Unreal Engine 5 project and Live Link plugin
- Windows deployment automation
- project-specific docs, version snapshots, and maintenance records

## What This Repo Is For

This repo is meant to solve a practical workflow:

1. run AlphaPose + MotionBERT locally on Windows
2. send realtime 3D pose to Unreal
3. test custom checkpoints without overwriting official ones
4. move the project between machines by rebuilding the environment instead of copying full Python installs

## Current Project Layers

This project is organized into four main layers:

1. `Git source layer`
   - code, configs, docs, bat/ps1 scripts, Unreal source/config/plugin files

2. `Deploy rebuild layer`
   - Windows one-click deploy scripts recreate Miniconda envs and Python dependencies

3. `Model asset layer`
   - checkpoints and pretrained weights are restored from Hugging Face

4. `Manual data layer`
   - large datasets stay outside git and are handled separately

## Main Entry Points

### Realtime MotionBERT

- [run_motionbert_realtime.bat](/d:/Git/Github/MotionBERT/run_motionbert_realtime.bat)
- [run_motionbert_realtime_global_5ep.bat](/d:/Git/Github/MotionBERT/run_motionbert_realtime_global_5ep.bat)
- [realtime_motionbert.py](/d:/Git/Github/MotionBERT/realtime_motionbert.py)

### Offline Replay To Unreal

- [run_motionbert_offline_livelink.bat](/d:/Git/Github/MotionBERT/run_motionbert_offline_livelink.bat)
- [run_motionbert_offline_livelink_global_5ep.bat](/d:/Git/Github/MotionBERT/run_motionbert_offline_livelink_global_5ep.bat)
- [replay_offline_to_ue.py](/d:/Git/Github/MotionBERT/replay_offline_to_ue.py)

### Inference

- [run_motionbert_pose.bat](/d:/Git/Github/MotionBERT/run_motionbert_pose.bat)
- [run_motionbert_pose_global_5ep.bat](/d:/Git/Github/MotionBERT/run_motionbert_pose_global_5ep.bat)
- [infer_wild.py](/d:/Git/Github/MotionBERT/infer_wild.py)

### Training

- [run_train_motionbert_global_5ep.bat](/d:/Git/Github/MotionBERT/run_train_motionbert_global_5ep.bat)
- [train.py](/d:/Git/Github/MotionBERT/train.py)
- [configs/pose3d/MB_train_h36m.yaml](/d:/Git/Github/MotionBERT/configs/pose3d/MB_train_h36m.yaml)
- [configs/pose3d/MB_train_h36m_global_5ep.yaml](/d:/Git/Github/MotionBERT/configs/pose3d/MB_train_h36m_global_5ep.yaml)

### Mesh

- [run_motionbert_mesh.bat](/d:/Git/Github/MotionBERT/run_motionbert_mesh.bat)
- [infer_wild_mesh.py](/d:/Git/Github/MotionBERT/infer_wild_mesh.py)
- [train_mesh.py](/d:/Git/Github/MotionBERT/train_mesh.py)

## Unreal Integration

The repository includes a UE project and a custom Live Link plugin:

- [Unreal/MotionBERT_UE/MotionBERT_UE.uproject](/d:/Git/Github/MotionBERT/Unreal/MotionBERT_UE/MotionBERT_UE.uproject)
- [Unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink](/d:/Git/Github/MotionBERT/Unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink)

Important docs:

- [docs/ue_livelink_integration.md](/d:/Git/Github/MotionBERT/docs/ue_livelink_integration.md)
- [docs/realtime_perf_ceiling_assessment.md](/d:/Git/Github/MotionBERT/docs/realtime_perf_ceiling_assessment.md)

Current UE path supports:

- UDP-based Live Link source
- MotionBERT 17-joint stream
- native preview character/game mode
- direct Manny preview workflow

## One-Click Deployment

This repo is designed so you do not need to put Python environments into git.

### Windows 5070 Ti

- [deploy_windows_5070ti.bat](/d:/Git/Github/MotionBERT/deploy_windows_5070ti.bat)
- [scripts/deploy_windows_5070ti.ps1](/d:/Git/Github/MotionBERT/scripts/deploy_windows_5070ti.ps1)
- [docs/deploy_windows_5070ti.md](/d:/Git/Github/MotionBERT/docs/deploy_windows_5070ti.md)

### Windows 5090

- [deploy_windows_5090.bat](/d:/Git/Github/MotionBERT/deploy_windows_5090.bat)
- [scripts/deploy_windows_5090.ps1](/d:/Git/Github/MotionBERT/scripts/deploy_windows_5090.ps1)
- [docs/deploy_windows_5090.md](/d:/Git/Github/MotionBERT/docs/deploy_windows_5090.md)

These deploy scripts rebuild:

- `.local/miniconda3`
- `motionbert` conda env
- `alphapose` conda env
- PyTorch 2.11.0 + cu128 stack
- required Python dependencies

They also now default to downloading model assets from:

- `EMOCJC/motionbert_models`

## Model And Data Strategy

This repo now follows a split strategy:

### In Git

- source code
- configs
- deployment scripts
- docs
- Unreal source/config/plugin files

### In Hugging Face Models

- MotionBERT checkpoints
- AlphaPose pretrained weights
- detector weights

Current model repo:

- `EMOCJC/motionbert_models`

### Outside Git

- full datasets
- local caches
- outputs
- conda envs

Related docs:

- [docs/assets_storage_strategy.md](/d:/Git/Github/MotionBERT/docs/assets_storage_strategy.md)
- [deploy/assets_manifest.template.json](/d:/Git/Github/MotionBERT/deploy/assets_manifest.template.json)

## Repository Notes

### Version Snapshots

- [versions/realtime/README.md](/d:/Git/Github/MotionBERT/versions/realtime/README.md)

This folder keeps important realtime script snapshots so regression and rollback are easier.

### Workspace Bundle

- [create_workspace_bundle.bat](/d:/Git/Github/MotionBERT/create_workspace_bundle.bat)
- [scripts/create_workspace_bundle.ps1](/d:/Git/Github/MotionBERT/scripts/create_workspace_bundle.ps1)
- [docs/workspace_bundle.md](/d:/Git/Github/MotionBERT/docs/workspace_bundle.md)

This is for moving the workspace without copying the whole Python environment.

### Maintenance Records

- [Agents/docs_MAINTENANCE_PROGRESS.md](/d:/Git/Github/MotionBERT/Agents/docs_MAINTENANCE_PROGRESS.md)
- [Agents/docs_TEST_EXECUTION_LOG.md](/d:/Git/Github/MotionBERT/Agents/docs_TEST_EXECUTION_LOG.md)
- [Agents/docs_REQUIREMENT_WORKFLOW.md](/d:/Git/Github/MotionBERT/Agents/docs_REQUIREMENT_WORKFLOW.md)

## Suggested Setup Flow

### New machine

1. Clone this repo.
2. Run `deploy_windows_5070ti.bat` or `deploy_windows_5090.bat`.
3. Let the deploy script restore model assets from `EMOCJC/motionbert_models`.
4. Manually place any private dataset files if needed.
5. Run the required bat entry point.

### Realtime to Unreal

1. Open the UE project.
2. Enable and use the MotionBERT Live Link source.
3. Start a realtime MotionBERT bat entry.
4. Preview the Manny-based character in UE.

## Upstream Credit

This project is based on the official MotionBERT paper and codebase:

- Paper: <https://arxiv.org/abs/2210.06551>
- Upstream repo: <https://github.com/Walter0807/MotionBERT>

This repository keeps `upstream` pointing to the official MotionBERT repo and `origin` pointing to the project-owned repo.
