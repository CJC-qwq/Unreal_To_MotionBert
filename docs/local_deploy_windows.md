# MotionBERT Windows 本地部署记录

这份文档记录的是当前工作区 `D:\Git\Github\MotionBERT` 已经实际跑通的一套本地部署流程，目标是：

1. 用 AlphaPose 从本地视频生成 `alphapose-results.json`
2. 用 MotionBERT 基于该 JSON 生成人体 3D 姿态结果

这不是官方原样环境，而是针对当前这台 Windows + NVIDIA GPU 机器做过兼容调整后的可运行方案。

## 1. 当前已跑通的结果

当前工作区已经成功生成过以下文件：

- 输入视频：`outputs\test.mp4`
- AlphaPose 结果：`outputs\alphapose-results.json`
- MotionBERT 可视化视频：`outputs\X3D.mp4`
- MotionBERT 数组结果：`outputs\X3D.npy`

当前默认启动脚本：

- [run_alphapose.bat](</d:/Git/Github/MotionBERT/run_alphapose.bat>)
- [run_motionbert_pose.bat](</d:/Git/Github/MotionBERT/run_motionbert_pose.bat>)

## 2. 官方文档依据

MotionBERT 官方文档：

- README: https://github.com/Walter0807/MotionBERT
- 自定义视频推理说明: https://github.com/Walter0807/MotionBERT/blob/main/docs/inference.md

AlphaPose 官方文档：

- README: https://github.com/MVIG-SJTU/AlphaPose
- 安装说明: https://github.com/MVIG-SJTU/AlphaPose/blob/master/docs/INSTALL.md
- 推理说明: https://github.com/MVIG-SJTU/AlphaPose/blob/master/docs/GETTING_STARTED.md
- 模型说明: https://github.com/MVIG-SJTU/AlphaPose/blob/master/docs/MODEL_ZOO.md

官方信息里的关键点：

- MotionBERT 自定义视频推理需要先提取 2D pose
- MotionBERT 的 `infer_wild.py` 预期输入为 AlphaPose 风格 JSON
- AlphaPose 在 Windows 上可用，但官方提示不建议依赖需要 CUDA 扩展的 `-dcn` 模型
- MotionBERT 代码里实际按 `Halpe 26 -> H36M 17` 的方式转换关键点

## 3. 这次采用的实际方案

### 3.1 MotionBERT 环境

在项目目录内安装了 Miniconda：

- `D:\Git\Github\MotionBERT\.local\miniconda3`

MotionBERT 独立环境：

- `D:\Git\Github\MotionBERT\.local\miniconda3\envs\motionbert`

实际使用的是：

- Python 3.10
- PyTorch 2.11.0 + CUDA 12.8

说明：

- MotionBERT README 写的是 `python=3.7` 与 `pytorch-cuda=11.6`
- 由于当前机器显卡较新，实际部署中改成了更适合当前环境的新版 PyTorch 组合

### 3.2 AlphaPose 环境

AlphaPose 源码目录：

- `D:\Git\Github\MotionBERT\.external\AlphaPose`

AlphaPose 独立环境：

- `D:\Git\Github\MotionBERT\.local\miniconda3\envs\alphapose`

AlphaPose 本次实际使用：

- `Halpe 26 FastPose`
- `YOLOX-X` 作为 detector

原因：

- Windows 下尽量避开依赖 DCN CUDA 扩展的模型
- MotionBERT 的 `dataset_wild.py` 直接按 `Halpe 26` 处理

## 4. 下载到本地的关键资源

### 4.1 MotionBERT 权重

已下载：

- `checkpoint\pose3d\FT_MB_lite_MB_ft_h36m_global_lite\best_epoch.bin`

### 4.2 AlphaPose 权重

已下载：

- `D:\Git\Github\MotionBERT\.external\AlphaPose\pretrained_models\halpe26_fast_res50_256x192.pth`
- `D:\Git\Github\MotionBERT\.external\AlphaPose\detector\yolox\data\yolox_x.pth`

### 4.3 本地缓存目录

为解决 Windows 权限与缓存问题，额外使用：

- `D:\Git\Github\MotionBERT\alpha_cache\matplotlib`
- `D:\Git\Github\MotionBERT\alpha_cache\torch`

## 5. 为 Windows 做过的兼容调整

### 5.1 MotionBERT 代码调整

修改文件：

- [infer_wild.py](</d:/Git/Github/MotionBERT/infer_wild.py>)
- [infer_wild_mesh.py](</d:/Git/Github/MotionBERT/infer_wild_mesh.py>)
- [lib/utils/vismo.py](</d:/Git/Github/MotionBERT/lib/utils/vismo.py>)

调整内容：

1. 给推理脚本补了 `main()` 和 `if __name__ == '__main__':`
2. Windows 下把 DataLoader 改为单进程，避免 `spawn` 问题
3. 给 `matplotlib` 指定项目内缓存目录
4. `infer_wild_mesh.py` 增加了设备选择逻辑，避免默认强制 `.cuda()`

### 5.2 AlphaPose 运行时调整

没有直接改 AlphaPose 源码，而是在 `run_alphapose.bat` 里补了这些环境变量：

- `PYTHONPATH`
- `MPLCONFIGDIR`
- `TORCH_HOME`

作用：

1. 让 `scripts\demo_inference.py` 能找到仓库内模块
2. 避免 `matplotlib` 往用户目录写缓存时报权限错误
3. 避免 `torchvision` 往用户缓存目录下载 ResNet50 底座权重时报权限错误

## 6. 当前推荐使用方式

### 6.1 第一步：运行 AlphaPose

在 `cmd` 中执行：

```cmd
cd /d D:\Git\Github\MotionBERT
run_alphapose.bat
```

脚本默认配置在：

- [run_alphapose.bat](</d:/Git/Github/MotionBERT/run_alphapose.bat>)

只需要修改顶部变量：

```bat
set "VIDEO_PATH=D:\Git\Github\MotionBERT\outputs\test.mp4"
set "OUTPUT_DIR=D:\Git\Github\MotionBERT\outputs"
```

运行成功后会生成：

- `OUTPUT_DIR\alphapose-results.json`

### 6.2 第二步：运行 MotionBERT

在 `cmd` 中执行：

```cmd
cd /d D:\Git\Github\MotionBERT
run_motionbert_pose.bat
```

脚本默认配置在：

- [run_motionbert_pose.bat](</d:/Git/Github/MotionBERT/run_motionbert_pose.bat>)

只需要修改顶部变量：

```bat
set "VIDEO_PATH=D:\Git\Github\MotionBERT\outputs\test.mp4"
set "JSON_PATH=D:\Git\Github\MotionBERT\outputs\alphapose-results.json"
set "OUTPUT_DIR=D:\Git\Github\MotionBERT\outputs"
```

运行成功后会生成：

- `OUTPUT_DIR\X3D.mp4`
- `OUTPUT_DIR\X3D.npy`

## 7. 一次完整流程

如果你已经有一个本地视频，比如：

- `D:\Videos\person.mp4`

那么推荐步骤如下。

### 7.1 修改 `run_alphapose.bat`

```bat
set "VIDEO_PATH=D:\Videos\person.mp4"
set "OUTPUT_DIR=D:\Videos\alphapose_out"
```

然后运行：

```cmd
cd /d D:\Git\Github\MotionBERT
run_alphapose.bat
```

生成：

- `D:\Videos\alphapose_out\alphapose-results.json`

### 7.2 修改 `run_motionbert_pose.bat`

```bat
set "VIDEO_PATH=D:\Videos\person.mp4"
set "JSON_PATH=D:\Videos\alphapose_out\alphapose-results.json"
set "OUTPUT_DIR=D:\Videos\motionbert_out"
```

然后运行：

```cmd
cd /d D:\Git\Github\MotionBERT
run_motionbert_pose.bat
```

生成：

- `D:\Videos\motionbert_out\X3D.mp4`
- `D:\Videos\motionbert_out\X3D.npy`

## 8. 已知限制

### 8.1 当前主线只验证了 3D pose

这次完整跑通的是：

- AlphaPose -> `alphapose-results.json`
- MotionBERT `infer_wild.py`

没有完整跑通 mesh 产线。

### 8.2 `infer_wild_mesh.py` 仍需要额外资源

Mesh 路线除了脚本和环境，还需要官方数据说明里提到的额外资源，例如：

- `data\mesh\SMPL_NEUTRAL.pkl`
- `smpl_mean_params.npz`
- `J_regressor_extra.npy`
- `J_regressor_h36m_correct.npy`

以及对应 mesh checkpoint。

### 8.3 AlphaPose 输出可能为空

如果视频里没有检测到人体，AlphaPose 可能生成：

```json
[]
```

这不是 JSON 格式错误，而是“没有检测结果”。MotionBERT 虽然能读取，但不会得到有意义结果。

### 8.4 多人视频需要额外注意

当前这套链路默认按单人视频最稳。

如果视频中有多人，建议：

1. 在 `run_alphapose.bat` 中启用 `POSE_TRACK=1`
2. 在 `run_motionbert_pose.bat` 中按需要设置 `FOCUS_ID`

## 9. 常见问题

### 9.1 `JSONDecodeError`

通常表示：

- `alphapose-results.json` 是空文件
- 或者内容不是合法 JSON

先检查文件大小和前几行内容。

### 9.2 `No such file`

通常是 `.bat` 顶部路径写错：

- 写成目录而不是文件
- 路径里多了空格或拼写错误

### 9.3 `PermissionError`

这次部署里遇到过的权限问题主要来自：

- `matplotlib` 用户缓存目录
- `torch` 用户缓存目录

当前脚本已经通过项目内缓存目录规避。

### 9.4 结果慢

这是正常现象，尤其是：

- AlphaPose 先逐帧做人检测与姿态估计
- MotionBERT 再对整段时序做 3D 推理与渲染

对长视频，整体耗时会明显增加。

## 10. 当前建议

如果后续继续使用这套仓库，建议优先保留并复用：

- [run_alphapose.bat](</d:/Git/Github/MotionBERT/run_alphapose.bat>)
- [run_motionbert_pose.bat](</d:/Git/Github/MotionBERT/run_motionbert_pose.bat>)

以及这几个目录：

- `.local\miniconda3`
- `.external\AlphaPose`
- `alpha_cache`
- `checkpoint`

这样以后更换输入视频时，只需要修改 `.bat` 顶部路径，不需要重新部署。

## 11. 实时 3D 版（摄像头 / 流）

当前仓库已经新增实时入口：

- [realtime_motionbert.py](</d:/Git/Github/MotionBERT/realtime_motionbert.py>)
- [run_motionbert_realtime.bat](</d:/Git/Github/MotionBERT/run_motionbert_realtime.bat>)

它的工作方式不是先落地 `alphapose-results.json`，而是：

1. 逐帧读取摄像头或视频流
2. 用 AlphaPose 做单帧 2D 关键点检测
3. 在内存里维护最近一段关键点序列
4. 直接调用 MotionBERT 做时序 3D 推理
5. 左侧显示 2D 骨架，右侧显示实时 3D 骨架

### 11.1 直接运行

在 `cmd` 里执行：

```cmd
cd /d D:\Git\Github\MotionBERT
run_motionbert_realtime.bat
```

默认会打开 `SOURCE=0` 的摄像头。

### 11.2 只需要改的变量

编辑 [run_motionbert_realtime.bat](</d:/Git/Github/MotionBERT/run_motionbert_realtime.bat>) 顶部这几个变量：

```bat
set "SOURCE=0"
set "SAVE_VIDEO="
set "FOCUS_ID="
set "MIN_FRAMES=27"
set "SHOW_FPS=1"
set "CAMERA_WIDTH=1280"
set "CAMERA_HEIGHT=720"
set "CAMERA_FPS=30"
```

说明：

- `SOURCE=0` 表示默认摄像头
- `SOURCE=1` 表示第二个摄像头
- `SOURCE=D:\Videos\demo.mp4` 可以把本地视频当成“伪实时源”调试
- `SAVE_VIDEO=` 留空表示不保存；填路径后会保存左右拼接的视频
- `MIN_FRAMES=27` 表示至少积累 27 帧再开始 3D 推理

### 11.3 当前验证范围

本次已经完成的验证是：

- 在 AlphaPose 环境内成功加载 `Halpe 26 FastPose`
- 成功加载 MotionBERT `best_epoch.bin`
- 用 `outputs\test.mp4` 作为实时输入源完成逐帧推理
- 成功生成实时预览输出文件 `outputs\realtime_preview.mp4`

说明：

- 这证明“实时链路代码”已经打通
- 但摄像头本身是否能正常打开，仍取决于你本机设备编号、权限和占用状态

### 11.4 实时版与离线版的区别

- 离线版：`run_alphapose.bat` + `run_motionbert_pose.bat`
- 实时版：`run_motionbert_realtime.bat`

离线版适合最终产出 `alphapose-results.json`、`X3D.mp4`、`X3D.npy`  
实时版适合边看边调，但速度会明显受 GPU 性能和摄像头分辨率影响。

### 11.5 常见调优建议

如果实时速度不够，可以按顺序尝试：

1. 把 `CAMERA_WIDTH` / `CAMERA_HEIGHT` 降到 `960x540` 或 `640x480`
2. 把 `SHOW_FPS=0`
3. 暂时不要设置 `SAVE_VIDEO`
4. 改用单人、背景干净的场景
5. 用视频文件先验证链路，再切回摄像头
