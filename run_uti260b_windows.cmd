@echo off
setlocal EnableExtensions EnableDelayedExpansion
set "DIR=%~dp0"
set "DEVICE_INDEX=0"
set "EXTRA_ARGS="
if "%~1"=="" goto run

set "FIRST_ARG=%~1"
if "!FIRST_ARG:~0,1!"=="-" goto all_args

set "DEVICE_INDEX=%~1"
shift /1
goto collect_args

:all_args
set "EXTRA_ARGS=%*"
goto run

:collect_args
if "%~1"=="" goto run
set "EXTRA_ARGS=!EXTRA_ARGS! %~1"
shift /1
goto collect_args

:run
echo UTi260B mode: auto-selecting USB\VID_0BDA^&PID_3901 when present.
echo GUI entry point: Thermal-Camera-Redux-GUI.exe
echo Extra args are passed through, e.g. -rotate 0 or -temp-offset-c -3.5.
"%DIR%Thermal-Camera-Redux.exe" -uti260b -d %DEVICE_INDEX% %EXTRA_ARGS%
