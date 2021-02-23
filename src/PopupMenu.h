#pragma once
#include <Windows.h>

class PopupMenu {
public:
    PopupMenu() : 
        hMenu(CreatePopupMenu()),
        uFlags(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON) { }
    ~PopupMenu() = default;

    void AddItem(UINT id, LPCTSTR text);
    void SetBackground(HBRUSH hBrush);
    inline void SetStyle(UINT uFlags) { this->uFlags = uFlags; }
    void Show(HWND hwnd, int x, int y);

private:
    HMENU hMenu;
    UINT uFlags;

};
