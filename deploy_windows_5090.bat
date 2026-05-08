@echo off
setlocal
if not defined NO_PAUSE set "PAUSE_ON_ERROR=1"
if defined NO_PAUSE set "PAUSE_ON_ERROR=0"
if not defined LOG_TO_FILE set "LOG_TO_FILE=0"
set "EXIT_CODE=0"

for %%I in ("%~dp0.") do set "SCRIPT_DIR=%%~fI"
set "PS1=%SCRIPT_DIR%\scripts\deploy_windows_5090.ps1"
set "LOG_PATH=%SCRIPT_DIR%\deploy_run.log"

if not exist "%PS1%" (
    echo [ERROR] Deploy script not found:
    echo         %PS1%
    set "EXIT_CODE=1"
    goto :end
)

echo [INFO] MotionBERT Windows 5090 one-click deploy
echo [INFO] Repo: %SCRIPT_DIR%
if "%LOG_TO_FILE%"=="1" (
    echo [INFO] Log:  %LOG_PATH%
)
echo.

if "%LOG_TO_FILE%"=="1" (
    powershell -NoProfile -ExecutionPolicy Bypass -Command "& '%PS1%' -RepoRoot '%SCRIPT_DIR%'" > "%LOG_PATH%" 2>&1
    set "EXIT_CODE=%ERRORLEVEL%"
    echo [INFO] Deployment output was written to:
    echo        %LOG_PATH%
) else (
    powershell -NoProfile -ExecutionPolicy Bypass -Command "& '%PS1%' -RepoRoot '%SCRIPT_DIR%'"
    set "EXIT_CODE=%ERRORLEVEL%"
)

:end
echo.
if "%EXIT_CODE%"=="0" (
    echo [OK] Deployment finished.
) else (
    echo [ERROR] Deployment failed with exit code %EXIT_CODE%.
    if "%PAUSE_ON_ERROR%"=="1" (
        echo [INFO] Press any key to close...
        pause >nul
    )
)

exit /b %EXIT_CODE%
