
#include "rect.h"


int RectWidth(RECT* prect)
{
	return prect->right - prect->left;
}


int RectHeight(RECT* prect)
{
	return prect->bottom - prect->top;
}


void RectWidthHeight(RECT* prect, int* piWidth, int* piHeight)
{
	*piWidth = prect->right - prect->left;
	*piHeight = prect->bottom - prect->top;
}


void RectSize(RECT* prect, POINT* pptSize)
{
	pptSize->x = prect->right - prect->left;
	pptSize->y = prect->bottom - prect->top;
}


void RectOrigin(RECT* prect, POINT* pptOrigin)
{
	pptOrigin->x = prect->left + ((prect->right - prect->left) / 2);
	pptOrigin->y = prect->top + ((prect->bottom - prect->top) / 2);
}


void RectToPos(RECT* prect, POINT* pptSize, POINT* pptOrigin)
{
	POINT ptSize;

	ptSize.x = prect->right - prect->left;
	ptSize.y = prect->bottom - prect->top;

	if (pptSize != NULL)
	{
		pptSize->x = ptSize.x;
		pptSize->y = ptSize.y;
	}

	if (pptOrigin != NULL)
	{
		pptOrigin->x = prect->left + (ptSize.x / 2);
		pptOrigin->y = prect->top + (ptSize.y / 2);
	}
}


bool EqualRectSize(RECT* prect1, RECT* prect2)
{
	return (
		(RectWidth(prect1) == RectWidth(prect2)) &&
		(RectHeight(prect1) == RectHeight(prect2))
		);
}

