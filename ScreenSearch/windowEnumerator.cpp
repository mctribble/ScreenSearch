#include "windowEnumerator.h"
#include <iostream>

using namespace std;

//predeclaration
BOOL CALLBACK addWindow(HWND hWnd, LPARAM lParam);

/*
 *	purges and repopulates the window list with all top-level windows.  Should be called prior to searching said list to ensure contents are accurate
 *	note that many things that users dont think about as windows are in fact windows, including taskbar icons, the start button, and individual browser tabs.
 *	as a result, this list is MUCH larger than you might expect.
 *	I intentionally chose not to cull this list in case there is ever a reason to poll such things, but it is straightforward to cull this list.  
 *	See https://blogs.msdn.microsoft.com/oldnewthing/20071008-00/?p=24863/ and https://msdn.microsoft.com/en-us/library/windows/desktop/ms633530%28v=vs.85%29.aspx
 */
void WindowEnumerator::topLevelWindowList()
{
	//empty the list
	windowList.clear();

	//fill it up again (windows calls addWindow once for each open window)
	EnumWindows(addWindow, NULL); 
}

//like above, but returns all windows that are children of the given window
void WindowEnumerator::childWindowList(HWND parentWindow)
{
	//empty the list
	windowList.clear();

	//fill it up again (windows calls addWindow once for each child)
	EnumChildWindows(parentWindow, addWindow, NULL);
}

HWND WindowEnumerator::findWindowByTitle(LPCWSTR title)
{
	HWND result = NULL;
	for (unsigned int i = 0; i < windowList.size(); i++)
	{
		if (lstrcmp(title, windowList.at(i).title) == 0)
		{
			result = windowList.at(i).handle;
			break;
		}
	}
	return result;
}

//adds hWnd to the window list.  Helper function for topLevelWindowList() and childWindowList() that windows calls once for each window
BOOL CALLBACK addWindow(HWND hWnd, LPARAM lParam)
{
	WindowData newData; //create a struct for this window

	//populate it
	newData.handle = hWnd; //save the handle
	DWORD result = GetWindowText(hWnd, newData.title, WindowData::MAX_TITLE_LENGTH); //get the window title
	
	//error checking
	if (result == 0)
	{
		//GetWindowText failed to give us a title.  put in a placeholder
		lstrcpy(newData.title, L"<untitled>");

		//now try to work out what happened
		int errorCode = GetLastError();

		//GetWindowText() can return zero for a lot of reasons, including the window just not having a title.
		//if the "error code" is 0, there wasn't an error at all and we can ignore it
		//otherwise, we print the error code (see https://msdn.microsoft.com/en-us/library/cc231199.aspx)
		if (errorCode != 0)
			wcout << "ERROR: " << errorCode << endl;
	}

	//add it to the list
	WindowEnumerator::getInstance()->getWindowList()->push_back(newData);

	return TRUE;
}
