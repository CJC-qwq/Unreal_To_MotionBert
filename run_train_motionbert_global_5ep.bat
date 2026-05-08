@echo off
setlocal EnableDelayedExpansion
if not defined NO_PAUSE set "PAUSE_ON_ERROR=1"
if defined NO_PAUSE set "PAUSE_ON_ERROR=0"
set "EXIT_CODE=0"

rem =========================
rem MotionBERT Global Pose 5-Epoch Smoke Test
rem This run is isolated from official checkpoints.
rem Only edit the variables in this section.
rem =========================

set "CONFIG=configs\pose3d\MB_train_h36m_global_5ep.yaml"
set "CHECKPOINT_DIR=checkpoint\pose3d\MB_train_h36m_global_5ep"
set "SEED=0"

rem Optional extra args appended to train.py
set "EXTRA_ARGS="

rem Optional PyTorch CUDA allocator hint for fragmentation.
set "PYTORCH_CUDA_ALLOC_CONF=expandable_segments:True"

rem =========================
rem Internal settings
rem =========================

set "SCRIPT_DIR=%~dp0"
set "PYTHON_EXE=%SCRIPT_DIR%.local\miniconda3\envs\motionbert\python.exe"

if not exist "%PYTHON_EXE%" (
    echo [ERROR] Python environment not found:
    echo         %PYTHON_EXE%
    echo [HINT] Run deploy_windows_5090.bat first on this machine.
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%SCRIPT_DIR%train.py" (
    echo [ERROR] train.py not found in:
    echo         %SCRIPT_DIR%
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%SCRIPT_DIR%%CONFIG%" (
    echo [ERROR] Config not found:
    echo         %SCRIPT_DIR%%CONFIG%
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%SCRIPT_DIR%data\motion3d\MB3D_f243s81\H36M-SH\train" (
    echo [ERROR] Training data not found:
    echo         %SCRIPT_DIR%data\motion3d\MB3D_f243s81\H36M-SH\train
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%SCRIPT_DIR%%CHECKPOINT_DIR%" mkdir "%SCRIPT_DIR%%CHECKPOINT_DIR%"

cd /d "%SCRIPT_DIR%"
echo [INFO] Running MotionBERT global 5-epoch smoke test...
echo [INFO] Config:      %CONFIG%
echo [INFO] Checkpoint:  %CHECKPOINT_DIR%
echo [INFO] Seed:        %SEED%
echo [INFO] Batch size:  4
echo.

"%PYTHON_EXE%" train.py --config "%CONFIG%" --checkpoint "%CHECKPOINT_DIR%" --seed %SEED% %EXTRA_ARGS%
set "EXIT_CODE=%ERRORLEVEL%"

:end
echo.
if "%EXIT_CODE%"=="0" (
    echo [OK] Training finished.
    echo [OK] Checkpoints saved under: %CHECKPOINT_DIR%
) else (
    echo [ERROR] Training failed with exit code %EXIT_CODE%.
    if "%PAUSE_ON_ERROR%"=="1" (
        echo [INFO] Press any key to close...
        pause >nul
    )
)

exit /b %EXIT_CODE%
