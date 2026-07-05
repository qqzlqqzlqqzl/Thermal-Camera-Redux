@echo off
setlocal
set "DIR=%~dp0"
set "DEVICE_INDEX=%~1"
if "%DEVICE_INDEX%"=="" (
  set "DEVICE_INDEX=0"
) else (
  shift
)
"%DIR%Thermal-Camera-Redux.exe" -uti260b -d %DEVICE_INDEX% %*
