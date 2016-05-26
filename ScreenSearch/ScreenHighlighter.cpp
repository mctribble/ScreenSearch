#include "ScreenHighlighter.h"

ScreenHighlighter::ScreenHighlighter()
{
	//get device context for the screen
	screenContext = GetDC(0);
}

ScreenHighlighter::~ScreenHighlighter()
{
	//done with the device context
	ReleaseDC(NULL, screenContext);
}

//draws a rectangular highlight at theese screen coordinates
void ScreenHighlighter::highlight(int left, int top, int right, int bottom)
{
	//change pen and brush to draw hollow red shapes
	HGDIOBJ oldSettings = SelectObject(screenContext, GetStockObject(DC_PEN));	//select draw color pen and save original settings
	SetDCPenColor(screenContext, RGB(255, 0, 0));								//set the pen draw color to red
	SelectObject(screenContext, GetStockObject(HOLLOW_BRUSH));					//use the hollow brush so we dont obscure highlighted regions

	//draw
	::Rectangle(screenContext, left, top, right, bottom);

	//restore original settings
	SelectObject(screenContext, oldSettings);
}

//fetches the rect of the given window and draws a highlight over it
void ScreenHighlighter::highlight(HWND hWnd)
{
	//fetch rect
	RECT windowRect;
	GetWindowRect(hWnd, &windowRect);

	//highlight it by calling the "standard" overload of this same function
	highlight(windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
}

//clears all highlights over the given window
void ScreenHighlighter::clearWindowHighlights(HWND hWnd)
{
	//invalidate the screen at the given window to force a redraw.  This results in removing leftover highlighting.
	RECT windowRect;
	GetWindowRect(hWnd, &windowRect);
	InvalidateRect(NULL, &windowRect, TRUE);
}
