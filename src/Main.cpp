#include <Windows.h>
#include "NotifyIcon.h"

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

NotifyIcon *g_notifyIcon;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
    constexpr LPCTSTR WindowClassName = TEXT("MainWindowClass");

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProcedure;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = WindowClassName;
    wcex.hIconSm = nullptr;

    if (!RegisterClassEx(&wcex)) {
        return -1;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST/* | WS_EX_NOACTIVATE*/,
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
                info.wID = 0;
                info.dwTypeData = (LPWSTR) TEXT("Quit");
                InsertMenuItem(hMenu, 0, true, &info);

                POINT point;
                GetCursorPos(&point);
                TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hwnd, nullptr);
                DestroyMenu(hMenu);

                break;
            }
        }
        return 0;

    case WM_TIMER:
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
