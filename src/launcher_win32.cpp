#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include <sstream>
#include <string>
#include <vector>
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

static const UINT_PTR TIMER_PROCESS = 1;
static const UINT_PTR TIMER_INITIAL_SYNC = 2;

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
	std::string result((size_t)bytes - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, &result[0], bytes, NULL, NULL);
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

static void setStatus(const wchar_t *en, const wchar_t *zh) {
	SetWindowTextW(g_status, g_zh ? zh : en);
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
	int y = 14;
	int labelX = 18;
	int controlX = 170;
	int labelW = 140;
	int controlW = 210;
	int rightX = 398;
	int rightW = 220;
	int rowH = 23;
	int gap = 27;

	addLabel(hwnd, L"Language", L"语言", labelX, y, labelW, rowH);
	g_language = addCombo(hwnd, IDC_LANGUAGE, controlX, y - 2, controlW, 160);
	addComboItems(g_language, g_languageItems);
	y += gap;

	addLabel(hwnd, L"Device index", L"设备序号", labelX, y, labelW, rowH);
	g_device = addEdit(hwnd, IDC_DEVICE, controlX, y - 2, 80, rowH);
	addLabel(hwnd, L"Change needs restart", L"修改后需重启", rightX, y, rightW, rowH);
	y += gap;

	addLabel(hwnd, L"Preset", L"预设方案", labelX, y, labelW, rowH);
	g_preset = addCombo(hwnd, IDC_PRESET, controlX, y - 2, controlW, 180);
	addComboItems(g_preset, g_presetItems);
	y += gap;

	addLabel(hwnd, L"Rotation", L"旋转", labelX, y, labelW, rowH);
	g_rotation = addCombo(hwnd, IDC_ROTATION, controlX, y - 2, controlW, 180);
	addComboItems(g_rotation, g_rotationItems);
	y += gap;

	addLabel(hwnd, L"Super-res scale", L"软件超分倍率", labelX, y, labelW, rowH);
	g_scale = addCombo(hwnd, IDC_SCALE, controlX, y - 2, controlW, 180);
	addComboItems(g_scale, g_scaleItems);
	y += gap;

	addLabel(hwnd, L"AI super-res", L"AI 超分", labelX, y, labelW, rowH);
	g_aiSuperres = addCombo(hwnd, IDC_AI_SUPERRES, controlX, y - 2, controlW + 88, 180);
	addComboItems(g_aiSuperres, g_aiSuperresItems);
	y += gap;

	addLabel(hwnd, L"Interpolation", L"插值算法", labelX, y, labelW, rowH);
	g_interp = addCombo(hwnd, IDC_INTERP, controlX, y - 2, controlW, 180);
	addComboItems(g_interp, g_interpItems);
	y += gap;

	addLabel(hwnd, L"Colormap", L"伪彩色", labelX, y, labelW, rowH);
	g_cmap = addCombo(hwnd, IDC_CMAP, controlX, y - 2, controlW, 360);
	reloadCmapCombo("4");
	y += gap;

	addLabel(hwnd, L"Temperature unit", L"温度单位", labelX, y, labelW, rowH);
	g_tempUnit = addCombo(hwnd, IDC_TEMP_UNIT, controlX, y - 2, controlW, 120);
	addComboItems(g_tempUnit, g_tempUnitItems);
	y += gap;

	addLabel(hwnd, L"Temp offset C", L"温度偏移 C", labelX, y, labelW, rowH);
	g_offset = addEdit(hwnd, IDC_OFFSET, controlX, y - 2, 90, rowH);
	y += gap;

	addLabel(hwnd, L"Drift C/min", L"温漂 C/分钟", labelX, y, labelW, rowH);
	g_drift = addEdit(hwnd, IDC_DRIFT, controlX, y - 2, 90, rowH);
	addLabel(hwnd, L"Default 0", L"默认 0", rightX, y, 92, rowH);
	y += gap;

	addLabel(hwnd, L"Blur", L"模糊", labelX, y, labelW, rowH);
	g_blur = addCombo(hwnd, IDC_BLUR, controlX, y - 2, controlW, 180);
	addComboItems(g_blur, g_blurItems);
	y += gap;

	addLabel(hwnd, L"Contrast / gain", L"对比/增益", labelX, y, labelW, rowH);
	g_contrast = addCombo(hwnd, IDC_CONTRAST, controlX, y - 2, controlW, 180);
	addComboItems(g_contrast, g_contrastItems);
	y += gap;

	addLabel(hwnd, L"Gaussian filter", L"高斯滤波", labelX, y, labelW, rowH);
	g_filter = addCombo(hwnd, IDC_FILTER, controlX, y - 2, controlW, 180);
	addComboItems(g_filter, g_levelItems);
	y += gap;

	addLabel(hwnd, L"Bilateral", L"双边滤波", labelX, y, labelW, rowH);
	g_bilateral = addCombo(hwnd, IDC_BILATERAL, controlX, y - 2, controlW, 180);
	addComboItems(g_bilateral, g_levelItems);
	y += gap;

	addLabel(hwnd, L"Temporal denoise", L"时域降噪", labelX, y, labelW, rowH);
	g_temporal = addCombo(hwnd, IDC_TEMPORAL, controlX, y - 2, controlW, 180);
	addComboItems(g_temporal, g_levelItems);
	y += gap;

	addLabel(hwnd, L"Sharpen", L"锐化", labelX, y, labelW, rowH);
	g_sharpen = addCombo(hwnd, IDC_SHARPEN, controlX, y - 2, controlW, 180);
	addComboItems(g_sharpen, g_levelItems);
	y += gap;

	addLabel(hwnd, L"Min/max delta C", L"极值阈值 C", labelX, y, labelW, rowH);
	g_threshold = addEdit(hwnd, IDC_THRESHOLD, controlX, y - 2, 90, rowH);
	y += gap;

	addLabel(hwnd, L"Auto range", L"自动范围锁定", labelX, y, labelW, rowH);
	g_autorange = addCombo(hwnd, IDC_AUTORANGE, controlX, y - 2, controlW, 180);
	addComboItems(g_autorange, g_autorangeItems);
	y += gap;

	addLabel(hwnd, L"Range mapping", L"范围映射", labelX, y, labelW, rowH);
	g_mapping = addCombo(hwnd, IDC_MAPPING, controlX, y - 2, controlW, 180);
	addComboItems(g_mapping, g_mappingItems);
	y += gap;

	g_manualEnable = addCheckbox(hwnd, IDC_MANUAL_ENABLE, L"Manual temp range", L"手动温度范围", controlX, y - 2, 180, rowH);
	addLabel(hwnd, L"Min / Max C", L"最小/最大 C", rightX, y, 92, rowH);
	g_manualMin = addEdit(hwnd, IDC_MANUAL_MIN, rightX + 92, y - 2, 54, rowH);
	g_manualMax = addEdit(hwnd, IDC_MANUAL_MAX, rightX + 152, y - 2, 54, rowH);
	y += gap;

	addLabel(hwnd, L"ROI mode", L"ROI 模式", labelX, y, labelW, rowH);
	g_roiMode = addCombo(hwnd, IDC_ROI_MODE, controlX, y - 2, controlW, 180);
	addComboItems(g_roiMode, g_roiItems);
	addLabel(hwnd, L"ROI size", L"ROI 大小", rightX, y, 70, rowH);
	g_roiSize = addCombo(hwnd, IDC_ROI_SIZE, rightX + 76, y - 2, 130, 160);
	addComboItems(g_roiSize, g_roiSizeItems);
	y += gap;

	g_isothermEnable = addCheckbox(hwnd, IDC_ISOTHERM_ENABLE, L"Isotherm / over-temp", L"等温/过温高亮", controlX, y - 2, 180, rowH);
	addLabel(hwnd, L"Threshold C", L"阈值 C", rightX, y, 92, rowH);
	g_isothermThreshold = addEdit(hwnd, IDC_ISOTHERM_THRESHOLD, rightX + 92, y - 2, 80, rowH);
	y += gap;

	g_histogram = addCheckbox(hwnd, IDC_HISTOGRAM, L"Histogram equalize", L"直方图均衡", controlX, y - 2, 180, rowH);
	g_fullscreen = addCheckbox(hwnd, IDC_FULLSCREEN, L"Fullscreen", L"全屏", rightX, y - 2, 120, rowH);
	y += gap;

	addLabel(hwnd, L"Rulers", L"标尺", labelX, y, labelW, rowH);
	g_rulers = addCombo(hwnd, IDC_RULERS, controlX, y - 2, controlW, 180);
	addComboItems(g_rulers, g_rulerItems);
	y += gap + 4;

	g_start = addButton(hwnd, IDC_START, L"Start", L"启动", controlX, y, 82, 28);
	g_stop = addButton(hwnd, IDC_STOP, L"Stop", L"停止", controlX + 90, y, 82, 28);
	g_snapshot = addButton(hwnd, IDC_SNAPSHOT, L"Snapshot", L"截图+RAW", controlX + 180, y, 96, 28);
	g_record = addButton(hwnd, IDC_RECORD, L"Record", L"录像", controlX + 284, y, 82, 28);
	y += 36;

	g_runtimeReset = addButton(hwnd, IDC_RUNTIME_RESET, L"Runtime reset", L"运行中复位", controlX, y, 116, 28);
	g_resetDefaults = addButton(hwnd, IDC_RESET_DEFAULTS, L"Defaults", L"默认参数", controlX + 126, y, 100, 28);
	y += 38;

	addLabel(hwnd, L"Command", L"启动命令", labelX, y, labelW, rowH);
	g_preview = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
		controlX, y - 2, 448, 56, hwnd, (HMENU)(INT_PTR)IDC_PREVIEW, NULL, NULL);
	setFont(g_preview);
	y += 66;

	g_status = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE,
		labelX, y, 610, 24, hwnd, (HMENU)(INT_PTR)IDC_STATUS, NULL, NULL);
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
					g_pipeName.clear();
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
			if ( id == IDC_RECORD && notify == BN_CLICKED ) {
				sendLiveCommand("record");
				return 0;
			}
			if ( id == IDC_RUNTIME_RESET && notify == BN_CLICKED ) {
				sendLiveCommand("reset");
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
		CW_USEDEFAULT, CW_USEDEFAULT, 660, 990,
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
