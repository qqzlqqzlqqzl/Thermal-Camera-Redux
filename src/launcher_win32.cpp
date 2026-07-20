#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstring>
#include <cstdlib>

struct ComboItem {
	const wchar_t *en;
	const wchar_t *zh;
	const char *value;
};

struct TextBinding {
	HWND hwnd;
	const wchar_t *en;
	const wchar_t *zh;
};

static const wchar_t *APP_TITLE_EN = L"Thermal Camera Redux UTi260B";
static const wchar_t *APP_TITLE_ZH = L"Thermal Camera Redux UTi260B 实时控制";

static const int IDC_LANGUAGE = 1000;
static const int IDC_DEVICE = 1001;
static const int IDC_ROTATION = 1002;
static const int IDC_SCALE = 1003;
static const int IDC_INTERP = 1004;
static const int IDC_CMAP = 1005;
static const int IDC_OFFSET = 1006;
static const int IDC_FILTER = 1007;
static const int IDC_BILATERAL = 1008;
static const int IDC_TEMPORAL = 1009;
static const int IDC_SHARPEN = 1010;
static const int IDC_FULLSCREEN = 1011;
static const int IDC_PREVIEW = 1012;
static const int IDC_STATUS = 1013;
static const int IDC_START = 1014;
static const int IDC_RESET_DEFAULTS = 1015;
static const int IDC_STOP = 1016;
static const int IDC_SNAPSHOT = 1017;
static const int IDC_RECORD = 1018;
static const int IDC_RUNTIME_RESET = 1019;
static const int IDC_PRESET = 1020;
static const int IDC_BLUR = 1021;
static const int IDC_THRESHOLD = 1022;
static const int IDC_AUTORANGE = 1023;
static const int IDC_MAPPING = 1024;
static const int IDC_MANUAL_ENABLE = 1025;
static const int IDC_MANUAL_MIN = 1026;
static const int IDC_MANUAL_MAX = 1027;
static const int IDC_ROI_MODE = 1028;
static const int IDC_ROI_SIZE = 1029;
static const int IDC_ISOTHERM_ENABLE = 1030;
static const int IDC_ISOTHERM_THRESHOLD = 1031;
static const int IDC_HISTOGRAM = 1032;
static const int IDC_RULERS = 1033;
static const int IDC_DRIFT = 1034;
static const int IDC_CONTRAST = 1035;
static const int IDC_TEMP_UNIT = 1036;
static const int IDC_AI_SUPERRES = 1037;
static const int IDC_BLACKBODY_TARGET = 1038;
static const int IDC_BLACKBODY_CALIBRATE = 1039;
static const int IDC_OPEN_MANUAL = 1040;
static const int IDC_TIMELAPSE_INTERVAL = 1041;
static const int IDC_TIMELAPSE_START = 1042;
static const int IDC_TIMELAPSE_STOP = 1043;
static const int IDC_TIMELAPSE_FOLDER = 1044;

static const UINT_PTR TIMER_PROCESS = 1;
static const UINT_PTR TIMER_INITIAL_SYNC = 2;
static const UINT_PTR TIMER_CALIBRATION_STATUS = 3;

static HWND g_main;
static HWND g_language;
static HWND g_device;
static HWND g_rotation;
static HWND g_scale;
static HWND g_aiSuperres;
static HWND g_interp;
static HWND g_cmap;
static HWND g_tempUnit;
static HWND g_offset;
static HWND g_drift;
static HWND g_blackbodyTarget;
static HWND g_blackbodyCalibrate;
static HWND g_filter;
static HWND g_bilateral;
static HWND g_temporal;
static HWND g_sharpen;
static HWND g_fullscreen;
static HWND g_preview;
static HWND g_status;
static HWND g_start;
static HWND g_stop;
static HWND g_snapshot;
static HWND g_record;
static HWND g_runtimeReset;
static HWND g_resetDefaults;
static HWND g_openManual;
static HWND g_timelapseInterval;
static HWND g_timelapseStart;
static HWND g_timelapseStop;
static HWND g_timelapseFolder;
static HWND g_preset;
static HWND g_blur;
static HWND g_contrast;
static HWND g_threshold;
static HWND g_autorange;
static HWND g_mapping;
static HWND g_manualEnable;
static HWND g_manualMin;
static HWND g_manualMax;
static HWND g_roiMode;
static HWND g_roiSize;
static HWND g_isothermEnable;
static HWND g_isothermThreshold;
static HWND g_histogram;
static HWND g_rulers;
static HFONT g_font;
static bool g_zh = true;
static bool g_suppressEvents = false;
static std::vector<TextBinding> g_textBindings;
static std::vector<std::wstring> g_cmapLabelsEn;
static std::vector<std::wstring> g_cmapLabelsZh;
static std::vector<std::string> g_cmapValues;
static HANDLE g_process = NULL;
static HANDLE g_pipe = INVALID_HANDLE_VALUE;
static DWORD g_processId = 0;
static std::string g_pipeName;
static std::string g_pendingCalibrationId;
static int g_calibrationPolls = 0;
static bool g_timelapseActive = false;
static std::wstring g_timelapseOutputFolder;

static std::vector<ComboItem> g_languageItems = {
	{ L"中文", L"中文", "zh-CN" },
	{ L"English", L"English", "en-US" }
};

static std::vector<ComboItem> g_tempUnitItems = {
	{ L"Celsius (C)", L"摄氏度 (C)", "celsius" },
	{ L"Fahrenheit (F)", L"华氏度 (F)", "fahrenheit" }
};

static std::vector<ComboItem> g_rotationItems = {
	{ L"0 deg", L"0 度", "0" },
	{ L"90 deg", L"90 度", "90" },
	{ L"180 deg", L"180 度", "180" },
	{ L"270 deg", L"270 度", "270" }
};

static std::vector<ComboItem> g_scaleItems = {
	{ L"1x native", L"1x 原始", "1" },
	{ L"2x", L"2x", "2" },
	{ L"3x", L"3x", "3" },
	{ L"4x", L"4x", "4" }
};

static std::vector<ComboItem> g_interpItems = {
	{ L"Nearest", L"最近邻", "nearest" },
	{ L"Bilinear", L"双线性", "linear" },
	{ L"Bicubic", L"双三次", "cubic" },
	{ L"Lanczos4", L"Lanczos4", "lanczos" }
};

static std::vector<ComboItem> g_aiSuperresItems = {
	{ L"Off", L"关闭", "off" },
	{ L"FSRCNN x2 CPU realtime", L"FSRCNN x2 CPU 实时", "fsrcnn2" },
	{ L"ESPCN x2 CPU realtime", L"ESPCN x2 CPU 实时", "espcn2" }
};

static std::vector<ComboItem> g_levelItems = {
	{ L"Off", L"关闭", "off" },
	{ L"Low", L"低", "low" },
	{ L"Medium", L"中", "medium" },
	{ L"Strong", L"强", "strong" }
};

static std::vector<ComboItem> g_blurItems = {
	{ L"Off", L"关闭", "0" },
	{ L"Low", L"低", "2" },
	{ L"Medium", L"中", "4" },
	{ L"Strong", L"强", "6" }
};

static std::vector<ComboItem> g_contrastItems = {
	{ L"1.00 normal", L"1.00 正常", "1.0" },
	{ L"1.15 mild", L"1.15 轻微", "1.15" },
	{ L"1.35 high", L"1.35 高", "1.35" },
	{ L"1.60 strong", L"1.60 强", "1.60" }
};

static std::vector<ComboItem> g_autorangeItems = {
	{ L"Off", L"关闭", "0" },
	{ L"Clip lock", L"裁剪锁定", "1" },
	{ L"Grow lock", L"增长锁定", "2" }
};

static std::vector<ComboItem> g_mappingItems = {
	{ L"Linear", L"线性", "1" },
	{ L"Sin180", L"中心增强", "3" },
	{ L"Center 2", L"中心增强 2", "5" },
	{ L"UC90", L"圆弧 90", "7" },
	{ L"Outer", L"边缘增强", "10" },
	{ L"Cos360", L"高对比", "12" }
};

static std::vector<ComboItem> g_roiItems = {
	{ L"Off", L"关闭", "off" },
	{ L"Center spot", L"中心点测温", "spot" },
	{ L"Center rectangle", L"中心矩形 ROI", "rect" }
};

static std::vector<ComboItem> g_roiSizeItems = {
	{ L"10%", L"10%", "10" },
	{ L"25%", L"25%", "25" },
	{ L"50%", L"50%", "50" },
	{ L"75%", L"75%", "75" }
};

static std::vector<ComboItem> g_rulerItems = {
	{ L"Off", L"关闭", "0" },
	{ L"One temp", L"单点", "1" },
	{ L"Crosshair", L"十字", "2" },
	{ L"Horizontal", L"水平", "3" },
	{ L"Vertical", L"垂直", "4" },
	{ L"Both", L"双向", "5" }
};

static std::vector<ComboItem> g_presetItems = {
	{ L"Custom", L"自定义", "custom" },
	{ L"PCB inspection", L"PCB 检测", "pcb" },
	{ L"HVAC / air leak", L"空调/漏风", "hvac" },
	{ L"Human body", L"人体", "human" },
	{ L"Low noise", L"低噪声", "low-noise" },
	{ L"High contrast", L"高对比", "high-contrast" },
	{ L"Raw display", L"原始显示", "raw" }
};

static std::wstring widenAscii(const std::string &value) {
	return std::wstring(value.begin(), value.end());
}

static std::string narrowAscii(const std::wstring &value) {
	return std::string(value.begin(), value.end());
}

static std::string utf8FromWide(const std::wstring &value) {
	if ( value.empty() ) {
		return std::string();
	}
	int bytes = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, NULL, 0, NULL, NULL);
	if ( bytes <= 0 ) {
		return std::string();
	}
	std::string result((size_t)bytes, '\0');
	if ( WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, &result[0], bytes, NULL, NULL) <= 0 ) {
		return std::string();
	}
	result.resize((size_t)bytes - 1);
	return result;
}

static std::wstring exeDirW() {
	wchar_t path[MAX_PATH] = {0};
	GetModuleFileNameW(NULL, path, MAX_PATH);
	std::wstring value(path);
	size_t slash = value.find_last_of(L"\\/");
	if ( slash == std::wstring::npos ) {
		return L".";
	}
	return value.substr(0, slash);
}

static std::wstring joinPathW(const std::wstring &dir, const std::wstring &name) {
	if ( dir.empty() ) {
		return name;
	}
	wchar_t last = dir[dir.size() - 1];
	if ( last == L'\\' || last == L'/' ) {
		return dir + name;
	}
	return dir + L"\\" + name;
}

static bool fileExistsW(const std::wstring &path) {
	DWORD attrs = GetFileAttributesW(path.c_str());
	return attrs != INVALID_FILE_ATTRIBUTES && 0 == (attrs & FILE_ATTRIBUTE_DIRECTORY);
}

static bool directoryExistsW(const std::wstring &path) {
	DWORD attrs = GetFileAttributesW(path.c_str());
	return attrs != INVALID_FILE_ATTRIBUTES && 0 != (attrs & FILE_ATTRIBUTE_DIRECTORY);
}

static bool createDirectoryIfMissingW(const std::wstring &path) {
	if ( CreateDirectoryW(path.c_str(), NULL) ) {
		return true;
	}
	return ERROR_ALREADY_EXISTS == GetLastError() && directoryExistsW(path);
}

static std::wstring quoteArgW(const std::wstring &arg) {
	std::wstring out = L"\"";
	for ( wchar_t ch : arg ) {
		if ( ch == L'"' ) {
			out += L"\\\"";
		} else {
			out += ch;
		}
	}
	out += L"\"";
	return out;
}

static std::wstring iniPathW() {
	return joinPathW(exeDirW(), L"thermal-camera-redux-gui.ini");
}

static std::wstring manualPathW() {
	return joinPathW(exeDirW(), L"THERMAL_CAMERA_GUI_MANUAL_zh-CN.md");
}

static std::string readIni(const char *key, const char *fallback) {
	wchar_t buffer[256] = {0};
	GetPrivateProfileStringW(L"settings", widenAscii(key).c_str(), widenAscii(fallback).c_str(), buffer, 256, iniPathW().c_str());
	return narrowAscii(buffer);
}

static void writeIni(const char *key, const std::string &value) {
	WritePrivateProfileStringW(L"settings", widenAscii(key).c_str(), widenAscii(value).c_str(), iniPathW().c_str());
}

static void setFont(HWND hwnd) {
	SendMessageW(hwnd, WM_SETFONT, (WPARAM)g_font, TRUE);
}

static void bindText(HWND hwnd, const wchar_t *en, const wchar_t *zh) {
	g_textBindings.push_back({ hwnd, en, zh });
	SetWindowTextW(hwnd, g_zh ? zh : en);
}

static void setText(HWND hwnd, const std::wstring &value) {
	SetWindowTextW(hwnd, value.c_str());
}

static std::wstring textOfW(HWND hwnd) {
	wchar_t buffer[256] = {0};
	GetWindowTextW(hwnd, buffer, (int)(sizeof(buffer) / sizeof(buffer[0])));
	return std::wstring(buffer);
}

static std::string textOfAscii(HWND hwnd) {
	return narrowAscii(textOfW(hwnd));
}

static bool looksNumeric(HWND hwnd) {
	std::wstring text = textOfW(hwnd);
	if ( text.empty() ) {
		return false;
	}
	for ( wchar_t ch : text ) {
		if ( !( (ch >= L'0' && ch <= L'9') || ch == L'.' || ch == L'-' || ch == L'+' ) ) {
			return false;
		}
	}
	return text != L"-" && text != L"+" && text != L"." && text != L"-." && text != L"+.";
}

static void addComboItems(HWND combo, const std::vector<ComboItem> &items) {
	for (const ComboItem &item : items) {
		SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)(g_zh ? item.zh : item.en));
	}
}

static void selectComboByValue(HWND combo, const std::vector<ComboItem> &items, const std::string &value, int fallbackIndex) {
	int selected = fallbackIndex;
	for (int i = 0; i < (int)items.size(); i++) {
		if (value == items[i].value) {
			selected = i;
			break;
		}
	}
	SendMessageW(combo, CB_SETCURSEL, selected, 0);
}

static std::string comboValue(HWND combo, const std::vector<ComboItem> &items) {
	int selected = (int)SendMessageW(combo, CB_GETCURSEL, 0, 0);
	if ( selected < 0 || selected >= (int)items.size() ) {
		selected = 0;
	}
	return items[selected].value;
}

static bool isPresetOwnedControl(int id) {
	switch (id) {
		case IDC_ROTATION:
		case IDC_SCALE:
		case IDC_AI_SUPERRES:
		case IDC_INTERP:
		case IDC_CMAP:
		case IDC_TEMP_UNIT:
		case IDC_OFFSET:
		case IDC_DRIFT:
		case IDC_FILTER:
		case IDC_BILATERAL:
		case IDC_TEMPORAL:
		case IDC_SHARPEN:
		case IDC_FULLSCREEN:
		case IDC_BLUR:
		case IDC_THRESHOLD:
		case IDC_AUTORANGE:
		case IDC_MAPPING:
		case IDC_MANUAL_ENABLE:
		case IDC_MANUAL_MIN:
		case IDC_MANUAL_MAX:
		case IDC_ROI_MODE:
		case IDC_ROI_SIZE:
		case IDC_ISOTHERM_ENABLE:
		case IDC_ISOTHERM_THRESHOLD:
		case IDC_HISTOGRAM:
		case IDC_RULERS:
		case IDC_CONTRAST:
			return true;
		default:
			return false;
	}
}

static void markPresetCustomForManualChange(int id) {
	if ( ! isPresetOwnedControl(id) || comboValue(g_preset, g_presetItems) == "custom" ) {
		return;
	}
	g_suppressEvents = true;
	selectComboByValue(g_preset, g_presetItems, "custom", 0);
	g_suppressEvents = false;
}

static void reloadCombo(HWND combo, const std::vector<ComboItem> &items, const std::string &value, int fallbackIndex) {
	SendMessageW(combo, CB_RESETCONTENT, 0, 0);
	addComboItems(combo, items);
	selectComboByValue(combo, items, value, fallbackIndex);
}

static void initCmapItems() {
	if ( ! g_cmapValues.empty() ) {
		return;
	}
	static const wchar_t *labelsEn[] = {
		L"No pseudo-color", L"Autumn yellow-red", L"Inverse autumn", L"Bone gray-white",
		L"Jet rainbow heat", L"Inverse jet", L"Winter blue-green", L"Rainbow",
		L"Inverse rainbow", L"Ocean deep blue", L"Inverse ocean", L"Summer green-yellow",
		L"Spring magenta-yellow", L"Cool cyan-magenta", L"HSV full hue", L"Inverse HSV",
		L"Pink gray-pink", L"Hot iron red", L"Cold inverse hot", L"Parula blue-yellow",
		L"Magma black-red-yellow", L"Inferno black-red-yellow", L"Plasma purple-yellow",
		L"Viridis blue-green-yellow", L"Cividis balanced", L"Twilight cyclic",
		L"Twilight shifted", L"Turbo high contrast", L"Inverse turbo", L"Deep green",
		L"HSL hot-to-cold", L"HSL cold-to-hot", L"Red to blue", L"Cold to black linear",
		L"Cold to black curved", L"Cold to white linear", L"Cold to white curved"
	};
	static const wchar_t *labelsZh[] = {
		L"无伪彩", L"暖黄红", L"反向暖黄红", L"灰白骨色",
		L"Jet 彩虹热图", L"反向 Jet 彩虹", L"冬季蓝绿", L"彩虹",
		L"反向彩虹", L"海洋深蓝", L"反向海洋蓝", L"夏季绿黄",
		L"春季紫黄", L"冷色青紫", L"HSV 色相环", L"反向 HSV",
		L"粉灰", L"Hot 铁红", L"Cold 冷色", L"Parula 蓝黄",
		L"Magma 岩浆", L"Inferno 黑红黄", L"Plasma 紫黄高对比",
		L"Viridis 蓝绿黄", L"Cividis 均衡蓝黄", L"Twilight 暮光循环",
		L"Twilight Shift 移位暮光", L"Turbo 高对比", L"反向 Turbo", L"深绿",
		L"HSL 热到冷", L"HSL 冷到热", L"红蓝", L"冷色到黑 线性",
		L"冷色到黑 弧形", L"冷色到白 线性", L"冷色到白 弧形"
	};
	for (int i = 0; i < (int)(sizeof(labelsEn) / sizeof(labelsEn[0])); i++) {
		std::wostringstream en;
		std::wostringstream zh;
		en << labelsEn[i] << L" (#" << i << L")";
		zh << labelsZh[i] << L"（#" << i << L"）";
		g_cmapLabelsEn.push_back(en.str());
		g_cmapLabelsZh.push_back(zh.str());
		g_cmapValues.push_back(std::to_string(i));
	}
}

static void reloadCmapCombo(const std::string &value) {
	initCmapItems();
	SendMessageW(g_cmap, CB_RESETCONTENT, 0, 0);
	const std::vector<std::wstring> &labels = g_zh ? g_cmapLabelsZh : g_cmapLabelsEn;
	for (const std::wstring &label : labels) {
		SendMessageW(g_cmap, CB_ADDSTRING, 0, (LPARAM)label.c_str());
	}
	int selected = 4;
	for (int i = 0; i < (int)g_cmapValues.size(); i++) {
		if (value == g_cmapValues[i]) {
			selected = i;
			break;
		}
	}
	SendMessageW(g_cmap, CB_SETCURSEL, selected, 0);
}

static std::string selectedCmapValue() {
	initCmapItems();
	int selected = (int)SendMessageW(g_cmap, CB_GETCURSEL, 0, 0);
	if ( selected < 0 || selected >= (int)g_cmapValues.size() ) {
		selected = 4;
	}
	return g_cmapValues[selected];
}

static bool checked(HWND hwnd) {
	return BST_CHECKED == SendMessageW(hwnd, BM_GETCHECK, 0, 0);
}

static void setChecked(HWND hwnd, bool value) {
	SendMessageW(hwnd, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0);
}

static std::string boolValue(HWND hwnd) {
	return checked(hwnd) ? "1" : "0";
}

static std::wstring buildCommandLineFromValues(
	const std::string &deviceValue,
	const std::string &rotationValue,
	const std::string &scaleValue,
	const std::string &aiSuperresValue,
	const std::string &interpValue,
	const std::string &cmapValue,
	const std::string &tempUnitValue,
	const std::string &offsetValue,
	const std::string &driftValue,
	const std::string &filterValue,
	const std::string &bilateralValue,
	const std::string &temporalValue,
	const std::string &sharpenValue,
	bool fullscreen,
	const std::string &pipeName) {
	std::wstring dir = exeDirW();
	std::wstring exe = joinPathW(dir, L"Thermal-Camera-Redux.exe");
	std::wstring device = widenAscii(deviceValue.empty() ? "0" : deviceValue);
	std::wstring offset = widenAscii(offsetValue.empty() ? "0.0" : offsetValue);
	std::wstring drift = widenAscii(driftValue.empty() ? "0.0" : driftValue);

	std::wostringstream cmd;
	cmd << quoteArgW(exe)
	    << L" -uti260b"
	    << L" -d " << quoteArgW(device)
	    << L" -rotate " << quoteArgW(widenAscii(rotationValue))
	    << L" -display-scale " << quoteArgW(widenAscii(scaleValue))
	    << L" -ai-superres " << quoteArgW(widenAscii(aiSuperresValue))
	    << L" -interp " << quoteArgW(widenAscii(interpValue))
	    << L" -cmap " << quoteArgW(widenAscii(cmapValue))
	    << (tempUnitValue == "fahrenheit" ? L" -fahrenheit" : L" -celsius")
	    << L" -temp-offset-c " << quoteArgW(offset)
	    << L" -temp-drift-c-per-min " << quoteArgW(drift)
	    << L" -filter-preset " << quoteArgW(widenAscii(filterValue))
	    << L" -bilateral " << quoteArgW(widenAscii(bilateralValue))
	    << L" -temporal-denoise " << quoteArgW(widenAscii(temporalValue))
	    << L" -sharpen " << quoteArgW(widenAscii(sharpenValue))
	    << L" -control-pipe " << quoteArgW(widenAscii(pipeName));
	if ( fullscreen ) {
		cmd << L" -fullscreen";
	}
	return cmd.str();
}

static std::wstring buildCommandLine() {
	std::string previewPipe = g_pipeName.empty() ? "thermal-camera-redux-live" : g_pipeName;
	return buildCommandLineFromValues(
		textOfAscii(g_device),
		comboValue(g_rotation, g_rotationItems),
		comboValue(g_scale, g_scaleItems),
		comboValue(g_aiSuperres, g_aiSuperresItems),
		comboValue(g_interp, g_interpItems),
		selectedCmapValue(),
		comboValue(g_tempUnit, g_tempUnitItems),
		textOfAscii(g_offset),
		textOfAscii(g_drift),
		comboValue(g_filter, g_levelItems),
		comboValue(g_bilateral, g_levelItems),
		comboValue(g_temporal, g_levelItems),
		comboValue(g_sharpen, g_levelItems),
		checked(g_fullscreen),
		previewPipe);
}

static void writeLastCommand(const std::wstring &commandLine) {
	std::string text = utf8FromWide(commandLine) + "\n";
	HANDLE file = CreateFileW(joinPathW(exeDirW(), L"thermal-camera-redux-last-command.txt").c_str(),
		GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( INVALID_HANDLE_VALUE == file ) {
		return;
	}
	DWORD written = 0;
	WriteFile(file, text.data(), (DWORD)text.size(), &written, NULL);
	CloseHandle(file);
}

static void updatePreview() {
	if ( g_preview ) {
		SetWindowTextW(g_preview, buildCommandLine().c_str());
	}
}

static void saveSettings();

static void setStatus(const wchar_t *en, const wchar_t *zh) {
	SetWindowTextW(g_status, g_zh ? zh : en);
}

static void setDynamicStatus(const std::wstring &en, const std::wstring &zh) {
	SetWindowTextW(g_status, g_zh ? zh.c_str() : en.c_str());
}

static std::wstring calibrationStatusPathW() {
	return joinPathW(exeDirW(), L"thermal-camera-redux-calibration-status.txt");
}

static std::string readSmallTextFileW(const std::wstring &path) {
	HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( INVALID_HANDLE_VALUE == file ) {
		return std::string();
	}
	DWORD size = GetFileSize(file, NULL);
	if ( INVALID_FILE_SIZE == size || size > 4096 ) {
		CloseHandle(file);
		return std::string();
	}
	std::string text(size, '\0');
	DWORD read = 0;
	if ( size > 0 ) {
		ReadFile(file, &text[0], size, &read, NULL);
		text.resize(read);
	}
	CloseHandle(file);
	return text;
}

static std::string statusToken(const std::string &text, const std::string &key) {
	std::string needle = key + "=";
	size_t start = text.find(needle);
	if ( start == std::string::npos ) {
		return std::string();
	}
	start += needle.size();
	size_t end = text.find_first_of(" \r\n\t", start);
	if ( end == std::string::npos ) {
		end = text.size();
	}
	return text.substr(start, end - start);
}

static void applyCalibrationStatusText(HWND hwnd, const std::string &text) {
	std::string status = statusToken(text, "status");
	std::string reason = statusToken(text, "reason");
	std::string target = statusToken(text, "target_c");
	std::string measured = statusToken(text, "measured_c");
	std::string delta = statusToken(text, "delta_c");
	std::string offset = statusToken(text, "offset_c");
	std::string driftReset = statusToken(text, "drift_reset");
	std::string roi = statusToken(text, "roi_percent");
	if ( status == "ok" ) {
		g_suppressEvents = true;
		setText(g_offset, widenAscii(offset));
		if ( driftReset == "1" ) {
			setText(g_drift, L"0.0");
		}
		g_suppressEvents = false;
		saveSettings();
		updatePreview();

		std::wostringstream en;
		en << L"Blackbody calibrated: target " << widenAscii(target)
		   << L" C, measured " << widenAscii(measured)
		   << L" C, delta " << widenAscii(delta)
		   << L" C, offset " << widenAscii(offset)
		   << L" C, ROI " << widenAscii(roi) << L"%.";
		std::wostringstream zh;
		zh << L"黑体校准完成：目标 " << widenAscii(target)
		   << L" C，测量 " << widenAscii(measured)
		   << L" C，补偿 " << widenAscii(delta)
		   << L" C，偏移 " << widenAscii(offset)
		   << L" C，ROI " << widenAscii(roi) << L"%。";
		if ( driftReset == "1" ) {
			en << L" Drift was folded into offset and reset.";
			zh << L" 温漂已折算进偏移并清零。";
		}
		setDynamicStatus(en.str(), zh.str());
	} else {
		std::wostringstream en;
		en << L"Blackbody calibration failed: " << widenAscii(reason.empty() ? "unknown" : reason) << L".";
		std::wostringstream zh;
		zh << L"黑体校准失败：" << widenAscii(reason.empty() ? "unknown" : reason) << L"。";
		setDynamicStatus(en.str(), zh.str());
	}
	KillTimer(hwnd, TIMER_CALIBRATION_STATUS);
	g_pendingCalibrationId.clear();
	g_calibrationPolls = 0;
}

static void pollCalibrationStatus(HWND hwnd) {
	if ( g_pendingCalibrationId.empty() ) {
		KillTimer(hwnd, TIMER_CALIBRATION_STATUS);
		return;
	}
	std::string text = readSmallTextFileW(calibrationStatusPathW());
	if ( ! text.empty() && statusToken(text, "id") == g_pendingCalibrationId ) {
		applyCalibrationStatusText(hwnd, text);
		return;
	}
	g_calibrationPolls++;
	if ( g_calibrationPolls > 24 ) {
		KillTimer(hwnd, TIMER_CALIBRATION_STATUS);
		g_pendingCalibrationId.clear();
		g_calibrationPolls = 0;
		setStatus(L"Calibration command sent, but no result was returned.", L"已发送校准命令，但未收到结果。");
	}
}

static void saveSettings() {
	writeIni("language", comboValue(g_language, g_languageItems));
	writeIni("device", textOfAscii(g_device));
	writeIni("rotation", comboValue(g_rotation, g_rotationItems));
	writeIni("scale", comboValue(g_scale, g_scaleItems));
	writeIni("ai_superres", comboValue(g_aiSuperres, g_aiSuperresItems));
	writeIni("interp", comboValue(g_interp, g_interpItems));
	writeIni("cmap", selectedCmapValue());
	writeIni("temp_unit", comboValue(g_tempUnit, g_tempUnitItems));
	writeIni("offset_c", textOfAscii(g_offset));
	writeIni("drift_c_per_min", textOfAscii(g_drift));
	writeIni("blackbody_target_c", textOfAscii(g_blackbodyTarget));
	writeIni("filter", comboValue(g_filter, g_levelItems));
	writeIni("bilateral", comboValue(g_bilateral, g_levelItems));
	writeIni("temporal", comboValue(g_temporal, g_levelItems));
	writeIni("sharpen", comboValue(g_sharpen, g_levelItems));
	writeIni("fullscreen", boolValue(g_fullscreen));
	writeIni("preset", comboValue(g_preset, g_presetItems));
	writeIni("blur", comboValue(g_blur, g_blurItems));
	writeIni("contrast", comboValue(g_contrast, g_contrastItems));
	writeIni("threshold_c", textOfAscii(g_threshold));
	writeIni("autorange", comboValue(g_autorange, g_autorangeItems));
	writeIni("mapping", comboValue(g_mapping, g_mappingItems));
	writeIni("manual_enable", boolValue(g_manualEnable));
	writeIni("manual_min_c", textOfAscii(g_manualMin));
	writeIni("manual_max_c", textOfAscii(g_manualMax));
	writeIni("roi_mode", comboValue(g_roiMode, g_roiItems));
	writeIni("roi_size", comboValue(g_roiSize, g_roiSizeItems));
	writeIni("isotherm_enable", boolValue(g_isothermEnable));
	writeIni("isotherm_threshold_c", textOfAscii(g_isothermThreshold));
	writeIni("histogram", boolValue(g_histogram));
	writeIni("rulers", comboValue(g_rulers, g_rulerItems));
	writeIni("timelapse_interval_s", textOfAscii(g_timelapseInterval));
}

static void applyLanguageToUi() {
	SetWindowTextW(g_main, g_zh ? APP_TITLE_ZH : APP_TITLE_EN);
	for (const TextBinding &binding : g_textBindings) {
		SetWindowTextW(binding.hwnd, g_zh ? binding.zh : binding.en);
	}

	g_suppressEvents = true;
	reloadCombo(g_language, g_languageItems, g_zh ? "zh-CN" : "en-US", g_zh ? 0 : 1);
	reloadCombo(g_rotation, g_rotationItems, comboValue(g_rotation, g_rotationItems), 1);
	reloadCombo(g_scale, g_scaleItems, comboValue(g_scale, g_scaleItems), 3);
	reloadCombo(g_aiSuperres, g_aiSuperresItems, comboValue(g_aiSuperres, g_aiSuperresItems), 0);
	reloadCombo(g_interp, g_interpItems, comboValue(g_interp, g_interpItems), 2);
	reloadCmapCombo(selectedCmapValue());
	reloadCombo(g_tempUnit, g_tempUnitItems, comboValue(g_tempUnit, g_tempUnitItems), 0);
	reloadCombo(g_filter, g_levelItems, comboValue(g_filter, g_levelItems), 0);
	reloadCombo(g_bilateral, g_levelItems, comboValue(g_bilateral, g_levelItems), 0);
	reloadCombo(g_temporal, g_levelItems, comboValue(g_temporal, g_levelItems), 0);
	reloadCombo(g_sharpen, g_levelItems, comboValue(g_sharpen, g_levelItems), 0);
	reloadCombo(g_preset, g_presetItems, comboValue(g_preset, g_presetItems), 0);
	reloadCombo(g_blur, g_blurItems, comboValue(g_blur, g_blurItems), 0);
	reloadCombo(g_contrast, g_contrastItems, comboValue(g_contrast, g_contrastItems), 0);
	reloadCombo(g_autorange, g_autorangeItems, comboValue(g_autorange, g_autorangeItems), 0);
	reloadCombo(g_mapping, g_mappingItems, comboValue(g_mapping, g_mappingItems), 0);
	reloadCombo(g_roiMode, g_roiItems, comboValue(g_roiMode, g_roiItems), 0);
	reloadCombo(g_roiSize, g_roiSizeItems, comboValue(g_roiSize, g_roiSizeItems), 1);
	reloadCombo(g_rulers, g_rulerItems, comboValue(g_rulers, g_rulerItems), 0);
	g_suppressEvents = false;
	updatePreview();
}

static void resetDefaults() {
	g_suppressEvents = true;
	setText(g_device, L"0");
	selectComboByValue(g_rotation, g_rotationItems, "90", 1);
	selectComboByValue(g_scale, g_scaleItems, "4", 3);
	selectComboByValue(g_aiSuperres, g_aiSuperresItems, "off", 0);
	selectComboByValue(g_interp, g_interpItems, "lanczos", 3);
	reloadCmapCombo("4");
	selectComboByValue(g_tempUnit, g_tempUnitItems, "celsius", 0);
	setText(g_offset, L"0.0");
	setText(g_drift, L"0.0");
	setText(g_blackbodyTarget, L"35.0");
	selectComboByValue(g_filter, g_levelItems, "off", 0);
	selectComboByValue(g_bilateral, g_levelItems, "off", 0);
	selectComboByValue(g_temporal, g_levelItems, "off", 0);
	selectComboByValue(g_sharpen, g_levelItems, "off", 0);
	setChecked(g_fullscreen, false);
	selectComboByValue(g_preset, g_presetItems, "custom", 0);
	selectComboByValue(g_blur, g_blurItems, "0", 0);
	selectComboByValue(g_contrast, g_contrastItems, "1.0", 0);
	setText(g_threshold, L"2.0");
	selectComboByValue(g_autorange, g_autorangeItems, "0", 0);
	selectComboByValue(g_mapping, g_mappingItems, "1", 0);
	setChecked(g_manualEnable, false);
	setText(g_manualMin, L"20.0");
	setText(g_manualMax, L"45.0");
	selectComboByValue(g_roiMode, g_roiItems, "off", 0);
	selectComboByValue(g_roiSize, g_roiSizeItems, "25", 1);
	setChecked(g_isothermEnable, false);
	setText(g_isothermThreshold, L"60.0");
	setChecked(g_histogram, false);
	selectComboByValue(g_rulers, g_rulerItems, "0", 0);
	setText(g_timelapseInterval, L"10");
	g_suppressEvents = false;
	updatePreview();
}

static void loadSettings() {
	g_suppressEvents = true;
	g_zh = readIni("language", "zh-CN") != "en-US";
	reloadCombo(g_language, g_languageItems, g_zh ? "zh-CN" : "en-US", g_zh ? 0 : 1);
	setText(g_device, widenAscii(readIni("device", "0")));
	selectComboByValue(g_rotation, g_rotationItems, readIni("rotation", "90"), 1);
	selectComboByValue(g_scale, g_scaleItems, readIni("scale", "4"), 3);
	selectComboByValue(g_aiSuperres, g_aiSuperresItems, readIni("ai_superres", "off"), 0);
	selectComboByValue(g_interp, g_interpItems, readIni("interp", "lanczos"), 3);
	reloadCmapCombo(readIni("cmap", "4"));
	selectComboByValue(g_tempUnit, g_tempUnitItems, readIni("temp_unit", "celsius"), 0);
	setText(g_offset, widenAscii(readIni("offset_c", "0.0")));
	setText(g_drift, widenAscii(readIni("drift_c_per_min", "0.0")));
	setText(g_blackbodyTarget, widenAscii(readIni("blackbody_target_c", "35.0")));
	selectComboByValue(g_filter, g_levelItems, readIni("filter", "off"), 0);
	selectComboByValue(g_bilateral, g_levelItems, readIni("bilateral", "off"), 0);
	selectComboByValue(g_temporal, g_levelItems, readIni("temporal", "off"), 0);
	selectComboByValue(g_sharpen, g_levelItems, readIni("sharpen", "off"), 0);
	setChecked(g_fullscreen, readIni("fullscreen", "0") == "1");
	selectComboByValue(g_preset, g_presetItems, readIni("preset", "custom"), 0);
	selectComboByValue(g_blur, g_blurItems, readIni("blur", "0"), 0);
	selectComboByValue(g_contrast, g_contrastItems, readIni("contrast", "1.0"), 0);
	setText(g_threshold, widenAscii(readIni("threshold_c", "2.0")));
	selectComboByValue(g_autorange, g_autorangeItems, readIni("autorange", "0"), 0);
	selectComboByValue(g_mapping, g_mappingItems, readIni("mapping", "1"), 0);
	setChecked(g_manualEnable, readIni("manual_enable", "0") == "1");
	setText(g_manualMin, widenAscii(readIni("manual_min_c", "20.0")));
	setText(g_manualMax, widenAscii(readIni("manual_max_c", "45.0")));
	selectComboByValue(g_roiMode, g_roiItems, readIni("roi_mode", "off"), 0);
	selectComboByValue(g_roiSize, g_roiSizeItems, readIni("roi_size", "25"), 1);
	setChecked(g_isothermEnable, readIni("isotherm_enable", "0") == "1");
	setText(g_isothermThreshold, widenAscii(readIni("isotherm_threshold_c", "60.0")));
	setChecked(g_histogram, readIni("histogram", "0") == "1");
	selectComboByValue(g_rulers, g_rulerItems, readIni("rulers", "0"), 0);
	setText(g_timelapseInterval, widenAscii(readIni("timelapse_interval_s", "10")));
	g_suppressEvents = false;
	applyLanguageToUi();
}

static HWND addLabel(HWND parent, const wchar_t *en, const wchar_t *zh, int x, int y, int w, int h) {
	HWND hwnd = CreateWindowExW(0, L"STATIC", g_zh ? zh : en, WS_CHILD | WS_VISIBLE,
		x, y, w, h, parent, NULL, NULL, NULL);
	setFont(hwnd);
	bindText(hwnd, en, zh);
	return hwnd;
}

static HWND addEdit(HWND parent, int id, int x, int y, int w, int h) {
	HWND hwnd = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL);
	setFont(hwnd);
	return hwnd;
}

static HWND addCombo(HWND parent, int id, int x, int y, int w, int h) {
	HWND hwnd = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
		x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL);
	setFont(hwnd);
	return hwnd;
}

static HWND addButton(HWND parent, int id, const wchar_t *en, const wchar_t *zh, int x, int y, int w, int h) {
	HWND hwnd = CreateWindowExW(0, L"BUTTON", g_zh ? zh : en, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL);
	setFont(hwnd);
	bindText(hwnd, en, zh);
	return hwnd;
}

static HWND addCheckbox(HWND parent, int id, const wchar_t *en, const wchar_t *zh, int x, int y, int w, int h) {
	HWND hwnd = CreateWindowExW(0, L"BUTTON", g_zh ? zh : en, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL);
	setFont(hwnd);
	bindText(hwnd, en, zh);
	return hwnd;
}

static void createControls(HWND hwnd) {
	g_main = hwnd;
	g_font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	int leftX = 18;
	int rightColX = 500;
	int labelW = 126;
	int controlW = 190;
	int leftControlX = leftX + labelW + 10;
	int rightControlX = rightColX + labelW + 10;
	int rowH = 23;
	int gap = 27;

	int yLeft = 14;
	int yRight = 14;

	addLabel(hwnd, L"Connection", L"连接", leftX, yLeft, labelW + controlW, rowH);
	yLeft += gap;
	addLabel(hwnd, L"Language", L"语言", leftX, yLeft, labelW, rowH);
	g_language = addCombo(hwnd, IDC_LANGUAGE, leftControlX, yLeft - 2, controlW, 160);
	addComboItems(g_language, g_languageItems);
	yLeft += gap;

	addLabel(hwnd, L"Device index", L"设备序号", leftX, yLeft, labelW, rowH);
	g_device = addEdit(hwnd, IDC_DEVICE, leftControlX, yLeft - 2, 74, rowH);
	addLabel(hwnd, L"Restart needed", L"需重启", leftControlX + 84, yLeft, 88, rowH);
	yLeft += gap;

	addLabel(hwnd, L"Preset", L"预设方案", leftX, yLeft, labelW, rowH);
	g_preset = addCombo(hwnd, IDC_PRESET, leftControlX, yLeft - 2, controlW, 180);
	addComboItems(g_preset, g_presetItems);
	yLeft += gap;

	addLabel(hwnd, L"Rotation", L"旋转", leftX, yLeft, labelW, rowH);
	g_rotation = addCombo(hwnd, IDC_ROTATION, leftControlX, yLeft - 2, controlW, 180);
	addComboItems(g_rotation, g_rotationItems);
	yLeft += gap + 8;

	addLabel(hwnd, L"Display", L"显示", leftX, yLeft, labelW + controlW, rowH);
	yLeft += gap;
	addLabel(hwnd, L"Super-res scale", L"软件超分倍率", leftX, yLeft, labelW, rowH);
	g_scale = addCombo(hwnd, IDC_SCALE, leftControlX, yLeft - 2, controlW, 180);
	addComboItems(g_scale, g_scaleItems);
	yLeft += gap;

	addLabel(hwnd, L"AI super-res", L"AI 超分", leftX, yLeft, labelW, rowH);
	g_aiSuperres = addCombo(hwnd, IDC_AI_SUPERRES, leftControlX, yLeft - 2, controlW + 90, 180);
	addComboItems(g_aiSuperres, g_aiSuperresItems);
	yLeft += gap;

	addLabel(hwnd, L"Interpolation", L"插值算法", leftX, yLeft, labelW, rowH);
	g_interp = addCombo(hwnd, IDC_INTERP, leftControlX, yLeft - 2, controlW, 180);
	addComboItems(g_interp, g_interpItems);
	yLeft += gap;

	addLabel(hwnd, L"Colormap", L"伪彩色", leftX, yLeft, labelW, rowH);
	g_cmap = addCombo(hwnd, IDC_CMAP, leftControlX, yLeft - 2, controlW + 90, 360);
	reloadCmapCombo("4");
	yLeft += gap;

	addLabel(hwnd, L"Temperature unit", L"温度单位", leftX, yLeft, labelW, rowH);
	g_tempUnit = addCombo(hwnd, IDC_TEMP_UNIT, leftControlX, yLeft - 2, controlW, 120);
	addComboItems(g_tempUnit, g_tempUnitItems);
	yLeft += gap + 8;

	addLabel(hwnd, L"Enhancement", L"画质增强", leftX, yLeft, labelW + controlW, rowH);
	yLeft += gap;
	addLabel(hwnd, L"Blur", L"模糊", leftX, yLeft, labelW, rowH);
	g_blur = addCombo(hwnd, IDC_BLUR, leftControlX, yLeft - 2, controlW, 180);
	addComboItems(g_blur, g_blurItems);
	yLeft += gap;

	addLabel(hwnd, L"Contrast / gain", L"对比/增益", leftX, yLeft, labelW, rowH);
	g_contrast = addCombo(hwnd, IDC_CONTRAST, leftControlX, yLeft - 2, controlW, 180);
	addComboItems(g_contrast, g_contrastItems);
	yLeft += gap;

	addLabel(hwnd, L"Gaussian filter", L"高斯滤波", leftX, yLeft, labelW, rowH);
	g_filter = addCombo(hwnd, IDC_FILTER, leftControlX, yLeft - 2, controlW, 180);
	addComboItems(g_filter, g_levelItems);
	yLeft += gap;

	addLabel(hwnd, L"Bilateral", L"双边滤波", leftX, yLeft, labelW, rowH);
	g_bilateral = addCombo(hwnd, IDC_BILATERAL, leftControlX, yLeft - 2, controlW, 180);
	addComboItems(g_bilateral, g_levelItems);
	yLeft += gap;

	addLabel(hwnd, L"Temporal denoise", L"时域降噪", leftX, yLeft, labelW, rowH);
	g_temporal = addCombo(hwnd, IDC_TEMPORAL, leftControlX, yLeft - 2, controlW, 180);
	addComboItems(g_temporal, g_levelItems);
	yLeft += gap;

	addLabel(hwnd, L"Sharpen", L"锐化", leftX, yLeft, labelW, rowH);
	g_sharpen = addCombo(hwnd, IDC_SHARPEN, leftControlX, yLeft - 2, controlW, 180);
	addComboItems(g_sharpen, g_levelItems);
	yLeft += gap;

	addLabel(hwnd, L"Temperature", L"温度校准", rightColX, yRight, labelW + controlW, rowH);
	yRight += gap;
	addLabel(hwnd, L"Temp offset C", L"温度偏移 C", rightColX, yRight, labelW, rowH);
	g_offset = addEdit(hwnd, IDC_OFFSET, rightControlX, yRight - 2, 88, rowH);
	yRight += gap;

	addLabel(hwnd, L"Drift C/min", L"温漂 C/分钟", rightColX, yRight, labelW, rowH);
	g_drift = addEdit(hwnd, IDC_DRIFT, rightControlX, yRight - 2, 88, rowH);
	addLabel(hwnd, L"Default 0", L"默认 0", rightControlX + 98, yRight, 82, rowH);
	yRight += gap;

	addLabel(hwnd, L"Blackbody target C", L"黑体目标 C", rightColX, yRight, labelW, rowH);
	g_blackbodyTarget = addEdit(hwnd, IDC_BLACKBODY_TARGET, rightControlX, yRight - 2, 88, rowH);
	g_blackbodyCalibrate = addButton(hwnd, IDC_BLACKBODY_CALIBRATE, L"Calibrate", L"黑体校准", rightControlX + 98, yRight - 4, 112, 28);
	yRight += gap;

	addLabel(hwnd, L"Min/max delta C", L"极值阈值 C", rightColX, yRight, labelW, rowH);
	g_threshold = addEdit(hwnd, IDC_THRESHOLD, rightControlX, yRight - 2, 88, rowH);
	yRight += gap;

	addLabel(hwnd, L"Auto range", L"自动范围锁定", rightColX, yRight, labelW, rowH);
	g_autorange = addCombo(hwnd, IDC_AUTORANGE, rightControlX, yRight - 2, controlW, 180);
	addComboItems(g_autorange, g_autorangeItems);
	yRight += gap;

	addLabel(hwnd, L"Range mapping", L"范围映射", rightColX, yRight, labelW, rowH);
	g_mapping = addCombo(hwnd, IDC_MAPPING, rightControlX, yRight - 2, controlW, 180);
	addComboItems(g_mapping, g_mappingItems);
	yRight += gap;

	g_manualEnable = addCheckbox(hwnd, IDC_MANUAL_ENABLE, L"Manual temp range", L"手动温度范围", rightColX, yRight - 2, 150, rowH);
	addLabel(hwnd, L"Min / Max C", L"最小/最大 C", rightControlX, yRight, 82, rowH);
	g_manualMin = addEdit(hwnd, IDC_MANUAL_MIN, rightControlX + 86, yRight - 2, 52, rowH);
	g_manualMax = addEdit(hwnd, IDC_MANUAL_MAX, rightControlX + 144, yRight - 2, 52, rowH);
	yRight += gap + 8;

	addLabel(hwnd, L"Analysis", L"分析叠加", rightColX, yRight, labelW + controlW, rowH);
	yRight += gap;
	addLabel(hwnd, L"ROI mode", L"ROI 模式", rightColX, yRight, labelW, rowH);
	g_roiMode = addCombo(hwnd, IDC_ROI_MODE, rightControlX, yRight - 2, 156, 180);
	addComboItems(g_roiMode, g_roiItems);
	addLabel(hwnd, L"Size", L"大小", rightControlX + 166, yRight, 38, rowH);
	g_roiSize = addCombo(hwnd, IDC_ROI_SIZE, rightControlX + 208, yRight - 2, 74, 160);
	addComboItems(g_roiSize, g_roiSizeItems);
	yRight += gap;

	g_isothermEnable = addCheckbox(hwnd, IDC_ISOTHERM_ENABLE, L"Isotherm / over-temp", L"等温/过温高亮", rightColX, yRight - 2, 150, rowH);
	addLabel(hwnd, L"Threshold C", L"阈值 C", rightControlX, yRight, 82, rowH);
	g_isothermThreshold = addEdit(hwnd, IDC_ISOTHERM_THRESHOLD, rightControlX + 86, yRight - 2, 74, rowH);
	yRight += gap;

	g_histogram = addCheckbox(hwnd, IDC_HISTOGRAM, L"Histogram equalize", L"直方图均衡", rightColX, yRight - 2, 150, rowH);
	g_fullscreen = addCheckbox(hwnd, IDC_FULLSCREEN, L"Fullscreen", L"全屏", rightControlX, yRight - 2, 120, rowH);
	yRight += gap;

	addLabel(hwnd, L"Rulers", L"标尺", rightColX, yRight, labelW, rowH);
	g_rulers = addCombo(hwnd, IDC_RULERS, rightControlX, yRight - 2, controlW, 180);
	addComboItems(g_rulers, g_rulerItems);
	yRight += gap + 8;

	addLabel(hwnd, L"Actions", L"操作", rightColX, yRight, labelW + controlW, rowH);
	yRight += gap;
	g_start = addButton(hwnd, IDC_START, L"Start", L"启动", rightColX, yRight, 82, 28);
	g_stop = addButton(hwnd, IDC_STOP, L"Stop", L"停止", rightColX + 90, yRight, 82, 28);
	g_snapshot = addButton(hwnd, IDC_SNAPSHOT, L"Snapshot", L"截图+RAW", rightColX + 180, yRight, 96, 28);
	g_record = addButton(hwnd, IDC_RECORD, L"Record", L"录像", rightColX + 284, yRight, 82, 28);
	yRight += 36;

	g_runtimeReset = addButton(hwnd, IDC_RUNTIME_RESET, L"Runtime reset", L"运行中复位", rightColX, yRight, 116, 28);
	g_resetDefaults = addButton(hwnd, IDC_RESET_DEFAULTS, L"Defaults", L"默认参数", rightColX + 126, yRight, 100, 28);
	g_openManual = addButton(hwnd, IDC_OPEN_MANUAL, L"Manual", L"说明书", rightColX + 236, yRight, 100, 28);
	yRight += 38;

	addLabel(hwnd, L"Timelapse seconds", L"连拍间隔秒", rightColX, yRight, 108, rowH);
	g_timelapseInterval = addEdit(hwnd, IDC_TIMELAPSE_INTERVAL, rightColX + 112, yRight - 2, 58, rowH);
	g_timelapseStart = addButton(hwnd, IDC_TIMELAPSE_START, L"Start", L"开始连拍", rightColX + 178, yRight - 4, 88, 28);
	g_timelapseStop = addButton(hwnd, IDC_TIMELAPSE_STOP, L"Stop", L"停止连拍", rightColX + 274, yRight - 4, 88, 28);
	g_timelapseFolder = addButton(hwnd, IDC_TIMELAPSE_FOLDER, L"Folder", L"打开目录", rightColX + 370, yRight - 4, 88, 28);
	EnableWindow(g_timelapseStop, FALSE);

	int commandY = 560;
	addLabel(hwnd, L"Command", L"启动命令", leftX, commandY, labelW, rowH);
	g_preview = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
		leftControlX, commandY - 2, 790, 58, hwnd, (HMENU)(INT_PTR)IDC_PREVIEW, NULL, NULL);
	setFont(g_preview);

	g_status = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE,
		leftX, commandY + 72, 928, 24, hwnd, (HMENU)(INT_PTR)IDC_STATUS, NULL, NULL);
	setFont(g_status);

	loadSettings();
	setStatus(L"Ready.", L"就绪。");
}

static std::string generatePipeName() {
	std::ostringstream pipe;
	pipe << "ThermalCameraReduxControl-" << GetCurrentProcessId() << "-" << GetTickCount64();
	return pipe.str();
}

static void closeControlPipe() {
	if ( INVALID_HANDLE_VALUE != g_pipe ) {
		CloseHandle(g_pipe);
		g_pipe = INVALID_HANDLE_VALUE;
	}
}

static bool isProcessRunning() {
	if ( ! g_process ) {
		return false;
	}
	DWORD code = 0;
		if ( ! GetExitCodeProcess(g_process, &code) ) {
			closeControlPipe();
			CloseHandle(g_process);
			g_process = NULL;
			g_processId = 0;
			return false;
	}
	if ( STILL_ACTIVE == code ) {
		return true;
	}
	closeControlPipe();
	CloseHandle(g_process);
	g_process = NULL;
	g_processId = 0;
	return false;
}

static bool ensureControlPipeOpen() {
	if ( ! isProcessRunning() || g_pipeName.empty() ) {
		return false;
	}
	if ( INVALID_HANDLE_VALUE != g_pipe ) {
		return true;
	}
	std::string pipePath = "\\\\.\\pipe\\" + g_pipeName;
	for (int attempt = 0; attempt < 8; attempt++) {
		g_pipe = CreateFileA(pipePath.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if ( INVALID_HANDLE_VALUE != g_pipe ) {
			return true;
		}
		WaitNamedPipeA(pipePath.c_str(), 200);
		Sleep(60);
	}
	setStatus(L"Live control pipe is not ready yet.", L"实时控制管道暂未就绪。");
	return false;
}

static bool sendControlText(const std::string &text) {
	if ( ! ensureControlPipeOpen() ) {
		return false;
	}
	DWORD written = 0;
	BOOL ok = WriteFile(g_pipe, text.data(), (DWORD)text.size(), &written, NULL);
	if ( ! ok || written != text.size() ) {
		closeControlPipe();
		return false;
	}
	return ok && written == text.size();
}

static void terminateCameraProcess() {
	if ( g_process ) {
		TerminateProcess(g_process, 0);
		WaitForSingleObject(g_process, 1500);
		closeControlPipe();
		CloseHandle(g_process);
		g_process = NULL;
		g_processId = 0;
		g_pipeName.clear();
		g_timelapseActive = false;
		EnableWindow(g_timelapseStart, TRUE);
		EnableWindow(g_timelapseStop, FALSE);
		updatePreview();
	}
}

static void appendSet(std::ostringstream &cmd, const char *key, const std::string &value) {
	cmd << "set " << key << " " << value << "\n";
}

static std::string manualRangeCommand() {
	std::ostringstream cmd;
	cmd << "set manual-range " << (checked(g_manualEnable) ? "on" : "off")
	    << " " << textOfAscii(g_manualMin)
	    << " " << textOfAscii(g_manualMax) << "\n";
	return cmd.str();
}

static std::string allLiveSettingsCommand() {
	std::ostringstream cmd;
	appendSet(cmd, "rotation", comboValue(g_rotation, g_rotationItems));
	appendSet(cmd, "scale", comboValue(g_scale, g_scaleItems));
	appendSet(cmd, "ai-superres", comboValue(g_aiSuperres, g_aiSuperresItems));
	appendSet(cmd, "interp", comboValue(g_interp, g_interpItems));
	appendSet(cmd, "cmap", selectedCmapValue());
	appendSet(cmd, "temp-unit", comboValue(g_tempUnit, g_tempUnitItems));
	appendSet(cmd, "offset-c", textOfAscii(g_offset));
	appendSet(cmd, "drift-c-per-min", textOfAscii(g_drift));
	appendSet(cmd, "blur", comboValue(g_blur, g_blurItems));
	appendSet(cmd, "contrast", comboValue(g_contrast, g_contrastItems));
	appendSet(cmd, "filter", comboValue(g_filter, g_levelItems));
	appendSet(cmd, "bilateral", comboValue(g_bilateral, g_levelItems));
	appendSet(cmd, "temporal", comboValue(g_temporal, g_levelItems));
	appendSet(cmd, "sharpen", comboValue(g_sharpen, g_levelItems));
	appendSet(cmd, "threshold-c", textOfAscii(g_threshold));
	appendSet(cmd, "autorange", comboValue(g_autorange, g_autorangeItems));
	appendSet(cmd, "mapping-filter", comboValue(g_mapping, g_mappingItems));
	cmd << manualRangeCommand();
	appendSet(cmd, "roi", comboValue(g_roiMode, g_roiItems));
	appendSet(cmd, "roi-size", comboValue(g_roiSize, g_roiSizeItems));
	appendSet(cmd, "isotherm", boolValue(g_isothermEnable));
	appendSet(cmd, "isotherm-threshold-c", textOfAscii(g_isothermThreshold));
	appendSet(cmd, "histogram", boolValue(g_histogram));
	appendSet(cmd, "rulers", comboValue(g_rulers, g_rulerItems));
	appendSet(cmd, "fullscreen", boolValue(g_fullscreen));
	return cmd.str();
}

static void sendLiveCommand(const std::string &command) {
	if ( command.empty() ) {
		return;
	}
	std::string text = command;
	if ( text.back() != '\n' ) {
		text += "\n";
	}
	if ( sendControlText(text) ) {
		setStatus(L"Live setting applied.", L"实时参数已应用。");
	}
}

static void sendBlackbodyCalibration(HWND hwnd) {
	if ( ! isProcessRunning() ) {
		setStatus(L"Start the camera before calibration.", L"请先启动相机再校准。");
		return;
	}
	if ( ! looksNumeric(g_blackbodyTarget) ) {
		setStatus(L"Blackbody target temperature is not numeric.", L"黑体目标温度不是数字。");
		return;
	}

	g_suppressEvents = true;
	selectComboByValue(g_roiMode, g_roiItems, "rect", 2);
	g_suppressEvents = false;
	updatePreview();
	saveSettings();

	std::ostringstream id;
	id << GetCurrentProcessId() << "-" << GetTickCount64();
	g_pendingCalibrationId = id.str();
	g_calibrationPolls = 0;

	std::ostringstream cmd;
	cmd << "calibrate blackbody "
	    << textOfAscii(g_blackbodyTarget)
	    << " " << comboValue(g_roiSize, g_roiSizeItems)
	    << " " << g_pendingCalibrationId
	    << "\n";
	if ( sendControlText(cmd.str()) ) {
		SetTimer(hwnd, TIMER_CALIBRATION_STATUS, 250, NULL);
		setStatus(L"Blackbody calibration sent; waiting for result.", L"已发送黑体校准，等待结果。");
	} else {
		g_pendingCalibrationId.clear();
		setStatus(L"Live control pipe is not ready; calibration was not sent.", L"实时控制管道未就绪，校准未发送。");
	}
}

static void openManual(HWND hwnd) {
	std::wstring manual = manualPathW();
	if ( ! fileExistsW(manual) ) {
		MessageBoxW(hwnd,
			g_zh ? L"说明书文件不在启动器旁边。" : L"The manual file was not found next to the launcher.",
			g_zh ? APP_TITLE_ZH : APP_TITLE_EN,
			MB_ICONERROR | MB_OK);
		return;
	}

	std::wstring commandLine = L"notepad.exe " + quoteArgW(manual);
	STARTUPINFOW si = {};
	PROCESS_INFORMATION pi = {};
	si.cb = sizeof(si);
	std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
	mutableCommand.push_back(L'\0');
	if ( CreateProcessW(NULL, mutableCommand.data(), NULL, NULL, FALSE, 0, NULL, exeDirW().c_str(), &si, &pi) ) {
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		setStatus(L"Manual opened.", L"说明书已打开。");
	} else {
		MessageBoxW(hwnd,
			g_zh ? L"无法打开说明书。" : L"Failed to open the manual.",
			g_zh ? APP_TITLE_ZH : APP_TITLE_EN,
			MB_ICONERROR | MB_OK);
	}
}

static bool createTimelapseSession(std::string &sessionName, std::wstring &outputFolder) {
	std::wstring root = joinPathW(exeDirW(), L"timelapse");
	if ( ! createDirectoryIfMissingW(root) ) {
		return false;
	}

	SYSTEMTIME now = {};
	GetLocalTime(&now);
	for (int attempt = 0; attempt < 100; attempt++) {
		std::wostringstream name;
		name << L"session-"
		     << std::setfill(L'0') << std::setw(4) << now.wYear
		     << std::setw(2) << now.wMonth
		     << std::setw(2) << now.wDay
		     << L"-"
		     << std::setw(2) << now.wHour
		     << std::setw(2) << now.wMinute
		     << std::setw(2) << now.wSecond
		     << L"-"
		     << std::setw(5) << ((GetTickCount64() + attempt) % 100000);
		std::wstring candidateName = name.str();
		std::wstring candidatePath = joinPathW(root, candidateName);
		if ( CreateDirectoryW(candidatePath.c_str(), NULL) ) {
			sessionName = narrowAscii(candidateName);
			outputFolder = candidatePath;
			return true;
		}
		if ( ERROR_ALREADY_EXISTS != GetLastError() ) {
			return false;
		}
	}
	return false;
}

static void startTimelapse() {
	if ( ! isProcessRunning() ) {
		setStatus(L"Start the camera before timelapse capture.", L"请先启动相机再开始连拍。");
		return;
	}
	if ( ! looksNumeric(g_timelapseInterval) ) {
		setStatus(L"Timelapse interval is not numeric.", L"连拍间隔不是数字。");
		return;
	}

	std::string intervalText = textOfAscii(g_timelapseInterval);
	char *intervalEnd = NULL;
	double intervalSeconds = std::strtod(intervalText.c_str(), &intervalEnd);
	if ( intervalEnd == intervalText.c_str() || '\0' != *intervalEnd ||
	     intervalSeconds < 1.0 || intervalSeconds > 86400.0 ) {
		setStatus(L"Timelapse interval must be 1 to 86400 seconds.", L"连拍间隔必须是 1 到 86400 秒。");
		return;
	}
	if ( ! ensureControlPipeOpen() ) {
		return;
	}

	std::string sessionName;
	std::wstring outputFolder;
	if ( ! createTimelapseSession(sessionName, outputFolder) ) {
		setStatus(L"Failed to create the timelapse output folder.", L"无法创建连拍输出目录。");
		return;
	}

	std::ostringstream command;
	command << "timelapse start " << intervalText << " " << sessionName << "\n";
	if ( ! sendControlText(command.str()) ) {
		setStatus(L"Failed to send the timelapse start command.", L"发送开始连拍命令失败。");
		return;
	}

	g_timelapseActive = true;
	g_timelapseOutputFolder = outputFolder;
	EnableWindow(g_timelapseStart, FALSE);
	EnableWindow(g_timelapseStop, TRUE);
	saveSettings();

	std::wostringstream en;
	en << L"Timelapse started every " << widenAscii(intervalText) << L" s: " << outputFolder;
	std::wostringstream zh;
	zh << L"已开始连拍，每 " << widenAscii(intervalText) << L" 秒一张：" << outputFolder;
	setDynamicStatus(en.str(), zh.str());
}

static void stopTimelapse() {
	if ( ! g_timelapseActive ) {
		setStatus(L"Timelapse is not running.", L"当前没有在连拍。");
		return;
	}
	if ( sendControlText("timelapse stop\n") ) {
		g_timelapseActive = false;
		EnableWindow(g_timelapseStart, TRUE);
		EnableWindow(g_timelapseStop, FALSE);
		std::wostringstream en;
		en << L"Timelapse stopped. Files: " << g_timelapseOutputFolder;
		std::wostringstream zh;
		zh << L"连拍已停止，文件位于：" << g_timelapseOutputFolder;
		setDynamicStatus(en.str(), zh.str());
	}
}

static void openTimelapseFolder(HWND hwnd) {
	std::wstring folder = g_timelapseOutputFolder.empty()
		? joinPathW(exeDirW(), L"timelapse")
		: g_timelapseOutputFolder;
	if ( ! createDirectoryIfMissingW(folder) ) {
		MessageBoxW(hwnd,
			g_zh ? L"无法创建或打开连拍目录。" : L"Failed to create or open the timelapse folder.",
			g_zh ? APP_TITLE_ZH : APP_TITLE_EN,
			MB_ICONERROR | MB_OK);
		return;
	}

	std::wstring commandLine = L"explorer.exe " + quoteArgW(folder);
	STARTUPINFOW si = {};
	PROCESS_INFORMATION pi = {};
	si.cb = sizeof(si);
	std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
	mutableCommand.push_back(L'\0');
	if ( CreateProcessW(NULL, mutableCommand.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, exeDirW().c_str(), &si, &pi) ) {
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	} else {
		MessageBoxW(hwnd,
			g_zh ? L"无法打开连拍目录。" : L"Failed to open the timelapse folder.",
			g_zh ? APP_TITLE_ZH : APP_TITLE_EN,
			MB_ICONERROR | MB_OK);
	}
}

static void sendChangedControl(int id) {
	if ( ! isProcessRunning() ) {
		return;
	}
	switch (id) {
		case IDC_ROTATION: sendLiveCommand("set rotation " + comboValue(g_rotation, g_rotationItems)); break;
		case IDC_SCALE: sendLiveCommand("set scale " + comboValue(g_scale, g_scaleItems)); break;
		case IDC_AI_SUPERRES: sendLiveCommand("set ai-superres " + comboValue(g_aiSuperres, g_aiSuperresItems)); break;
		case IDC_INTERP: sendLiveCommand("set interp " + comboValue(g_interp, g_interpItems)); break;
		case IDC_CMAP: sendLiveCommand("set cmap " + selectedCmapValue()); break;
		case IDC_TEMP_UNIT: sendLiveCommand("set temp-unit " + comboValue(g_tempUnit, g_tempUnitItems)); break;
		case IDC_OFFSET:
			if ( looksNumeric(g_offset) ) sendLiveCommand("set offset-c " + textOfAscii(g_offset));
			break;
		case IDC_DRIFT:
			if ( looksNumeric(g_drift) ) sendLiveCommand("set drift-c-per-min " + textOfAscii(g_drift));
			break;
		case IDC_BLUR: sendLiveCommand("set blur " + comboValue(g_blur, g_blurItems)); break;
		case IDC_CONTRAST: sendLiveCommand("set contrast " + comboValue(g_contrast, g_contrastItems)); break;
		case IDC_FILTER: sendLiveCommand("set filter " + comboValue(g_filter, g_levelItems)); break;
		case IDC_BILATERAL: sendLiveCommand("set bilateral " + comboValue(g_bilateral, g_levelItems)); break;
		case IDC_TEMPORAL: sendLiveCommand("set temporal " + comboValue(g_temporal, g_levelItems)); break;
		case IDC_SHARPEN: sendLiveCommand("set sharpen " + comboValue(g_sharpen, g_levelItems)); break;
		case IDC_THRESHOLD:
			if ( looksNumeric(g_threshold) ) sendLiveCommand("set threshold-c " + textOfAscii(g_threshold));
			break;
		case IDC_AUTORANGE: sendLiveCommand("set autorange " + comboValue(g_autorange, g_autorangeItems)); break;
		case IDC_MAPPING: sendLiveCommand("set mapping-filter " + comboValue(g_mapping, g_mappingItems)); break;
		case IDC_MANUAL_ENABLE:
		case IDC_MANUAL_MIN:
		case IDC_MANUAL_MAX:
			if ( looksNumeric(g_manualMin) && looksNumeric(g_manualMax) ) sendLiveCommand(manualRangeCommand());
			break;
		case IDC_ROI_MODE: sendLiveCommand("set roi " + comboValue(g_roiMode, g_roiItems)); break;
		case IDC_ROI_SIZE: sendLiveCommand("set roi-size " + comboValue(g_roiSize, g_roiSizeItems)); break;
		case IDC_ISOTHERM_ENABLE: sendLiveCommand("set isotherm " + boolValue(g_isothermEnable)); break;
		case IDC_ISOTHERM_THRESHOLD:
			if ( looksNumeric(g_isothermThreshold) ) sendLiveCommand("set isotherm-threshold-c " + textOfAscii(g_isothermThreshold));
			break;
		case IDC_HISTOGRAM: sendLiveCommand("set histogram " + boolValue(g_histogram)); break;
		case IDC_RULERS: sendLiveCommand("set rulers " + comboValue(g_rulers, g_rulerItems)); break;
		case IDC_FULLSCREEN: sendLiveCommand("set fullscreen " + boolValue(g_fullscreen)); break;
		default: break;
	}
}

static void applyPresetToGui(const std::string &preset) {
	g_suppressEvents = true;
	if ( preset != "custom" ) {
		selectComboByValue(g_aiSuperres, g_aiSuperresItems, "off", 0);
	}
	if ( preset == "raw" ) {
		reloadCmapCombo("0");
		selectComboByValue(g_blur, g_blurItems, "0", 0);
		selectComboByValue(g_filter, g_levelItems, "off", 0);
		selectComboByValue(g_bilateral, g_levelItems, "off", 0);
		selectComboByValue(g_temporal, g_levelItems, "off", 0);
		selectComboByValue(g_sharpen, g_levelItems, "off", 0);
		selectComboByValue(g_contrast, g_contrastItems, "1.0", 0);
		setChecked(g_isothermEnable, false);
		setChecked(g_manualEnable, false);
	} else if ( preset == "pcb" ) {
		selectComboByValue(g_scale, g_scaleItems, "4", 3);
		selectComboByValue(g_interp, g_interpItems, "lanczos", 3);
		reloadCmapCombo("27");
		selectComboByValue(g_filter, g_levelItems, "low", 1);
		selectComboByValue(g_bilateral, g_levelItems, "low", 1);
		selectComboByValue(g_temporal, g_levelItems, "low", 1);
		selectComboByValue(g_sharpen, g_levelItems, "medium", 2);
		selectComboByValue(g_contrast, g_contrastItems, "1.15", 1);
		setChecked(g_isothermEnable, true);
		setText(g_isothermThreshold, L"55.0");
	} else if ( preset == "hvac" ) {
		selectComboByValue(g_scale, g_scaleItems, "4", 3);
		selectComboByValue(g_interp, g_interpItems, "lanczos", 3);
		reloadCmapCombo("4");
		selectComboByValue(g_filter, g_levelItems, "medium", 2);
		selectComboByValue(g_bilateral, g_levelItems, "low", 1);
		selectComboByValue(g_temporal, g_levelItems, "low", 1);
		selectComboByValue(g_sharpen, g_levelItems, "low", 1);
		selectComboByValue(g_contrast, g_contrastItems, "1.15", 1);
		setChecked(g_isothermEnable, true);
		setText(g_isothermThreshold, L"35.0");
	} else if ( preset == "human" ) {
		selectComboByValue(g_scale, g_scaleItems, "4", 3);
		selectComboByValue(g_interp, g_interpItems, "cubic", 2);
		reloadCmapCombo("17");
		selectComboByValue(g_filter, g_levelItems, "low", 1);
		selectComboByValue(g_bilateral, g_levelItems, "low", 1);
		selectComboByValue(g_temporal, g_levelItems, "medium", 2);
		selectComboByValue(g_sharpen, g_levelItems, "low", 1);
		selectComboByValue(g_contrast, g_contrastItems, "1.0", 0);
		setChecked(g_manualEnable, true);
		setText(g_manualMin, L"20.0");
		setText(g_manualMax, L"42.0");
	} else if ( preset == "low-noise" ) {
		selectComboByValue(g_filter, g_levelItems, "medium", 2);
		selectComboByValue(g_bilateral, g_levelItems, "medium", 2);
		selectComboByValue(g_temporal, g_levelItems, "medium", 2);
		selectComboByValue(g_sharpen, g_levelItems, "low", 1);
		selectComboByValue(g_contrast, g_contrastItems, "1.0", 0);
	} else if ( preset == "high-contrast" ) {
		selectComboByValue(g_filter, g_levelItems, "off", 0);
		selectComboByValue(g_bilateral, g_levelItems, "off", 0);
		selectComboByValue(g_temporal, g_levelItems, "off", 0);
		selectComboByValue(g_sharpen, g_levelItems, "medium", 2);
		selectComboByValue(g_contrast, g_contrastItems, "1.35", 2);
	}
	g_suppressEvents = false;
	updatePreview();
}

static void launchCamera(HWND hwnd) {
	if ( isProcessRunning() ) {
		setStatus(L"Already running.", L"已经在运行。");
		return;
	}

	std::wstring dir = exeDirW();
	std::wstring exe = joinPathW(dir, L"Thermal-Camera-Redux.exe");
	if ( ! fileExistsW(exe) ) {
		MessageBoxW(hwnd,
			g_zh ? L"Thermal-Camera-Redux.exe 不在启动器旁边。" : L"Thermal-Camera-Redux.exe was not found next to the launcher.",
			g_zh ? APP_TITLE_ZH : APP_TITLE_EN,
			MB_ICONERROR | MB_OK);
		return;
	}

	closeControlPipe();
	g_pipeName = generatePipeName();
	std::wstring commandLine = buildCommandLine();
	writeLastCommand(commandLine);
	saveSettings();
	updatePreview();

	STARTUPINFOW si = {};
	PROCESS_INFORMATION pi = {};
	si.cb = sizeof(si);
	std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
	mutableCommand.push_back(L'\0');

	BOOL ok = CreateProcessW(NULL, mutableCommand.data(), NULL, NULL, FALSE, 0, NULL, dir.c_str(), &si, &pi);
	if ( ! ok ) {
		MessageBoxW(hwnd,
			g_zh ? L"启动 Thermal-Camera-Redux.exe 失败。" : L"Failed to start Thermal-Camera-Redux.exe.",
			g_zh ? APP_TITLE_ZH : APP_TITLE_EN,
			MB_ICONERROR | MB_OK);
		return;
	}

	CloseHandle(pi.hThread);
	g_process = pi.hProcess;
	g_processId = pi.dwProcessId;
	g_timelapseActive = false;
	EnableWindow(g_timelapseStart, TRUE);
	EnableWindow(g_timelapseStop, FALSE);
	SetTimer(hwnd, TIMER_PROCESS, 1000, NULL);
	SetTimer(hwnd, TIMER_INITIAL_SYNC, 900, NULL);
	setStatus(L"Started. Live controls will sync shortly.", L"已启动，实时控制即将同步。");
}

static void stopCamera() {
	if ( ! isProcessRunning() ) {
		setStatus(L"Not running.", L"未运行。");
		return;
	}
	if ( sendControlText("quit\n") ) {
		g_timelapseActive = false;
		EnableWindow(g_timelapseStart, TRUE);
		EnableWindow(g_timelapseStop, FALSE);
		if ( WAIT_TIMEOUT == WaitForSingleObject(g_process, 1500) ) {
			setStatus(L"Stop command sent; waiting for exit.", L"已发送停止命令，等待退出。");
		} else {
			isProcessRunning();
			g_pipeName.clear();
			updatePreview();
			setStatus(L"Process exited.", L"进程已退出。");
		}
	} else {
		terminateCameraProcess();
		setStatus(L"Pipe unavailable; process was terminated.", L"管道不可用，已终止进程。");
	}
}

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	(void)lParam;
	switch (msg) {
		case WM_CREATE:
			createControls(hwnd);
			return 0;
		case WM_TIMER:
			if ( wParam == TIMER_PROCESS ) {
				if ( ! isProcessRunning() ) {
					KillTimer(hwnd, TIMER_PROCESS);
					KillTimer(hwnd, TIMER_INITIAL_SYNC);
					KillTimer(hwnd, TIMER_CALIBRATION_STATUS);
					g_pendingCalibrationId.clear();
					g_pipeName.clear();
					g_timelapseActive = false;
					EnableWindow(g_timelapseStart, TRUE);
					EnableWindow(g_timelapseStop, FALSE);
					updatePreview();
					setStatus(L"Process exited.", L"进程已退出。");
				}
				return 0;
			}
			if ( wParam == TIMER_INITIAL_SYNC ) {
				if ( sendControlText(allLiveSettingsCommand()) ) {
					KillTimer(hwnd, TIMER_INITIAL_SYNC);
					setStatus(L"Initial live settings synced.", L"初始实时参数已同步。");
				}
				return 0;
			}
			if ( wParam == TIMER_CALIBRATION_STATUS ) {
				pollCalibrationStatus(hwnd);
				return 0;
			}
			return 0;
		case WM_COMMAND: {
			int id = LOWORD(wParam);
			int notify = HIWORD(wParam);
			if ( g_suppressEvents ) {
				return 0;
			}
			if ( id == IDC_START && notify == BN_CLICKED ) {
				launchCamera(hwnd);
				return 0;
			}
			if ( id == IDC_STOP && notify == BN_CLICKED ) {
				stopCamera();
				return 0;
			}
			if ( id == IDC_SNAPSHOT && notify == BN_CLICKED ) {
				sendLiveCommand("snapshot");
				return 0;
			}
			if ( id == IDC_TIMELAPSE_START && notify == BN_CLICKED ) {
				startTimelapse();
				return 0;
			}
			if ( id == IDC_TIMELAPSE_STOP && notify == BN_CLICKED ) {
				stopTimelapse();
				return 0;
			}
			if ( id == IDC_TIMELAPSE_FOLDER && notify == BN_CLICKED ) {
				openTimelapseFolder(hwnd);
				return 0;
			}
			if ( id == IDC_RECORD && notify == BN_CLICKED ) {
				sendLiveCommand("record");
				return 0;
			}
			if ( id == IDC_RUNTIME_RESET && notify == BN_CLICKED ) {
				sendLiveCommand("reset");
				return 0;
			}
			if ( id == IDC_BLACKBODY_CALIBRATE && notify == BN_CLICKED ) {
				sendBlackbodyCalibration(hwnd);
				return 0;
			}
			if ( id == IDC_OPEN_MANUAL && notify == BN_CLICKED ) {
				openManual(hwnd);
				return 0;
			}
			if ( id == IDC_RESET_DEFAULTS && notify == BN_CLICKED ) {
				resetDefaults();
				saveSettings();
				if ( isProcessRunning() ) {
					sendControlText(allLiveSettingsCommand());
				}
				setStatus(L"Defaults restored.", L"默认参数已恢复。");
				return 0;
			}
			if ( id == IDC_LANGUAGE && notify == CBN_SELCHANGE ) {
				g_zh = comboValue(g_language, g_languageItems) != "en-US";
				applyLanguageToUi();
				saveSettings();
				return 0;
			}
			if ( id == IDC_PRESET && notify == CBN_SELCHANGE ) {
				std::string preset = comboValue(g_preset, g_presetItems);
				applyPresetToGui(preset);
				saveSettings();
				if ( preset != "custom" && isProcessRunning() ) {
					sendLiveCommand("preset " + preset);
				}
				return 0;
			}
			if ( notify == CBN_SELCHANGE || notify == EN_CHANGE || notify == BN_CLICKED ) {
				markPresetCustomForManualChange(id);
				updatePreview();
				saveSettings();
				if ( id == IDC_DEVICE ) {
					if ( isProcessRunning() ) {
						setStatus(L"Device index changes after restart.", L"设备序号需要重启后生效。");
					}
				} else {
					sendChangedControl(id);
				}
			}
			return 0;
		}
		case WM_DESTROY:
			KillTimer(hwnd, TIMER_CALIBRATION_STATUS);
			g_timelapseActive = false;
			saveSettings();
			if ( isProcessRunning() ) {
				if ( ! sendControlText("quit\n") || WAIT_TIMEOUT == WaitForSingleObject(g_process, 1200) ) {
					terminateCameraProcess();
				}
			}
			closeControlPipe();
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
}

static int dryRun() {
	std::wstring commandLine = buildCommandLineFromValues(
		"0", "90", "4", "off", "lanczos", "4", "celsius", "0.0", "0.0",
		"off", "off", "off", "off", false, "dry-run-pipe");
	writeLastCommand(commandLine);
	if ( commandLine.find(L"Thermal-Camera-Redux.exe") == std::wstring::npos ) {
		return 1;
	}
	if ( commandLine.find(L"-control-pipe") == std::wstring::npos ) {
		return 2;
	}
	if ( commandLine.find(L"-celsius") == std::wstring::npos ) {
		return 3;
	}
	if ( commandLine.find(L"-ai-superres") == std::wstring::npos ) {
		return 4;
	}
	if ( commandLine.find(L"-ai-superres \"off\"") == std::wstring::npos ) {
		return 5;
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR cmdLine, int nCmdShow) {
	INITCOMMONCONTROLSEX icc = {};
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES;
	InitCommonControlsEx(&icc);

	if ( cmdLine && std::strstr(cmdLine, "--dry-run") ) {
		return dryRun();
	}

	WNDCLASSW wc = {};
	wc.lpfnWndProc = wndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = L"ThermalCameraReduxLiveControl";
	if ( ! RegisterClassW(&wc) ) {
		return 1;
	}

	HWND hwnd = CreateWindowExW(0, wc.lpszClassName, g_zh ? APP_TITLE_ZH : APP_TITLE_EN,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 980, 720,
		NULL, NULL, hInstance, NULL);
	if ( ! hwnd ) {
		return 1;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while ( GetMessageW(&msg, NULL, 0, 0) > 0 ) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return (int)msg.wParam;
}
