#include <stdexcept>
#include "NotifyIcon.h"

NotifyIcon::NotifyIcon(HWND hwnd, UINT id, HICON hIcon, LPCTSTR tip)
    : hwnd(hwnd), id(id), alive(true) {
    NOTIFYICONDATA data;
    data.cbSize = sizeof(NOTIFYICONDATA);
    data.hWnd = hwnd;
    data.uID = id;
    data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    data.uCallbackMessage = WM_NOTIFYICON;
    data.hIcon = hIcon;
    _tcscpy_s(data.szTip, tip);

    if (!Shell_NotifyIcon(NIM_ADD, &data)) {
        throw std::runtime_error { "Failed to run Shell_NotifyIcon function." };
    }
};

NotifyIcon::~NotifyIcon() {
    if (!alive) {
        return;
    }

    NOTIFYICONDATA data;
    data.cbSize = sizeof(NOTIFYICONDATA);
    data.hWnd = hwnd;
    data.uID = id;
    data.uFlags = 0;

    Shell_NotifyIcon(NIM_DELETE, &data);
}

bool NotifyIcon::ChangeIcon(HICON hIcon) {
    NOTIFYICONDATA data;
    data.cbSize = sizeof(NOTIFYICONDATA);
    data.hWnd = hwnd;
    data.uID = id;
    data.uFlags = NIF_ICON;
    data.hIcon = hIcon;

    return Shell_NotifyIcon(NIM_MODIFY, &data);
}
