#include "PopupMenu.h"

void PopupMenu::AddItem(UINT id, LPCTSTR text) {
    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_ID | MIIM_STRING;
    mii.wID = id;
    mii.dwTypeData = (LPTSTR) text;

    InsertMenuItem(hMenu, GetMenuItemCount(hMenu), true, &mii);
}

void PopupMenu::SetBackground(HBRUSH hBrush) {
    MENUINFO mi;
    mi.cbSize = sizeof(mi);
    mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
    mi.hbrBack = hBrush;

    SetMenuInfo(hMenu, &mi);
}

void PopupMenu::Show(HWND hwnd, int x, int y) {
    SetForegroundWindow(hwnd);
    TrackPopupMenuEx(hMenu, uFlags, x, y, hwnd, nullptr);
    SendMessage(hwnd, WM_NULL, 0, 0);
}
