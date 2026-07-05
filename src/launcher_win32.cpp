#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>

struct ComboItem {
	const char *label;
	const char *value;
};

static const char *APP_TITLE = "Thermal Camera Redux UTi260B";
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
static const int IDC_RESET = 1015;

static HWND g_device;
static HWND g_rotation;
static HWND g_scale;
static HWND g_interp;
static HWND g_cmap;
static HWND g_offset;
static HWND g_filter;
static HWND g_bilateral;
static HWND g_temporal;
static HWND g_sharpen;
static HWND g_fullscreen;
static HWND g_preview;
static HWND g_status;
static HFONT g_font;

static std::vector<ComboItem> g_rotationItems = {
	{ "0 deg", "0" },
	{ "90 deg", "90" },
	{ "180 deg", "180" },
	{ "270 deg", "270" }
};

static std::vector<ComboItem> g_scaleItems = {
	{ "1x", "1" },
	{ "2x", "2" },
	{ "3x", "3" },
	{ "4x", "4" }
};

static std::vector<ComboItem> g_interpItems = {
	{ "Nearest", "nearest" },
	{ "Bilinear", "linear" },
	{ "Bicubic", "cubic" },
	{ "Lanczos", "lanczos" }
};

static std::vector<ComboItem> g_levelItems = {
	{ "Off", "off" },
	{ "Low", "low" },
	{ "Medium", "medium" },
	{ "Strong", "strong" }
};

static std::vector<std::string> g_cmapLabels;
static std::vector<std::string> g_cmapValues;

static std::string exeDir() {
	char path[MAX_PATH] = {0};
	GetModuleFileNameA(NULL, path, MAX_PATH);
	std::string value(path);
	size_t slash = value.find_last_of("\\/");
	if ( slash == std::string::npos ) {
		return ".";
	}
	return value.substr(0, slash);
}

static std::string joinPath(const std::string &dir, const std::string &name) {
	if ( dir.empty() ) {
		return name;
	}
	char last = dir[dir.size() - 1];
	if ( last == '\\' || last == '/' ) {
		return dir + name;
	}
	return dir + "\\" + name;
}

static bool fileExists(const std::string &path) {
	DWORD attrs = GetFileAttributesA(path.c_str());
	return attrs != INVALID_FILE_ATTRIBUTES && 0 == (attrs & FILE_ATTRIBUTE_DIRECTORY);
}

static std::string quoteArg(const std::string &arg) {
	std::string out = "\"";
	for ( char ch : arg ) {
		if ( ch == '"' ) {
			out += "\\\"";
		} else {
			out += ch;
		}
	}
	out += "\"";
	return out;
}

static std::string iniPath() {
	return joinPath(exeDir(), "thermal-camera-redux-gui.ini");
}

static std::string readIni(const char *key, const char *fallback) {
	char buffer[256] = {0};
	GetPrivateProfileStringA("settings", key, fallback, buffer, sizeof(buffer), iniPath().c_str());
	return std::string(buffer);
}

static void writeIni(const char *key, const std::string &value) {
	WritePrivateProfileStringA("settings", key, value.c_str(), iniPath().c_str());
}

static void setFont(HWND hwnd) {
	SendMessageA(hwnd, WM_SETFONT, (WPARAM)g_font, TRUE);
}

static void addComboItems(HWND combo, const std::vector<ComboItem> &items) {
	for (const ComboItem &item : items) {
		SendMessageA(combo, CB_ADDSTRING, 0, (LPARAM)item.label);
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
	SendMessageA(combo, CB_SETCURSEL, selected, 0);
}

static std::string comboValue(HWND combo, const std::vector<ComboItem> &items) {
	int selected = (int)SendMessageA(combo, CB_GETCURSEL, 0, 0);
	if ( selected < 0 || selected >= (int)items.size() ) {
		selected = 0;
	}
	return items[selected].value;
}

static void initCmapItems() {
	if ( ! g_cmapLabels.empty() ) {
		return;
	}
	for (int i = 0; i <= 36; i++) {
		std::ostringstream label;
		label << i;
		if ( i == 0 ) {
			label << " None";
		} else if ( i == 4 ) {
			label << " Iron/Jet default";
		}
		g_cmapLabels.push_back(label.str());
		g_cmapValues.push_back(std::to_string(i));
	}
}

static void addCmapItems(HWND combo) {
	initCmapItems();
	for (const std::string &label : g_cmapLabels) {
		SendMessageA(combo, CB_ADDSTRING, 0, (LPARAM)label.c_str());
	}
}

static void selectCmapByValue(HWND combo, const std::string &value) {
	initCmapItems();
	int selected = 4;
	for (int i = 0; i < (int)g_cmapValues.size(); i++) {
		if (value == g_cmapValues[i]) {
			selected = i;
			break;
		}
	}
	SendMessageA(combo, CB_SETCURSEL, selected, 0);
}

static std::string selectedCmapValue(HWND combo) {
	initCmapItems();
	int selected = (int)SendMessageA(combo, CB_GETCURSEL, 0, 0);
	if ( selected < 0 || selected >= (int)g_cmapValues.size() ) {
		selected = 4;
	}
	return g_cmapValues[selected];
}

static std::string textOf(HWND hwnd) {
	char buffer[256] = {0};
	GetWindowTextA(hwnd, buffer, sizeof(buffer));
	return std::string(buffer);
}

static void setText(HWND hwnd, const std::string &value) {
	SetWindowTextA(hwnd, value.c_str());
}

static std::string buildCommandLineFromValues(
	const std::string &deviceValue,
	const std::string &rotationValue,
	const std::string &scaleValue,
	const std::string &interpValue,
	const std::string &cmapValue,
	const std::string &offsetValue,
	const std::string &filterValue,
	const std::string &bilateralValue,
	const std::string &temporalValue,
	const std::string &sharpenValue,
	bool fullscreen) {
	std::string dir = exeDir();
	std::string exe = joinPath(dir, "Thermal-Camera-Redux.exe");
	std::string device = deviceValue;
	std::string offset = offsetValue;
	if ( device.empty() ) {
		device = "0";
	}
	if ( offset.empty() ) {
		offset = "0.0";
	}

	std::ostringstream cmd;
	cmd << quoteArg(exe)
	    << " -uti260b"
	    << " -d " << quoteArg(device)
	    << " -rotate " << quoteArg(rotationValue)
	    << " -display-scale " << quoteArg(scaleValue)
	    << " -interp " << quoteArg(interpValue)
	    << " -cmap " << quoteArg(cmapValue)
	    << " -temp-offset-c " << quoteArg(offset)
	    << " -filter-preset " << quoteArg(filterValue)
	    << " -bilateral " << quoteArg(bilateralValue)
	    << " -temporal-denoise " << quoteArg(temporalValue)
	    << " -sharpen " << quoteArg(sharpenValue);
	if ( fullscreen ) {
		cmd << " -fullscreen";
	}
	return cmd.str();
}

static std::string buildCommandLine() {
	return buildCommandLineFromValues(
		textOf(g_device),
		comboValue(g_rotation, g_rotationItems),
		comboValue(g_scale, g_scaleItems),
		comboValue(g_interp, g_interpItems),
		selectedCmapValue(g_cmap),
		textOf(g_offset),
		comboValue(g_filter, g_levelItems),
		comboValue(g_bilateral, g_levelItems),
		comboValue(g_temporal, g_levelItems),
		comboValue(g_sharpen, g_levelItems),
		BST_CHECKED == SendMessageA(g_fullscreen, BM_GETCHECK, 0, 0));
}

static void writeLastCommand(const std::string &commandLine) {
	std::ofstream out(joinPath(exeDir(), "thermal-camera-redux-last-command.txt").c_str(), std::ios::out | std::ios::trunc);
	out << commandLine << "\n";
}

static void updatePreview() {
	if ( g_preview ) {
		setText(g_preview, buildCommandLine());
	}
}

static void saveSettings() {
	writeIni("device", textOf(g_device));
	writeIni("rotation", comboValue(g_rotation, g_rotationItems));
	writeIni("scale", comboValue(g_scale, g_scaleItems));
	writeIni("interp", comboValue(g_interp, g_interpItems));
	writeIni("cmap", selectedCmapValue(g_cmap));
	writeIni("offset_c", textOf(g_offset));
	writeIni("filter", comboValue(g_filter, g_levelItems));
	writeIni("bilateral", comboValue(g_bilateral, g_levelItems));
	writeIni("temporal", comboValue(g_temporal, g_levelItems));
	writeIni("sharpen", comboValue(g_sharpen, g_levelItems));
	writeIni("fullscreen", BST_CHECKED == SendMessageA(g_fullscreen, BM_GETCHECK, 0, 0) ? "1" : "0");
}

static void resetDefaults() {
	setText(g_device, "0");
	selectComboByValue(g_rotation, g_rotationItems, "90", 1);
	selectComboByValue(g_scale, g_scaleItems, "4", 3);
	selectComboByValue(g_interp, g_interpItems, "cubic", 2);
	selectCmapByValue(g_cmap, "4");
	setText(g_offset, "0.0");
	selectComboByValue(g_filter, g_levelItems, "off", 0);
	selectComboByValue(g_bilateral, g_levelItems, "off", 0);
	selectComboByValue(g_temporal, g_levelItems, "off", 0);
	selectComboByValue(g_sharpen, g_levelItems, "off", 0);
	SendMessageA(g_fullscreen, BM_SETCHECK, BST_UNCHECKED, 0);
	updatePreview();
}

static void loadSettings() {
	setText(g_device, readIni("device", "0"));
	selectComboByValue(g_rotation, g_rotationItems, readIni("rotation", "90"), 1);
	selectComboByValue(g_scale, g_scaleItems, readIni("scale", "4"), 3);
	selectComboByValue(g_interp, g_interpItems, readIni("interp", "cubic"), 2);
	selectCmapByValue(g_cmap, readIni("cmap", "4"));
	setText(g_offset, readIni("offset_c", "0.0"));
	selectComboByValue(g_filter, g_levelItems, readIni("filter", "off"), 0);
	selectComboByValue(g_bilateral, g_levelItems, readIni("bilateral", "off"), 0);
	selectComboByValue(g_temporal, g_levelItems, readIni("temporal", "off"), 0);
	selectComboByValue(g_sharpen, g_levelItems, readIni("sharpen", "off"), 0);
	SendMessageA(g_fullscreen, BM_SETCHECK, readIni("fullscreen", "0") == "1" ? BST_CHECKED : BST_UNCHECKED, 0);
	updatePreview();
}

static HWND addLabel(HWND parent, const char *text, int x, int y, int w, int h) {
	HWND hwnd = CreateWindowExA(0, "STATIC", text, WS_CHILD | WS_VISIBLE, x, y, w, h, parent, NULL, NULL, NULL);
	setFont(hwnd);
	return hwnd;
}

static HWND addEdit(HWND parent, int id, int x, int y, int w, int h) {
	HWND hwnd = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL);
	setFont(hwnd);
	return hwnd;
}

static HWND addCombo(HWND parent, int id, int x, int y, int w, int h) {
	HWND hwnd = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
		x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL);
	setFont(hwnd);
	return hwnd;
}

static HWND addButton(HWND parent, int id, const char *text, int x, int y, int w, int h) {
	HWND hwnd = CreateWindowExA(0, "BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL);
	setFont(hwnd);
	return hwnd;
}

static void createControls(HWND hwnd) {
	g_font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	int y = 16;
	int labelX = 18;
	int controlX = 168;
	int labelW = 138;
	int controlW = 300;
	int rowH = 24;
	int gap = 32;

	addLabel(hwnd, "Device index", labelX, y, labelW, rowH);
	g_device = addEdit(hwnd, IDC_DEVICE, controlX, y - 2, 80, rowH);
	y += gap;

	addLabel(hwnd, "Rotation", labelX, y, labelW, rowH);
	g_rotation = addCombo(hwnd, IDC_ROTATION, controlX, y - 2, controlW, 180);
	addComboItems(g_rotation, g_rotationItems);
	y += gap;

	addLabel(hwnd, "Super-res scale", labelX, y, labelW, rowH);
	g_scale = addCombo(hwnd, IDC_SCALE, controlX, y - 2, controlW, 180);
	addComboItems(g_scale, g_scaleItems);
	y += gap;

	addLabel(hwnd, "Interpolation", labelX, y, labelW, rowH);
	g_interp = addCombo(hwnd, IDC_INTERP, controlX, y - 2, controlW, 180);
	addComboItems(g_interp, g_interpItems);
	y += gap;

	addLabel(hwnd, "Colormap", labelX, y, labelW, rowH);
	g_cmap = addCombo(hwnd, IDC_CMAP, controlX, y - 2, controlW, 360);
	addCmapItems(g_cmap);
	y += gap;

	addLabel(hwnd, "Temp offset C", labelX, y, labelW, rowH);
	g_offset = addEdit(hwnd, IDC_OFFSET, controlX, y - 2, 110, rowH);
	y += gap;

	addLabel(hwnd, "Filter preset", labelX, y, labelW, rowH);
	g_filter = addCombo(hwnd, IDC_FILTER, controlX, y - 2, controlW, 180);
	addComboItems(g_filter, g_levelItems);
	y += gap;

	addLabel(hwnd, "Bilateral", labelX, y, labelW, rowH);
	g_bilateral = addCombo(hwnd, IDC_BILATERAL, controlX, y - 2, controlW, 180);
	addComboItems(g_bilateral, g_levelItems);
	y += gap;

	addLabel(hwnd, "Temporal denoise", labelX, y, labelW, rowH);
	g_temporal = addCombo(hwnd, IDC_TEMPORAL, controlX, y - 2, controlW, 180);
	addComboItems(g_temporal, g_levelItems);
	y += gap;

	addLabel(hwnd, "Sharpen", labelX, y, labelW, rowH);
	g_sharpen = addCombo(hwnd, IDC_SHARPEN, controlX, y - 2, controlW, 180);
	addComboItems(g_sharpen, g_levelItems);
	y += gap;

	g_fullscreen = CreateWindowExA(0, "BUTTON", "Fullscreen", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		controlX, y - 2, 140, rowH, hwnd, (HMENU)(INT_PTR)IDC_FULLSCREEN, NULL, NULL);
	setFont(g_fullscreen);
	y += gap;

	addLabel(hwnd, "Command", labelX, y, labelW, rowH);
	g_preview = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
		controlX, y - 2, controlW, 54, hwnd, (HMENU)(INT_PTR)IDC_PREVIEW, NULL, NULL);
	setFont(g_preview);
	y += 66;

	addButton(hwnd, IDC_START, "Start", controlX, y, 92, 30);
	addButton(hwnd, IDC_RESET, "Reset", controlX + 104, y, 92, 30);
	g_status = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_VISIBLE,
		labelX, y + 42, 450, 22, hwnd, (HMENU)(INT_PTR)IDC_STATUS, NULL, NULL);
	setFont(g_status);

	loadSettings();
}

static void launchCamera(HWND hwnd) {
	std::string dir = exeDir();
	std::string exe = joinPath(dir, "Thermal-Camera-Redux.exe");
	if ( ! fileExists(exe) ) {
		MessageBoxA(hwnd, "Thermal-Camera-Redux.exe was not found next to the launcher.", APP_TITLE, MB_ICONERROR | MB_OK);
		return;
	}

	std::string commandLine = buildCommandLine();
	writeLastCommand(commandLine);
	saveSettings();

	STARTUPINFOA si = {};
	PROCESS_INFORMATION pi = {};
	si.cb = sizeof(si);
	std::vector<char> mutableCommand(commandLine.begin(), commandLine.end());
	mutableCommand.push_back('\0');

	BOOL ok = CreateProcessA(NULL, mutableCommand.data(), NULL, NULL, FALSE, 0, NULL, dir.c_str(), &si, &pi);
	if ( ! ok ) {
		MessageBoxA(hwnd, "Failed to start Thermal-Camera-Redux.exe.", APP_TITLE, MB_ICONERROR | MB_OK);
		return;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	setText(g_status, "Started. Last command was saved to thermal-camera-redux-last-command.txt");
}

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	(void)lParam;
	switch (msg) {
		case WM_CREATE:
			createControls(hwnd);
			return 0;
		case WM_COMMAND: {
			int id = LOWORD(wParam);
			int notify = HIWORD(wParam);
			if ( id == IDC_START && notify == BN_CLICKED ) {
				launchCamera(hwnd);
				return 0;
			}
			if ( id == IDC_RESET && notify == BN_CLICKED ) {
				resetDefaults();
				saveSettings();
				setText(g_status, "Defaults restored.");
				return 0;
			}
			if ( notify == CBN_SELCHANGE || notify == EN_CHANGE || notify == BN_CLICKED ) {
				updatePreview();
			}
			return 0;
		}
		case WM_DESTROY:
			saveSettings();
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProcA(hwnd, msg, wParam, lParam);
	}
}

static int dryRun() {
	std::string commandLine = buildCommandLineFromValues(
		"0", "90", "4", "cubic", "4", "0.0",
		"off", "off", "off", "off", false);
	writeLastCommand(commandLine);
	return commandLine.find("Thermal-Camera-Redux.exe") == std::string::npos ? 1 : 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR cmdLine, int nCmdShow) {
	INITCOMMONCONTROLSEX icc = {};
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&icc);

	if ( cmdLine && std::strstr(cmdLine, "--dry-run") ) {
		return dryRun();
	}

	WNDCLASSA wc = {};
	wc.lpfnWndProc = wndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = "ThermalCameraReduxLauncher";
	if ( ! RegisterClassA(&wc) ) {
		return 1;
	}

	HWND hwnd = CreateWindowExA(0, wc.lpszClassName, APP_TITLE,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 520, 600,
		NULL, NULL, hInstance, NULL);
	if ( ! hwnd ) {
		return 1;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while ( GetMessageA(&msg, NULL, 0, 0) > 0 ) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
	return (int)msg.wParam;
}
