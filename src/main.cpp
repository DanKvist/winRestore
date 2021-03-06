#ifndef UNICODE
#define UNICODE
#endif

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

#include <windows.h>

#include "resource.h"

#define WM_USER_TRAY_ICON (WM_USER + 0x0100)

// Forward declarations
void OnSize(HWND hwnd, UINT flag, int width, int height);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lPAram);
BOOL CALLBACK myMonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT rect, LPARAM data);
BOOL CALLBACK myEnumWindowProc(HWND hwnd, LPARAM data);
bool CompareMONITORINFOEX(const MONITORINFOEX *monitor1, const MONITORINFOEX *monitor2);
bool compareMonitorSetting(const std::vector<MONITORINFOEX> *setting1, const std::vector<MONITORINFOEX> *setting2);
void RestoreWindows();
void ShowContextMenu(HWND hwnd, POINT pt);


//
// Class to store information about a window. Holds a handle to the window.
//
//  bool SetData(HWND hwnd)
//  Saves the Windows current placement state
//
//  bool RestoreWindow()
//  Restore the window to its' saved state
//
class WindowData
{
    HWND winHwnd;
    WINDOWPLACEMENT winPlace;
    WINDOWINFO winInfo;

public:
    WindowData() 
    {
        winHwnd = NULL;
        memset(&winPlace, 0, sizeof(WINDOWPLACEMENT));
        memset(&winInfo, 0, sizeof(WINDOWINFO));
    }

    bool SetData(HWND hwnd)
    {
        winHwnd = hwnd;
        winPlace.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwnd, &winPlace);
        winInfo.cbSize = sizeof(WINDOWINFO);
        GetWindowInfo(winHwnd, &winInfo);
        return true;
    }

    void RestoreWindow()
    {
        if (IsWindow(winHwnd))
        {
            switch (winPlace.showCmd)
            {
            case SW_MAXIMIZE:
                winPlace.showCmd = SW_SHOWNOACTIVATE;
                SetWindowPlacement(winHwnd, &winPlace);
                winPlace.showCmd = SW_MAXIMIZE;
                break;

            case SW_MINIMIZE:
            case SW_SHOWMINIMIZED:
                winPlace.showCmd = SW_SHOWMINNOACTIVE;
                break;

            case SW_NORMAL:
                winPlace.showCmd = SW_SHOWNOACTIVATE;
                break;
            
            default:
                break;
            }
            SetWindowPlacement(winHwnd, &winPlace);

            SetWindowPos(   winHwnd, 
                            HWND_TOP, 
                            winInfo.rcWindow.left, 
                            winInfo.rcWindow.top, 
                            winInfo.rcWindow.right - winInfo.rcWindow.left, 
                            winInfo.rcWindow.bottom - winInfo.rcWindow.top,
                            SWP_ASYNCWINDOWPOS | SWP_NOZORDER);
        }
    }

    bool DumpWindowInfo()
    {
        int bufSize = 512;
        char appName[bufSize];
        SendMessage(winHwnd, WM_GETTEXT, bufSize, (LPARAM)appName);
        printf("%S\n", appName);

        TCHAR winClass[40];
        RealGetWindowClass(winHwnd, winClass, 40);
        printf("Class: %S\n", winClass);

        GetWindowPlacement(winHwnd, &winPlace);
        if (winPlace.showCmd == SW_MAXIMIZE)
            printf("Is maximized\n");
        printf("winPlace flags: %i\n", winPlace.flags);
        printf("winPlace showCmd: %i\n", winPlace.showCmd);
        printf("winPlace ptMinPosition: (%i,%i)\n", winPlace.ptMinPosition.x, winPlace.ptMinPosition.y);
        printf("winPlace ptMaxPosition: (%i,%i)\n", winPlace.ptMaxPosition.x, winPlace.ptMaxPosition.y);
        printf("winPlace rcNormalPosition: (%i,%i), (%i,%i)\n", 
                winPlace.rcNormalPosition.top, winPlace.rcNormalPosition.left, 
                winPlace.rcNormalPosition.bottom, winPlace.rcNormalPosition.right);
        printf("winPlace size: %i x %i\n", 
                winPlace.rcNormalPosition.right - winPlace.rcNormalPosition.left, 
                winPlace.rcNormalPosition.bottom - winPlace.rcNormalPosition.top);

        GetWindowInfo(winHwnd, &winInfo);
        printf("winInfo position: %i x %i\n", winInfo.rcWindow.left, winInfo.rcWindow.top);
        printf("winInfo size: %i x %i\n\n", 
                winInfo.rcWindow.right - winInfo.rcWindow.left, 
                winInfo.rcWindow.bottom - winInfo.rcWindow.top);

        return true;
    }
};


// Global variables
HINSTANCE g_hInst = NULL;
const wchar_t CLASS_NAME[] = L"Sample Window Class";
std::vector<WindowData> windowDataList;
std::vector<MONITORINFOEX> savedMonitorSetup;
NOTIFYICONDATA niData;
UINT_PTR myTimer;
bool pollWindowSetup;


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevINstance, LPSTR lpCmdLine, int cmdShow) {
    g_hInst = hInstance;    

    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style;
    wcex.lpfnWndProc = WindowProc;;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground;
    wcex.lpszMenuName;
    wcex.lpszClassName = CLASS_NAME;
    wcex.hIconSm = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);

    RegisterClassEx(&wcex);


    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Learn to Program Windows",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        NULL,
        NULL,
        hInstance,
        NULL
        );

    if (hwnd == NULL)
        return 1;

    ZeroMemory(&niData, sizeof(NOTIFYICONDATA));
    niData.cbSize = sizeof(niData);
    niData.hWnd = hwnd;
    niData.uID = 1;
    niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
    niData.uCallbackMessage = WM_USER_TRAY_ICON;
    niData.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
    memcpy(niData.szTip, L"My tooltip!", 128);
    niData.uVersion = NOTIFYICON_VERSION_4;

    Shell_NotifyIcon(NIM_ADD, &niData);
    Shell_NotifyIcon(NIM_SETVERSION, &niData);

    // Grab monitor stuff
    std::cout << std::endl << "Display info:" << std::endl;
    EnumDisplayMonitors(NULL, NULL, &myMonitorEnumProc, reinterpret_cast<LPARAM>(&savedMonitorSetup));

    // for (int idx = 0; idx < savedMonitorSetup.size(); idx++)
    // {
    //     if (savedMonitorSetup[idx].dwFlags & MONITORINFOF_PRIMARY) 
    //         printf("Primary\n");

    //     printf("Device name is: %S\n", savedMonitorSetup[idx].szDevice);

    //     LONG width = savedMonitorSetup[idx].rcMonitor.right - savedMonitorSetup[idx].rcMonitor.left;
    //     LONG height = savedMonitorSetup[idx].rcMonitor.bottom - savedMonitorSetup[idx].rcMonitor.top;
    //     printf("Device resolution is: %i x %i\n\n", width, height);
    // }

    // Grab window stuff
    std::cout << std::endl << "Window info:" << std::endl;
    EnumDesktopWindows(NULL, &myEnumWindowProc, reinterpret_cast<LPARAM>(&windowDataList));

    // for (int idx = 0; idx < windowDataList.size(); idx++)
    // {
    //     windowDataList[idx].DumpWindowInfo();
    // }

    SetTimer(hwnd, myTimer, 10000, NULL);

    ShowWindow(hwnd, cmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }
    
    
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    // std::cout << "Message: " << uMsg << std::endl;

    switch (uMsg)
    {
    case WM_CREATE:
        {
            CreateWindow(L"BUTTON", L"Save", WS_VISIBLE | WS_CHILD, 20, 50, 80, 25, hwnd, (HMENU) IDM_SAVE, NULL, NULL);

            HWND hwndEdit;
            hwndEdit = CreateWindow(L"EDIT", L"DebugWin", WS_BORDER | WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY, 150, 20, 200, 300, hwnd, NULL, NULL, NULL);

            
            SetWindowText(hwndEdit, L"myTestString");
        }
        return 0;
    case WM_SIZE:
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            OnSize(hwnd, (UINT)wParam, width, height);
        }
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+2));

            EndPaint(hwnd, &ps);
        }
        return 0;

    case WM_POWERBROADCAST:
        if (wParam == PBT_APMSUSPEND) 
        {
            std::cout << "Suspended" << std::endl;
        } else if (wParam == PBT_APMRESUMEAUTOMATIC) {
            std::cout << "Resumed" << std::endl;
        }
        return 0;

    case WM_DISPLAYCHANGE:
        {
            std::vector<MONITORINFOEX> tempMonitorSetup;
            EnumDisplayMonitors(NULL, NULL, &myMonitorEnumProc, reinterpret_cast<LPARAM>(&tempMonitorSetup));

            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            std::cout << "Display change message" << std::endl;

            if (compareMonitorSetting(&savedMonitorSetup, &tempMonitorSetup))
            {
                std::cout << "Original resolution" << std::endl;
                Sleep(2000);

                RestoreWindows();
                pollWindowSetup = true;
            } else {
                std::cout << "Another resolution" << std::endl;
                pollWindowSetup = false;
            }
        }
        return 0;

    case WM_USER_TRAY_ICON:
        {
            switch (LOWORD(lParam))
            {
            case WM_LBUTTONUP:
                std::cout << "Left clicked the tray icon!" << std::endl;
                ShowWindow(hwnd, SW_SHOW);
                break;

            case WM_CONTEXTMENU:
            {
                std::cout << "User desired context menu!" << std::endl;
                POINT pt;
                GetCursorPos(&pt);
                std::cout << "Pressed at (" << pt.x << ", " << pt.y << ")" << std::endl;
                ShowContextMenu(hwnd, pt);
            }
            break;

            default:
                break;
            }
        }
        return 0;

    case WM_COMMAND:
        {
            int command = LOWORD(wParam);
            switch (command)
            {
            case IDM_EXIT:
                DestroyWindow(hwnd);
                break;

            case IDM_ABOUT:
                MessageBox(hwnd, L"This is a simple program designed to\naid in window management with dumb monitors", L"About winRestore", MB_OK | MB_ICONINFORMATION);
                break;

            case IDM_SAVE:
                std::cout << "User pressed save" << std::endl;
                break;
            
            default:
                std::cout << "Unknown message received" << std::endl;
                break;
            }
        }

    case WM_TIMER:
        if (pollWindowSetup)
        {
            std::cout << "Polled the windows" << std::endl;
            windowDataList.erase(windowDataList.begin(), windowDataList.end());
            EnumDesktopWindows(NULL, &myEnumWindowProc, reinterpret_cast<LPARAM>(&windowDataList));
        }

        return 0;

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        return 0;

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &niData);
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);   
    }
    return 0;
}

BOOL CALLBACK myMonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT rect, LPARAM lParam)
{
    std::vector<MONITORINFOEX>* pMonitors = reinterpret_cast<std::vector<MONITORINFOEX>*>(lParam);
    MONITORINFOEX iMonitor;
    iMonitor.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(hMonitor, &iMonitor);

    pMonitors->push_back(iMonitor);

    return 1;
}

BOOL CALLBACK myEnumWindowProc(HWND hwnd, LPARAM lParam)
{
    if (IsWindowVisible(hwnd) && GetParent(hwnd) == NULL)
    {
        DWORD dwExStyle = (DWORD)GetWindowLong(hwnd, GWL_EXSTYLE);
        if (((dwExStyle & WS_EX_OVERLAPPEDWINDOW) != 0) || ((dwExStyle & WS_EX_APPWINDOW) != 0) && ((dwExStyle & WS_EX_NOACTIVATE) == 0))
        {
            WindowData thisWindow;
            thisWindow.SetData(hwnd);

            auto pWindowDataList = reinterpret_cast<std::vector<WindowData>*>(lParam);
            pWindowDataList->push_back(thisWindow);
        }
    }
    return 1;
}

void RestoreWindows() 
{
    for (int idx = 0; idx < windowDataList.size(); idx++)
    {
        windowDataList[idx].RestoreWindow();
    }
}

bool compareMonitorSetting(const std::vector<MONITORINFOEX> *setting1, const std::vector<MONITORINFOEX> *setting2)
{
    if (setting1->size() != setting2->size())
        return false;

    for (int idx = 0; idx < setting1->size(); idx++)
    {
        if (!CompareMONITORINFOEX(&(setting1->at(idx)), &(setting2->at(idx))))
            return false;
    }

    return true;
}

bool CompareMONITORINFOEX(const MONITORINFOEX *monitor1, const MONITORINFOEX *monitor2)
{
    return (    EqualRect(&monitor1->rcMonitor, &monitor2->rcMonitor)
                && (monitor1->dwFlags == monitor2->dwFlags)
                && !wcscmp(monitor1->szDevice, monitor2->szDevice)
                );
}


void OnSize(HWND hwnd, UINT flag, int width, int height) 
{
    //Do stuff
}

void ShowContextMenu(HWND hwnd, POINT pt)
{
    HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDC_CONTEXTMENU));
    if (hMenu)
    {
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        if (hSubMenu)
        {
            // our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
            SetForegroundWindow(hwnd);

            // respect menu drop alignment
            UINT uFlags = TPM_RIGHTBUTTON;
            if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
            {
                uFlags |= TPM_RIGHTALIGN;
            }
            else
            {
                uFlags |= TPM_LEFTALIGN;
            }

            TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}