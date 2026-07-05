@echo off
setlocal
set "DIR=%~dp0"
set "DEVICE_INDEX=%~1"
if "%DEVICE_INDEX%"=="" (
  set "DEVICE_INDEX=0"
) else (
  shift
)
echo UTi260B mode: auto-selecting USB\VID_0BDA^&PID_3901 when present.
"%DIR%Thermal-Camera-Redux.exe" -uti260b -d %DEVICE_INDEX% %*
