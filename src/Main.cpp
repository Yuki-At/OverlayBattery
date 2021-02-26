#include <Windows.h>
#include <windowsx.h>
#include "NotifyIcon.h"
#include "PopupMenu.h"
#include "Resource.h"
#include "wintoastlib.h"


using namespace WinToastLib;

class WinToastHandler : public IWinToastHandler {
public:
    WinToastHandler() = default;
    void toastActivated() const override { }
    void toastActivated(int actionIndex) const override { }
    void toastDismissed(WinToastDismissalReason state) const override { }
    void toastFailed() const override { }
};


enum class BatteryLevel {
    HIGH,
    NORMAL,
    LOW,
    UNKNOWN,
};


constexpr UINT IDM_QUIT = 1000;
constexpr UINT IDM_SETTING = 1001;

constexpr UINT BatteryHigh = 60;
constexpr UINT BatteryLow = 40;


inline BatteryLevel GetBatteryLevel(BYTE percentage) {
    if (percentage > 100){
        return BatteryLevel::UNKNOWN;
    }
    if (percentage > BatteryHigh) {
        return BatteryLevel::HIGH;
    }
    if (percentage > BatteryLow) {
        return BatteryLevel::NORMAL;
    }
    return BatteryLevel::LOW;
}


void InitWinToast();
bool OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void OnDestroy(HWND hwnd);
void OnTimer(HWND hwnd, UINT id);
void Update(BatteryLevel level);
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DialogProcedure(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


HINSTANCE g_hInstance;
NotifyIcon *g_notifyIcon;
SYSTEM_POWER_STATUS g_prevPowerStatus;
WinToastHandler g_winToastHandler;
PopupMenu g_popupMenu;


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
    constexpr LPCTSTR WindowClassName = TEXT("MainWindowClass");

    g_hInstance = hInstance;

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProcedure;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = WindowClassName;
    wcex.hIconSm = nullptr;

    if (!RegisterClassEx(&wcex)) {
        return -1;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        WindowClassName, TEXT("OverlayBattery"),
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, 320, 240,
        nullptr, nullptr,
        hInstance, nullptr
    );

    if (hwnd == INVALID_HANDLE_VALUE) {
        return -1;
    }

    MSG msg { };
    while (GetMessage(&msg, nullptr, 0, 0)) {
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

void InitWinToast() {
    WinToast::WinToastError error;
    WinToast::instance()->setAppName(TEXT("OverlayBattery"));
    const auto aumi = WinToast::configureAUMI(TEXT("company"), TEXT("wintoast"), TEXT("wintoastexample"), TEXT("20201012"));
    WinToast::instance()->setAppUserModelId(aumi);

    if (!WinToast::instance()->initialize(&error)) {
        wchar_t buf[250];
        swprintf_s(buf, TEXT("Failed to initialize WinToast :%d"), error);
        MessageBox(nullptr, buf, TEXT("ERROR"), MB_ICONERROR | MB_OK);
    }
}

bool OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    g_notifyIcon = new NotifyIcon(hwnd, 1, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON)), TEXT("OverlayBattery"));
    InitWinToast();
    g_popupMenu.AddItem(IDM_SETTING, TEXT("Setting"));
    g_popupMenu.AddItem(IDM_QUIT, TEXT("Quit"));

    GetSystemPowerStatus(&g_prevPowerStatus);
    Update(GetBatteryLevel(g_prevPowerStatus.BatteryLifePercent));

    SetTimer(hwnd, 0, 10, nullptr);
    SetLayeredWindowAttributes(hwnd, 0, 0x7f, LWA_ALPHA);
    // ShowWindow(hwnd, SW_SHOW);

    return true;
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    switch (id) {
    case IDM_QUIT:
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        break;
    case IDM_SETTING:
        DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG1), nullptr, DialogProcedure);
        break;
    }
}

void OnDestroy(HWND hwnd) {
    PostQuitMessage(0);
    delete g_notifyIcon;
}

void OnTimer(HWND hwnd, UINT id) {
    // SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    SYSTEM_POWER_STATUS status;
    GetSystemPowerStatus(&status);

    switch (status.ACLineStatus) {
    case 0:
        // offline
        break;
    case 1:
        // online
        break;
    case 0xff:
        // unknown
        break;
    }

    if (status.BatteryFlag == 0xff) {
        // unknown status
    }

    if (status.BatteryFlag | 0x80) {
        // no system battery
    }

    if (status.BatteryFlag | 0x08) {
        // charging
    }

    if (GetBatteryLevel(g_prevPowerStatus.BatteryLifePercent) != GetBatteryLevel(status.BatteryLifePercent)) {
        Update(GetBatteryLevel(status.BatteryLifePercent));
    }

    status.BatteryLifeTime; // life time(sec), if unknown or connected AC line set -1.

    g_prevPowerStatus = status;
}

void Update(BatteryLevel level) {
    WinToastTemplate toast(WinToastTemplate::Text02);
    toast.setTextField(TEXT("Battery Alert!"), WinToastTemplate::FirstLine);

    switch (level) {
    case BatteryLevel::HIGH:
        toast.setTextField(TEXT("The battery level is HIGH."), WinToastTemplate::SecondLine);
        WinToast::instance()->showToast(toast, &g_winToastHandler);
        break;
    case BatteryLevel::LOW:
        toast.setTextField(TEXT("The battery level is LOW."), WinToastTemplate::SecondLine);
        WinToast::instance()->showToast(toast, &g_winToastHandler);
        break;
    }
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwnd, WM_TIMER, OnTimer);

    case NotifyIcon::WM_NOTIFYICON:
        if (wParam == g_notifyIcon->GetID()) {
            POINT cursorPos;

            switch (lParam) {
            case WM_RBUTTONDOWN:
                GetCursorPos(&cursorPos);
                g_popupMenu.Show(hwnd, cursorPos.x, cursorPos.y);
                break;
            }
        }
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK DialogProcedure(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        SendDlgItemMessage(hdlg, ID_SLIDER_HIGH, TBM_SETRANGE, true, MAKELONG(0, 100));
        SendDlgItemMessage(hdlg, ID_SLIDER_HIGH, TBM_SETPOS  , true, 60);
        return true;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EndDialog(hdlg, 0);
            break;

        case IDCANCEL:
            EndDialog(hdlg, 0);
            break;

        case ID_CHECK_ENABLETOAST:

            break;
        }
        OutputDebugString(TEXT("WM_COMMAND\n"));
        return true;

    case WM_HSCROLL:
        if ((HWND) lParam == GetDlgItem(hdlg, ID_SLIDER_HIGH)) {
            TCHAR buf[256];
            wsprintf(buf, TEXT("%d%%"), (int) SendDlgItemMessage(hdlg, ID_SLIDER_HIGH, TBM_GETPOS, 0, 0));
            SetWindowText(GetDlgItem(hdlg, ID_TEXT_HIGH), buf);
        }
        return true;
    }

    return false;
}
