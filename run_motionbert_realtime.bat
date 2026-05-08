@echo off
setlocal EnableDelayedExpansion
if not defined NO_PAUSE set "PAUSE_ON_ERROR=1"
if defined NO_PAUSE set "PAUSE_ON_ERROR=0"
set "EXIT_CODE=0"

rem =========================
rem MotionBERT Realtime 3D Runner
rem Only edit the variables in this section.
rem =========================

rem Camera source:
rem   0  = default webcam
rem   1  = second webcam
rem   D:\Videos\demo.mp4 = local video file for debugging
if not defined SOURCE set "SOURCE=0"

rem Optional: save the side-by-side realtime output video.
if not defined SAVE_VIDEO set "SAVE_VIDEO="

rem Optional: tracked person id. Leave blank for the most confident person.
if not defined FOCUS_ID set "FOCUS_ID="

rem Optional: warmup frame count before 3D starts.
if not defined MIN_FRAMES set "MIN_FRAMES=27"

rem Optional: set 1 to show FPS text.
if not defined SHOW_FPS set "SHOW_FPS=1"

rem Optional: camera request size.
if not defined CAMERA_WIDTH set "CAMERA_WIDTH=1280"
if not defined CAMERA_HEIGHT set "CAMERA_HEIGHT=720"
if not defined CAMERA_FPS set "CAMERA_FPS=30"

rem Optional preset:
rem   quality = clearer local preview, higher load
rem   smooth  = better for UE skeleton smoothness
rem   fast    = reduce load first
if not defined UE_PRESET set "UE_PRESET=smooth"

rem Optional realtime tuning:
rem   DISPLAY_SCALE  < 1 reduces preview window cost
rem   NO_DISPLAY=1   disables preview window for max performance
rem   INFER_EVERY_N  > 1 reduces 3D inference frequency
rem   MOTION_SMOOTH  0 disables smoothing, 0.3~0.7 is smoother but adds lag
rem   UE_INTERPOLATE=1 enables fixed-rate pose interpolation before sending to UE
rem   UE_BUFFER_MS  small buffer for smoother UE motion, lower = less lag
if not defined DISPLAY_SCALE set "DISPLAY_SCALE=1.0"
if not defined NO_DISPLAY set "NO_DISPLAY=0"
if not defined INFER_EVERY_N set "INFER_EVERY_N=1"
if not defined MOTION_SMOOTH set "MOTION_SMOOTH=0.0"
if not defined UE_INTERPOLATE set "UE_INTERPOLATE=1"
if not defined UE_BUFFER_MS set "UE_BUFFER_MS=45"

if /I "%UE_PRESET%"=="quality" (
    set "CAMERA_WIDTH=1280"
    set "CAMERA_HEIGHT=720"
    set "CAMERA_FPS=30"
    set "DISPLAY_SCALE=1.0"
    set "NO_DISPLAY=0"
    set "INFER_EVERY_N=1"
    set "MOTION_SMOOTH=0.15"
    set "UE_INTERPOLATE=1"
    set "UE_BUFFER_MS=35"
)
if /I "%UE_PRESET%"=="smooth" (
    set "CAMERA_WIDTH=960"
    set "CAMERA_HEIGHT=540"
    set "CAMERA_FPS=30"
    set "DISPLAY_SCALE=0.8"
    set "NO_DISPLAY=1"
    set "INFER_EVERY_N=1"
    set "MOTION_SMOOTH=0.45"
    set "UE_INTERPOLATE=1"
    set "UE_BUFFER_MS=55"
)
if /I "%UE_PRESET%"=="fast" (
    set "CAMERA_WIDTH=640"
    set "CAMERA_HEIGHT=360"
    set "CAMERA_FPS=30"
    set "DISPLAY_SCALE=0.75"
    set "NO_DISPLAY=1"
    set "INFER_EVERY_N=2"
    set "MOTION_SMOOTH=0.35"
    set "UE_INTERPOLATE=1"
    set "UE_BUFFER_MS=30"
)

rem Optional: Unreal Live Link UDP streaming.
rem Default to localhost so UE integration works out of the box.
if not defined UE_LIVELINK_HOST set "UE_LIVELINK_HOST=127.0.0.1"
if not defined UE_LIVELINK_PORT set "UE_LIVELINK_PORT=7001"
if not defined UE_LIVELINK_SUBJECT set "UE_LIVELINK_SUBJECT=MotionBERTPose"
if not defined UE_LIVELINK_SCALE set "UE_LIVELINK_SCALE=100.0"
if not defined UE_SEND_FPS set "UE_SEND_FPS=60"

rem =========================
rem Internal settings
rem =========================

set "SCRIPT_DIR=%~dp0"
set "ALPHAPOSE_DIR=%SCRIPT_DIR%.external\AlphaPose"
set "PYTHON_EXE=%SCRIPT_DIR%.local\miniconda3\envs\alphapose\python.exe"
set "EXTRA_ARGS="

if not exist "%PYTHON_EXE%" (
    echo [ERROR] AlphaPose Python environment not found:
    echo         %PYTHON_EXE%
    echo [HINT] Run deploy_windows_5090.bat first on this machine.
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%SCRIPT_DIR%realtime_motionbert.py" (
    echo [ERROR] realtime_motionbert.py not found in:
    echo         %SCRIPT_DIR%
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%ALPHAPOSE_DIR%\scripts\demo_inference.py" (
    echo [ERROR] AlphaPose dependency checkout not found:
    echo         %ALPHAPOSE_DIR%
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%SCRIPT_DIR%alpha_cache\matplotlib" mkdir "%SCRIPT_DIR%alpha_cache\matplotlib"
if not exist "%SCRIPT_DIR%alpha_cache\torch\hub\checkpoints" mkdir "%SCRIPT_DIR%alpha_cache\torch\hub\checkpoints"

if not "%FOCUS_ID%"=="" set "EXTRA_ARGS=!EXTRA_ARGS! --focus %FOCUS_ID%"
if "%SHOW_FPS%"=="1" set "EXTRA_ARGS=!EXTRA_ARGS! --show_fps"
if "%NO_DISPLAY%"=="1" set "EXTRA_ARGS=!EXTRA_ARGS! --no_display"
if not "%DISPLAY_SCALE%"=="1.0" set "EXTRA_ARGS=!EXTRA_ARGS! --display_scale %DISPLAY_SCALE%"
if not "%INFER_EVERY_N%"=="1" set "EXTRA_ARGS=!EXTRA_ARGS! --infer_every_n %INFER_EVERY_N%"
if not "%MOTION_SMOOTH%"=="0.0" set "EXTRA_ARGS=!EXTRA_ARGS! --motion_smooth %MOTION_SMOOTH%"
if "%UE_INTERPOLATE%"=="1" set "EXTRA_ARGS=!EXTRA_ARGS! --ue_interpolate"
if not "%UE_BUFFER_MS%"=="45" set "EXTRA_ARGS=!EXTRA_ARGS! --ue_buffer_ms %UE_BUFFER_MS%"
if not "%SAVE_VIDEO%"=="" (
    set "EXTRA_ARGS=!EXTRA_ARGS! --save_video "%SAVE_VIDEO%""
)
if not "%UE_LIVELINK_HOST%"=="" (
    set "EXTRA_ARGS=!EXTRA_ARGS! --ue_livelink_host %UE_LIVELINK_HOST% --ue_livelink_port %UE_LIVELINK_PORT% --ue_livelink_subject %UE_LIVELINK_SUBJECT% --ue_livelink_scale %UE_LIVELINK_SCALE% --ue_send_fps %UE_SEND_FPS%"
)
if defined EXTRA_RUNTIME_ARGS set "EXTRA_ARGS=!EXTRA_ARGS! %EXTRA_RUNTIME_ARGS%"

set "PYTHONPATH=%ALPHAPOSE_DIR%;%SCRIPT_DIR%"
set "MPLCONFIGDIR=%SCRIPT_DIR%alpha_cache\matplotlib"
set "TORCH_HOME=%SCRIPT_DIR%alpha_cache\torch"

cd /d "%SCRIPT_DIR%"
echo [INFO] Running MotionBERT realtime 3D...
echo [INFO] Source:  %SOURCE%
echo [INFO] Python:  %PYTHON_EXE%
echo [INFO] Preset:  %UE_PRESET%
echo [INFO] Tuning:  scale=%DISPLAY_SCALE% no_display=%NO_DISPLAY% infer_every_n=%INFER_EVERY_N% smooth=%MOTION_SMOOTH% interp=%UE_INTERPOLATE% buffer_ms=%UE_BUFFER_MS%
if not "%UE_LIVELINK_HOST%"=="" (
    echo [INFO] UE Live Link: %UE_LIVELINK_HOST%:%UE_LIVELINK_PORT% subject=%UE_LIVELINK_SUBJECT% send_fps=%UE_SEND_FPS%
) else (
    echo [INFO] UE Live Link: disabled
)
echo.

"%PYTHON_EXE%" realtime_motionbert.py --source "%SOURCE%" --camera_width %CAMERA_WIDTH% --camera_height %CAMERA_HEIGHT% --camera_fps %CAMERA_FPS% --min_frames %MIN_FRAMES% !EXTRA_ARGS!
set "EXIT_CODE=%ERRORLEVEL%"

:end
echo.
if "%EXIT_CODE%"=="0" (
    echo [OK] Realtime MotionBERT finished.
) else (
    echo [ERROR] Realtime MotionBERT failed with exit code %EXIT_CODE%.
    if "%PAUSE_ON_ERROR%"=="1" (
        echo [INFO] Press any key to close...
        pause >nul
    )
)

exit /b %EXIT_CODE%
