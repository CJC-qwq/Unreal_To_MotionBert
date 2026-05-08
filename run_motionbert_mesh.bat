@echo off
setlocal EnableDelayedExpansion
if not defined NO_PAUSE set "PAUSE_ON_ERROR=1"
if defined NO_PAUSE set "PAUSE_ON_ERROR=0"
set "EXIT_CODE=0"

rem =========================
rem MotionBERT Mesh Runner
rem Only edit the variables in this section.
rem =========================

set "VIDEO_PATH=D:\Videos\test.mp4"
set "JSON_PATH=D:\Videos\alphapose_out\alphapose-results.json"
set "OUTPUT_DIR=D:\Videos\motionbert_mesh_output"

rem Optional: leave as-is unless you want another checkpoint.
set "CHECKPOINT=checkpoint\mesh\FT_MB_release_MB_ft_pw3d\best_epoch.bin"

rem Optional: set to a 3D pose result file for better root trajectory, or leave blank.
set "REF_3D_MOTION_PATH="

rem Optional: set to a person id for multi-person results, or leave blank.
set "FOCUS_ID="

rem Optional: set to 1 to add --pixel, otherwise keep 0.
set "USE_PIXEL=0"

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

if not exist "%SCRIPT_DIR%infer_wild_mesh.py" (
    echo [ERROR] infer_wild_mesh.py not found in:
    echo         %SCRIPT_DIR%
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%VIDEO_PATH%" (
    echo [ERROR] Video file not found:
    echo         %VIDEO_PATH%
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%JSON_PATH%" (
    echo [ERROR] AlphaPose JSON not found:
    echo         %JSON_PATH%
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%SCRIPT_DIR%%CHECKPOINT%" (
    echo [ERROR] Checkpoint not found:
    echo         %SCRIPT_DIR%%CHECKPOINT%
    echo.
    echo [HINT] Mesh mode also needs mesh checkpoint and SMPL data under data\mesh.
    set "EXIT_CODE=1"
    goto :end
)

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

if not "%REF_3D_MOTION_PATH%"=="" set "EXTRA_ARGS=!EXTRA_ARGS! --ref_3d_motion_path ""%REF_3D_MOTION_PATH%"""
if not "%FOCUS_ID%"=="" set "EXTRA_ARGS=!EXTRA_ARGS! --focus %FOCUS_ID%"
if "%USE_PIXEL%"=="1" set "EXTRA_ARGS=!EXTRA_ARGS! --pixel"

cd /d "%SCRIPT_DIR%"
echo [INFO] Running MotionBERT mesh inference...
echo [INFO] Video:   %VIDEO_PATH%
echo [INFO] JSON:    %JSON_PATH%
echo [INFO] Output:  %OUTPUT_DIR%
echo.

"%PYTHON_EXE%" infer_wild_mesh.py --vid_path "%VIDEO_PATH%" --json_path "%JSON_PATH%" --out_path "%OUTPUT_DIR%" -e "%CHECKPOINT%" !EXTRA_ARGS!
set "EXIT_CODE=%ERRORLEVEL%"

:end
echo.
if "%EXIT_CODE%"=="0" (
    echo [OK] Done.
    echo [OK] Output video: %OUTPUT_DIR%\mesh.mp4
) else (
    echo [ERROR] MotionBERT mesh failed with exit code %EXIT_CODE%.
    if "%PAUSE_ON_ERROR%"=="1" (
        echo [INFO] Press any key to close...
        pause >nul
    )
)

exit /b %EXIT_CODE%
