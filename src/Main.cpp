#include <Windows.h>
#include <windowsx.h>
#include "NotifyIcon.h"
#include "Resource.h"
#include "wintoastlib.h"


class WinToastHandler : public WinToastLib::IWinToastHandler {
public:
    void toastActivated() const override { }
    void toastActivated(int actionIndex) const override { }
    void toastDismissed(WinToastDismissalReason state) const override { }
    void toastFailed() const override { }
};


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void OnDestroy(HWND hwnd);
void OnTimer(HWND hwnd, UINT id);


constexpr UINT IDM_QUIT = 1000;
constexpr UINT IDM_CHANGEICON = 1001;


HINSTANCE g_hInstance;
NotifyIcon *g_notifyIcon;
SYSTEM_POWER_STATUS g_prevPowerStatus;


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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OVERLAYBATTERY_ICON));
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

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwnd, WM_TIMER, OnTimer);

    case NotifyIcon::WM_NOTIFYICON:
        if (wParam == g_notifyIcon->GetID()) {
            switch (lParam) {
            case WM_RBUTTONDOWN:
                POINT position;

                GetCursorPos(&position);
                HMENU hMenu = CreatePopupMenu();

                MENUITEMINFO info;
                info.cbSize = sizeof(MENUITEMINFO);
                info.fMask = MIIM_ID | MIIM_STRING;
                info.wID = IDM_QUIT;
                info.dwTypeData = (LPWSTR) TEXT("Quit");
                InsertMenuItem(hMenu, 0, true, &info);

                info.wID = IDM_CHANGEICON;
                info.dwTypeData = (LPWSTR) TEXT("Change Icon");
                InsertMenuItem(hMenu, 0, true, &info);

                POINT point;
                GetCursorPos(&point);

                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, 0, hwnd, nullptr);
                SendMessage(hwnd, WM_NULL, 0, 0);

                break;
            }
        }
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    g_notifyIcon = new NotifyIcon(hwnd, 1, LoadIcon(nullptr, IDI_APPLICATION), TEXT("OverlayBattery\n20%"));

    SetTimer(hwnd, 0, 10, nullptr);
    SetLayeredWindowAttributes(hwnd, 0, 0x7f, LWA_ALPHA);
    ShowWindow(hwnd, SW_SHOW);

    return true;
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    switch (id) {
    case IDM_QUIT:
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        break;
    case IDM_CHANGEICON:
        g_notifyIcon->ChangeIcon(LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_OVERLAYBATTERY_ICON)));
        break;
    }
}

void OnDestroy(HWND hwnd) {
    PostQuitMessage(0);
    delete g_notifyIcon;
}

void OnTimer(HWND hwnd, UINT id) {
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

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

    status.BatteryLifePercent; // percentage, if unknown status set 255.
    status.BatteryLifeTime; // life time(sec), if unknown or connected AC line set -1.
}
