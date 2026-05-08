param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"
[Reflection.Assembly]::LoadWithPartialName("System.IO.Compression.FileSystem") | Out-Null
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "[STEP] $Message" -ForegroundColor Cyan
}

function Ensure-Directory {
    param([string]$Path)
    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Force -Path $Path | Out-Null
    }
}

function Invoke-Checked {
    param(
        [string]$FilePath,
        [string[]]$Arguments
    )

    Write-Host ("[RUN] {0} {1}" -f $FilePath, ($Arguments -join " "))
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code ${LASTEXITCODE}: $FilePath"
    }
}

function Download-File {
    param(
        [string]$Url,
        [string]$Destination
    )

    Ensure-Directory ([System.IO.Path]::GetDirectoryName($Destination))
    Write-Host "[DOWNLOAD] $Url"
    Invoke-WebRequest -Uri $Url -OutFile $Destination
}

function Download-FileResumable {
    param(
        [string]$Url,
        [string]$Destination
    )

    Ensure-Directory ([System.IO.Path]::GetDirectoryName($Destination))

    $curl = Get-Command curl.exe -ErrorAction SilentlyContinue
    if ($curl) {
        Write-Host "[DOWNLOAD] $Url"
        Invoke-Checked -FilePath $curl.Source -Arguments @(
            "-L",
            "-C", "-",
            "--retry", "20",
            "--retry-delay", "5",
            "--output", $Destination,
            $Url
        )
        return
    }

    Download-File -Url $Url -Destination $Destination
}

function Test-WheelFile {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return $false
    }

    try {
        $zip = [System.IO.Compression.ZipFile]::OpenRead($Path)
        $entryCount = $zip.Entries.Count
        $zip.Dispose()
        return ($entryCount -gt 0)
    } catch {
        return $false
    }
}

function Ensure-Miniconda {
    param(
        [string]$InstallRoot,
        [string]$CacheDir
    )

    $pythonExe = Join-Path $InstallRoot "python.exe"
    if (Test-Path -LiteralPath $pythonExe) {
        Write-Host "[OK] Miniconda already present: $InstallRoot"
        return
    }

    Ensure-Directory $CacheDir
    $installer = Join-Path $CacheDir "Miniconda3-latest-Windows-x86_64.exe"
    if (-not (Test-Path -LiteralPath $installer)) {
        Download-File -Url "https://repo.anaconda.com/miniconda/Miniconda3-latest-Windows-x86_64.exe" -Destination $installer
    }

    Write-Host "[INFO] Installing Miniconda to $InstallRoot"
    Start-Process -FilePath $installer -ArgumentList @(
        "/InstallationType=JustMe",
        "/RegisterPython=0",
        "/S",
        "/D=$InstallRoot"
    ) -Wait -NoNewWindow

    if (-not (Test-Path -LiteralPath $pythonExe)) {
        throw "Miniconda installation did not produce $pythonExe"
    }
}

function Ensure-AlphaPoseCheckout {
    param(
        [string]$AlphaPoseDir,
        [string]$CacheDir
    )

    $demoScript = Join-Path $AlphaPoseDir "scripts\demo_inference.py"
    if (Test-Path -LiteralPath $demoScript) {
        Write-Host "[OK] AlphaPose checkout found: $AlphaPoseDir"
        return
    }

    Ensure-Directory $CacheDir
    $zipPath = Join-Path $CacheDir "AlphaPose-master.zip"
    $extractRoot = Join-Path $CacheDir "AlphaPose-extract"

    Download-File -Url "https://codeload.github.com/MVIG-SJTU/AlphaPose/zip/refs/heads/master" -Destination $zipPath

    if (Test-Path -LiteralPath $extractRoot) {
        Remove-Item -LiteralPath $extractRoot -Recurse -Force
    }
    Expand-Archive -LiteralPath $zipPath -DestinationPath $extractRoot -Force

    $extracted = Join-Path $extractRoot "AlphaPose-master"
    if (-not (Test-Path -LiteralPath $extracted)) {
        throw "Unexpected AlphaPose archive layout."
    }

    Ensure-Directory ([System.IO.Path]::GetDirectoryName($AlphaPoseDir))
    Move-Item -LiteralPath $extracted -Destination $AlphaPoseDir
}

function Ensure-CondaEnv {
    param(
        [string]$CondaExe,
        [string]$EnvPath
    )

    $envPython = Join-Path $EnvPath "python.exe"
    if (Test-Path -LiteralPath $envPython) {
        Write-Host "[OK] Conda env already present: $EnvPath"
        return
    }

    Invoke-Checked -FilePath $CondaExe -Arguments @("create", "--prefix", $EnvPath, "--yes", "python=3.10.20", "pip")
}

function Accept-CondaTos {
    param([string]$CondaExe)

    $channels = @(
        "https://repo.anaconda.com/pkgs/main",
        "https://repo.anaconda.com/pkgs/r",
        "https://repo.anaconda.com/pkgs/msys2"
    )

    foreach ($channel in $channels) {
        Invoke-Checked -FilePath $CondaExe -Arguments @(
            "tos", "accept",
            "--override-channels",
            "--channel", $channel
        )
    }
}

function Invoke-CondaRun {
    param(
        [string]$CondaExe,
        [string]$EnvPath,
        [string[]]$Command
    )

    Invoke-Checked -FilePath $CondaExe -Arguments (@("run", "--prefix", $EnvPath) + $Command)
}

function Ensure-TorchWheels {
    param([string]$WheelCacheDir)

    Ensure-Directory $WheelCacheDir

    $wheelMap = [ordered]@{
        "torch-2.11.0+cu128-cp310-cp310-win_amd64.whl" = "https://download.pytorch.org/whl/cu128/torch-2.11.0%2Bcu128-cp310-cp310-win_amd64.whl"
        "torchvision-0.26.0+cu128-cp310-cp310-win_amd64.whl" = "https://download.pytorch.org/whl/cu128/torchvision-0.26.0%2Bcu128-cp310-cp310-win_amd64.whl"
        "torchaudio-2.11.0+cu128-cp310-cp310-win_amd64.whl" = "https://download.pytorch.org/whl/cu128/torchaudio-2.11.0%2Bcu128-cp310-cp310-win_amd64.whl"
    }

    $wheelPaths = @()
    foreach ($name in $wheelMap.Keys) {
        $path = Join-Path $WheelCacheDir $name
        if (Test-Path -LiteralPath $path) {
            if (Test-WheelFile -Path $path) {
                Write-Host "[OK] Cached wheel present: $path"
            } else {
                Write-Warning "Cached wheel is invalid and will be re-downloaded: $path"
                Remove-Item -LiteralPath $path -Force
                Download-FileResumable -Url $wheelMap[$name] -Destination $path
            }
        } else {
            Download-FileResumable -Url $wheelMap[$name] -Destination $path
        }

        if (-not (Test-WheelFile -Path $path)) {
            throw "Downloaded wheel is still invalid: $path"
        }

        $wheelPaths += $path
    }

    return ,$wheelPaths
}

function Install-TorchStack {
    param(
        [string]$CondaExe,
        [string]$EnvPath,
        [string[]]$WheelPaths
    )

    Invoke-CondaRun -CondaExe $CondaExe -EnvPath $EnvPath -Command @("python", "-m", "pip", "install", "--upgrade", "pip", "setuptools", "wheel")
    Invoke-CondaRun -CondaExe $CondaExe -EnvPath $EnvPath -Command (@(
        "python", "-m", "pip", "install"
    ) + $WheelPaths)
}

function Install-RequirementsFile {
    param(
        [string]$CondaExe,
        [string]$EnvPath,
        [string]$RequirementsPath
    )

    Invoke-CondaRun -CondaExe $CondaExe -EnvPath $EnvPath -Command @("python", "-m", "pip", "install", "-r", $RequirementsPath)
}

function Install-MotionBertExtras {
    param(
        [string]$CondaExe,
        [string]$EnvPath
    )

    Invoke-CondaRun -CondaExe $CondaExe -EnvPath $EnvPath -Command @(
        "python", "-m", "pip", "install",
        "--no-build-isolation",
        "chumpy==0.70"
    )
}

function Install-AlphaPoseExtras {
    param(
        [string]$CondaExe,
        [string]$EnvPath
    )

    Invoke-CondaRun -CondaExe $CondaExe -EnvPath $EnvPath -Command @(
        "python", "-m", "pip", "install",
        "--no-build-isolation",
        "visdom==0.2.4"
    )
}

function Ensure-Checkpoint {
    param(
        [string]$Path,
        [string]$Url
    )

    if (Test-Path -LiteralPath $Path) {
        Write-Host "[OK] Asset present: $Path"
        return
    }

    if ([string]::IsNullOrWhiteSpace($Url)) {
        Write-Warning "Missing asset: $Path"
        Write-Warning "Please copy it from the working machine, then rerun deploy."
        return
    }

    Download-File -Url $Url -Destination $Path
}

function Verify-Env {
    param(
        [string]$CondaExe,
        [string]$EnvPath,
        [string[]]$Command
    )

    Invoke-CondaRun -CondaExe $CondaExe -EnvPath $EnvPath -Command $Command
}

$RepoRoot = $RepoRoot.Trim().Trim('"')
$RepoRoot = (Resolve-Path $RepoRoot).Path.TrimEnd("\")
$LocalRoot = Join-Path $RepoRoot ".local"
$CacheRoot = Join-Path $RepoRoot "alpha_cache"
$DownloadCache = Join-Path $RepoRoot ".cache\deploy"
$WheelCacheDir = Join-Path $DownloadCache "torch-cu128"
$MinicondaRoot = Join-Path $LocalRoot "miniconda3"
$CondaExe = Join-Path $MinicondaRoot "Scripts\conda.exe"
$AlphaPoseDir = Join-Path $RepoRoot ".external\AlphaPose"
$MotionEnv = Join-Path $MinicondaRoot "envs\motionbert"
$AlphaEnv = Join-Path $MinicondaRoot "envs\alphapose"

$MotionReq = Join-Path $RepoRoot "deploy\requirements-motionbert-5090.txt"
$AlphaReq = Join-Path $RepoRoot "deploy\requirements-alphapose-5090.txt"

$MotionCkpt = Join-Path $RepoRoot "checkpoint\pose3d\FT_MB_lite_MB_ft_h36m_global_lite\best_epoch.bin"
$MotionCkptUrl = "https://huggingface.co/EMOCJC/motionbert_models/resolve/main/checkpoint/pose3d/FT_MB_lite_MB_ft_h36m_global_lite/best_epoch.bin"
$AlphaPoseCkpt = Join-Path $AlphaPoseDir "pretrained_models\halpe26_fast_res50_256x192.pth"
$AlphaPoseCkptUrl = "https://huggingface.co/EMOCJC/motionbert_models/resolve/main/alphapose/pretrained_models/halpe26_fast_res50_256x192.pth"
$YoloXCkpt = Join-Path $AlphaPoseDir "detector\yolox\data\yolox_x.pth"
$YoloXCkptUrl = "https://huggingface.co/EMOCJC/motionbert_models/resolve/main/alphapose/detector/yolox/data/yolox_x.pth"

$env:MPLCONFIGDIR = Join-Path $CacheRoot "matplotlib"
$env:TORCH_HOME = Join-Path $CacheRoot "torch"
$env:PYTHONPATH = "$AlphaPoseDir;$RepoRoot"

Write-Host "[INFO] Repo root: $RepoRoot"
Write-Host "[INFO] Target stack: Python 3.10 + PyTorch 2.11.0 cu128 + AlphaPose + MotionBERT"

Write-Step "Prepare local folders"
Ensure-Directory $LocalRoot
Ensure-Directory $CacheRoot
Ensure-Directory (Join-Path $CacheRoot "matplotlib")
Ensure-Directory (Join-Path $CacheRoot "torch\hub\checkpoints")
Ensure-Directory $DownloadCache

Write-Step "Install Miniconda if needed"
Ensure-Miniconda -InstallRoot $MinicondaRoot -CacheDir $DownloadCache

Write-Step "Accept Conda channel Terms of Service"
Accept-CondaTos -CondaExe $CondaExe

Write-Step "Ensure AlphaPose checkout"
Ensure-AlphaPoseCheckout -AlphaPoseDir $AlphaPoseDir -CacheDir $DownloadCache

Write-Step "Create conda environments"
Ensure-CondaEnv -CondaExe $CondaExe -EnvPath $MotionEnv
Ensure-CondaEnv -CondaExe $CondaExe -EnvPath $AlphaEnv

Write-Step "Download cached PyTorch wheels"
$TorchWheels = Ensure-TorchWheels -WheelCacheDir $WheelCacheDir

Write-Step "Install MotionBERT environment"
Install-TorchStack -CondaExe $CondaExe -EnvPath $MotionEnv -WheelPaths $TorchWheels
Install-RequirementsFile -CondaExe $CondaExe -EnvPath $MotionEnv -RequirementsPath $MotionReq
Install-MotionBertExtras -CondaExe $CondaExe -EnvPath $MotionEnv

Write-Step "Install AlphaPose environment"
Install-TorchStack -CondaExe $CondaExe -EnvPath $AlphaEnv -WheelPaths $TorchWheels
Install-RequirementsFile -CondaExe $CondaExe -EnvPath $AlphaEnv -RequirementsPath $AlphaReq
Install-AlphaPoseExtras -CondaExe $CondaExe -EnvPath $AlphaEnv

Write-Step "Ensure key model assets"
Ensure-Checkpoint -Path $MotionCkpt -Url $MotionCkptUrl
Ensure-Checkpoint -Path $YoloXCkpt -Url $YoloXCkptUrl
Ensure-Checkpoint -Path $AlphaPoseCkpt -Url $AlphaPoseCkptUrl

Write-Step "Smoke checks"
Verify-Env -CondaExe $CondaExe -EnvPath $MotionEnv -Command @("python", "--version")
Verify-Env -CondaExe $CondaExe -EnvPath $MotionEnv -Command @("python", "-c", "import torch; print(torch.__version__); print(torch.cuda.is_available())")
Verify-Env -CondaExe $CondaExe -EnvPath $MotionEnv -Command @("python", "infer_wild.py", "--help")
Verify-Env -CondaExe $CondaExe -EnvPath $AlphaEnv -Command @("python", "--version")
Verify-Env -CondaExe $CondaExe -EnvPath $AlphaEnv -Command @("python", "-c", "import torch; print(torch.__version__); print(torch.cuda.is_available())")
Verify-Env -CondaExe $CondaExe -EnvPath $AlphaEnv -Command @("python", ".external/AlphaPose/scripts/demo_inference.py", "--help")
Verify-Env -CondaExe $CondaExe -EnvPath $AlphaEnv -Command @("python", "realtime_motionbert.py", "--help")

Write-Host ""
Write-Host "[DONE] Windows 5090 deployment is ready." -ForegroundColor Green
Write-Host "[NOTE] Model assets now default to your Hugging Face repo: EMOCJC/motionbert_models"
