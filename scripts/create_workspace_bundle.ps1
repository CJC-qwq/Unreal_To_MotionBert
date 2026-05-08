param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [ValidateSet("lean", "full")]
    [string]$BundleMode = "lean",
    [ValidateSet(0, 1)]
    [int]$IncludeOutputs = 0,
    [string]$BundleName = ""
)

$ErrorActionPreference = "Stop"

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

function Remove-DirectoryIfExists {
    param([string]$Path)
    if (Test-Path -LiteralPath $Path) {
        cmd /c rmdir /s /q "$Path" | Out-Null
    }
}

function Get-TransientDirectories {
    param([string]$Root)

    $names = @("Intermediate", "Saved", "DerivedDataCache", "Binaries", "obj", ".vs")
    $distRoot = Join-Path $Root "dist"
    $localRoot = Join-Path $Root ".local"
    $dirs = Get-ChildItem -LiteralPath $Root -Directory -Recurse -Force -ErrorAction SilentlyContinue |
        Where-Object {
            ($names -contains $_.Name) -and
            (-not $_.FullName.StartsWith($distRoot, [System.StringComparison]::OrdinalIgnoreCase)) -and
            (-not $_.FullName.StartsWith($localRoot, [System.StringComparison]::OrdinalIgnoreCase))
        } |
        Select-Object -ExpandProperty FullName
    return @($dirs)
}

function Invoke-Robocopy {
    param(
        [string]$Source,
        [string]$Destination,
        [string[]]$ExtraArgs = @()
    )

    Ensure-Directory $Destination
    $args = @($Source, $Destination, "/E", "/R:1", "/W:1", "/NFL", "/NDL", "/NJH", "/NJS", "/NP") + $ExtraArgs
    Write-Host ("[RUN] robocopy {0}" -f ($args -join " "))
    & robocopy @args | Out-Null
    $code = $LASTEXITCODE
    if ($code -gt 7) {
        throw "robocopy failed with exit code $code"
    }
}

$RepoRoot = (Resolve-Path $RepoRoot).Path.TrimEnd("\")
$TimeStamp = Get-Date -Format "yyyyMMdd_HHmmss"
$BundleId = if ([string]::IsNullOrWhiteSpace($BundleName)) { "MotionBERT_workspace_${BundleMode}_$TimeStamp" } else { $BundleName.Trim() }
$DistRoot = Join-Path $RepoRoot "dist\workspace_bundle"
$StageRoot = Join-Path $DistRoot "${BundleId}_stage"
$BundleRoot = Join-Path $StageRoot "MotionBERT"
$ZipPath = Join-Path $DistRoot "$BundleId.zip"
$ManifestPath = Join-Path $BundleRoot "workspace_bundle_manifest.json"
$GuidePath = Join-Path $BundleRoot "WORKSPACE_BUNDLE_README.txt"

Write-Host "[INFO] Repo root:    $RepoRoot"
Write-Host "[INFO] Bundle mode:  $BundleMode"
Write-Host "[INFO] Zip output:   $ZipPath"

Write-Step "Prepare bundle staging area"
Ensure-Directory $DistRoot
Remove-DirectoryIfExists $StageRoot
if (Test-Path -LiteralPath $ZipPath) {
    Remove-Item -LiteralPath $ZipPath -Force
}
Ensure-Directory $BundleRoot

$excludeDirs = @(
    (Join-Path $RepoRoot ".git"),
    (Join-Path $RepoRoot ".cache"),
    (Join-Path $RepoRoot ".local"),
    (Join-Path $RepoRoot "alpha_tmp"),
    (Join-Path $RepoRoot "__pycache__"),
    (Join-Path $RepoRoot "dist")
)

if ($BundleMode -eq "lean") {
    $excludeDirs += (Join-Path $RepoRoot "alpha_cache")
}

if ($IncludeOutputs -ne 1) {
    $excludeDirs += (Join-Path $RepoRoot "outputs")
}

$excludeDirs += Get-TransientDirectories -Root $RepoRoot
$excludeDirs = $excludeDirs | Sort-Object -Unique

$robocopyArgs = @("/XD") + $excludeDirs + @("/XF", "*.pyc", "*.pyo")

Write-Step "Copy workspace files"
Invoke-Robocopy -Source $RepoRoot -Destination $BundleRoot -ExtraArgs $robocopyArgs

Write-Step "Write bundle manifest"
$manifest = [ordered]@{
    bundle_name = $BundleId
    bundle_mode = $BundleMode
    include_outputs = ($IncludeOutputs -eq 1)
    created_at = (Get-Date -Format "yyyy-MM-dd HH:mm:ss")
    source_repo_root = $RepoRoot
    excluded_directories = $excludeDirs
    next_step = "Unzip on target machine, then run deploy_windows_5090.bat"
}
$manifest | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $ManifestPath -Encoding UTF8

$guide = @"
MotionBERT workspace bundle

Source repo:
$RepoRoot

Bundle mode:
$BundleMode

What this bundle contains:
- Current workspace files
- .external/AlphaPose
- checkpoint
- configs
- docs
- Unreal
- scripts and .bat entrypoints

What this bundle does not contain:
- .local/miniconda3
- Any copied conda environment

Recommended target-machine flow:
1. Unzip this bundle to the target machine.
2. Open cmd in the unpacked repo root.
3. Run:
   deploy_windows_5090.bat
4. After deployment, use:
   run_alphapose.bat
   run_motionbert_pose.bat
   run_motionbert_realtime.bat

Notes:
- This bundle is meant for moving the workspace itself.
- Python / conda environments are intentionally not included.
"@
$guide | Set-Content -LiteralPath $GuidePath -Encoding UTF8

Write-Step "Create zip archive"
Compress-Archive -Path $BundleRoot -DestinationPath $ZipPath -CompressionLevel Optimal

Write-Host ""
Write-Host "[DONE] Workspace bundle ready." -ForegroundColor Green
Write-Host "[DONE] Folder: $BundleRoot"
Write-Host "[DONE] Zip:    $ZipPath"
