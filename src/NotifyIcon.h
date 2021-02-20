#include <Windows.h>

class NotifyIcon {
public:
    NotifyIcon(HWND &hwnd);
    ~NotifyIcon();

    void Show();

private:
    HWND &hwnd;

};
