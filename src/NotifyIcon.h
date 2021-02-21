#include <Windows.h>
#include <tchar.h>

class NotifyIcon {
public:
    NotifyIcon(HWND hwnd, UINT id, HICON hIcon, LPCTSTR tip);
    ~NotifyIcon();

    static constexpr UINT WM_NOTIFYICON = WM_USER + 100;

    inline UINT GetID() { return id; }

    bool ChangeIcon(HICON hIcon);

private:
    HWND hwnd;
    UINT id;

    bool alive;

};
