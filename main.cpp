#ifndef UNICODE
#define UNICODE
#endif

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <Windows.h>

// Forward declarations
void OnSize(HWND hwnd, UINT flag, int width, int height);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lPAram);
BOOL CALLBACK myMonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT rect, LPARAM data);
BOOL CALLBACK myEnumWindowProc(HWND hwnd, LPARAM data);
bool CompareMonitorSetup(const MONITORINFOEX *monitor1, const MONITORINFOEX *monitor2);
void RestoreWindows();


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
const wchar_t CLASS_NAME[] = L"Sample Window Class";
std::vector<WindowData> windowDataList;
std::vector<MONITORINFOEX> savedMonitorSetup;


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

    for (int idx = 0; idx < windowDataList.size(); idx++)
    {
        windowDataList[idx].DumpWindowInfo();
    }


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

    // std::cout << "Message: " << uMsg << std::endl;

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

    case WM_DISPLAYCHANGE:
        {
            std::vector<MONITORINFOEX> tempMonitorSetup;
            EnumDisplayMonitors(NULL, NULL, &myMonitorEnumProc, reinterpret_cast<LPARAM>(&tempMonitorSetup));

            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            std::cout << "Display change message" << std::endl;

            if (savedMonitorSetup.size() == tempMonitorSetup.size())
            {
                std::cout << "Same monitor count" << std::endl;

                bool sameSettings = true;
                for (int idx = 0; idx < savedMonitorSetup.size(); idx++)
                {
                    if (!CompareMonitorSetup(&savedMonitorSetup[idx], &tempMonitorSetup[idx]))
                        sameSettings = false;
                }

                if (sameSettings)
                {
                    std::cout << "Original resolution" << std::endl;
                    Sleep(2000);

                    RestoreWindows();
                }                
            }
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

bool CompareMonitorSetup(const MONITORINFOEX *monitor1, const MONITORINFOEX *monitor2)
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