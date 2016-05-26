#include "windowEnumerator.h"

//adds hWnd to the window list.  Helper function for updateWindowList() that windows calls once for each window
BOOL CALLBACK addWindow(HWND hWnd, LPARAM lParam);

//purges and repopulates the window list.  Should be called prior to searching said list to ensure contents are accurate
void updateWindowList()
{
	EnumWindows(addWindow, NULL);
}

//adds hWnd to the window list.  Helper function for updateWindowList() that windows calls once for each window
BOOL CALLBACK addWindow(HWND hWnd, LPARAM lParam)
{
	//create a struct for this window
	WindowData newData;

	//populate it
	newData.handle = hWnd;
	GetWindowText(hWnd, newData.title, newData.MAX_TITLE_LENGTH);

	//add it to the list

	return TRUE;
}
