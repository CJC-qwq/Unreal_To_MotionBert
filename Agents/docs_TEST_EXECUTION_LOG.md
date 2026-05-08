# 测试执行日志

> 说明
> - 本文档用于记录正式测试步骤的逐步执行情况。
> - 每个正式测试步骤完成后立即追加记录。
> - 步骤编号必须连续。

## 2026-05-04 实时 3D 与 UE Live Link 集成

### Step 1

- 时间：2026-05-04 13:03:17
- 操作：启动实时 3D 方案分析，准备检查 AlphaPose 与 MotionBERT 现有入口和可复用模块。
- 结果：已进入实时链路分析阶段。
- 状态：完成

### Step 2

- 时间：2026-05-04 13:09:03
- 操作：执行 `.\.local\miniconda3\envs\alphapose\python.exe realtime_motionbert.py --help` 验证实时脚本基础启动能力。
- 结果：首次失败，定位到 `lib/data/dataset_wild.py` 顶层依赖 `ipdb`，而 `alphapose` 环境未安装该调试依赖。
- 状态：已记录问题

### Step 3

- 时间：2026-05-04 13:09:58
- 操作：执行 `.\.local\miniconda3\envs\alphapose\python.exe realtime_motionbert.py --source outputs\test.mp4 --no_display --max_frames 6 --min_frames 4 --show_fps` 做首轮链路验证。
- 结果：首次失败，定位到 MotionBERT checkpoint 中 `module.` 前缀与实时脚本初版加载逻辑不兼容。
- 状态：已记录问题

### Step 4

- 时间：2026-05-04 13:10:33
- 操作：修复 MotionBERT 权重加载后再次执行同一命令，继续验证实时链路。
- 结果：再次失败，定位到 AlphaPose YOLOX 权重相对路径解析问题。
- 状态：已记录问题

### Step 5

- 时间：2026-05-04 13:10:33
- 操作：修复 AlphaPose 检测器权重路径后重新执行实时推理。
- 结果：成功加载 AlphaPose、YOLOX 与 MotionBERT，并在样例视频源上完成 6 帧实时闭环验证。
- 状态：完成

### Step 6

- 时间：2026-05-04 13:10:57
- 操作：执行 `.\.local\miniconda3\envs\alphapose\python.exe realtime_motionbert.py --source outputs\test.mp4 --no_display --max_frames 12 --min_frames 4 --show_fps --save_video outputs\realtime_preview.mp4`。
- 结果：成功生成 `outputs\realtime_preview.mp4`。
- 状态：完成

### Step 7

- 时间：2026-05-04 13:15:45
- 操作：通过 `cmd /c` 方式验证 `run_motionbert_realtime.bat`，并使用环境变量临时覆盖输入与输出参数。
- 结果：批处理脚本成功拉起实时推理链路并正常结束，确认 `cmd` 入口可用。
- 状态：完成

### Step 8

- 时间：2026-05-04 17:32:04
- 操作：在用户反馈“实时 3D 头脚倒置”后，修正 `realtime_motionbert.py` 的实时 3D 屏幕纵向映射，并回归验证。
- 结果：运行成功，生成 `outputs\realtime_fix_preview.mp4`。
- 状态：完成

### Step 9

- 时间：2026-05-04 17:32:41
- 操作：从 `outputs\realtime_fix_preview.mp4` 抽取预览帧 `outputs\realtime_fix_preview_frame.png` 并人工检查。
- 结果：确认右侧 3D 骨架恢复为头在上、腿在下，倒转问题消失。
- 状态：完成

### Step 10

- 时间：2026-05-04 20:06:12
- 操作：审查 UE 5.6 官方 Live Link Retarget 路径与本地引擎源码，确认 `Live Link Pose` 节点的 `Retarget Asset` 为类引用，并验证 `ULiveLinkRetargetAsset` / `ULiveLinkRemapAsset` 的落点。
- 结果：确认当前最小可落地路线为在 `MotionBERTLiveLink` 插件中新增原生 retarget 类，而不是继续扩展自定义传输协议。
- 状态：完成

### Step 11

- 时间：2026-05-04 20:08:34
- 操作：检查 `unreal/MotionBERT_UE` 默认 Third Person 角色资产链路，确认角色 Mesh 与 AnimBP 的真实设置点。
- 结果：确认默认链路位于 `BP_ThirdPersonGameMode -> BP_ThirdPersonCharacter`，角色 Mesh / AnimBP 由蓝图资产持有，适合作为后续 Live Link 预览接线入口。
- 状态：完成

### Step 12

- 时间：2026-05-04 20:10:49
- 操作：首次执行 UE 5.6 工程编译，验证新增 `MotionBERTLiveLinkRetargetAsset` 的语法与依赖。
- 结果：编译失败，定位到 `MotionBERTLiveLinkRetargetAsset.cpp` 中的两类问题：
  - `const FVector` 误用 `Normalize()`
  - `FCompactPoseBoneIndex` 未显式初始化
- 状态：已记录问题

### Step 13

- 时间：2026-05-04 20:11:22
- 操作：修复 `MotionBERTLiveLinkRetargetAsset.cpp` 后重新执行 UE 5.6 工程编译。
- 结果：`MotionBERT_UEEditor` 构建成功，`UnrealEditor-MotionBERTLiveLink.dll` 成功链接，确认插件与新增 retarget 类可被 UE 5.6 正常构建。
- 状态：完成

### Step 14

- 时间：2026-05-04 20:16:08
- 操作：检查 UE 5.6 本地源码中的 `ULiveLinkInstance` 与 `ULiveLinkComponent`，确认是否存在可直接复用的官方运行时动画实例路径。
- 结果：确认 `ULiveLinkInstance` 为官方可用的 Live Link 动画实例，内部直接封装 `FAnimNode_LiveLinkPose`，适合替代手工创建 AnimBP 资产的方案。
- 状态：完成

### Step 15

- 时间：2026-05-04 20:20:04
- 操作：在项目中新增 `MotionBERTMocapPreviewCharacter`、`MotionBERTMocapPreviewGameMode`，修改模块依赖与默认 GameMode 后，重新执行 UE 5.6 工程编译。
- 结果：`MotionBERT_UEEditor` 再次构建成功，确认原生 mocap preview 角色链路可以通过编译。
- 状态：完成

### Step 16

- 时间：2026-05-04 20:36:28
- 操作：在 `MotionBERTMocapPreviewCharacter` 加入 Tick 重试绑定后，执行 `Build.bat MotionBERT_UEEditor` 验证新修复是否已合入 UE 5.6。
- 结果：首次失败，原因不是代码错误，而是 UnrealBuildTool 检测到 `Live Coding` 仍处于激活状态，阻止构建。
- 状态：已记录问题

### Step 17

- 时间：2026-05-04 20:39:36
- 操作：在用户关闭 UE 编辑器后，再次执行 `Build.bat MotionBERT_UEEditor`。
- 结果：构建成功，确认 `MotionBERTMocapPreviewCharacter.cpp` 的最新 Live Link 重试绑定修复已编译进 UE 5.6 工程。
- 状态：完成

### Step 18

- 时间：2026-05-04 20:46:45
- 操作：排查 Live Link 面板始终只有 `Listening` 的原因，复核 `run_motionbert_realtime.bat` 的 UE 发送参数拼接逻辑。
- 结果：确认旧版批处理仅在 `UE_LIVELINK_HOST` 非空时才启用 Live Link 发送，而默认值为空，导致脚本默认根本不向 UE 发包。
- 状态：完成

### Step 19

- 时间：2026-05-04 20:46:45
- 操作：修复 `run_motionbert_realtime.bat`，将 `UE_LIVELINK_HOST` 默认值改为 `127.0.0.1`，并增加启动时的 Live Link 目标打印。
- 结果：后续直接运行批处理即可默认向本机 `127.0.0.1:7001` 发送 `MotionBERTPose`，不再依赖手动设置环境变量。
- 状态：完成

### Step 20

- 时间：2026-05-04 20:51:55
- 操作：根据 UE 运行截图回溯 `MotionBERTLiveLinkRetargetAsset.cpp`，检查 Manny 局部关节扭曲是否来自目标骨索引映射错误与 twist bone 干扰。
- 结果：确认存在两个高风险点：
  - 直接用 `CompactPoseBoneIndex` 取 `ReferenceSkeleton` 骨名
  - 用“第一个子骨头”自动推断参考方向
- 状态：完成

### Step 21

- 时间：2026-05-04 20:51:55
- 操作：修复 retarget 逻辑，改用 `MakeMeshPoseIndex()` 获取正确目标骨名，并为 spine / arm / leg 显式指定参考子骨头后重新编译 UE 5.6 工程。
- 结果：构建成功，确认新的 `UnrealEditor-MotionBERTLiveLink.dll` 已生成并可用于回归测试。
- 状态：完成

### Step 22

- 时间：2026-05-04 20:58:16
- 操作：根据用户对“头、手、脚未驱动”和“躯干到四肢抽搐拉伸”的反馈，继续审查 retarget 骨链分配策略。
- 结果：确认上一版主要问题是：
  - 末端骨未单独解算
  - spine / neck / clavicle 等多节骨链重复吃满同类方向信息
- 状态：完成

### Step 23

- 时间：2026-05-04 20:58:16
- 操作：为 retarget 增加骨链权重分配与末端骨映射，补充 `head`、`hand_*`、`foot_*` 的朝向解算，并重新编译 UE 5.6 工程。
- 结果：构建成功，确认新的 `UnrealEditor-MotionBERTLiveLink.dll` 已生成，可用于下一轮回归测试。
- 状态：完成

### Step 24

- 时间：2026-05-04 21:03:17
- 操作：梳理原项目离线输出路径，确认 `infer_wild.py` 会生成 `X3D.npy`，并评估其是否适合作为 UE retarget 调试基线输入。
- 结果：确认 `X3D.npy` 是稳定的 `[T,17,3]` 离线 3D 结果，适合直接回放到 Live Link 进行固定动作调试。
- 状态：完成

### Step 25

- 时间：2026-05-04 21:03:17
- 操作：实现 `replay_offline_to_ue.py`、`run_motionbert_offline_livelink.bat`，并将 Live Link 发送逻辑抽取到 `lib/utils/ue_livelink.py` 供 realtime 与 offline 共用。
- 结果：离线 `X3D.npy -> MotionBERTPose` 回放链路已完成，支持范围播放、循环与 FPS 控制。
- 状态：完成

### Step 26

- 时间：2026-05-04 21:03:17
- 操作：执行 `replay_offline_to_ue.py --help` 以及短帧回放自检。
- 结果：帮助命令正常；短回放 `--start_frame 0 --end_frame 2 --fps 30` 正常结束，确认脚本可运行。
- 状态：完成

### Step 27

- 时间：2026-05-04 22:26:32
- 操作：根据“先不要硬套 Manny，而是先做 MotionBERT 原生骨架”的新方案，设计 UE 侧最小闭环调试视图。
- 结果：确定采用 `ILiveLinkClient -> EvaluateFrame_AnyThread -> 重建 17 点 component-space 位置 -> 世界调试绘制` 的最小方案，不再依赖角色 retarget。
- 状态：完成

### Step 28

- 时间：2026-05-04 22:26:32
- 操作：实现 `AMotionBERTDebugSkeletonActor`，并修改 `AMotionBERTMocapPreviewGameMode` 使默认预览流程改为 Spectator + MotionBERT 原生调试骨架。
- 结果：默认 mocap preview 已从 Manny 预览切换为 MotionBERT 原生 17 点调试视图。
- 状态：完成

### Step 29

- 时间：2026-05-04 22:26:32
- 操作：重新编译 `MotionBERT_UEEditor`，验证新增 Actor 与 GameMode 修改可正常构建。
- 结果：构建成功，确认 `UnrealEditor-MotionBERT_UE.dll` 已更新。
- 状态：完成

### Step 30

- 时间：2026-05-04 22:36:02
- 操作：根据用户反馈“只看到骨架名字在动，并且位置融入建筑”，检查 MotionBERT 原生调试骨架的居中与生成位置策略。
- 结果：确认存在两个问题：
  - 调试骨架未减去 root/pelvis 平移，导致整体位置受源数据平移影响
  - 调试 actor 生成点过于接近关卡原点/建筑区域
- 状态：完成

### Step 31

- 时间：2026-05-04 22:36:02
- 操作：为 debug skeleton 增加 root 居中、提高可视化尺寸和前景优先级，并将 GameMode 的生成位置改为 `PlayerStart` 前方抬高区域后重新编译工程。
- 结果：构建成功，确认新的显示策略已编译进 UE 5.6 工程。
- 状态：完成

### Step 32

- 时间：2026-05-04 22:38:08
- 操作：根据用户反馈“MotionBERT 原生调试骨架上下倒转”，回查公共 `MotionBERT -> UE` 坐标转换逻辑。
- 结果：确认问题更接近垂直轴符号错误，而不是 Live Link 链路或骨架显示策略问题。
- 状态：完成

### Step 33

- 时间：2026-05-04 22:38:08
- 操作：修改 `lib/utils/ue_livelink.py` 中的 `motionbert_to_ue_vector()`，将垂直轴映射改为 `y -> -Z`。
- 结果：由于 realtime 与 offline 共用该发送模块，本次修复会同时影响两条发送路径，且无需重新编译 UE。
- 状态：完成

### Step 34

- 时间：2026-05-04 22:47:54
- 操作：根据用户要求“先不要抹掉 root 位移，直接看原始数据里有没有前后/上下移动”，调整 UE 调试骨架的 root 去中心化策略。
- 结果：将 root 居中从硬编码行为改为可切换选项，并将默认值改为关闭。
- 状态：完成

### Step 35

- 时间：2026-05-04 22:47:54
- 操作：重新编译 `MotionBERT_UEEditor`，使新的 raw root trajectory 显示行为生效。
- 结果：构建成功，确认新的 UE 调试骨架会默认显示原始 root 轨迹。
- 状态：完成

### Step 36

- 时间：2026-05-04 22:53:10
- 操作：根据“MotionBERT 原始 pose 没有可靠全局前后位移”的结论，设计估计型全局位移层。
- 结果：确定使用 bbox 中心与 bbox 高度相对首帧变化作为离线/实时共享的位移估计输入。
- 状态：完成

### Step 37

- 时间：2026-05-04 22:53:10
- 操作：在 `lib/utils/ue_livelink.py` 中实现 `GlobalMotionEstimator`，并将 root translation 注入公共 Live Link 发送路径。
- 结果：realtime 与 offline 两条发送链已具备叠加估计型全局位移的能力。
- 状态：完成

### Step 38

- 时间：2026-05-04 22:53:10
- 操作：更新 `replay_offline_to_ue.py`、`realtime_motionbert.py` 及两份 bat 入口，并执行命令行自检。
- 结果：
  - 离线短回放已能在 `--estimate_global_motion` 模式下正常启动
  - realtime 帮助命令已正常显示新参数
- 状态：完成

### Step 39

- 时间：2026-05-04 23:01:48
- 操作：根据用户要求撤销 bbox 推导全局位移方案，回退到只传输 MotionBERT 模型原始输出的发送链路。
- 结果：确认不再使用 `alphapose-results.json` 或实时 bbox 为 UE 注入伪全局位移。
- 状态：完成

### Step 40

- 时间：2026-05-04 23:01:48
- 操作：回退 `lib/utils/ue_livelink.py`、`replay_offline_to_ue.py`、`realtime_motionbert.py` 以及两份 bat 入口中的估计位移逻辑。
- 结果：发送链路重新收敛为“只发模型真实 3D 姿态输出”，且无需重新编译 UE 工程。
- 状态：完成

### Step 41

- 时间：2026-05-04 23:06:48
- 操作：静态审查 `infer_wild.py`、`train.py`、`lib/data/dataset_wild.py`、`lib/data/datareader_h36m.py` 与 `configs/pose3d/MB_ft_h36m_global_lite.yaml`，定位“位移输出”相关开关与数据语义。
- 结果：确认仓库级关键开关是 `rootrel`，其中 `MB_ft_h36m_global_lite.yaml` 仅设置 `rootrel: False`；该配置不等价于“可靠世界位移输出”。
- 状态：完成

### Step 42

- 时间：2026-05-04 23:06:48
- 操作：复核 `train.py` 中 `rootrel=False` 的训练与评估分支，并核对官方评测是否真的评价了全局轨迹。
- 结果：确认训练时只是将整段序列的首帧 root 深度对零；评估时仍会把预测和 GT 都转换回 root-relative 后再计算误差，因此官方 pose 指标并不覆盖世界位移质量。
- 状态：完成

### Step 43

- 时间：2026-05-04 23:06:48
- 操作：复核 wild 推理输入预处理，并对当前 `outputs/X3D.npy` 做 root 轨迹量级检查。
- 结果：确认 `dataset_wild.py` 会对 2D 输入做居中缩放 / `crop_scale()`，显著弱化绝对平移信息；当前 `X3D.npy` 中 root 的 `z` 变化量远小于 `x/y`，不表现为可直接驱动 UE 前后位移的稳定全局轨迹。
- 状态：完成

### Step 44

- 时间：2026-05-05 11:19:12
- 操作：检查用户提供的 `h36m_sh_conf_cam_source_final.pkl.zip`，确认其可作为官方 `docs/pose3d.md` 所需的 H36M 预处理训练包使用。
- 结果：确认压缩包存在，大小正常，可解压进入官方期望目录 `data/motion3d/`。
- 状态：完成

### Step 45

- 时间：2026-05-05 11:19:12
- 操作：创建 `data/motion3d/`，解压 `h36m_sh_conf_cam_source_final.pkl.zip`，并检查关键文件落位。
- 结果：成功生成 `data/motion3d/h36m_sh_conf_cam_source_final.pkl`；同时发现并记录压缩包附带的 `__MACOSX/` 杂项目录。
- 状态：完成

### Step 46

- 时间：2026-05-05 11:19:12
- 操作：执行 `.\.local\miniconda3\envs\motionbert\python.exe tools\convert_h36m.py`，将官方预处理数据切片为训练用目录结构。
- 结果：命令执行成功，生成 `data/motion3d/MB3D_f243s81/H36M-SH/train/` 与 `test/`；切片统计为 train 17748、test 2228。
- 状态：完成

### Step 47

- 时间：2026-05-05 11:19:12
- 操作：清理解压附带的 `data/motion3d/__MACOSX/`，并复核最终训练目录。
- 结果：目录已清理，当前工作区已满足官方 3D pose 训练数据前置条件。
- 状态：完成

### Step 48

- 时间：2026-05-05 11:21:59
- 操作：新增独立的 `rootrel=False` 5 轮验证配置 `configs/pose3d/MB_train_h36m_global_5ep.yaml`，用于最低成本验证位移输出相关设置。
- 结果：配置已生成，关键参数为 `finetune: False`、`epochs: 5`、`rootrel: False`，与官方 120 轮 scratch 配置结构保持一致。
- 状态：完成

### Step 49

- 时间：2026-05-05 11:21:59
- 操作：新增独立启动脚本 `run_train_motionbert_global_5ep.bat`，并将 checkpoint 输出目录隔离到 `checkpoint/pose3d/MB_train_h36m_global_5ep`。
- 结果：验证训练入口已与官方现成 checkpoint 完全隔离，不会覆盖 `FT_MB_lite_MB_ft_h36m_global_lite`。
- 状态：完成

### Step 50

- 时间：2026-05-05 11:25:03
- 操作：根据用户实际训练反馈，复核 `MB_train_h36m_global_5ep` 的首轮 OOM 报错，并判断是否属于显存容量问题而非训练逻辑错误。
- 结果：确认 `train.py` 已正常进入 `rootrel=False` 训练分支，失败原因为 DSTformer scratch 训练在 `batch_size: 32` 下触发 CUDA out of memory。
- 状态：完成

### Step 51

- 时间：2026-05-05 11:25:03
- 操作：将 `configs/pose3d/MB_train_h36m_global_5ep.yaml` 的 `batch_size` 从 `32` 下调到 `4`，并在 bat 中加入 `PYTORCH_CUDA_ALLOC_CONF=expandable_segments:True`。
- 结果：保留 `rootrel=False`、`epochs: 5` 与独立 checkpoint 隔离策略，同时将 smoke test 调整为更省显存的版本。
- 状态：完成

### Step 52

- 时间：2026-05-05 13:37:13
- 操作：检查 `checkpoint/pose3d/MB_train_h36m_global_5ep/` 目录，确认 5 轮训练后的 checkpoint 与 TensorBoard 日志是否完整落盘。
- 结果：确认 `latest_epoch.bin`、`best_epoch.bin`、`epoch_4.bin` 与 `logs/` 全部存在，训练产物完整。
- 状态：完成

### Step 53

- 时间：2026-05-05 13:37:13
- 操作：读取 `latest_epoch.bin`、`best_epoch.bin` 与 `epoch_4.bin` 的 checkpoint 元数据，确认训练是否真实完成到第 5 轮。
- 结果：确认三个 checkpoint 元数据均可读取，其中 `latest_epoch.bin` 与 `best_epoch.bin` 的 `epoch=5`；`best_epoch.bin` 的 `min_loss` 优于 `latest_epoch.bin`。
- 状态：完成

### Step 54

- 时间：2026-05-05 13:39:34
- 操作：新增 `run_motionbert_pose_global_5ep.bat`，将 5 轮 `rootrel=False` 训练得到的 `best_epoch.bin` 接入独立推理入口。
- 结果：已形成与官方模型隔离的验证闭环：训练 checkpoint 独立、推理入口独立、输出目录独立。
- 状态：完成

### Step 55

- 时间：2026-05-05 13:40:29
- 操作：新增 `run_motionbert_offline_livelink_global_5ep.bat`，将 `outputs/global_5ep_test/X3D.npy` 直接回放到 UE Live Link。
- 结果：5 轮 `rootrel=False` 测试模型现已具备独立的 UE 观察入口，无需覆盖原有 offline Live Link 脚本。
- 状态：完成

### Step 56

- 时间：2026-05-05 13:42:54
- 操作：根据用户反馈，排查 `run_motionbert_pose_global_5ep.bat` 在 `infer_wild.py` 加载自训 `best_epoch.bin` 时的 `pickle.UnpicklingError`。
- 结果：确认根因是 PyTorch 2.6+ 将 `torch.load` 的默认 `weights_only` 改为 `True`，而本项目自训 checkpoint 需要完整反序列化。
- 状态：完成

### Step 57

- 时间：2026-05-05 13:42:54
- 操作：为 `infer_wild.py`、`infer_wild_mesh.py`、`realtime_motionbert.py` 与 `train.py` 中的 MotionBERT checkpoint 加载调用补充 `weights_only=False`。
- 结果：已完成对本项目主要 checkpoint 入口的加载兼容修复，避免后续自训模型在推理或恢复训练时再次触发同类报错。
- 状态：完成

### Step 58

- 时间：2026-05-05 13:54:05
- 操作：新增 `run_motionbert_realtime_global_5ep.bat`，将 5 轮 `rootrel=False` 测试模型接入现有 realtime 推理链。
- 结果：当前测试模型已具备独立的实时入口，且可继续复用现有 UE Live Link 发送链。
- 状态：完成

### Step 59

- 时间：2026-05-05 14:14:58
- 操作：为 `run_motionbert_realtime.bat` 与 `run_motionbert_realtime_global_5ep.bat` 增加 `UE_PRESET=quality/smooth/fast` 三挡预设。
- 结果：realtime 入口现已支持通过单一预设快速切换到偏画质、偏顺滑或偏性能的参数组合，默认值为 `smooth`。
- 状态：完成

### Step 60

- 时间：2026-05-05 14:21:12
- 操作：将 `realtime_motionbert.py` 的主循环从串行同步处理改为“采集线程 + 处理线程 + 主线程显示”的最新帧异步流水线。
- 结果：realtime 链路现已采用低延迟优先策略，允许丢弃旧帧，避免 UE 持续消费排队后的陈旧姿态结果。
- 状态：完成

### Step 61

- 时间：2026-05-05 14:21:12
- 操作：执行 `.\.local\miniconda3\envs\alphapose\python.exe realtime_motionbert.py --help` 进行轻量自检。
- 结果：命令成功，确认异步改造后脚本仍可正常解析全部参数。
- 状态：完成

### Step 62

- 时间：2026-05-05 14:45:13
- 操作：为 `realtime_motionbert.py` 新增 UE 发送重采样层，加入 `--ue_interpolate` 与 `--ue_buffer_ms`，并将单帧共享结构升级为带时间戳的姿态缓冲。
- 结果：realtime 发送端现在不再只是固定频率重复最后一帧，而是可以对最近两帧 3D 姿态做时间插值后再发往 UE。
- 状态：完成

### Step 63

- 时间：2026-05-05 14:45:13
- 操作：更新 `run_motionbert_realtime.bat` 与 `run_motionbert_realtime_global_5ep.bat`，加入 `UE_INTERPOLATE`、`UE_BUFFER_MS`，并接入 `quality / smooth / fast` 三档预设。
- 结果：用户现在可以只改 bat 顶部变量，就直接调节 UE 端顺滑度与延迟权衡。
- 状态：完成

### Step 64

- 时间：2026-05-05 14:45:13
- 操作：执行 `./.local/miniconda3/envs/alphapose/python.exe realtime_motionbert.py --help` 做参数自检。
- 结果：命令成功，已确认帮助输出包含 `--ue_interpolate` 与 `--ue_buffer_ms`。
- 状态：完成

### Step 65

- 时间：2026-05-05 14:45:13
- 操作：执行 `cmd /c run_motionbert_realtime.bat` 与 `cmd /c run_motionbert_realtime_global_5ep.bat` 做 bat 回归检查。
- 结果：两份 bat 的参数拼装均成功；当前环境无法打开 `camera:0`，因此在采集阶段退出，不属于本次参数改动故障。
- 状态：完成

### Step 66

- 时间：2026-05-05 14:45:13
- 操作：使用本地样例视频 `outputs\test.mp4` 运行 `run_motionbert_realtime.bat` 做一次无摄像头主流程验证。
- 结果：脚本成功进入 realtime 主流程，加载 AlphaPose、YOLOX 与 MotionBERT 后正常跑完并退出，确认新插值发送路径未破坏主链路。
- 状态：完成

### Step 67

- 时间：2026-05-05 18:42:44
- 操作：按用户要求直接修改 `run_motionbert_realtime.bat` 默认值，将 realtime 入口改为更偏 UE 实时预览的低负载配置。
- 结果：默认已切换为 `640x360 + NO_DISPLAY=1 + UE_PRESET=fast + MOTION_SMOOTH=0.15 + UE_BUFFER_MS=30`。
- 状态：完成

### Step 68

- 时间：2026-05-05 18:42:44
- 操作：同步修改 `run_motionbert_realtime_global_5ep.bat` 默认值，避免官方模型入口与 5 轮测试模型入口行为不一致。
- 结果：两份 realtime bat 现已保持相同的 UE 实时默认策略。
- 状态：完成

### Step 69

- 时间：2026-05-05 18:52:19
- 操作：按用户要求回退 `run_motionbert_realtime.bat` 的默认参数代设。
- 结果：默认值已恢复为 `1280x720 + UE_PRESET=smooth + NO_DISPLAY=0 + MOTION_SMOOTH=0.0 + UE_BUFFER_MS=45`。
- 状态：完成

### Step 70

- 时间：2026-05-05 18:52:19
- 操作：同步回退 `run_motionbert_realtime_global_5ep.bat` 的默认参数代设。
- 结果：两个 realtime bat 再次恢复一致，且仅回退默认值，不移除实时插值功能。
- 状态：完成

### Step 71

- 时间：2026-05-05 19:05:39
- 操作：将 `UnrealLiveLinkSender` 默认发送格式改为紧凑 `positions` 包，并停止逐帧附带完整骨架 schema 与发送端时间戳。
- 结果：单帧 UDP 载荷由约 `1699 bytes` 降至约 `302 bytes`。
- 状态：完成

### Step 72

- 时间：2026-05-05 19:05:39
- 操作：修改 UE 插件 `MotionBERTLiveLinkSource.cpp`，使接收线程每轮只消费最新 UDP 包，并将 socket 等待超时从 `10ms` 降到 `1ms`。
- 结果：传输层不再回放积压历史包，UE 侧更偏向“最新姿态优先”的实时策略。
- 状态：完成

### Step 73

- 时间：2026-05-05 19:05:39
- 操作：修改 `realtime_motionbert.py`，在 `--no_display` 且不保存视频时彻底跳过可视化生成。
- 结果：减少了无意义的 2D 绘制、3D 渲染与拼图开销，为实时链路释放了一部分 CPU/内存带宽。
- 状态：完成

### Step 74

- 时间：2026-05-05 19:05:39
- 操作：执行 `realtime_motionbert.py --help`、`replay_offline_to_ue.py --help`、短程离线回放，以及 UE 5.6 工程编译验证。
- 结果：Python 侧自检通过；离线回放脚本正常；UE 5.6 `MotionBERT_UEEditor` 构建成功，确认插件源码改动可编译。
- 状态：完成

### Step 75

- 时间：2026-05-05 19:08:57
- 操作：按用户要求将“效果优先、资源可尽量使用、先查成熟方案”的长期偏好写入 `Agents/AGENTS.md`。
- 结果：项目 agent 规则现已显式要求，在用户允许的前提下优先使用更多算力换取更好的实时效果，并优先评估官方能力与成熟方案。
- 状态：完成

### Step 76

- 时间：2026-05-05 19:12:37
- 操作：回查并修正紧凑 `positions` 发送格式的语义错误，将其从全局坐标改为局部骨骼位移。
- 结果：`positions` 已重新与原 `transforms.translation` 对齐，不再破坏 UE 骨架姿态关系。
- 状态：完成

### Step 77

- 时间：2026-05-05 19:12:37
- 操作：执行 Python 对照自检与短程离线回放验证。
- 结果：自检返回 `True`，确认紧凑包与原局部 transform 平移量一致；离线回放脚本可正常启动。
- 状态：完成

### Step 78

- 时间：2026-05-05 19:20:46
- 操作：将 `realtime_motionbert.py` 从单一串行 `process_loop` 拆分为 `pose2d_loop + pose3d_loop + livelink_loop` 的流水线结构。
- 结果：2D、3D 与 UE 发送现可重叠执行，不再严格串行阻塞。
- 状态：完成

### Step 79

- 时间：2026-05-05 19:20:46
- 操作：为 2D 阶段新增 `SharedLatest2D`，并在采集入口尝试设置 `cv2.CAP_PROP_BUFFERSIZE=1`。
- 结果：2D 结果可独立供 3D 线程消费，输入缓存也更偏向最新帧优先。
- 状态：完成

### Step 80

- 时间：2026-05-05 19:20:46
- 操作：执行 `realtime_motionbert.py --help` 与基于 `outputs\test.mp4` 的 realtime 回归运行。
- 结果：脚本帮助命令正常；样例视频主流程成功跑完并正常退出，确认流水线改造未破坏当前入口。
- 状态：完成

### Step 81

- 时间：2026-05-05 19:26:15
- 操作：按用户反馈将 `realtime_motionbert.py` 主入口回退到上一版更快的稳定实现。
- 结果：默认 realtime 入口恢复为用户体感更优的版本。
- 状态：完成

### Step 82

- 时间：2026-05-05 19:26:15
- 操作：为较慢的流水线改造版创建独立快照与专用 runner，并补充 `versions/realtime/README.md`。
- 结果：当前已形成“稳定版 + 实验版”并行入口，后续可直接切换回退，无需手工重改源码。
- 状态：完成

### Step 83

- 时间：2026-05-05 19:26:15
- 操作：修正实验版脚本的项目根目录解析，并执行样例视频回归运行。
- 结果：`run_motionbert_realtime_pipeline_experimental.bat` 可正常跑通 `outputs\test.mp4` 样例流程。
- 状态：完成

### Step 84

- 时间：2026-05-05 19:40:53
- 操作：根据当前稳定版现场表现与资源占用反馈，整理独立性能天花板判断文档 `docs/realtime_perf_ceiling_assessment.md`。
- 结果：已形成可供后续做优化投入判断的书面结论，明确区分“小幅继续优化”和“路线级调整”的边界。
- 状态：完成

## 2026-05-06 Windows 5090 一键部署

### Step 24

- 时间：2026-05-06 11:21:01
- 操作：重新读取 `Agents/` 规范文档、现有部署文档与核心 `.bat` 入口，确认当前项目在 Windows 下的真实运行基线。
- 结果：确认当前可交付部署目标为仓库内 Miniconda 双环境方案，且现有运行入口分别覆盖 AlphaPose、MotionBERT 离线 3D 与 realtime 链路。
- 状态：完成

### Step 25

- 时间：2026-05-06 11:21:01
- 操作：导出当前工作机的 `motionbert` 与 `alphapose` conda 环境，提取已验证版本作为新机一键部署目标版本。
- 结果：确认部署目标版本为 Python `3.10.20`、PyTorch `2.11.0+cu128`、torchvision `0.26.0+cu128`、torchaudio `2.11.0+cu128`，并据此整理出两份 requirements 清单。
- 状态：完成

### Step 26

- 时间：2026-05-06 11:21:01
- 操作：查阅官方资料，确认 Windows 静默安装 Miniconda、PyTorch cu128 安装通路、MotionBERT checkpoint 来源和 AlphaPose detector 权重来源。
- 结果：确认可以基于官方来源设计自动化脚本；其中 MotionBERT checkpoint 可直接走作者 Hugging Face 仓库，YOLOX-X detector 可走 AlphaPose 文档指向的官方发布地址。
- 状态：完成

### Step 27

- 时间：2026-05-06 11:21:01
- 操作：实现 `deploy_windows_5090.bat` 与 `scripts/deploy_windows_5090.ps1`，串起 Miniconda 安装、环境创建、依赖安装、AlphaPose 源码补齐、关键权重检查与 smoke check。
- 结果：一键部署入口已落地，并同步新增 `docs/deploy_windows_5090.md` 说明使用方式与已知前提。
- 状态：完成

## 2026-05-06 工作区搬家包

### Step 28

- 时间：2026-05-06 12:17:03
- 操作：根据用户新要求，将原本“离线搬家包”思路调整为“只搬工作区，不搬 conda / python 环境”。
- 结果：确认新的交付边界为：打包 workspace，自带源码、配置、权重和外部依赖目录，但环境仍在目标机器本地安装。
- 状态：完成

### Step 29

- 时间：2026-05-06 12:17:03
- 操作：实现 `create_workspace_bundle.bat` 与 `scripts/create_workspace_bundle.ps1`，支持 `lean/full` 模式与 `INCLUDE_OUTPUTS` 控制。
- 结果：工作区打包入口已落地，并新增 `docs/workspace_bundle.md` 说明目标机使用方式。
- 状态：完成

### Step 30

- 时间：2026-05-06 12:17:03
- 操作：实际执行 `powershell -File .\scripts\create_workspace_bundle.ps1 -RepoRoot . -BundleMode lean -IncludeOutputs 0 -BundleName MotionBERT_workspace_lean_v2` 做正式打包验证。
- 结果：成功生成 `dist\workspace_bundle\MotionBERT_workspace_lean_v2.zip`；同时验证默认会排除 `.local`、`outputs` 与 Unreal 中间构建目录。
- 状态：完成

### Step 31

- 时间：2026-05-06 12:21:52
- 操作：清理工作区打包验证过程中遗留的 `MotionBERT_workspace_lean_test_stage` 旧 staging 目录，并一并移除中转空目录。
- 结果：确认 `dist\workspace_bundle` 仅剩当前正式版本相关产物。
- 状态：完成

### Step 32

- 时间：2026-05-06 12:25:03
- 操作：删除当前正式工作区搬家包对应的 `MotionBERT_workspace_lean_v2_stage` staging 目录，仅保留 zip 成品。
- 结果：确认 `dist\workspace_bundle` 目录中只剩最终交付文件 `MotionBERT_workspace_lean_v2.zip`。
- 状态：完成

### Step 33

- 时间：2026-05-06 13:31:58
- 操作：本地执行 `cmd /c deploy_windows_5090.bat` 复现用户反馈的“路径具有非法字符”问题，并定位 batch 向 PowerShell 传递末尾反斜杠路径时的引号转义问题。
- 结果：确认根因不是仓库内容本身，而是 `%~dp0` 直接带尾部反斜杠参与带引号传参，导致 `Resolve-Path` 接收到脏路径字符串。
- 状态：完成

### Step 34

- 时间：2026-05-06 13:31:58
- 操作：修复 `deploy_windows_5090.bat` 与 `create_workspace_bundle.bat` 的路径归一化方式，并在 `scripts/deploy_windows_5090.ps1` 中补充输入路径去引号清洗。
- 结果：重新执行 `cmd /c deploy_windows_5090.bat` 后，脚本已能正常进入后续部署步骤，不再报“Illegal characters in path”。
- 状态：完成

### Step 35

- 时间：2026-05-06 14:04:44
- 操作：根据目标机器上 `deploy_windows_5090.bat` 的真实输出，定位环境创建失败根因为 Conda 默认渠道 Terms of Service 未接受。
- 结果：确认不是路径或项目依赖问题，而是 Conda 在非交互环境下阻止 `create --prefix` 执行。
- 状态：完成

### Step 36

- 时间：2026-05-06 14:04:44
- 操作：在 `scripts/deploy_windows_5090.ps1` 中加入 `conda tos accept` 自动接受逻辑，并同步更新部署文档说明。
- 结果：后续在目标机器重新运行部署脚本时，将先自动完成默认 Anaconda 渠道 ToS 接受，再继续创建环境。
- 状态：完成

### Step 37

- 时间：2026-05-06 14:29:53
- 操作：根据用户对“想直接看到下载进度条”的要求，调整 `deploy_windows_5090.bat` 为默认实时输出模式，并保留可选日志重定向模式。
- 结果：部署入口默认将直接显示 PowerShell / conda / pip 的实时输出；仅在设置 `LOG_TO_FILE=1` 时才写入 `deploy_run.log`。
- 状态：完成

### Step 38

- 时间：2026-05-06 15:29:10
- 操作：根据目标机器的真实安装输出，定位部署失败根因为 PyTorch cu128 wheel 在线下载中途中断，出现 `IncompleteRead` / `ProtocolError`。
- 结果：确认不是版本错误或环境冲突，而是 2.7GB 级别 wheel 在当前网络条件下不适合直接让 pip 在线拉取。
- 状态：完成

### Step 39

- 时间：2026-05-06 15:29:10
- 操作：修改部署脚本，新增 PyTorch wheel 本地缓存与 `curl.exe` 断点续传下载逻辑，再由两个环境复用同一批本地 wheel 离线安装。
- 结果：部署链路已从“每个环境单独在线装 torch”调整为“先缓存、再复用安装”的更稳方案。
- 状态：完成

### Step 40

- 时间：2026-05-06 15:46:20
- 操作：根据目标机器输出，定位新失败原因为断点续传留下的损坏 PyTorch wheel 被缓存命中，随后本地安装时报 `Wheel ... is invalid`。
- 结果：确认部署链路已从“在线中断”推进到“缓存损坏识别”阶段，问题不在 pip 安装命令本身，而在缓存文件有效性判断缺失。
- 状态：完成

### Step 41

- 时间：2026-05-06 15:46:20
- 操作：为 `scripts\deploy_windows_5090.ps1` 增加 wheel zip 完整性校验，并在缓存损坏时自动删除后重下。
- 结果：后续部署将不再把损坏的 `.whl` 文件当成有效缓存复用。
- 状态：完成

### Step 42

- 时间：2026-05-06 16:25:04
- 操作：根据目标机器输出，定位 MotionBERT 依赖安装失败根因为 `chumpy` 在 pip build isolation 环境下构建时报 `ModuleNotFoundError: No module named 'pip'`。
- 结果：确认问题不在整套 requirements，而是 `chumpy` 这个旧包与当前隔离构建机制兼容性差。
- 状态：完成

### Step 43

- 时间：2026-05-06 16:25:04
- 操作：将 `chumpy` 从 `requirements-motionbert-5090.txt` 中拆出，并在部署脚本里改为单独执行 `pip install --no-build-isolation chumpy==0.70`。
- 结果：后续 MotionBERT 依赖安装将不再因为 `chumpy` 的隔离构建而中断。
- 状态：完成
### Step 44

- 时间：2026-05-08 10:20:00
- 操作：新增 `deploy_windows_5070ti.bat`、`scripts/deploy_windows_5070ti.ps1` 与 `docs/deploy_windows_5070ti.md`，为本地 5070 Ti 机器提供独立的一键部署入口。
- 结果：5070 Ti 现在也可以走与 5090 相同的“源码单独管理、环境由部署脚本重建”的方案，且不影响已验证的 5090 部署链路。
- 状态：完成

### Step 45

- 时间：2026-05-08 10:42:00
- 操作：新增 `docs/assets_storage_strategy.md` 与 `deploy/assets_manifest.template.json`，为仓库的大文件资源设计正式分层方案与下载清单模板。
- 结果：当前项目已经具备清晰的资产管理结构，可将 checkpoint、预训练权重和私有资源逐步迁移到“部署下载补齐”模式，而不是塞进 git。
- 状态：完成

### Step 46

- 时间：2026-05-08 22:35:00
- 操作：将 `scripts/deploy_windows_5070ti.ps1` 与 `scripts/deploy_windows_5090.ps1` 中的模型资源下载地址改为优先使用 `EMOCJC/motionbert_models`。
- 结果：后续新机器部署将优先从用户自己的 Hugging Face models 仓库拉取 MotionBERT checkpoint、AlphaPose backbone 和 YOLOX detector 权重。
- 状态：完成
