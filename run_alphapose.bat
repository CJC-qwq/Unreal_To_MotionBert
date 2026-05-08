@echo off
setlocal EnableDelayedExpansion
if not defined NO_PAUSE set "PAUSE_ON_ERROR=1"
if defined NO_PAUSE set "PAUSE_ON_ERROR=0"
set "EXIT_CODE=0"

rem =========================
rem AlphaPose JSON Runner
rem Only edit the variables in this section.
rem =========================

set "VIDEO_PATH=D:\Git\Github\MotionBERT\outputs\test.mp4"
set "OUTPUT_DIR=D:\Git\Github\MotionBERT\outputs"

rem Optional: set to 1 if you also want AlphaPose to render a video.
set "SAVE_VIDEO=0"

rem Optional: set to 1 to enable pose tracking for multi-person videos.
set "POSE_TRACK=0"

rem Optional: reduce if you hit out-of-memory.
set "DET_BATCH=5"
set "POSE_BATCH=64"

rem =========================
rem Internal settings
rem =========================

set "SCRIPT_DIR=%~dp0"
set "ALPHAPOSE_DIR=%SCRIPT_DIR%.external\AlphaPose"
set "PYTHON_EXE=%SCRIPT_DIR%.local\miniconda3\envs\alphapose\python.exe"
set "ALPHAPOSE_CFG=configs\halpe_26\resnet\256x192_res50_lr1e-3_1x.yaml"
set "ALPHAPOSE_CKPT=pretrained_models\halpe26_fast_res50_256x192.pth"
set "EXTRA_ARGS="

if not exist "%PYTHON_EXE%" (
    echo [ERROR] AlphaPose environment not found:
    echo         %PYTHON_EXE%
    echo [HINT] Run deploy_windows_5090.bat first on this machine.
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%ALPHAPOSE_DIR%\scripts\demo_inference.py" (
    echo [ERROR] AlphaPose script not found:
    echo         %ALPHAPOSE_DIR%\scripts\demo_inference.py
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%VIDEO_PATH%" (
    echo [ERROR] Video file not found:
    echo         %VIDEO_PATH%
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%ALPHAPOSE_DIR%\%ALPHAPOSE_CFG%" (
    echo [ERROR] AlphaPose config not found:
    echo         %ALPHAPOSE_DIR%\%ALPHAPOSE_CFG%
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%ALPHAPOSE_DIR%\%ALPHAPOSE_CKPT%" (
    echo [ERROR] AlphaPose checkpoint not found:
    echo         %ALPHAPOSE_DIR%\%ALPHAPOSE_CKPT%
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"
if not exist "%SCRIPT_DIR%alpha_cache\matplotlib" mkdir "%SCRIPT_DIR%alpha_cache\matplotlib"
if not exist "%SCRIPT_DIR%alpha_cache\torch\hub\checkpoints" mkdir "%SCRIPT_DIR%alpha_cache\torch\hub\checkpoints"

if "%SAVE_VIDEO%"=="1" set "EXTRA_ARGS=!EXTRA_ARGS! --save_video"
if "%POSE_TRACK%"=="1" set "EXTRA_ARGS=!EXTRA_ARGS! --pose_track"

set "PYTHONPATH=%ALPHAPOSE_DIR%"
set "MPLCONFIGDIR=%SCRIPT_DIR%alpha_cache\matplotlib"
set "TORCH_HOME=%SCRIPT_DIR%alpha_cache\torch"

cd /d "%ALPHAPOSE_DIR%"
echo [INFO] Running AlphaPose...
echo [INFO] Video:   %VIDEO_PATH%
echo [INFO] Output:  %OUTPUT_DIR%
echo.

"%PYTHON_EXE%" scripts\demo_inference.py --detector yolox-x --cfg "%ALPHAPOSE_CFG%" --checkpoint "%ALPHAPOSE_CKPT%" --video "%VIDEO_PATH%" --outdir "%OUTPUT_DIR%" --detbatch %DET_BATCH% --posebatch %POSE_BATCH% !EXTRA_ARGS!
set "EXIT_CODE=%ERRORLEVEL%"

:end
echo.
if "%EXIT_CODE%"=="0" (
    echo [OK] Done.
    echo [OK] JSON output: %OUTPUT_DIR%\alphapose-results.json
) else (
    echo [ERROR] AlphaPose failed with exit code %EXIT_CODE%.
    if "%PAUSE_ON_ERROR%"=="1" (
        echo [INFO] Press any key to close...
        pause >nul
    )
)

exit /b %EXIT_CODE%
