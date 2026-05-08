# MotionBERT Windows 5070 Ti One-Click Deploy

This document matches:
- [deploy_windows_5070ti.bat](/d:/Git/Github/MotionBERT/deploy_windows_5070ti.bat)

Target scenario:

- another Windows machine
- RTX 5070 Ti
- no ready Python / Conda / MotionBERT / AlphaPose environment yet
- you have already copied this repo over

## Usage

Run this directly on the target machine:

```cmd
cd /d D:\Git\Github\MotionBERT
deploy_windows_5070ti.bat
```

Default mode prints real-time install output in the console.

If you want a log file instead:

```cmd
cd /d D:\Git\Github\MotionBERT
set LOG_TO_FILE=1
deploy_windows_5070ti.bat
```

The default log path is:

`deploy_run_5070ti.log`

## What it does

1. Installs Miniconda into `.local/miniconda3`
2. Creates two conda envs:
   - `motionbert`
   - `alphapose`
3. Installs the validated stack used by this repo:
   - Python `3.10.20`
   - PyTorch `2.11.0+cu128`
   - torchvision `0.26.0+cu128`
   - torchaudio `2.11.0+cu128`
4. Installs MotionBERT and AlphaPose Python dependencies
5. Downloads AlphaPose source automatically if `.external/AlphaPose` is missing
6. Ensures these key assets:
   - `checkpoint/pose3d/FT_MB_lite_MB_ft_h36m_global_lite/best_epoch.bin`
   - `.external/AlphaPose/detector/yolox/data/yolox_x.pth`
7. Caches PyTorch wheels locally before offline install into both envs
8. Runs smoke checks

## Important note

This means your repo can use:

- `git` for source code and docs
- deploy scripts for rebuilding environments

So yes: you do not need to commit `.local/` or the Python environments into `git`.

You still need to manage large runtime assets separately when needed, such as:

- some checkpoints
- some pretrained model files
- datasets
- output files

## After deploy

You can continue using the existing entry points, for example:

- [run_alphapose.bat](/d:/Git/Github/MotionBERT/run_alphapose.bat)
- [run_motionbert_pose.bat](/d:/Git/Github/MotionBERT/run_motionbert_pose.bat)
- [run_motionbert_realtime.bat](/d:/Git/Github/MotionBERT/run_motionbert_realtime.bat)
