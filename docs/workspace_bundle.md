# MotionBERT 工作区搬家包

如果你不想搬 `.local/miniconda3`，只想把当前工作区内容打包带到另一台机器，这一套就是给这个场景准备的。

入口：

- [create_workspace_bundle.bat](</d:/Git/Github/MotionBERT/create_workspace_bundle.bat>)

## 适用场景

- 你已经在当前机器把仓库整理好了
- 想把“工作区内容”搬到另一台电脑
- 但不想把 conda / python 环境一起复制过去
- 到目标机器后，再现场执行：
  - [deploy_windows_5090.bat](</d:/Git/Github/MotionBERT/deploy_windows_5090.bat>)

## 默认行为

直接运行：

```cmd
cd /d D:\Git\Github\MotionBERT
create_workspace_bundle.bat
```

默认会生成一个 `lean` 搬家包，输出到：

`dist\workspace_bundle\`

默认排除这些目录：

- `.local`
- `.cache`
- `.git`
- `alpha_tmp`
- `__pycache__`
- `dist`
- `alpha_cache`
- `outputs`

也就是说，默认包更像是“可部署工作区”，不是“整机镜像”。

## 会保留什么

默认仍然会保留这些关键内容：

- `.external/AlphaPose`
- `checkpoint`
- `configs`
- `data`
- `docs`
- `lib`
- `scripts`
- `deploy`
- `Unreal`
- 各种 `.bat` / `.py` 入口

## 如果你想把 outputs 也带上

在 `cmd` 里这样跑：

```cmd
cd /d D:\Git\Github\MotionBERT
set INCLUDE_OUTPUTS=1
create_workspace_bundle.bat
```

## 如果你想做更完整的包

可以切到 `full`：

```cmd
cd /d D:\Git\Github\MotionBERT
set BUNDLE_MODE=full
create_workspace_bundle.bat
```

`full` 模式下不会排除 `alpha_cache`；`outputs` 是否打包仍然由 `INCLUDE_OUTPUTS` 控制。

## 目标机器怎么用

1. 把 zip 拷到目标机器
2. 解压成工作区
3. 进入仓库根目录
4. 运行：

```cmd
deploy_windows_5090.bat
```

脚本会在目标机器本地重新安装 `.local/miniconda3` 和两个 conda 环境。

## 这条路线的好处

1. 包体比“连 conda 一起搬”小很多
2. 不依赖目标机器和源机器的 Python 环境路径一致
3. 新机器环境还是本地现装，兼容性更稳
