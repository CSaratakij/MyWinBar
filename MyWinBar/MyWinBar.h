
#pragma once

#include "resource.h"

#define MAX_APPBAR_HEIGHT 20

#define TIMER_REDRAW_RATE 5000
#define TIMER_REDRAW_BAR_CENTER_RATE 30000

#define IDT_REDRAW_TIMER 1000
#define IDT_REDRAW_BAR_CENTER_TIMER 1001

#define black RGB(0, 0, 0)
#define white RGB(255, 255, 255)
#define goldYellow RGB(255, 185, 0)

#define colorBatteryHigh RGB(164, 244, 66)
#define colorBatteryLow RGB(244, 223, 65)
#define colorBatteryCritical RGB(255, 28, 28)

void MoveToRightSideOfScreen(HDC);
void PaintWorkspace(HDC);
void PaintCurrentFocusWindow(HDC);
void PaintBatteryInfo(HDC);
void PaintLocalTime(HDC);
