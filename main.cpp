#ifndef UNICODE
#define UNICODE
#endif

#include <iostream>
#include <fstream>
#include <Windows.h>

void OnSize(HWND hwnd, UINT flag, int width, int height);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lPAram);

BOOL CALLBACK myInfoEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT rect, LPARAM data);

BOOL CALLBACK myEnumWindowProc(HWND hwnd, LPARAM data);

const wchar_t CLASS_NAME[] = L"Sample Window Class";

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevINstance, LPSTR lpCmdLine, int cmdShow) {
    std::ofstream logFile;
    
    logFile.open("log.txt");
    if (!logFile.is_open()) {
        std::cout << "Error: file not opened." << std::endl;
        return 1;
    }

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Learn to Program Windows",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
        );

    if (hwnd == NULL)
        return 1;


    std::cout << std::endl << "Display info:" << std::endl;
    EnumDisplayMonitors(NULL, NULL, &myInfoEnumProc, NULL);

    std::cout << std::endl << "Window info:" << std::endl;
    EnumWindows(&myEnumWindowProc, NULL);

    //Attempts to get display devices
    DISPLAY_DEVICE myDisplay;
    myDisplay.cb = sizeof(DISPLAY_DEVICE);
    
    ShowWindow(hwnd, cmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
//        std::cout << "Hello sir!" << std::endl;
//        Sleep(2000);

        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }
        

    

    logFile.close();
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            OnSize(hwnd, (UINT)wParam, width, height);
        }
        return 1;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+2));

            EndPaint(hwnd, &ps);
        }
        return 1;

    case WM_POWERBROADCAST:
        if (wParam == PBT_APMSUSPEND) 
        {
            std::cout << "Suspended" << std::endl;
        } else if (wParam == PBT_APMRESUMEAUTOMATIC) {
            std::cout << "Resumed" << std::endl;
        } 
        return 1;

    case WM_CLOSE:
        if (MessageBox(hwnd, L"Really quit?", L"My application", MB_OKCANCEL) == IDOK)
        {
            DestroyWindow(hwnd);
        }
        return 1;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 1;

    default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }        
    }
}

BOOL CALLBACK myInfoEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT rect, LPARAM data)
{
    MONITORINFOEX iMonitor;
    iMonitor.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(hMonitor, &iMonitor);

    if (iMonitor.dwFlags & MONITORINFOF_PRIMARY) 
        printf("Primary\n");

    printf("Device name is: %S\n", iMonitor.szDevice);

    LONG width = rect->right - rect->left;
    LONG height = rect->bottom - rect->top;
    printf("Device resolution is: %i x %i\n\n", width, height);
    return 1;
}

BOOL CALLBACK myEnumWindowProc(HWND hwnd, LPARAM data)
{
    WINDOWINFO winInfo;
    winInfo.cbSize = sizeof(WINDOWINFO);

    GetWindowInfo(hwnd, &winInfo);
    printf("Window position: %i x %i\n\n", winInfo.rcWindow.left, winInfo.rcWindow.top);

    return 1;
}


void OnSize(HWND hwnd, UINT flag, int width, int height) 
{
    //Do stuff
}