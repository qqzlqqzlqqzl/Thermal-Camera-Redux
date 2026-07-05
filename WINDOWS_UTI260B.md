# Windows UTi260B Runtime Build

This branch adds a Windows runtime build for the UVC thermal camera profile
used by `USB\VID_0BDA&PID_3901`.

## Build model

- Heavy build dependencies stay in GitHub Actions.
- The local Windows machine does not need MSYS2, OpenCV headers, or OpenCV
  development libraries to run the packaged build.
- The uploaded artifact is intended to be the distributable runtime folder:
  `Thermal-Camera-Redux.exe`, only the required runtime DLLs, and
  `run_uti260b_windows.cmd`.
- Do not redistribute the full OpenCV SDK, headers, import libraries, or MSYS2
  package cache with the runtime folder.
- The Windows CI build compiles a minimal OpenCV with Win32 UI and DirectShow
  enabled, while Qt, FFmpeg, GStreamer, OpenCL, tests, docs, Python, Java, and
  examples are disabled to keep the runtime small.
- UTi260B capture on Windows uses the app's Media Foundation raw YUY2 path,
  because OpenCV/DirectShow can expose this device as converted RGB and corrupt
  the thermal matrix.

## Run

From the extracted artifact folder:

```bat
run_uti260b_windows.cmd
```

In `-uti260b` mode the Windows build auto-selects the camera whose device path
contains `VID_0BDA` and `PID_3901`. The optional numeric argument is retained for
the generic camera-index CLI shape, but it is not used to choose between the
laptop camera and the UTi260B profile when this VID/PID is present.

The launcher expands to:

```bat
Thermal-Camera-Redux.exe -uti260b -d <device-index>
```

The tested Windows device list was:

- `USB Camera`: `USB\VID_0BDA&PID_3901&MI_00\...`
- `HP HD Camera`: `USB\VID_04F2&PID_B6C0&MI_00\...`

The runtime log should include:

```text
Media Foundation selected VID_0BDA&PID_3901 camera index 1: USB Camera
Backend: MediaFoundation raw YUY2 256x386 stride(512)
```

## Camera profile

The `-uti260b` shortcut selects the same settings as:

```bat
Thermal-Camera-Redux.exe -profile uti260b -d <device-index>
```

Expected raw UVC format:

- `256x386` YUY2/YUYV
- rows `0..191`: visible image
- rows `192..383`: `256x192` little-endian thermal matrix
- rows `384..385`: trailer metadata ignored by this branch
- temperature conversion: `raw / 16 - 273.15`

The camera may output invalid high temperatures during the first few seconds
after startup. Let it warm up before judging readings.
