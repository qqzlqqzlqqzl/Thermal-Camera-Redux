# Windows UTi260B Runtime Build

This branch adds a Windows runtime build for the UVC thermal camera profile
used by `USB\VID_0BDA&PID_3901`.

## Build model

- Heavy build dependencies stay in GitHub Actions.
- The local Windows machine does not need MSYS2, OpenCV headers, or OpenCV
  development libraries to run the packaged build.
- The uploaded artifact is intended to be the distributable runtime folder:
  `Thermal-Camera-Redux.exe`, the required runtime DLLs, optional Qt/OpenCV
  runtime plugins, and `run_uti260b_windows.cmd`.
- Do not redistribute the full OpenCV SDK, headers, import libraries, or MSYS2
  package cache with the runtime folder.

## Run

From the extracted artifact folder:

```bat
run_uti260b_windows.cmd 0
```

If the thermal camera is not device index `0`, try `1`, `2`, and so on.
The launcher expands to:

```bat
Thermal-Camera-Redux.exe -uti260b -d <device-index>
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
