
#pragma once

#include <windows.h>

int RectWidth(RECT* prect);
int RectHeight(RECT* prect);
void RectWidthHeight(RECT* prect, int* piWidth, int* piHeight);
void RectSize(RECT* prect, POINT* pptSize);
void RectOrigin(RECT* prect, POINT* pptOrigin);
void RectToPos(RECT* prect, POINT* pptSize, POINT* pptOrigin);
bool EqualRectSize(RECT* prect1, RECT* prect2);

