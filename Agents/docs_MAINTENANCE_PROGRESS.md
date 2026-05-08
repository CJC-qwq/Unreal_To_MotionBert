# 维护进度

> 说明
> - 本文档面向当前项目开发者，记录 MotionBERT 项目的真实维护进度。
> - 所有记录按时间顺序追加，时间精确到秒。
> - 每次非 trivial 修改后都要同步更新本文档。

## 2026-05-04 12:56:45

### 任务

- 解析并执行 `Agents/AGENTS.md`、`Agents/docs_REQUIREMENT_WORKFLOW.md`、`Agents/docs_MAINTENANCE_PROGRESS.md`
- 为 MotionBERT 项目建立可执行的 agent 启动规则
- 清理示例内容，建立真实维护基线

### 变更

- 重写 `Agents/AGENTS.md`
- 重写 `Agents/docs_REQUIREMENT_WORKFLOW.md`
- 重写 `Agents/docs_MAINTENANCE_PROGRESS.md`

### 验证

- 已确认后续修改前需先读取 `Agents/` 规则文档
- 已确认关键运行入口包括 `run_alphapose.bat`、`run_motionbert_pose.bat`、`run_motionbert_mesh.bat`、`run_motionbert_realtime.bat`

### 状态

- 已完成

### 下一步

- 后续每次真实修改都按本文档格式继续维护

## 2026-05-04 13:00:54

### 任务

- 将维护进度文档调整为面向开发者的中文版本

### 变更

- 将进度文档整体改为中文维护格式

### 验证

- 已确认标题、说明与记录均为中文

### 状态

- 已完成

### 下一步

- 后续持续按中文格式维护

## 2026-05-04 13:03:17

### 任务

- 开始实现 MotionBERT 实时 3D 版本
- 建立正式测试日志文档

### 变更

- 新增 `Agents/docs_TEST_EXECUTION_LOG.md`
- 建立实时 3D 开发任务入口

### 验证

- 已进入实时链路分析阶段

### 状态

- 进行中

### 下一步

- 分析 AlphaPose 与 MotionBERT 的实时集成方案

## 2026-05-04 13:11:09

### 任务

- 完成实时 3D 第一阶段实现

### 变更

- 新增 `realtime_motionbert.py`
- 新增 `run_motionbert_realtime.bat`
- 更新 `Agents/AGENTS.md`
- 更新 `docs/local_deploy_windows.md`

### 验证

- 已验证 `realtime_motionbert.py --help`
- 已打通 AlphaPose + MotionBERT 的实时闭环
- 已生成 `outputs/realtime_preview.mp4`

### 状态

- 已完成

### 下一步

- 在真实摄像头场景下继续验证

## 2026-05-04 13:15:45

### 任务

- 补充验证 `run_motionbert_realtime.bat` 在 `cmd` 下的可执行性

### 变更

- 调整 `run_motionbert_realtime.bat` 支持通过环境变量覆盖关键参数
- 增加 `EXTRA_RUNTIME_ARGS`

### 验证

- 已通过 `cmd /c` 成功拉起实时推理链路

### 状态

- 已完成

### 下一步

- 继续做真实设备验证

## 2026-05-04 17:32:41

### 任务

- 修复实时 3D 画面头脚倒置问题

### 变更

- 更新 `realtime_motionbert.py` 的 `render_3d_pose()`，移除多余的 Y 轴翻转

### 验证

- 已重新运行 `run_motionbert_realtime.bat`
- 已生成 `outputs/realtime_fix_preview.mp4`
- 已抽帧确认骨架朝向恢复正常

### 状态

- 已完成

### 下一步

- 继续观察真实摄像头下的稳定性

## 2026-05-04 17:43:02

### 任务

- 为 agent 规则增加“先搜索现成插件 / API / SDK，再决定是否自研”的强制要求

### 变更

- 更新 `Agents/AGENTS.md`
- 更新 `Agents/docs_REQUIREMENT_WORKFLOW.md`

### 验证

- 已确认规则写入两份文档

### 状态

- 已完成

### 下一步

- 后续非 trivial 开发先做外部能力检索

## 2026-05-04 17:50:30

### 任务

- 为项目 agent 体系增加 Unreal Engine 专家模式

### 变更

- 更新 `Agents/AGENTS.md`
- 明确 MotionBERT 到 UE 的优先路线：
  1. 官方 Live Link
  2. 官方运行时插件能力
  3. 维护中的第三方插件
  4. 自定义传输或 UE 插件

### 验证

- 已通过官方文档确认 Live Link 与 Anim Blueprint 的适用路线

### 状态

- 已完成

### 下一步

- 等 UE 工程进入工作区后按该路线实施

## 2026-05-04 20:11:45

### 任务

- 继续推进 MotionBERT 到 Unreal Engine 5.6 的直接 Live Link 路线
- 在现有 UDP Live Link Source 基础上补齐可驱动默认人形骨架的 retarget 层
- 明确默认 Third Person 工程的角色、Mesh、AnimBP 插入点，并同步更新接入文档与测试记录

### 变更

- 在 `unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink` 中新增 `UMotionBERTLiveLinkRetargetAsset`
  - `Public/MotionBERTLiveLinkRetargetAsset.h`
  - `Private/MotionBERTLiveLinkRetargetAsset.cpp`
- 更新 `MotionBERTLiveLink.Build.cs`，补充 `AnimationCore` 与 `LiveLinkAnimationCore` 依赖
- 在 retarget 逻辑中实现：
  - 从 MotionBERT Live Link 本地骨骼变换重建源关节的 component-space 位置
  - 将 MotionBERT 17 点骨架映射到 UE mannequin 的 pelvis / spine / neck / arm / leg 主干骨骼
  - 保留 UE 参考姿势骨长，仅重建主要局部旋转，作为第一阶段可用姿态解
- 更新 `docs/ue_livelink_integration.md`，补充当前 retarget 状态、默认 Third Person 资产链路以及 `Live Link Pose` 节点接线步骤

### 验证

- 已通过官方 UE Live Link / Retarget 路径调研确认应优先走 `ULiveLinkRetargetAsset`
- 已检查当前 UE 项目默认角色链路为：
  - `DefaultEngine.ini`
  - `BP_ThirdPersonGameMode`
  - `BP_ThirdPersonCharacter`
  - 默认角色 Mesh / AnimBP 由蓝图资产持有，而非当前 C++ 构造函数直接绑定
- 已使用 UE 5.6 `Build.bat` 对 `MotionBERT_UEEditor` 进行工程级编译验证
- 首轮编译发现并修复 `MotionBERTLiveLinkRetargetAsset.cpp` 中的 C++ 细节问题：
  - `const FVector` 归一化写法错误
  - `FCompactPoseBoneIndex` 缺少显式初始化
- 修复后重新编译成功，确认插件与新增 retarget 类可被 UE 5.6 正常构建

### 状态

- 进行中

### 说明

- 当前已从“仅能收 Live Link 数据包”推进到“UE 工程内已有可用的 MotionBERT retarget 类”
- 这一版属于可运行的第一阶段骨架驱动实现，优先保证主干肢体可动
- 手指、脚趾、足底滚动、精细肩胯扭转仍未进入本阶段范围，后续可继续细化

### 下一步

- 在 UE 编辑器内将 `MotionBERTLiveLinkRetargetAsset` 接到 `Live Link Pose` 节点上，完成默认角色的实际预览闭环
- 视预览结果继续细化 pelvis 平移、肩部朝向、脚部朝向与脚底稳定性
- 如需减少手动操作，可继续评估是否为项目增加专用 mocap 预览角色蓝图或专用 AnimBP

## 2026-05-04 20:20:40

### 任务

- 将 UE 编辑器里的最终接线方案从“文档步骤”推进到“工程内默认可运行的 mocap 预览链路”
- 尽量减少对自定义 AnimBP 资产的依赖，让项目打开后可直接进入 MotionBERT 预览路径

### 变更

- 调研并确认 UE 5.6 自带 `ULiveLinkInstance`，可直接作为官方 Live Link 动画实例使用
- 更新 `unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERT_UE.Build.cs`
  - 新增 `LiveLink`
  - 新增 `LiveLinkAnimationCore`
  - 新增 `LiveLinkInterface`
  - 新增 `MotionBERTLiveLink`
- 新增专用 mocap 预览角色：
  - `MotionBERTMocapPreviewCharacter.h`
  - `MotionBERTMocapPreviewCharacter.cpp`
- 新增专用 mocap 预览 GameMode：
  - `MotionBERTMocapPreviewGameMode.h`
  - `MotionBERTMocapPreviewGameMode.cpp`
- 预览角色中完成：
  - 原生绑定 `SKM_Manny_Simple`
  - 原生绑定 `IA_Jump`、`IA_Move`、`IA_Look`、`IA_MouseLook`
  - 原生切换到 `ULiveLinkInstance`
  - 原生写入 Live Link subject `MotionBERTPose`
  - 原生写入 retarget 类 `UMotionBERTLiveLinkRetargetAsset`
  - 挂载官方 `LiveLink Skeletal Animation` 组件
- 更新 `unreal/MotionBERT_UE/Config/DefaultEngine.ini`
  - 将 `GlobalDefaultGameMode` 切换为 `/Script/MotionBERT_UE.MotionBERTMocapPreviewGameMode`
- 更新 `docs/ue_livelink_integration.md`
  - 补充 direct preview path
  - 说明当前仓库已可不依赖自定义 AnimBP 直接走原生预览角色

### 验证

- 已检查 UE 5.6 本地源码，确认 `ULiveLinkInstance` 内部直接封装 `FAnimNode_LiveLinkPose`
- 已确认该路径属于官方 Live Link 动画实例能力，适合优先复用
- 已使用 UE 5.6 `Build.bat` 重新编译整个 `MotionBERT_UEEditor`
- 编译成功，确认新增 preview 角色与 preview GameMode 均可正常构建

### 状态

- 已完成

### 说明

- 当前仓库已经从“需要手动搭 Live Link Pose 节点”推进到“默认 GameMode 会生成一个原生 MotionBERT mocap 预览角色”
- 这条路径优先解决的是“打开项目后最快看到 MotionBERT 驱动角色”
- 如果后续需要更复杂的状态机、Locomotion 混合、面向角色蓝图的玩法逻辑，再回到专用 AnimBP 资产路线会更合适

### 下一步

- 在 UE 编辑器中实际启动 Live Link source 与 MotionBERT 发送端，观察 Manny 实机预览效果
- 根据预览结果继续细化 retarget 质量，优先关注肩部、骨盆和脚部稳定性

## 2026-05-04 20:38:20

### 任务

- 排查 UE 默认地图已切换到 mocap preview 角色后，角色仍不跟随 Live Link 动作的原因
- 验证 `MotionBERTMocapPreviewCharacter` 的 Live Link 绑定时机修复是否已真正合入

### 变更

- 检查 `MotionBERTMocapPreviewCharacter.cpp`，确认该角色已有
  - `Tick()` 重试绑定逻辑
  - `bLiveLinkConfigured` 状态位
  - `ConfigureLiveLink()` 成功后才停止重试
- 检查 `MotionBERTLiveLinkSource.cpp`，确认 UDP 收包后会推送 static data 与 frame data
- 检查 UE 5.6 官方 `ULiveLinkInstance` / `FAnimNode_LiveLinkPose` 源码，确认运行时需在 AnimInstance 创建后再写入 Subject 与 RetargetAsset

### 验证

- 已尝试编译一次，但被 `Live Coding` 拦截，未能合入新修复
- 用户关闭 UE 编辑器后，已重新执行 `Build.bat MotionBERT_UEEditor`
- 编译成功，确认 `MotionBERTMocapPreviewCharacter` 的新二进制已合入到 UE 5.6 编辑器

### 状态
- 进行中

### 说明

- 目前最可能的卡点不再是地图或 GameMode 选错，而是 Live Link subject 数据是否真正进入 `ULiveLinkInstance`，以及 retarget 条件是否满足
- 最新的预览角色已具备 Tick 重试绑定能力，可以先排除“绑定太早”的时机问题

### 下一步
- 用户重新打开 UE 5.6 工程并回归测试
- 确认 Live Link 面板中 `MotionBERTPose` 是否绿标更新
- 若仍不动，继续定位是 subject、retarget 还是 preview character 调用层卡住

## 2026-05-04 20:46:45

### 任务

- 排查 Live Link source 一直停留在 `Listening`、未进入收包状态的原因
- 修复 Windows 批处理入口默认未开启 UE Live Link 发送的问题

### 变更

- 更新 `run_motionbert_realtime.bat`
- 将 `UE_LIVELINK_HOST` 默认值从空改为 `127.0.0.1`
- 增加启动时 Live Link 目标打印，显示 host、port 与 subject

### 验证

- 已检查批处理逻辑：仅当 `UE_LIVELINK_HOST` 非空时才会追加 `--ue_livelink_host` 参数
- 已确认旧版默认值为空，会导致 realtime 脚本根本不向 UE 发送 UDP 数据
- 已完成脚本修复，后续运行 `run_motionbert_realtime.bat` 时将默认向本机 `127.0.0.1:7001` 发送 `MotionBERTPose`

### 状态
- 进行中

### 说明

- 该问题的根因更接近“发送端默认关闭”，而不是 UE Live Link 插件或预览角色逻辑失效
- 这也解释了为什么 UE 面板始终只有 `Listening`，却看不到 `Active` 或 `MotionBERTPose`

### 下一步
- 重新运行 `run_motionbert_realtime.bat`
- 回 UE Live Link 面板确认 source 是否从 `Listening` 进入 `Active`
- 若仍未收到包，再继续增加插件侧收包日志

## 2026-05-04 20:51:55

### 任务

- 修复 Live Link 已进入 `Active` 后，Manny 仅有局部关节抽象扭曲、未形成可用全身动作的问题
- 收紧 UE retarget 的骨骼索引映射与方向参考逻辑

### 变更

- 更新 `unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink/Source/MotionBERTLiveLink/Private/MotionBERTLiveLinkRetargetAsset.cpp`
- 修复 `BuildTargetIndex()`：改为通过 `FBoneContainer.MakeMeshPoseIndex()` 获取正确的目标骨骼名，不再直接把 `CompactPoseBoneIndex` 当作 mesh/ref skeleton 索引
- 调整 `FDirectionMapping` 结构，新增 `TargetChildBone`
- 将脊柱、锁骨、上臂、前臂、大腿、小腿的参考方向从“自动取第一个子骨头”改为“显式指定目标子骨头”
- 避开 Manny skeleton 中 twist bone / 辅助 bone 对主骨朝向解算的干扰

### 验证

- 已重新执行 `Build.bat MotionBERT_UEEditor`
- 编译成功，`UnrealEditor-MotionBERTLiveLink.dll` 已重新链接
- 确认新的 retarget 解算逻辑已合入 UE 5.6 工程

### 状态
- 进行中

### 说明

- 当前问题已从“数据链路未通”推进到“retarget 数学质量修正”阶段
- 从用户截图判断，之前更像是目标骨名映射偏差与 twist bone 参考方向选错叠加导致的局部扭曲，而不是 Live Link 没驱动

### 下一步
- 用户重新打开 UE 并回归测试当前 Manny 预览姿态
- 观察躯干、双臂、双腿是否从局部扭曲改善为整体随动
- 若仍存在系统性偏转，再继续收紧坐标轴映射与 pelvis / shoulder basis 计算

## 2026-05-04 20:58:16

### 任务

- 继续修复 Live Link 已激活但 Manny 仍出现躯干到四肢胡乱抽搐拉伸的问题
- 将 retarget 从“粗粒度主链朝向”推进到“分段加权 + 末端骨补驱动”

### 变更

- 继续更新 `unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink/Source/MotionBERTLiveLink/Private/MotionBERTLiveLinkRetargetAsset.cpp`
- 为 `FDirectionMapping` 新增 `RotationAlpha`
- 对 spine / neck / clavicle 链引入分段权重，避免同一方向信息在多节骨链上重复吃满角度
- 新增 `FEndBoneMapping` 与 `TryApplyEndBone()`
- 为 `head`、`hand_l`、`hand_r`、`foot_l`、`foot_r` 增加末端骨朝向解算
- 重新编译 UE 5.6 工程，确认插件 DLL 更新完成

### 验证

- 已重新执行 `Build.bat MotionBERT_UEEditor`
- 构建成功，`UnrealEditor-MotionBERTLiveLink.dll` 已重新链接
- 确认新的“分段加权 + 末端骨驱动” retarget 逻辑已合入工程

### 状态
- 进行中

### 说明

- 用户反馈“头、手、脚没有一起驱动”是有效信号，说明之前的 retarget 主要只覆盖了主链骨，而末端骨未单独处理
- 用户反馈“躯干到四肢胡乱抽搐拉伸”同样合理，说明脊柱等多节骨链重复吃同一源方向会放大姿态误差

### 下一步
- 用户重新打开 UE 并回归测试最新姿态效果
- 若仍存在系统性错位，则进一步收敛到 MotionBERT 坐标轴到 UE 坐标轴的转换问题
- 如确认 Manny 方案持续失真，将评估改为先驱动一个 H36M/17 点中间骨架，再转入 UE 角色骨架

## 2026-05-04 21:03:17

### 任务

- 为 UE 调试补充离线 MotionBERT 数据通路，不再只依赖 realtime 摄像头链路
- 将原项目 `infer_wild.py` 产出的 `X3D.npy` 接入同一条 Live Link subject 调试路径

### 变更

- 新增 `lib/utils/ue_livelink.py`
  - 抽取公共 Live Link bone schema
  - 抽取 MotionBERT 到 UE 的坐标转换
  - 抽取 `UnrealLiveLinkSender`
- 更新 `realtime_motionbert.py`
  - 改为复用公共 `ue_livelink` 模块
- 新增 `replay_offline_to_ue.py`
  - 支持把 `X3D.npy` 逐帧回放到 UE Live Link
  - 支持 FPS 指定或从原视频自动读取 FPS
  - 支持起止帧范围与循环播放
- 新增 `run_motionbert_offline_livelink.bat`
  - 提供离线 `X3D.npy -> MotionBERTPose` 的 cmd 入口
- 更新 `docs/ue_livelink_integration.md`
  - 补充离线调试通路说明

### 验证

- 已验证 `replay_offline_to_ue.py --help`
- 已验证 `run_motionbert_offline_livelink.bat` 可正常启动离线回放流程
- 已验证 `replay_offline_to_ue.py --npy_path outputs\X3D.npy --video_path outputs\test.mp4 --start_frame 0 --end_frame 2 --fps 30` 可正常发送并结束

### 状态
- 完成

### 说明

- 现在 UE 调试已有两条输入路径：
  - realtime：摄像头/视频 -> AlphaPose -> MotionBERT -> UE
  - offline：`X3D.npy` -> UE
- 后续 retarget 质量调试建议优先使用 offline 路径，先固定输入动作，再分析骨架映射问题

### 下一步
- 使用 `run_motionbert_offline_livelink.bat` 在 UE 中回放固定动作序列
- 对比 offline 与 realtime 下的同一 subject 表现是否一致
- 若 offline 同样扭曲，则重点继续定位 UE retarget / 坐标系问题

## 2026-05-04 22:26:32

### 任务

- 放弃继续以 Manny 作为第一承载体
- 在 UE 中落地 MotionBERT 原生调试骨架视图，直接可视化 `MotionBERTPose` 的 17 点骨架数据

### 变更

- 新增 `unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERTDebugSkeletonActor.h`
- 新增 `unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERTDebugSkeletonActor.cpp`
- 该 Actor 直接通过 `ILiveLinkClient` 读取 `MotionBERTPose`
- 对 `FLiveLinkAnimationFrameData` 的本地变换逐级重建 component-space 关节点位置
- 在世界中绘制：
  - 17 个关节点
  - H36M/MotionBERT 主骨架连线
  - 可选关节点标签
- 更新 `unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERTMocapPreviewGameMode.h`
- 更新 `unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERTMocapPreviewGameMode.cpp`
  - 默认 Pawn 改为 `ASpectatorPawn`
  - BeginPlay 时生成 `AMotionBERTDebugSkeletonActor`
  - 现有 mocap preview 默认进入 MotionBERT 原生调试视图，而不是 Manny 角色预览

### 验证

- 已重新执行 `Build.bat MotionBERT_UEEditor`
- 构建成功
- 确认新的 debug skeleton actor 与 preview game mode 变更已编译进 UE 5.6 工程

### 状态
- 完成

### 说明

- 这一步的目标不是最终角色效果，而是先把“MotionBERT 数据在 UE 里到底长什么样”独立出来
- 该视图直接绕过 Manny retarget，因此更适合判断：
  - Live Link 数据是否稳定
  - 坐标映射是否正确
  - 离线 `X3D.npy` 与 realtime 数据是否一致

### 下一步
- 打开 UE，进入默认 mocap preview 流程，确认是否能直接看到 MotionBERT 原生 17 点骨架
- 用 `run_motionbert_offline_livelink.bat` 先回放固定离线动作做基线验证
- 若 17 点骨架正确，再推进到 MotionBERT 专用 mesh / 中间骨架资产路线

## 2026-05-04 22:36:02

### 任务

- 修复 MotionBERT 原生调试骨架“只看到名字在动、骨架位置融入建筑”的显示问题
- 让调试骨架以 root 居中，并生成在更合理的场景位置

### 变更

- 更新 `unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERTDebugSkeletonActor.cpp`
  - 在重建 component-space 关节点后，减去 root/pelvis 位置，使整套骨架以原点居中显示
  - 增大关节点半径与骨线粗细
  - 关闭关节点名字默认显示
  - 提高调试绘制前景优先级
- 更新 `unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERTMocapPreviewGameMode.cpp`
  - 生成调试骨架时优先使用 `PlayerStart`
  - 将骨架生成到 `PlayerStart` 前方并抬高，避免埋入关卡建筑
  - 增加生成位置日志输出

### 验证

- 已重新执行 `Build.bat MotionBERT_UEEditor`
- 构建成功
- 确认新的 debug skeleton 居中与生成位置修正已合入 UE 5.6 工程

### 状态
- 完成

### 说明

- 当前修复的是“调试骨架显示策略”，不是 MotionBERT 数据本体或后续 mesh 路线
- 目标是先让用户在 UE 里清楚地看到一套脱离建筑干扰的 MotionBERT 原生 17 点骨架

### 下一步
- 重新打开 UE 并回归观察调试骨架是否出现在玩家前方可见区域
- 若骨架已可见，再判断其运动形态是否正常
- 若仍出现坐标错位，再继续收紧 MotionBERT 到 UE 的坐标轴映射

## 2026-05-04 22:38:08

### 任务

- 修复 MotionBERT 原生调试骨架在 UE 中上下倒转的问题
- 收紧 MotionBERT 到 UE 的公共坐标轴映射

### 变更

- 更新 `lib/utils/ue_livelink.py`
- 修改 `motionbert_to_ue_vector()`
- 将 UE 垂直轴映射从 `y -> +Z` 改为 `y -> -Z`
- 由于 realtime 与 offline 共用该模块，本次修复会同时作用于：
  - `run_motionbert_realtime.bat`
  - `run_motionbert_offline_livelink.bat`

### 验证

- 已确认当前问题表现为“MotionBERT 原生 17 点骨架可以显示，但上下倒转”
- 已完成发送端公共坐标转换修复
- 本次为 Python 发送侧修改，无需重新编译 UE 工程

### 状态
- 完成

### 说明

- 这一步说明当前主要问题已收敛到坐标轴符号，而不再是 Live Link 链路、骨架生成位置或 Manny retarget 层
- 修复后重新运行离线或实时发送端即可观察效果变化

### 下一步
- 重新运行 `run_motionbert_offline_livelink.bat` 或 `run_motionbert_realtime.bat`
- 回 UE 观察 17 点骨架是否恢复为头朝上、脚朝下
- 若仍存在前后或左右方向异常，再继续收紧其余坐标轴映射

## 2026-05-04 22:47:54

### 任务

- 恢复 UE 调试骨架对原始 root 轨迹的显示，不再默认强制 pelvis 居中
- 先验证 MotionBERT 原始输出里到底带了多少前后/上下位移信息

### 变更

- 更新 `unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERTDebugSkeletonActor.h`
  - 新增 `bCenterOnRoot`
- 更新 `unreal/MotionBERT_UE/Source/MotionBERT_UE/MotionBERTDebugSkeletonActor.cpp`
  - 默认 `bCenterOnRoot = false`
  - 仅当 `bCenterOnRoot` 为真时，才执行 root/pelvis 去中心化
- 重新编译 UE 5.6 工程，使新的 root 轨迹显示行为生效

### 验证

- 已重新执行 `Build.bat MotionBERT_UEEditor`
- 构建成功
- 确认新的 debug skeleton 默认会显示原始 root 轨迹，而不是始终锁定在原点附近

### 状态
- 完成

### 说明

- 这一步不是在补“真实世界绝对位移”，而是在停止额外抹掉 MotionBERT 当前已有的 root 平移信息
- 其目的是先判断原始离线 / 实时数据本体里到底存在多少全局位移趋势

### 下一步
- 重新运行离线或实时发送端
- 回 UE 观察骨架是否开始出现前后或上下的整体位移趋势
- 若仍主要体现为缩放而非真实位移，则再进入全局运动估计方案

## 2026-05-04 22:53:10

### 任务

- 在 MotionBERT 原始姿态链之外补充“估计型全局位移层”
- 让 UE 里的调试骨架能够从 bbox 中心与尺度变化中获得前后/上下/左右整体位移趋势

### 变更

- 更新 `lib/utils/ue_livelink.py`
  - 新增 `apply_root_translation()`
  - 新增 `GlobalMotionEstimator`
  - `UnrealLiveLinkSender.send()` 支持额外传入 `root_translation`
- 更新 `replay_offline_to_ue.py`
  - 新增 `--json_path`
  - 新增 `--focus_id`
  - 新增 `--estimate_global_motion`
  - 支持从 `alphapose-results.json` 中读取每帧 bbox
  - 支持根据 bbox 中心与高度估计全局位移后叠加到 `X3D.npy`
- 更新 `realtime_motionbert.py`
  - 新增 `--estimate_global_motion`
  - 在实时链路中根据当前选中人物 bbox 与帧大小估计全局位移
- 更新 `run_motionbert_offline_livelink.bat`
  - 默认增加 `JSON_PATH`
  - 默认开启 `ESTIMATE_GLOBAL_MOTION=1`
- 更新 `run_motionbert_realtime.bat`
  - 默认开启 `ESTIMATE_GLOBAL_MOTION=1`

### 验证

- 已验证 `replay_offline_to_ue.py --help`
- 已验证 `replay_offline_to_ue.py --npy_path outputs\X3D.npy --video_path outputs\test.mp4 --json_path outputs\alphapose-results.json --estimate_global_motion --start_frame 0 --end_frame 2 --fps 30`
- 已验证 `realtime_motionbert.py --help`
- 当前为 Python 发送层修改，无需重新编译 UE 工程

### 状态
- 完成

### 说明

- 这一步补的是“调试可用的估计型全局位移”，不是严格真实世界标定后的物理位移
- 其目标是让 UE 调试骨架从“只有相对姿态变化”推进到“至少有合理的整体位移趋势”
- 估计依据是：
  - bbox 中心水平偏移 -> 左右位移
  - bbox 中心垂直偏移 -> 上下位移
  - bbox 高度相对首帧变化 -> 前后位移

### 下一步
- 重新运行 `run_motionbert_offline_livelink.bat`
- 回 UE 观察调试骨架是否开始出现整体前后/上下漂移
- 若趋势正确，再根据观感继续调节尺度与平滑系数

## 2026-05-04 23:01:48

### 任务

- 撤销基于 bbox 的估计型全局位移层
- 回到只传输 MotionBERT 模型真实输出的 UE 数据链路
- 后续位移需求改为优先评估 MotionBERT 原生支持位移输出的训练/推理配置

### 变更

- 更新 `lib/utils/ue_livelink.py`
  - 删除 `apply_root_translation()`
  - 删除 `GlobalMotionEstimator`
  - `UnrealLiveLinkSender.send()` 恢复为仅发送 joints 数据
- 更新 `replay_offline_to_ue.py`
  - 删除 `--json_path`
  - 删除 `--focus_id`
  - 删除 `--estimate_global_motion`
  - 删除 bbox 读取与估计位移逻辑
- 更新 `realtime_motionbert.py`
  - 删除 `--estimate_global_motion`
  - 删除实时 bbox 位移估计注入逻辑
- 更新 `run_motionbert_offline_livelink.bat`
  - 删除 `JSON_PATH`
  - 删除估计位移相关配置
- 更新 `run_motionbert_realtime.bat`
  - 删除 `ESTIMATE_GLOBAL_MOTION` 相关配置

### 验证

- 本次为 Python 发送层回退，无需重新编译 UE 工程
- 已确认发送链路重新收敛为“只发模型原始 3D 姿态输出”

### 状态
- 完成

### 说明

- 用户明确指出：项目本身存在可输出位移的训练设置，只是当前模型没有启用，不应再使用 bbox 推导的伪位移补丁
- 因此当前 UE 调试链回到更干净的状态：
  - realtime：AlphaPose -> MotionBERT -> UE
  - offline：X3D.npy -> UE
- 若后续需要真正位移，应改为接 MotionBERT 原生支持位移的模型或配置，而不是继续估计补偿

### 下一步
- 维持当前纯姿态链用于骨架与数据流调试
- 后续单独评估 MotionBERT 中支持位移输出的配置、checkpoint 与推理入口

## 2026-05-04 23:06:48

### 任务

- 排查 MotionBERT 项目中“支持位移输出”的真实开关位置与语义
- 确认 `MB_ft_h36m_global_lite` / `rootrel: False` 是否等同于真实全局轨迹输出
- 复核 wild 推理链路是否在输入归一化阶段抹掉了大部分绝对位移信息

### 变更

- 无代码改动
- 补充仓库级结论：
  - `configs/pose3d/MB_ft_h36m_global_lite.yaml` 中的关键开关是 `rootrel: False`
  - 训练与评估逻辑位于 `train.py`
  - wild 推理逻辑位于 `infer_wild.py`
  - wild 2D 输入归一化逻辑位于 `lib/data/dataset_wild.py`
  - H36M 3D 标签读取与反归一化逻辑位于 `lib/data/datareader_h36m.py`

### 验证

- 已确认 `infer_wild.py` 默认加载：
  - `configs/pose3d/MB_ft_h36m_global_lite.yaml`
  - `checkpoint/pose3d/FT_MB_lite_MB_ft_h36m_global_lite/best_epoch.bin`
- 已确认 `rootrel: False` 时：
  - 训练阶段不会做逐帧 root-relative，而是仅将整段序列的“首帧 root 深度”对零
  - 推理阶段不会清零整段 root，只会执行 `predicted_3d_pos[:, 0, 0, 2] = 0`
- 已确认该仓库评测仍在 `train.py` 中把预测与 GT 都转回 root-relative 后再算误差，因此官方 pose 指标并不评估世界位移质量
- 已确认 `dataset_wild.py` 在 wild 链路中会把 2D 关键点做居中/缩放或 `crop_scale()`，这意味着单目视频里的绝对平移线索本来就被显著弱化
- 已对当前 `outputs/X3D.npy` 做了 root 轨迹量级检查：
  - root `x/y` 范围明显大于 `z`
  - root `z` 变化极小，更接近“相机/图像坐标下的弱深度变化”，不表现为可靠的前后位移轨迹

### 状态
- 已完成

### 说明

- 当前仓库里的 `global` 更准确地说是“非 root-relative 的 3D pose 表达”，不是“已经学出可直接拿去驱动 UE 世界位移的真实全局轨迹”
- 如果后续要得到更可靠的整体位移，不能只改 bat 或 UE 端，核心还是：
  1. 使用真正带全局轨迹监督的数据定义
  2. 使用对应训练配置与 checkpoint
  3. 必要时重新训练或至少重新 finetune

### 下一步
- 继续向上追 `joint3d_image` / `joints_2.5d_image` / `2.5d_factor` 的原始数据准备来源
- 评估是否需要单独新增“带全局轨迹监督”的训练配置，而不是复用当前 wild inference checkpoint

## 2026-05-05 11:19:12

### 任务

- 处理用户已下载的官方 H36M 预处理训练包
- 将训练数据放置到 MotionBERT 官方训练流程期望的目录
- 完成 `convert_h36m.py` 切片，生成可直接训练的 `MB3D_f243s81` 数据结构

### 变更

- 新建 `data/motion3d/`
- 解压 `h36m_sh_conf_cam_source_final.pkl.zip` 到 `data/motion3d/`
- 生成 `data/motion3d/h36m_sh_conf_cam_source_final.pkl`
- 执行 `tools/convert_h36m.py`
- 生成：
  - `data/motion3d/MB3D_f243s81/H36M-SH/train/`
  - `data/motion3d/MB3D_f243s81/H36M-SH/test/`
- 清理解压附带的 `data/motion3d/__MACOSX/`

### 验证

- 已确认原始预处理包存在：
  - `h36m_sh_conf_cam_source_final.pkl.zip`
- 已确认官方训练入口需要的主数据文件存在：
  - `data/motion3d/h36m_sh_conf_cam_source_final.pkl`
- 已成功运行 `tools/convert_h36m.py`
- 切片结果数量：
  - train: 17748
  - test: 2228

### 状态
- 已完成

### 说明

- 当前工作区已经满足官方 `docs/pose3d.md` 里的 H36M 训练数据前置条件
- 接下来如果要做 `rootrel` 开关验证或正式训练，不需要再处理数据包，只需要准备对应训练配置与启动脚本

### 下一步
- 新增最小成本的 `rootrel=False` smoke test 配置
- 或直接新增 120 轮 `rootrel=False` 的正式训练配置与 bat 入口

## 2026-05-05 11:21:59

### 任务

- 为 `rootrel=False` 位移输出开关验证新增独立的 5 轮 smoke test 训练配置
- 保证验证训练不覆盖官方现成 checkpoint

### 变更

- 新增 `configs/pose3d/MB_train_h36m_global_5ep.yaml`
  - 基于官方 `MB_train_h36m.yaml`
  - 将 `rootrel` 改为 `False`
  - 将 `epochs` 改为 `5`
- 新增 `run_train_motionbert_global_5ep.bat`
  - 独立使用 `checkpoint/pose3d/MB_train_h36m_global_5ep`
  - 不复用、不覆盖官方 `FT_MB_lite_MB_ft_h36m_global_lite`

### 验证

- 已确认新配置存在且关键参数正确：
  - `finetune: False`
  - `epochs: 5`
  - `rootrel: False`
- 已确认新 bat 指向独立 checkpoint 目录：
  - `checkpoint\pose3d\MB_train_h36m_global_5ep`
- 已确认当前官方 checkpoint 目录仍仅有：
  - `checkpoint\pose3d\FT_MB_lite_MB_ft_h36m_global_lite`

### 状态
- 已完成

### 说明

- 这套 5 轮配置的目标不是训练最终模型，而是最低成本验证：
  - `rootrel=False` 训练分支是否正常跑通
  - 数据、配置、训练入口是否闭环
  - checkpoint 是否能与官方现有结果完全隔离

### 下一步
- 运行 `run_train_motionbert_global_5ep.bat`
- 再决定是否补正式的 120 轮 `rootrel=False` 配置

## 2026-05-05 11:25:03

### 任务

- 处理 `rootrel=False` 5 轮验证训练的 CUDA OOM
- 在不改训练目标的前提下，将 smoke test 调整为更省显存的版本

### 变更

- 更新 `configs/pose3d/MB_train_h36m_global_5ep.yaml`
  - `batch_size: 32 -> 4`
- 更新 `run_train_motionbert_global_5ep.bat`
  - 增加 `PYTORCH_CUDA_ALLOC_CONF=expandable_segments:True`
  - 增加启动信息中的 batch size 提示

### 验证

- 已根据用户实际报错确认失败点为首轮训练前向阶段的 CUDA out of memory
- 已确认本次调整不影响：
  - `rootrel: False`
  - `epochs: 5`
  - 独立 checkpoint 输出目录

### 状态
- 已完成

### 说明

- 当前目的是先验证 `rootrel=False` 训练分支，而不是追求训练吞吐
- 对 16GB 档显卡来说，官方 scratch 配置的 `batch_size: 32` 对 DSTformer 过重，smoke test 先降到 `4` 更合理

### 下一步
- 重新运行 `run_train_motionbert_global_5ep.bat`
- 若仍 OOM，再继续降到 `batch_size: 2`

## 2026-05-05 13:37:13

### 任务

- 复核 `rootrel=False` 5 轮 smoke test 是否完整跑通
- 确认 checkpoint 产物是否正常生成且与官方模型隔离

### 变更

- 无代码改动
- 检查 `checkpoint/pose3d/MB_train_h36m_global_5ep/` 下的训练产物

### 验证

- 已确认以下文件存在：
  - `checkpoint/pose3d/MB_train_h36m_global_5ep/latest_epoch.bin`
  - `checkpoint/pose3d/MB_train_h36m_global_5ep/best_epoch.bin`
  - `checkpoint/pose3d/MB_train_h36m_global_5ep/epoch_4.bin`
- 已确认 checkpoint 元数据：
  - `latest_epoch.bin` -> `epoch=5`
  - `best_epoch.bin` -> `epoch=5`
  - `best_epoch.bin` 的 `min_loss` 优于 `latest_epoch.bin`
- 已确认 TensorBoard 日志文件生成于：
  - `checkpoint/pose3d/MB_train_h36m_global_5ep/logs/`

### 状态
- 已完成

### 说明

- 这次 5 轮验证已经证明：
  - 官方训练数据准备链路正常
  - `train.py` 的 scratch 训练链路正常
  - `rootrel=False` 配置可以被实际训练流程接受并跑完
  - 验证 checkpoint 与官方 wild checkpoint 保持完全隔离
- 这次 5 轮验证尚未单独证明：
  - 最终模型一定能输出你想要的可用世界位移
  - 5 轮 checkpoint 已足够用于高质量 UE 驱动

### 下一步
- 用 `best_epoch.bin` 做一次独立推理验证
- 再决定是否补 120 轮 `rootrel=False` 正式训练配置

## 2026-05-05 13:39:34

### 任务

- 为 5 轮 `rootrel=False` 验证模型补独立推理入口
- 方便将自训 checkpoint 与官方 wild checkpoint 做并行对比

### 变更

- 新增 `run_motionbert_pose_global_5ep.bat`
  - 指向 `configs/pose3d/MB_train_h36m_global_5ep.yaml`
  - 指向 `checkpoint/pose3d/MB_train_h36m_global_5ep/best_epoch.bin`
  - 独立输出目录：`outputs/global_5ep_test`

### 验证

- 已确认新 bat 不引用官方 checkpoint：
  - 不使用 `FT_MB_lite_MB_ft_h36m_global_lite`
- 已确认新 bat 使用独立输出目录，避免覆盖原有 `outputs/X3D.*`

### 状态
- 已完成

### 说明

- 这一步的目标是把“训练是否跑通”推进到“训练出的 5 轮模型和官方模型实际输出差异如何”
- 当前仍优先做低风险对比验证，不直接进入 120 轮正式训练

### 下一步
- 运行 `run_motionbert_pose_global_5ep.bat`
- 对比 `outputs/global_5ep_test/X3D.npy` 与官方模型输出差异

## 2026-05-05 13:40:29

### 任务

- 为 5 轮 `rootrel=False` 测试模型补充直连 UE 的离线 Live Link 回放入口

### 变更

- 新增 `run_motionbert_offline_livelink_global_5ep.bat`
  - 输入：
    - `outputs/global_5ep_test/X3D.npy`
  - 输出路径：
    - 直接发送到 UE Live Link `MotionBERTPose`

### 验证

- 已确认新 bat 复用现有 `replay_offline_to_ue.py`
- 已确认新 bat 默认不引用官方 `outputs/X3D.npy`
- 已确认当 `outputs/global_5ep_test/X3D.npy` 不存在时会给出明确提示

### 状态
- 已完成

### 说明

- 当前已经形成完整的低风险验证闭环：
  - `rootrel=False` 5轮训练
  - 独立推理输出 `outputs/global_5ep_test/X3D.npy`
  - 独立 UE 回放入口

### 下一步
- 先运行 `run_motionbert_pose_global_5ep.bat`
- 再运行 `run_motionbert_offline_livelink_global_5ep.bat`
- 在 UE 中直接观察该 5 轮模型的骨架表现

## 2026-05-05 13:54:05

### 任务

- 为 5 轮 `rootrel=False` 测试模型补充实时推理入口
- 支持直接走摄像头/视频源并可继续发送到 UE Live Link

### 变更

- 新增 `run_motionbert_realtime_global_5ep.bat`
  - 指向 `configs/pose3d/MB_train_h36m_global_5ep.yaml`
  - 指向 `checkpoint/pose3d/MB_train_h36m_global_5ep/best_epoch.bin`
  - 复用现有 `realtime_motionbert.py`
  - 保留 UE Live Link 参数

### 验证

- 已确认 `realtime_motionbert.py` 支持：
  - `--config`
  - `--evaluate`
- 已确认新 bat 不覆盖原有 `run_motionbert_realtime.bat`

### 状态
- 已完成

### 说明

- 现在 5 轮测试模型已经同时具备：
  - 独立离线推理入口
  - 独立离线 UE 回放入口
  - 独立实时推理入口

### 下一步
- 运行 `run_motionbert_realtime_global_5ep.bat`
- 在 UE 中观察该 5 轮模型的实时表现

## 2026-05-05 14:14:58

### 任务

- 为 realtime 入口增加面向 UE 骨架顺滑度的三挡预设

### 变更

- 更新 `run_motionbert_realtime.bat`
- 更新 `run_motionbert_realtime_global_5ep.bat`
- 新增 `UE_PRESET`：
  - `quality`
  - `smooth`
  - `fast`
- 默认预设设为：
  - `UE_PRESET=smooth`

### 验证

- 已确认两个 bat 均包含：
  - `UE_PRESET`
  - 对应 preset 参数覆盖逻辑
  - 启动日志中的 preset 输出

### 状态
- 已完成

### 说明

- 现在用户不需要再手动记住一组分辨率、显示开关、平滑系数和推理频率组合
- `smooth` 预设优先服务于“UE 里骨架别一顿一顿”的目标

### 下一步
- 先用默认 `UE_PRESET=smooth` 跑实时链路
- 如仍不够顺，再切 `fast` 或继续手调 `MOTION_SMOOTH`

## 2026-05-05 14:21:12

### 任务

- 将 realtime 链路从串行阻塞模式改为面向低延迟的最新帧异步流水线

### 变更

- 更新 `realtime_motionbert.py`
- 新增共享状态结构：
  - `SharedLatestFrame`
  - `SharedLatestResult`
- 将主链路拆为：
  - `capture_loop`
  - `process_loop`
  - 主线程显示/退出轮询
- 运行策略从“逐帧排队处理”改为“始终处理最新帧，允许丢旧帧以降低延迟”

### 验证

- 已通过 `realtime_motionbert.py --help` 验证脚本语法与参数入口正常
- 已确认现有参数仍保留：
  - `--motion_smooth`
  - `--infer_every_n`
  - `--ue_livelink_*`

### 状态
- 已完成

### 说明

- 这次改动的核心收益不是提高理论 FPS，而是降低端到端排队延迟
- 对 UE 骨架观感来说，“新鲜数据优先、旧帧允许丢弃”通常比“逐帧完整处理”更重要

### 下一步
- 用 `UE_PRESET=smooth` 先跑一轮
- 观察 UE 骨架是否从“明显落后”改善为“偶尔丢帧但更跟手”

## 2026-05-05 13:42:54

### 任务

- 修复 PyTorch 2.6+ 下自训 checkpoint 在推理入口加载失败的问题

### 变更

- 更新 `infer_wild.py`
- 更新 `infer_wild_mesh.py`
- 更新 `realtime_motionbert.py`
- 更新 `train.py`
- 对 MotionBERT 自身 checkpoint 的 `torch.load(...)` 增加：
  - `weights_only=False`

### 验证

- 已确认用户当前报错来自 `infer_wild.py` 加载：
  - `checkpoint/pose3d/MB_train_h36m_global_5ep/best_epoch.bin`
- 已确认报错根因是 PyTorch 2.6+ 默认 `weights_only=True` 与本项目 checkpoint 序列化内容不兼容
- 已确认本次修复覆盖：
  - 自训 checkpoint 推理
  - mesh 推理
  - realtime 推理
  - train.py 中 resume / evaluate / finetune 加载

### 状态
- 已完成

### 说明

- 这不是模型文件损坏，而是运行环境升级后的加载兼容问题
- 当前修复假设这些 checkpoint 来自本地受信任训练或官方仓库路径，因此允许完整反序列化

### 下一步
- 重新运行 `run_motionbert_pose_global_5ep.bat`
- 成功生成 `outputs/global_5ep_test/X3D.npy` 后再接入 UE

## 2026-05-05 14:45:13

### 任务

- 针对“实时进 UE 掉帧、时不时卡顿，而离线回放更顺”的问题继续优化实时发送链路
- 将 UE 发送层从“固定频率重复最后一帧”升级为“固定频率重采样 + 插值缓冲”
- 保留可调入口，方便后续按延迟与顺滑度做现场调参

### 变更

- 更新 `realtime_motionbert.py`
  - 新增 `--ue_interpolate`
  - 新增 `--ue_buffer_ms`
  - 将 `SharedLatestPose` 升级为 `SharedPoseBuffer`
  - 在 Live Link 发送线程中加入基于时间戳的姿态插值重采样
- 更新 `run_motionbert_realtime.bat`
  - 新增 `UE_INTERPOLATE`
  - 新增 `UE_BUFFER_MS`
  - 将两项参数接入 `quality / smooth / fast` 预设
- 更新 `run_motionbert_realtime_global_5ep.bat`
  - 同步新增 `UE_INTERPOLATE`
  - 同步新增 `UE_BUFFER_MS`
  - 同步接入三档预设

### 验证

- 已执行 `./.local/miniconda3/envs/alphapose/python.exe realtime_motionbert.py --help`
- 已确认帮助输出包含：
  - `--ue_interpolate`
  - `--ue_buffer_ms`
- 已执行 `cmd /c run_motionbert_realtime.bat`
  - 参数拼装成功
  - 当前机器未能打开 `camera:0`，因此在视频采集阶段退出
- 已执行 `cmd /c run_motionbert_realtime_global_5ep.bat`
  - 参数拼装成功
  - 当前机器未能打开 `camera:0`，因此在视频采集阶段退出
- 已执行基于本地样例视频的回归验证：
  - `SOURCE=outputs\test.mp4`
  - `UE_PRESET=fast`
  - `NO_DISPLAY=1`
  - 脚本成功完成一次 realtime 主流程运行并正常退出

### 状态

- 已完成

### 说明

- 本次不是单纯再加平滑，而是把实时 3D 的“非均匀关键帧”转换成 UE 更容易消费的“固定时钟姿态流”
- 离线回放更顺，本质上就是因为它天然是稳定节拍；这次改动是在 realtime 侧主动补这一层
- 新增的 `UE_BUFFER_MS` 是当前最关键的现场调参入口：
  - 更小：更跟手，但可能更抖
  - 更大：更顺，但会增加一点延迟

### 下一步

- 让用户直接在 UE 中复测新链路的观感差异
- 如果仍有明显卡顿，下一步优先评估 2D 检测频率与跟踪复用，而不是继续堆发送层平滑

## 2026-05-05 18:42:44

### 任务

- 按用户要求直接代为设置 realtime bat 默认参数
- 将默认入口调整为更偏向 UE 实时预览顺滑度与低阻塞的配置

### 变更

- 更新 `run_motionbert_realtime.bat`
  - 默认 `CAMERA_WIDTH=640`
  - 默认 `CAMERA_HEIGHT=360`
  - 默认 `UE_PRESET=fast`
  - 默认 `NO_DISPLAY=1`
  - 默认 `MOTION_SMOOTH=0.15`
  - 默认 `UE_BUFFER_MS=30`
- 更新 `run_motionbert_realtime_global_5ep.bat`
  - 同步以上默认值

### 验证

- 已静态确认两份 bat 的默认值均已改为 UE 实时优先配置
- 当前预设与默认值组合下，直接双击 bat 即会以：
  - 640x360
  - 无本地预览窗口
  - `UE_PRESET=fast`
  - `MOTION_SMOOTH=0.15`
  - `UE_INTERPOLATE=1`
  - `UE_BUFFER_MS=30`
  - `UE_SEND_FPS=60`
  启动

### 状态

- 已完成

### 说明

- 这次改的是“默认开机即用”的体验，不影响你后续继续手动覆盖环境变量
- 如果后面你接 Blackmagic 或更强机器，再回头单独拉高分辨率会更合适

### 下一步

- 让用户直接用当前默认 bat 复测 UE 骨架观感
- 根据现场结果再决定是否继续做 2D 检测降频与跟踪复用优化

## 2026-05-05 18:52:19

### 任务

- 按用户要求撤销上一轮对 realtime bat 默认参数的直接代设
- 恢复到修改前的默认值，不回退实时插值能力本身

### 变更

- 更新 `run_motionbert_realtime.bat`
  - 恢复 `CAMERA_WIDTH=1280`
  - 恢复 `CAMERA_HEIGHT=720`
  - 恢复 `UE_PRESET=smooth`
  - 恢复 `NO_DISPLAY=0`
  - 恢复 `MOTION_SMOOTH=0.0`
  - 恢复 `UE_BUFFER_MS=45`
- 更新 `run_motionbert_realtime_global_5ep.bat`
  - 同步恢复以上默认值

### 验证

- 已静态确认两份 bat 的默认值均已恢复到代设之前的版本
- 已保留以下功能不回退：
  - `UE_INTERPOLATE`
  - `UE_SEND_FPS`
  - `UE_BUFFER_MS` 可调入口
  - realtime 侧的固定频率发送与插值缓冲实现

### 状态

- 已完成

### 说明

- 本次仅回退默认设置，不移除前面已经实现的实时平滑发送能力
- 这样你仍然可以按需手动打开这些参数，而不是被默认值强制带着走

### 下一步

- 如用户需要，可只提供一组推荐手动设置，而不再直接改默认值

## 2026-05-05 19:05:39

### 任务

- 根据用户最新要求，不替换 MotionBERT 路线，只优化当前 realtime -> UE 的数据传输与调度
- 目标是在保留现有写法前提下，提高 UE 端帧感并压低端到端延迟

### 变更

- 更新 `lib/utils/ue_livelink.py`
  - `UnrealLiveLinkSender.send()` 默认改为发送紧凑 `positions` 包
  - 默认不再逐帧发送 `bone_names`、`bone_parents`、`transforms`
  - 默认不再主动附带发送端时间戳
- 更新 `realtime_motionbert.py`
  - Live Link 发送改为调用紧凑包路径
  - `--no_display` 且不保存视频时，跳过 2D 绘制、3D 面板渲染与拼图合成
- 更新 `replay_offline_to_ue.py`
  - 离线回放同样改为紧凑包发送路径
- 更新 `unreal/MotionBERT_UE/Plugins/MotionBERTLiveLink/Source/MotionBERTLiveLink/Private/MotionBERTLiveLinkSource.cpp`
  - UDP 接收线程从“处理全部积压包”改为“每轮只消费最新包”
  - Socket 等待超时从 `10ms` 降到 `1ms`
  - 继续复用现有 `positions` 解析路径与默认骨架补全逻辑

### 验证

- 已执行 `./.local/miniconda3/envs/alphapose/python.exe realtime_motionbert.py --help`
- 已执行 `./.local/miniconda3/envs/alphapose/python.exe replay_offline_to_ue.py --help`
- 已计算紧凑化前后单包体积：
  - 旧包约 `1699 bytes`
  - 新包约 `302 bytes`
- 已执行 `replay_offline_to_ue.py --npy_path outputs\X3D.npy --start_frame 0 --end_frame 2 --fps 30`
  - 脚本正常启动并完成短回放
- 已使用 UE 5.6 编译项目：
  - `Build.bat MotionBERT_UEEditor Win64 Development ...`
  - 构建成功

### 状态

- 已完成

### 说明

- 这一轮是严格围绕“传输与调度层”做的，不涉及替换 MotionBERT 模型
- 当前最关键的两项低延迟策略是：
  - Python 侧发更小、更轻的包
  - UE 侧永远优先最新包，不再把积压历史包补放出来
- 这两项更符合单路 mocap 预览场景，也更贴近 Unreal Live Link 官方推荐的低延迟使用思路

### 下一步

- 让用户在 UE 中复测当前链路
- 如仍明显偏慢，下一步优先继续做“2D 阶段与 3D 阶段分线程/分缓冲”，而不是替换模型

## 2026-05-05 19:08:57

### 任务

- 将用户新增的长期偏好固化到项目 agent 规则中
- 明确后续优化默认走“效果优先、资源可尽量使用、先查成熟方案”路线

### 变更

- 更新 `Agents/AGENTS.md`
- 新增 `Performance Priority Rule`
- 新增 `Mature Solution First Rule`

### 验证

- 已确认规则覆盖：
  - 资源使用优先级
  - 端到端实时链路的性能优化范围
  - 先查官方/成熟插件/成熟项目的约束

### 状态

- 已完成

### 说明

- 这次不是功能改动，而是把用户的工程偏好写入项目执行规范
- 后续只要用户没有改口，默认将优先追求：
  - 质量
  - 稳定性
  - 低延迟
  - 高帧感
  而不是先省资源

### 下一步

- 后续所有实时链路优化默认按该偏好执行

## 2026-05-05 19:12:37

### 任务

- 修正上一轮传输压缩方案导致的姿态对不上问题
- 在不放弃低延迟方向的前提下，恢复与 UE 骨架语义一致的数据格式

### 变更

- 更新 `lib/utils/ue_livelink.py`
  - 新增 `joints_to_livelink_local_positions()`
  - 将紧凑 `positions` 包从“全局坐标”改为“局部骨骼位移”
  - 保持紧凑发送路径不变，但修正语义以匹配 UE Live Link 动画骨架消费方式

### 验证

- 已执行 Python 自检，确认：
  - `positions` 与原 `transforms.translation` 一一对应
  - 语义已恢复一致
- 已测得当前包体积：
  - 全量 `transforms` 包约 `2447 bytes`
  - 紧凑 `positions` 包约 `1034 bytes`
- 已执行 `replay_offline_to_ue.py --npy_path outputs\X3D.npy --start_frame 0 --end_frame 2 --fps 30`
  - 脚本正常启动

### 状态

- 已完成

### 说明

- 上一轮问题根因是：为压包体改用了 `positions`，但发送成了全局坐标，而 UE 当前这条骨架链路实际需要局部骨骼位移
- 本次修正后，继续保留：
  - 最新包优先
  - 跳过无意义可视化
  - 紧凑发送
  但不再破坏骨架语义

### 下一步

- 让用户在 UE 中复测当前姿态是否重新与捕捉一致
- 如果一致，再继续推进更激进的并行调度优化

## 2026-05-05 19:20:46

### 任务

- 在保持 MotionBERT 路线与姿态语义不变的前提下，继续压低实时链路延迟并提升帧感
- 通过更激进的并行调度，尽量让 2D、3D 与 UE 发送重叠执行

### 变更

- 更新 `realtime_motionbert.py`
- 将原单一 `process_loop` 拆分为：
  - `pose2d_loop`
  - `pose3d_loop`
  - `livelink_loop`
- 新增 `SharedLatest2D`
  - 传递最新 2D 结果、对应帧索引与可视化左屏
- 保持 `SharedPoseBuffer`
  - 继续服务于 UE 发送插值层
- 更新 `make_capture()`
  - 对摄像头/视频源尝试设置 `cv2.CAP_PROP_BUFFERSIZE=1`
- 保持 `--no_display` 下跳过无意义可视化生成的优化

### 验证

- 已执行 `./.local/miniconda3/envs/alphapose/python.exe realtime_motionbert.py --help`
- 已执行基于本地样例视频的回归验证：
  - `SOURCE=outputs\test.mp4`
  - `UE_PRESET=fast`
  - `NO_DISPLAY=1`
- 脚本成功完成一次 realtime 主流程运行并正常退出

### 状态

- 已完成

### 说明

- 本次核心变化不是调参数，而是把原本串行的：
  - AlphaPose
  - MotionBERT
  - UE 发送
  改成了可重叠的流水线结构
- 这更符合用户“宁可多吃资源，也优先要更低延迟和更高帧感”的偏好
- 当前这版仍未替换模型，仅优化调度层

### 下一步

- 让用户实测当前版的体感变化
- 若仍需更快，下一步优先评估更短实时窗口与更激进的 inference cadence 策略

## 2026-05-05 19:26:15

### 任务

- 按用户反馈回退到上一版更快的 realtime 主入口
- 为 realtime 链路补上版本管理，便于后续自行回退和对比

### 变更

- 更新 `realtime_motionbert.py`
  - 回退到上一版稳定实现
  - 撤销本轮较慢的 `pose2d_loop + pose3d_loop` 流水线主入口改动
- 新增 `versions/realtime/realtime_motionbert_pipeline_experimental.py`
  - 封存较慢的流水线实验版
- 新增 `versions/realtime/realtime_motionbert_transport_stable.py`
  - 封存当前稳定版快照
- 新增 `run_motionbert_realtime_pipeline_experimental.bat`
  - 可直接切换运行实验版
- 新增 `versions/realtime/README.md`
  - 记录当前稳定版与实验版的入口、状态与用途

### 验证

- 已执行 `./.local/miniconda3/envs/alphapose/python.exe realtime_motionbert.py --help`
- 已执行实验版回归：
  - `SOURCE=outputs\test.mp4`
  - `UE_PRESET=fast`
  - `NO_DISPLAY=1`
  - `run_motionbert_realtime_pipeline_experimental.bat`
- 实验版在修正根目录解析后可正常完成一次样例视频主流程运行

### 状态

- 已完成

### 说明

- 当前默认入口重新回到用户反馈更快的稳定版
- 本次没有丢弃流水线实验，只是把它移出主入口，改为可单独调用的实验版本
- 后续如再做更激进实验，会继续按该版本管理方式保留稳定版与实验版并行入口

### 下一步

- 用户直接继续使用 `run_motionbert_realtime.bat` 作为默认稳定入口
- 如需复测实验版，使用 `run_motionbert_realtime_pipeline_experimental.bat`

## 2026-05-05 19:40:53

### 任务

- 按用户要求输出当前机器上的 realtime 性能天花板判断报告
- 将当前瓶颈、优化边界与后续建议沉淀为独立文档

### 变更

- 新增 `docs/realtime_perf_ceiling_assessment.md`

### 验证

- 已确认文档覆盖：
  - 当前资源占用解读
  - 主要瓶颈判断
  - 当前稳定版包含的有效优化
  - 后续优化收益预期
  - 是否继续投入的建议结论

### 状态

- 已完成

### 说明

- 该文档的目标不是使用说明，而是帮助后续做技术路线和投入产出判断
- 当前结论明确偏向：
  - 继续小修有空间
  - 但大收益更可能来自路线级调整

### 下一步

- 如用户继续推进，可基于该报告选择“低延迟参数实验”或“路线级替代方案调研”两条线
## 2026-05-06 11:21:01

### 任务

- 为另一台全新 Windows + RTX 5090 机器补齐 MotionBERT 项目的一键部署方案
- 在现有“本机已跑通”的基础上，把环境安装、外部依赖、权重与基础校验沉淀成可直接双击执行的部署入口

### 变更

- 新增 `deploy_windows_5090.bat`
- 新增 `scripts/deploy_windows_5090.ps1`
- 新增 `deploy/requirements-motionbert-5090.txt`
- 新增 `deploy/requirements-alphapose-5090.txt`
- 新增 `docs/deploy_windows_5090.md`

### 验证

- 已重新审阅当前项目启动规范与现有运行入口：
  - `run_alphapose.bat`
  - `run_motionbert_pose.bat`
  - `run_motionbert_realtime.bat`
- 已基于当前工作机实际环境导出并固化部署目标版本：
  - Python `3.10.20`
  - PyTorch `2.11.0+cu128`
  - torchvision `0.26.0+cu128`
  - torchaudio `2.11.0+cu128`
- 已通过官方资料确认：
  - PyTorch 官方 Windows 安装页存在 cu128 安装通路
  - Miniconda 官方支持 Windows 静默安装
  - MotionBERT 官方文档提供 checkpoint 下载入口
  - AlphaPose 官方文档提供 YOLOX detector 权重来源

### 状态
- 已完成

### 说明

- 这次不是单纯补文档，而是把“复制仓库到新机后如何从零安装”落成了可执行脚本
- 当前脚本优先复用仓库里已经存在的 `.external/AlphaPose`、`checkpoint` 和本地权重；缺失时再自动补下载
- `halpe26_fast_res50_256x192.pth` 目前仍以“优先复用现有工作机文件”为主，因为这部分在当前仓库链路里本来就已经是本地资产

### 下一步?
- 在目标 5090 机器上实际执行 `deploy_windows_5090.bat`
- 若该机器也需要离线包，可进一步补一版“打包当前工作机资产并搬运”的便携部署方案
## 2026-05-06 12:17:03

### 任务

- 按用户最新边界，把“离线搬家”收敛为“只搬工作区，不搬 conda 环境”
- 提供一个可直接在当前机器打包 workspace 的脚本，并让目标机器继续使用本地部署脚本安装环境

### 变更

- 新增 `create_workspace_bundle.bat`
- 新增 `scripts/create_workspace_bundle.ps1`
- 新增 `docs/workspace_bundle.md`

### 验证

- 已静态校验 `scripts/create_workspace_bundle.ps1` PowerShell 语法通过
- 已实际执行：
  - `powershell -File .\scripts\create_workspace_bundle.ps1 -RepoRoot . -BundleMode lean -IncludeOutputs 0 -BundleName MotionBERT_workspace_lean_v2`
- 已成功生成：
  - `dist\workspace_bundle\MotionBERT_workspace_lean_v2.zip`

### 状态
- 已完成

### 说明

- 这条路线明确不搬 `.local/miniconda3`
- 工作区包默认保留源码、配置、AlphaPose 外部目录、checkpoint、Unreal 工程等核心内容
- 目标机器拿到工作区后，继续运行 `deploy_windows_5090.bat` 即可在本机重建环境

### 下一步?
- 把 `dist\workspace_bundle\MotionBERT_workspace_lean_v2.zip` 复制到目标机器
- 解压后直接运行 `deploy_windows_5090.bat`
## 2026-05-06 12:21:52

### 任务

- 清理工作区搬家包测试阶段遗留的旧 staging 目录

### 变更

- 删除 `dist\workspace_bundle\MotionBERT_workspace_lean_test_stage`
- 删除临时空目录 `dist\workspace_bundle\__empty__`

### 验证

- 已确认 `dist\workspace_bundle` 当前仅保留：
  - `MotionBERT_workspace_lean_v2_stage`
  - `MotionBERT_workspace_lean_v2.zip`

### 状态
- 已完成

### 下一步?
- 直接使用 `MotionBERT_workspace_lean_v2.zip` 搬到目标机器
## 2026-05-06 12:25:03

### 任务

- 删除当前正式工作区搬家包对应的 staging 目录，仅保留最终 zip 成品

### 变更

- 删除 `dist\workspace_bundle\MotionBERT_workspace_lean_v2_stage`
- 删除临时空目录 `dist\workspace_bundle\__empty2__`

### 验证

- 已确认 `dist\workspace_bundle` 当前仅保留：
  - `MotionBERT_workspace_lean_v2.zip`

### 状态
- 已完成

### 下一步?
- 直接复制 `MotionBERT_workspace_lean_v2.zip` 到目标机器
## 2026-05-06 13:31:58

### 任务

- 修复 `deploy_windows_5090.bat` 启动时报“路径具有非法字符”的问题
- 同步修正同类路径传参隐患到 `create_workspace_bundle.bat`

### 变更

- 更新 `deploy_windows_5090.bat`
- 更新 `create_workspace_bundle.bat`
- 更新 `scripts/deploy_windows_5090.ps1`

### 验证

- 已本地复现原始报错：`Resolve-Path : Illegal characters in path.`
- 已确认根因是 batch 传给 PowerShell 的仓库路径末尾带反斜杠，导致闭合引号被转义污染
- 已重新执行 `cmd /c deploy_windows_5090.bat`，确认脚本可正常进入部署流程，不再在启动阶段报非法路径

### 状态
- 已完成

### 下一步?
- 在目标机器重新运行 `deploy_windows_5090.bat`
- 若后续还有网络或权限层面的安装报错，再按新报错继续处理
## 2026-05-06 14:04:44

### 任务

- 修复目标机器上 `deploy_windows_5090.bat` 被 Conda Terms of Service 阻塞环境创建的问题

### 变更

- 更新 `scripts/deploy_windows_5090.ps1`
- 更新 `docs/deploy_windows_5090.md`

### 验证

- 已根据目标机器返回的真实报错定位为：
  - `CondaToSNonInteractiveError`
  - 默认 Anaconda 渠道未接受 Terms of Service
- 已在部署脚本中补充自动执行：
  - `conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/main`
  - `conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/r`
  - `conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/msys2`

### 状态
- 已完成

### 下一步?
- 在目标机器重新运行 `deploy_windows_5090.bat`
- 继续根据新一轮输出处理后续依赖或网络问题
## 2026-05-06 14:29:53

### 任务

- 按用户要求调整部署入口，默认直接显示安装进度，避免只能通过日志观察下载与安装过程

### 变更

- 更新 `deploy_windows_5090.bat`
- 更新 `docs/deploy_windows_5090.md`

### 验证

- `deploy_windows_5090.bat` 现改为默认使用 PowerShell `-Command` 路线直接输出实时日志
- 新增 `LOG_TO_FILE=1` 可选模式，启用后写入 `deploy_run.log`
- 默认模式下不再把控制台输出重定向到日志文件，因此可以直接看到 pip / conda 的实时输出

### 状态
- 已完成

### 下一步?
- 在目标机器直接重新运行 `deploy_windows_5090.bat`
- 如需保留文件日志，再手动设置 `LOG_TO_FILE=1`
## 2026-05-06 15:29:10

### 任务

- 修复目标机器在安装 PyTorch cu128 wheel 时因大文件下载中断导致部署失败的问题

### 变更

- 更新 `scripts/deploy_windows_5090.ps1`
- 更新 `docs/deploy_windows_5090.md`

### 验证

- 已根据目标机器真实错误定位为：
  - `pip._vendor.urllib3.exceptions.ProtocolError`
  - `IncompleteRead`
  - 发生在 `torch-2.11.0+cu128` 约 2.7GB wheel 在线下载阶段
- 已将部署策略改为：
  - 先下载 PyTorch wheel 到 `.cache\deploy\torch-cu128`
  - 优先使用 `curl.exe` 断点续传
  - 两个 conda 环境复用同一批本地 wheel 离线安装

### 状态
- 已完成

### 下一步?
- 将新版 `scripts\deploy_windows_5090.ps1` 同步到目标机器
- 重新执行部署，优先走本地缓存 wheel 的安装路径
## 2026-05-06 15:46:20

### 任务

- 修复目标机器上 PyTorch wheel 缓存已损坏但被脚本误判为可复用的问题

### 变更

- 更新 `scripts/deploy_windows_5090.ps1`

### 验证

- 已根据目标机器真实错误定位为：
  - `Wheel 'torch' ... is invalid`
- 已在脚本中新增：
  - wheel zip 完整性校验
  - 对损坏缓存 wheel 的自动删除与重新下载
- 当前建议目标机器立即执行：
  - 删除 `.cache\deploy\torch-cu128\*.whl`
  - 再重新运行部署

### 状态
- 已完成

### 下一步?
- 将新版 `scripts\deploy_windows_5090.ps1` 覆盖到目标机器
- 清空损坏的 wheel 缓存后重新运行部署
## 2026-05-06 16:25:04

### 任务

- 修复目标机器上 `chumpy` 在 MotionBERT 依赖安装阶段构建失败的问题

### 变更

- 更新 `deploy/requirements-motionbert-5090.txt`
- 更新 `scripts/deploy_windows_5090.ps1`

### 验证

- 已根据目标机器真实错误定位为：
  - `Failed to build 'chumpy' when getting requirements to build wheel`
  - 构建隔离环境里 `ModuleNotFoundError: No module named 'pip'`
- 已调整部署策略：
  - 从主 requirements 中移除 `chumpy`
  - 改为单独执行 `pip install --no-build-isolation chumpy==0.70`

### 状态
- 已完成

### 下一步?
- 将新版 `deploy/requirements-motionbert-5090.txt` 与 `scripts/deploy_windows_5090.ps1` 同步到目标机器
- 重新执行 MotionBERT 依赖安装步骤
## 2026-05-08 10:20:00

### 任务

- 为本地 RTX 5070 Ti 机器补充一套独立的一键部署入口，并明确源码与环境分离的版本管理方式

### 变更

- 新增 `deploy_windows_5070ti.bat`
- 新增 `scripts/deploy_windows_5070ti.ps1`
- 新增 `docs/deploy_windows_5070ti.md`

### 验证

- 复用当前已跑通的 Windows 部署逻辑：
  - 仓库内 `.local/miniconda3`
  - `motionbert` / `alphapose` 双环境
  - PyTorch `2.11.0+cu128`
  - 依赖安装、模型资产检查、smoke check
- 5070 Ti 入口仅新增独立包装与文档，不改动已验证的 5090 部署文件

### 状态

- 已完成

### 下一步

- 如需进一步收敛版本管理，可继续补 `.gitignore` 与“哪些目录不进 git”的正式清单

## 2026-05-08 10:42:00

### 任务

- 为仓库设计大文件资源分层方案，明确哪些走 git、哪些走部署下载、哪些保留手动补充

### 变更

- 新增 `docs/assets_storage_strategy.md`
- 新增 `deploy/assets_manifest.template.json`

### 验证

- 已将当前 MotionBERT 项目拆分为三层：
  - git 源码层
  - deploy 自动下载层
  - manual-only 手动资源层
- 已结合当前仓库给出分类建议：
  - 官方 checkpoint / YOLOX 权重适合自动下载
  - 自训 checkpoint 适合私有直链下载
  - 受限数据集保留手动补充

### 状态

- 已完成

### 下一步

- 可继续把 `assets_manifest` 接入 `deploy_windows_5070ti.ps1` / `deploy_windows_5090.ps1`，实现按清单自动补全资源

## 2026-05-08 22:35:00

### 任务

- 将 Windows 一键部署脚本中的模型下载来源切换为优先从 `EMOCJC/motionbert_models` 拉取

### 变更

- 更新 `scripts/deploy_windows_5070ti.ps1`
- 更新 `scripts/deploy_windows_5090.ps1`

### 验证

- `FT_MB_lite_MB_ft_h36m_global_lite/best_epoch.bin` 已切换为从用户 Hugging Face models 仓库下载
- `halpe26_fast_res50_256x192.pth` 已切换为从用户 Hugging Face models 仓库下载
- `yolox_x.pth` 已切换为从用户 Hugging Face models 仓库下载
- 未改动其余 deploy 流程、requirements、环境创建与 smoke check 逻辑

### 状态

- 已完成

## 2026-05-08 22:52:00

### 任务

- 将仓库根 README 从官方 MotionBERT 论文仓口径更新为当前工程化项目说明

### 变更

- 重写 `README.md`

### 验证

- README 已改为描述当前仓库真实用途：
  - Windows 一键部署
  - Unreal Live Link 集成
  - realtime / offline / training 入口
  - Hugging Face 模型仓策略
  - 数据与环境分层管理
- 保留了对官方 MotionBERT 论文与上游仓库的致谢说明

### 状态

- 已完成
