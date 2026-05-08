# MotionBERT Windows 5090 一键部署

这份文档对应仓库根目录的：

- [deploy_windows_5090.bat](</d:/Git/Github/MotionBERT/deploy_windows_5090.bat>)

目标场景是：

- 另一台全新的 Windows 机器
- RTX 5090
- 还没有 Python / Conda / MotionBERT / AlphaPose 环境
- 你已经把当前仓库复制过去

## 使用方式

在目标机器上直接执行：

```cmd
cd /d D:\Git\Github\MotionBERT
deploy_windows_5090.bat
```

默认会直接在窗口里显示实时安装输出，方便你看到下载和安装进度。

如果你想把部署输出写到日志文件，再执行：

```cmd
cd /d D:\Git\Github\MotionBERT
set LOG_TO_FILE=1
deploy_windows_5090.bat
```

日志默认写到仓库根目录的：

`deploy_run.log`

## 它会做什么

1. 把 Miniconda 安装到仓库内的 `.local/miniconda3`
2. 创建两个环境：
   - `motionbert`
   - `alphapose`
3. 安装当前项目已经跑通的版本组合：
   - Python `3.10.20`
   - PyTorch `2.11.0+cu128`
   - torchvision `0.26.0+cu128`
   - torchaudio `2.11.0+cu128`
4. 安装 MotionBERT 和 AlphaPose 所需 Python 依赖
5. 如果 `.external/AlphaPose` 不存在，就自动从官方 GitHub 下载源码包
6. 自动补齐这些资产：
   - `checkpoint/pose3d/FT_MB_lite_MB_ft_h36m_global_lite/best_epoch.bin`
   - `.external/AlphaPose/detector/yolox/data/yolox_x.pth`
7. 先把 PyTorch `cu128` 的 wheel 下载到本地缓存，再离线安装到两个环境，避免大文件重复在线拉取
7. 创建本地缓存目录：
   - `alpha_cache/matplotlib`
   - `alpha_cache/torch/hub/checkpoints`
   - `.cache/deploy/torch-cu128`
8. 做基础 smoke check

## 当前已知前提

- 目标机器需要联网
- 目标机器建议已经装好 NVIDIA 驱动
- `halpe26_fast_res50_256x192.pth` 目前默认优先复用你现有工作机里的文件
- 新版 Conda 在首次使用默认 Anaconda 渠道时，可能要求接受渠道 Terms of Service；当前部署脚本已经会自动执行接受步骤

也就是说，如果部署脚本最后提示这个文件缺失，直接把当前工作机里的：

`D:\Git\Github\MotionBERT\.external\AlphaPose\pretrained_models\halpe26_fast_res50_256x192.pth`

复制到目标机器同路径即可。

## 为什么这样做

这套脚本不是照 README 生搬硬套，而是按当前仓库已经实际跑通的 Windows 方案固化的。重点是三件事：

1. 用仓库内本地 Miniconda，避免污染系统环境
2. 固定到当前已验证的 PyTorch cu128 组合，减少新显卡兼容性不确定性
3. AlphaPose 优先按“源码目录 + `PYTHONPATH`”运行，少走一层 Windows 上不必要的可编辑安装风险

## 部署完成后怎么验证

脚本跑完后，可以继续用现有入口：

- [run_alphapose.bat](</d:/Git/Github/MotionBERT/run_alphapose.bat>)
- [run_motionbert_pose.bat](</d:/Git/Github/MotionBERT/run_motionbert_pose.bat>)
- [run_motionbert_realtime.bat](</d:/Git/Github/MotionBERT/run_motionbert_realtime.bat>)

如果只是先验环境是否通了，脚本本身已经会自动执行：

- `python --version`
- `python -c "import torch; ..."`
- `python infer_wild.py --help`
- `python .external/AlphaPose/scripts/demo_inference.py --help`
- `python realtime_motionbert.py --help`
