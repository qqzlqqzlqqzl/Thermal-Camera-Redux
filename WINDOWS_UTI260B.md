# Windows UTi260B Runtime Build

This branch adds a Windows runtime build for the UVC thermal camera profile
used by `USB\VID_0BDA&PID_3901`.

## Build model

- Heavy build dependencies stay in GitHub Actions.
- The local Windows machine does not need MSYS2, OpenCV headers, or OpenCV
  development libraries to run the packaged build.
- The uploaded artifact is intended to be the distributable runtime folder:
  `Thermal-Camera-Redux.exe`, `Thermal-Camera-Redux-GUI.exe`, only the
  required runtime DLLs, and `run_uti260b_windows.cmd`.
- Do not redistribute the full OpenCV SDK, headers, import libraries, or MSYS2
  package cache with the runtime folder.
- The Windows CI build compiles a minimal OpenCV with Win32 UI and DirectShow
  enabled, while Qt, FFmpeg, GStreamer, OpenCL, tests, docs, Python, Java, and
  examples are disabled to keep the runtime small.
- UTi260B capture on Windows uses the app's Media Foundation raw YUY2 path,
  because OpenCV/DirectShow can expose this device as converted RGB and corrupt
  the thermal matrix.
- This profile renders from the `256x192` thermal matrix directly. The first
  half of this camera's UVC frame is not treated as a usable visible-light image
  plane.
- Window layout switching remains on the thermal view for this profile, because
  image/double layouts would otherwise expose the ignored noisy image plane.
- The UTi260B profile defaults to a 90 degree clockwise display rotation. Use
  `-rotate 0`, `-rotate 180`, or `-rotate 270` after the launcher command if the
  physical mount needs a different orientation.

## Run

From the extracted artifact folder:

```bat
Thermal-Camera-Redux-GUI.exe
```

This is the normal Windows entry point. The GUI is a live control panel: it
saves settings in `thermal-camera-redux-gui.ini`, writes the exact command it
launches to `thermal-camera-redux-last-command.txt`, starts
`Thermal-Camera-Redux.exe` from the same folder, then sends runtime setting
changes over a Windows named pipe.

The GUI can switch between Chinese and English without restarting. Device index
changes still require restart; display, filtering, ROI, range, and threshold
changes are applied while the thermal window is running.

The command-line launcher is still available:

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

Extra arguments are passed through, for example:

```bat
run_uti260b_windows.cmd 0 -rotate 0
run_uti260b_windows.cmd 0 -temp-offset-c -3.5
run_uti260b_windows.cmd 0 -temp-offset-f -6.3
run_uti260b_windows.cmd 0 -display-scale 4 -interp lanczos
run_uti260b_windows.cmd 0 -filter-preset medium -bilateral low -temporal-denoise low -sharpen medium
```

`-temp-offset-c` and `-temp-offset-f` adjust displayed temperatures only. They
do not rewrite `.raw` snapshots or change the thermal matrix used for color
auto-ranging.

Display enhancement controls:

- `Super-res scale`: display upscale factor. GUI exposes 1x to 4x for stable
  realtime use; command-line `-display-scale`/`-scale` is clamped to the build's
  maximum scale.
- `Interpolation`: classic software super-resolution/display upscale method.
  Supported GUI choices are Nearest, Bilinear, Bicubic, and Lanczos.
- `Filter preset`: display-only Gaussian smoothing, Off/Low/Medium/Strong.
- `Bilateral`: display-only edge-preserving smoothing, Off/Low/Medium/Strong.
- `Temporal denoise`: display-only frame blending, Off/Low/Medium/Strong.
- `Sharpen`: display-only unsharp-mask sharpening, Off/Low/Medium/Strong.
- `Blur`: classic display blur/smoothing, Off/Low/Medium/Strong.
- `Contrast / gain`: display contrast multiplier using the original Redux
  contrast path.
- `Temp offset C` and `Drift C/min`: displayed-temperature calibration. Offset
  is a fixed correction; drift is a linear Celsius-per-minute correction from
  program start or the last drift change. Both leave RAW data untouched.
- `Manual temp range`: fixed min/max Celsius range for thermal-to-colormap
  mapping. This is display mapping only; temperature calculations still use the
  original thermal matrix.
- `Auto range`: existing colormap range lock modes, exposed in the GUI.
- `Range mapping`: existing nonlinear range mapping filters, exposed in the GUI.
- `ROI mode`: center spot or center rectangle ROI overlay. Rectangle ROI shows
  min/avg/max from the original thermal matrix.
- `Isotherm / over-temp`: highlights pixels above the selected Celsius
  threshold on the displayed frame.
- `Snapshot`: writes the current PNG plus the original `.raw` frame.
- `Record`: toggles the existing AVI recording path.
- `Preset`: quick groups for PCB, HVAC, human body, low-noise, high-contrast,
  and raw display workflows.

These enhancement controls only affect the rendered display frame before the
HUD/markers are drawn. Temperature readings, min/max/avg, center temperature,
rulers, ROI statistics, and `.raw` snapshots continue to use the original
`256x192` thermal matrix.

The GUI live-control transport is started with:

```bat
Thermal-Camera-Redux.exe ... -control-pipe <pipe-name>
```

This is normally managed by `Thermal-Camera-Redux-GUI.exe`; direct users do not
need to create the pipe manually.

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
- rows `0..191`: auxiliary/noisy image plane, ignored for UTi260B rendering
- rows `192..383`: `256x192` little-endian thermal matrix
- rows `384..385`: trailer metadata ignored by this branch
- temperature conversion: `raw / 16 - 273.15 + offset_c`

The camera may output invalid high temperatures during the first few seconds
after startup. Let it warm up before judging readings.
