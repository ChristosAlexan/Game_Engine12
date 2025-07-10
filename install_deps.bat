@echo off
setlocal

REM Set the triplet you want to use (adjust if needed)
set TRIPLET=x64-windows

REM Clone vcpkg if it doesn't exist
IF NOT EXIST "vcpkg" (
    echo Cloning vcpkg...
    git clone https://github.com/microsoft/vcpkg.git
    if %errorlevel% neq 0 exit /b %errorlevel%
)

REM Bootstrap vcpkg if necessary
IF NOT EXIST "vcpkg\vcpkg.exe" (
    echo Bootstrapping vcpkg...
    pushd vcpkg
    call bootstrap-vcpkg.bat
    popd
    if %errorlevel% neq 0 exit /b %errorlevel%
)

REM Install dependencies using manifest mode
echo Installing vcpkg dependencies from vcpkg.json...
vcpkg\vcpkg install --triplet %TRIPLET%
if %errorlevel% neq 0 exit /b %errorlevel%

echo Done.
