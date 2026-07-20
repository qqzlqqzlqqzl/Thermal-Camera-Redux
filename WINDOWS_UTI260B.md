# Windows UTi260B Runtime Build

This branch adds a Windows runtime build for the UVC thermal camera profile
used by `USB\VID_0BDA&PID_3901`.

## Build model

- Heavy build dependencies stay in GitHub Actions.
- The local Windows machine does not need MSYS2, OpenCV headers, or OpenCV
  development libraries to run the packaged build.
- The uploaded artifact is intended to be the distributable runtime folder:
  `Thermal-Camera-Redux.exe`, `Thermal-Camera-Redux-GUI.exe`, only the
  required runtime DLLs, `models/`, and `run_uti260b_windows.cmd`.
- Do not redistribute the full OpenCV SDK, headers, import libraries, or MSYS2
  package cache with the runtime folder.
- The Windows CI build compiles a minimal OpenCV with Win32 UI and DirectShow
  enabled, plus OpenCV DNN/dnn_superres for CPU AI super-resolution. Qt,
  FFmpeg, GStreamer, OpenCL, tests, docs, Python, Java, and examples are
  disabled to keep the runtime small.
- AI super-resolution models are downloaded by GitHub Actions into
  `models/`: `FSRCNN_x2.pb` from Saafke/FSRCNN_Tensorflow and `ESPCN_x2.pb`
  from fannymonori/TF-ESPCN. Both upstream model repositories are Apache-2.0.
  CI pins these files by SHA256 before packaging them.
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

For a control-by-control Chinese manual, open
`THERMAL_CAMERA_GUI_MANUAL_zh-CN.md` from the runtime folder or press the GUI
`说明书` button.

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
run_uti260b_windows.cmd 0 -celsius
run_uti260b_windows.cmd 0 -fahrenheit
run_uti260b_windows.cmd 0 -temp-offset-c -3.5
run_uti260b_windows.cmd 0 -temp-offset-f -6.3
run_uti260b_windows.cmd 0 -display-scale 4 -interp lanczos
run_uti260b_windows.cmd 0 -display-scale 4 -ai-superres fsrcnn2
run_uti260b_windows.cmd 0 -display-scale 4 -ai-superres espcn2
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
- `AI super-res`: CPU-only x2 neural upscaling for the realtime display path.
  Supported modes are Off, FSRCNN x2 CPU realtime, and ESPCN x2 CPU realtime.
  The integration follows the paper-level processing flow instead of stacking
  arbitrary filters: FSRCNN receives the low-resolution display-intensity image
  and performs feature extraction, shrinking, nonlinear mapping, expanding, and
  upsampling. The original FSRCNN paper uses deconvolution for the final x2
  stage; the packaged Saafke TensorFlow export is OpenCV-compatible and uses a
  sub-pixel upsampling layer in that position. ESPCN performs convolution in
  low-resolution space and upsamples through sub-pixel convolution/pixel
  shuffle x2. The model output is then resized further by the selected classic
  interpolation if the GUI display scale is greater than 2x. If OpenCV DNN is
  unavailable, a model file is missing, or inference fails, the program falls
  back to the selected classic interpolation and logs the reason.
- `Colormap`: pseudo-color palettes are labeled with readable names such as
  Jet rainbow heat, Hot iron red, Turbo high contrast, Bone gray-white, and
  Deep green. The numeric index is kept in parentheses only for reproducible
  command-line use.
- `Temperature unit`: live Celsius/Fahrenheit display switch. The GUI defaults
  to Celsius and sends `-celsius` at launch; runtime changes use the live
  control pipe and do not require restart.
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
- `Blackbody target C` + `Calibrate`: manual one-point blackbody correction.
  Cover the lens with a stable isothermal blackbody or uniform-temperature
  target, wait about 5 seconds for the camera output to settle, enter the
  target Celsius value, choose the ROI size, then press `Calibrate`. The
  runtime measures the center rectangle ROI average and adjusts `Temp offset C` by
  `target - measured_average`. The result is written to
  `thermal-camera-redux-calibration-status.txt`, and the GUI reads it back to
  show the measured temperature, applied delta, new offset, and ROI percent.
  If `Drift C/min` is active, the current drift contribution is folded into the
  new offset and drift is reset to 0 so the calibrated display does not keep
  moving after the blackbody reference step.
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
`256x192` thermal matrix. AI super-resolution is also display-only: it never
rewrites `rawFrame` or `thermalFrame` and does not create new measured
temperature samples.

The GUI live-control transport is started with:

```bat
Thermal-Camera-Redux.exe ... -control-pipe <pipe-name>
```

This is normally managed by `Thermal-Camera-Redux-GUI.exe`; direct users do not
need to create the pipe manually.

The GUI also exposes interval/timelapse capture:

- Set an interval from 1 to 86400 seconds.
- `Start timelapse` captures the first displayed frame immediately, then keeps
  capturing at the selected interval.
- `Stop timelapse` stops timed capture without closing the camera.
- Each run creates `timelapse/session-YYYYMMDD-HHMMSS-id` next to the
  executable.
- Files are PNG-only and use
  `thermal-YYYYMMDD-HHMMSS-000001.png` style names. Manual `Snapshot` continues
  to save both PNG and RAW.
- Timelapse PNG files exclude the runtime HUD so average temperature,
  super-resolution/interpolation state, and FPS do not obscure the thermal
  image. Measurement overlays such as ROI, isotherm, and temperature markers
  still follow their normal controls.
- The runtime HUD is hidden by default and, when selected with `h`, is reduced
  to three compact lines.

The corresponding live-control commands are:

```text
timelapse start <intervalSeconds> [sessionName]
timelapse stop
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
- rows `0..191`: auxiliary/noisy image plane, ignored for UTi260B rendering
- rows `192..383`: `256x192` little-endian thermal matrix
- rows `384..385`: trailer metadata ignored by this branch
- temperature conversion: `raw / 16 - 273.15 + offset_c`

The camera may output invalid high temperatures during the first few seconds
after startup. Let it warm up before judging readings.
