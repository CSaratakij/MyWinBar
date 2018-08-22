
#include "stdafx.h"
#include "MyWinBar.h"

#define MAX_LOADSTRING 100

UINT currentWorkspace;
unsigned short currentWorkspaceInfo = 0;

HWND currentFocusWindow;
APPBARDATA appbarData;

RECT rectLeft;
RECT rectCenter;
RECT rectRight;

int focusWindowTextBufferLength = MAX_LOADSTRING;
TCHAR focusWindowTextBuffer[MAX_LOADSTRING];

RECT rectBatteryBG;
RECT rectBatteryStatusBG;

HBRUSH brushBatteryHigh;
HBRUSH brushBatteryLow;
HBRUSH brushBatteryCritical;

SYSTEMTIME localTime;
SYSTEM_POWER_STATUS powerStatus;

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

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MYWINBAR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
        return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYWINBAR));
    MSG msg;

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

   LONG appStyle = (0 | WS_DISABLED | WS_CHILD);

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

   //Split bar equally
   int barPortion = GetSystemMetrics(SM_CXSCREEN) / 3;
   int shrinkPercent = 30;
   int shrinkPortion = (int) ((barPortion * shrinkPercent) / 100);

   rectLeft.left = 0;
   rectLeft.top = 0;
   rectLeft.right = (barPortion - shrinkPortion);
   rectLeft.bottom = MAX_APPBAR_HEIGHT;

   rectCenter.left = rectLeft.right;
   rectCenter.top = 0;
   rectCenter.right = (barPortion * 2) + shrinkPortion;
   rectCenter.bottom = MAX_APPBAR_HEIGHT;

   rectRight.left = rectCenter.right;
   rectRight.top = 0;
   rectRight.right = (barPortion * 3);
   rectRight.bottom = MAX_APPBAR_HEIGHT;

   brushBatteryHigh = CreateSolidBrush(colorBatteryHigh);
   brushBatteryLow = CreateSolidBrush(colorBatteryLow);
   brushBatteryCritical = CreateSolidBrush(colorBatteryCritical);

   GetLocalTime(&localTime);
   GetSystemPowerStatus(&powerStatus);

   SetTimer(hWnd, IDT_REDRAW_TIMER, TIMER_REDRAW_RATE, NULL);
   SetTimer(hWnd, IDT_REDRAW_BAR_CENTER_TIMER, TIMER_REDRAW_BAR_CENTER_RATE, NULL);

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
			GetLocalTime(&localTime);
			GetSystemPowerStatus(&powerStatus);
			InvalidateRect(hWnd, &rectRight, TRUE);
			break;
		}

		case IDT_REDRAW_BAR_CENTER_TIMER:
		{
			InvalidateRect(hWnd, &rectCenter, TRUE);
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
		PaintCurrentFocusWindow(hdc);

		PaintBatteryInfo(hdc);

		MoveToRightSideOfScreen(hdc);
		PaintLocalTime(hdc);

		EndPaint(hWnd, &ps);
        break;
	}

	case WM_COPYDATA:
	{
		PCOPYDATASTRUCT p = (PCOPYDATASTRUCT) lParam;

		if (p->dwData == APPBAR_UPDATE_CURRENT_WORKSPACE) {
			currentWorkspaceInfo = *(unsigned short*)p->lpData;
			currentWorkspace = (currentWorkspaceInfo >> MAX_WORKSPACE);
			InvalidateRect(hWnd, &rectLeft, TRUE);
		}
		else if (p->dwData == APPBAR_UPDATE_CURRENT_FOCUS_WINDOW) {
			currentFocusWindow = *(HWND*)p->lpData;
			InvalidateRect(hWnd, &rectCenter, TRUE);
		}

		break;
	}

    case WM_DESTROY:
	{
		KillTimer(hWnd, IDT_REDRAW_TIMER);
		KillTimer(hWnd, IDT_REDRAW_BAR_CENTER_TIMER);

		DeleteObject(brushBatteryHigh);
		DeleteObject(brushBatteryLow);
		DeleteObject(brushBatteryCritical);

		SHAppBarMessage(ABM_REMOVE, &appbarData);
        PostQuitMessage(0);

        break;
	}

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

void PaintWorkspace(HDC hdc)
{
	TCHAR buffer[5];
	UINT offset = 18;

	unsigned short checkWorkspaceFlag = 1;
	int offsetMultiplier = 0;

	for (UINT i = 0; i < MAX_WORKSPACE; ++i) {

		if (i == (currentWorkspace - 1)) {
			SetBkColor(hdc, goldYellow);
			SetTextColor(hdc, black);

			int length = wsprintf(buffer, _T(" %d "), currentWorkspace);
			TextOut(hdc, (offset * offsetMultiplier), 0, buffer, length);
			offsetMultiplier += 1;
		}
		else {
			SetBkColor(hdc, black);
			SetTextColor(hdc, white);

			if ((checkWorkspaceFlag & currentWorkspaceInfo) == checkWorkspaceFlag) {
				int length = wsprintf(buffer, _T(" %d "), i + 1);
				TextOut(hdc, (offset * offsetMultiplier), 0, buffer, length);
				offsetMultiplier += 1;
			}
		}

		checkWorkspaceFlag <<= 1;
	}
}

void PaintCurrentFocusWindow(HDC hdc)
{
	SetBkColor(hdc, black);
	SetTextColor(hdc, goldYellow);

	HWND hWnd = currentFocusWindow;

	if (hWnd == NULL) {
		DrawTextW(hdc, 0, 0, &rectCenter, DT_CENTER | DT_VCENTER);
	}
	else {
		int length = GetWindowText(hWnd, focusWindowTextBuffer, focusWindowTextBufferLength);
		if (length > 0)
			DrawTextW(hdc, focusWindowTextBuffer, length, &rectCenter, DT_CENTER | DT_VCENTER);
	}
}

void PaintBatteryInfo(HDC hdc)
{
	if (powerStatus.BatteryFlag == 128 || powerStatus.BatteryFlag == 255)
		return;

	rectBatteryBG;
	rectBatteryStatusBG;

	int offset = 2;

	rectBatteryBG.left = GetSystemMetrics(SM_CXSCREEN) * 92 / 100;
	rectBatteryBG.top = 2;
	rectBatteryBG.right = GetSystemMetrics(SM_CXSCREEN) * 94 / 100;
	rectBatteryBG.bottom = (MAX_APPBAR_HEIGHT - 6);

	rectBatteryStatusBG.left = rectBatteryBG.left + offset;
	rectBatteryStatusBG.top = rectBatteryBG.top + offset;
	rectBatteryStatusBG.right = rectBatteryBG.right - offset;
	rectBatteryStatusBG.bottom = rectBatteryBG.bottom - offset;

	Rectangle(hdc, rectBatteryBG.left, rectBatteryBG.top, rectBatteryBG.right, rectBatteryBG.bottom);

	int rectBatteryStatusBGSize = (rectBatteryStatusBG.right - rectBatteryStatusBG.left);
	int rectBatteryStatusBGSizePortion = (rectBatteryStatusBGSize / 4);

	if (powerStatus.BatteryLifePercent == 255)
		return;

	if (powerStatus.BatteryLifePercent >= 90) {
		FillRect(hdc, &rectBatteryStatusBG, brushBatteryHigh);
	}

	else if (powerStatus.BatteryLifePercent >= 60 && powerStatus.BatteryLifePercent < 90) {
		rectBatteryStatusBG.left = rectBatteryStatusBG.right - (rectBatteryStatusBGSizePortion * 3);
		FillRect(hdc, &rectBatteryStatusBG, brushBatteryHigh);
	}

	else if (powerStatus.BatteryLifePercent >= 30 && powerStatus.BatteryLifePercent < 60) {
		rectBatteryStatusBG.left = rectBatteryStatusBG.right - (rectBatteryStatusBGSizePortion * 2);
		FillRect(hdc, &rectBatteryStatusBG, brushBatteryHigh);
	}

	else if (powerStatus.BatteryLifePercent >= 15 && powerStatus.BatteryLifePercent < 30) {
		rectBatteryStatusBG.left = rectBatteryStatusBG.right - (rectBatteryStatusBGSizePortion);
		FillRect(hdc, &rectBatteryStatusBG, brushBatteryLow);
	}

	else if (powerStatus.BatteryLifePercent >= 0 && powerStatus.BatteryLifePercent < 15) {
		rectBatteryStatusBG.left = rectBatteryStatusBG.right - (rectBatteryStatusBGSizePortion / 2);
		FillRect(hdc, &rectBatteryStatusBG, brushBatteryCritical);
	}
}

void MoveToRightSideOfScreen(HDC hdc)
{
	SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_UPDATECP);

	SetBkColor(hdc, black);
	SetTextColor(hdc, white);

	int beginPos = GetSystemMetrics(SM_CXSCREEN) * 95 / 100;
	MoveToEx(hdc, beginPos, 0, NULL);
}

void PaintLocalTime(HDC hdc)
{
	WORD resultHour;

	if (localTime.wHour == 0)
		resultHour = 12;

	else if (localTime.wHour < 13)
		resultHour = localTime.wHour;

	else
		resultHour = (localTime.wHour - 12);

	LPCWSTR labelTimePeriod = (localTime.wHour < 12) ? _T("AM") : _T("PM");

	TCHAR txtTime[9];
	int txtTimeLength = wsprintf(txtTime, _T("%02d:%02d %s"), resultHour, localTime.wMinute, labelTimePeriod);

	TextOut(hdc, 0, 0, txtTime, txtTimeLength);
}
