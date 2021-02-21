#include <Windows.h>
#include "NotifyIcon.h"
#include "Resource.h"

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

constexpr UINT IDM_QUIT = 1000;
constexpr UINT IDM_CHANGEICON = 1001;

HINSTANCE g_hInstance;
NotifyIcon *g_notifyIcon;

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

    g_notifyIcon = new NotifyIcon(hwnd, 1, LoadIcon(nullptr, IDI_APPLICATION), TEXT("Hello world!"));

    SetTimer(hwnd, 0, 10, nullptr);
    SetLayeredWindowAttributes(hwnd, 0, 0x7f, LWA_ALPHA);
    ShowWindow(hwnd, SW_SHOW);

    MSG msg { };
    while (GetMessage(&msg, nullptr, 0, 0)) {
        DispatchMessage(&msg);
    }

    delete g_notifyIcon;

    return msg.wParam;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

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

    case WM_TIMER:
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_QUIT:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case IDM_CHANGEICON:
            g_notifyIcon->ChangeIcon(LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_OVERLAYBATTERY_ICON)));
            break;
        }
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
