@echo off
setlocal EnableDelayedExpansion
if not defined NO_PAUSE set "PAUSE_ON_ERROR=1"
if defined NO_PAUSE set "PAUSE_ON_ERROR=0"
set "EXIT_CODE=0"

rem =========================
rem MotionBERT Offline -> Unreal Live Link Runner
rem Global 5-epoch test checkpoint output
rem Only edit the variables in this section.
rem =========================

set "NPY_PATH=D:\Git\Github\MotionBERT\outputs\global_5ep_test\X3D.npy"
set "VIDEO_PATH=D:\Git\Github\MotionBERT\outputs\test.mp4"

rem Optional: set playback fps manually. Keep 0 to auto-read from VIDEO_PATH, else fallback to 25.
set "PLAYBACK_FPS=0"

rem Optional: playback range.
set "START_FRAME=0"
set "END_FRAME=-1"

rem Optional: set to 1 to loop until Ctrl+C.
set "LOOP=0"

rem Unreal Live Link target.
set "UE_LIVELINK_HOST=127.0.0.1"
set "UE_LIVELINK_PORT=7001"
set "UE_LIVELINK_SUBJECT=MotionBERTPose"
set "UE_LIVELINK_SCALE=100.0"

rem =========================
rem Internal settings
rem =========================

set "SCRIPT_DIR=%~dp0"
set "PYTHON_EXE=%SCRIPT_DIR%.local\miniconda3\envs\motionbert\python.exe"
set "EXTRA_ARGS="

if not exist "%PYTHON_EXE%" (
    echo [ERROR] Python environment not found:
    echo         %PYTHON_EXE%
    echo [HINT] Run deploy_windows_5090.bat first on this machine.
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%SCRIPT_DIR%replay_offline_to_ue.py" (
    echo [ERROR] replay_offline_to_ue.py not found in:
    echo         %SCRIPT_DIR%
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%NPY_PATH%" (
    echo [ERROR] MotionBERT X3D.npy not found:
    echo         %NPY_PATH%
    echo.
    echo [HINT] Run run_motionbert_pose_global_5ep.bat first to generate the test output.
    set "EXIT_CODE=1"
    goto :end
)

if "%LOOP%"=="1" set "EXTRA_ARGS=!EXTRA_ARGS! --loop"

cd /d "%SCRIPT_DIR%"
echo [INFO] Replaying global 5-epoch MotionBERT 3D to Unreal Live Link...
echo [INFO] NPY:       %NPY_PATH%
echo [INFO] Video:     %VIDEO_PATH%
echo [INFO] Range:     %START_FRAME% to %END_FRAME%
echo [INFO] FPS:       %PLAYBACK_FPS%
echo [INFO] UE Target: %UE_LIVELINK_HOST%:%UE_LIVELINK_PORT% subject=%UE_LIVELINK_SUBJECT%
echo.

"%PYTHON_EXE%" replay_offline_to_ue.py --npy_path "%NPY_PATH%" --video_path "%VIDEO_PATH%" --fps %PLAYBACK_FPS% --start_frame %START_FRAME% --end_frame %END_FRAME% --ue_livelink_host %UE_LIVELINK_HOST% --ue_livelink_port %UE_LIVELINK_PORT% --ue_livelink_subject %UE_LIVELINK_SUBJECT% --ue_livelink_scale %UE_LIVELINK_SCALE% !EXTRA_ARGS!
set "EXIT_CODE=%ERRORLEVEL%"

:end
echo.
if "%EXIT_CODE%"=="0" (
    echo [OK] Offline replay finished.
) else (
    echo [ERROR] Offline replay failed with exit code %EXIT_CODE%.
    if "%PAUSE_ON_ERROR%"=="1" (
        echo [INFO] Press any key to close...
        pause >nul
    )
)

exit /b %EXIT_CODE%
