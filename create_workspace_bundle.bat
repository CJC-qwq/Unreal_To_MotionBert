@echo off
setlocal

for %%I in ("%~dp0.") do set "SCRIPT_DIR=%%~fI"
set "PS1=%SCRIPT_DIR%\scripts\create_workspace_bundle.ps1"

if not defined BUNDLE_MODE set "BUNDLE_MODE=lean"
if not defined INCLUDE_OUTPUTS set "INCLUDE_OUTPUTS=0"
if not defined BUNDLE_NAME set "BUNDLE_NAME="

if not exist "%PS1%" (
    echo [ERROR] Workspace bundle script not found:
    echo         %PS1%
    exit /b 1
)

echo [INFO] MotionBERT workspace bundle
echo [INFO] Repo:          %SCRIPT_DIR%
echo [INFO] Bundle mode:   %BUNDLE_MODE%
echo [INFO] Include out:   %INCLUDE_OUTPUTS%
echo.

powershell -NoProfile -ExecutionPolicy Bypass -File "%PS1%" -RepoRoot "%SCRIPT_DIR%" -BundleMode "%BUNDLE_MODE%" -IncludeOutputs %INCLUDE_OUTPUTS% -BundleName "%BUNDLE_NAME%"
set "EXIT_CODE=%ERRORLEVEL%"

echo.
if "%EXIT_CODE%"=="0" (
    echo [OK] Workspace bundle created.
) else (
    echo [ERROR] Workspace bundle failed with exit code %EXIT_CODE%.
)

exit /b %EXIT_CODE%
