
#pragma once

#include "resource.h"

#define MAX_APPBAR_HEIGHT 20
#define TIMER_REDRAW_RATE 5000

#define IDT_REDRAW_TIMER 1000

#define black RGB(0, 0, 0)
#define white RGB(255, 255, 255)
#define goldYellow RGB(255, 185, 0)

void MoveToRightSideOfScreen(HDC);
void PaintWorkspace(HDC);
void PaintLocalTime(HDC);
