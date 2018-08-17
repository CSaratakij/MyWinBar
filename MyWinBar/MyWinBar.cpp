
#include "stdafx.h"
#include "MyWinBar.h"

#define MAX_LOADSTRING 100

UINT currentWorkspace;
SYSTEMTIME localTime;
APPBARDATA appbarData;

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MYWINBAR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYWINBAR));
    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYWINBAR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(black);
	wcex.lpszMenuName = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, 0, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
      return FALSE;

   LONG appStyle = 0 | WS_DISABLED | WS_CHILD;

   SetWindowLong(hWnd, GWL_STYLE, appStyle);
   SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

   SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED); 

   appbarData.cbSize = sizeof(APPBARDATA);
   appbarData.hWnd = hWnd;

   //Register our bar (app bar)
   SHAppBarMessage(ABM_NEW, &appbarData);

   //Resize an appbar
   appbarData.rc.left = 0;
   appbarData.rc.right = GetSystemMetrics(SM_CXSCREEN);

   appbarData.rc.top = 0;
   appbarData.rc.bottom = 0;

   appbarData.uEdge = ABE_TOP;

   //Query pos
   SHAppBarMessage(ABM_QUERYPOS, &appbarData);

   //Adjust appbar size
   appbarData.rc.bottom = (appbarData.rc.top + MAX_APPBAR_HEIGHT);

   //Pass final rect to system
   SHAppBarMessage(ABM_SETPOS, &appbarData);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   MoveWindow(hWnd,
	   appbarData.rc.left,
	   appbarData.rc.top,
	   appbarData.rc.right - appbarData.rc.left,
	   appbarData.rc.bottom - appbarData.rc.top,
	   TRUE
   );

   SetTimer(hWnd, IDT_REDRAW_TIMER, TIMER_REDRAW_RATE, NULL);
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

	case WM_TIMER:
	{
		switch (wParam)
		{
		case IDT_REDRAW_TIMER:
		{
			InvalidateRect(hWnd, 0, TRUE);
			break;
		}

		default:
			break;
		}

		break;
	}

    case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		PaintWorkspace(hdc);
		MoveToRightSideOfScreen(hdc);
		PaintLocalTime(hdc);

		EndPaint(hWnd, &ps);
        break;
	}

	case WM_COPYDATA:
	{
		PCOPYDATASTRUCT p = (PCOPYDATASTRUCT) lParam;

		if (p->dwData == 1) {
			currentWorkspace = *((UINT*)p->lpData);
			InvalidateRect(hWnd, 0, TRUE);
		}

		break;
	}

    case WM_DESTROY:
	{
		KillTimer(hWnd, IDT_REDRAW_TIMER);
		SHAppBarMessage(ABM_REMOVE, &appbarData);
        PostQuitMessage(0);
        break;
	}

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

void MoveToRightSideOfScreen(HDC hdc)
{
	SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_UPDATECP);

	SetBkColor(hdc, black);
	SetTextColor(hdc, white);

	int beginPos = GetSystemMetrics(SM_CXSCREEN) * 97 / 100;
	MoveToEx(hdc, beginPos, 0, NULL);
}

void PaintWorkspace(HDC hdc)
{
	TCHAR buffer[5];
	UINT offset = 18;

	for (UINT i = 0; i < 10; ++i) {
		if (i == (currentWorkspace - 1)) {
			SetBkColor(hdc, goldYellow);
			SetTextColor(hdc, black);
		}
		else {
			SetBkColor(hdc, black);
			SetTextColor(hdc, white);
		}
		
		int length = wsprintf(buffer, _T(" %d "), i + 1);
		TextOut(hdc, offset * i, 0, buffer, length);
	}
}

void PaintLocalTime(HDC hdc)
{
	GetLocalTime(&localTime);

	TCHAR txtTime[6];
	int txtTimeLength = wsprintf(txtTime, _T("%02d:%02d"), localTime.wHour, localTime.wMinute);

	TextOut(hdc, 0, 0, txtTime, txtTimeLength);
}
