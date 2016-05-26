// ScreenSearch.cpp : Application entry point
// ScreenSeach by Christopher Hake (mctribble)
// locates various ojects in images either captured at runtime or imported from files

#include <iostream>
#include "windowEnumerator.h"
#include <io.h>
#include <fcntl.h>

using namespace std;

int main(int argc, wchar_t* argv[])
{
	//ensure wcout will actually support unicode (http://stackoverflow.com/a/19258509)
	//WARNING: this seems to break cout, though there isnt much reason to use it anyway in a unicode app
	_setmode(_fileno(stdout), _O_U16TEXT);

	//TEST 1

	wcout << "TEST 1: fetch top level windows" << endl;

	//populate and fetch the list of open windows
	WindowEnumerator::getInstance()->topLevelWindowList();
	vector<WindowData>* windowList = WindowEnumerator::getInstance()->getWindowList();

	//print info of each
	for (unsigned int i = 0; i < windowList->size(); i++)
	{
		wcout << windowList->at(i).handle << ": " << windowList->at(i).title << endl;
	}

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();

	//END TEST 1

	//TEST 2

	//declaring this way is a slight memory waste, but ensures the window title set here will actually fit the space alloted.
	wchar_t TARGET_WINDOW_TITLE[WindowData::MAX_TITLE_LENGTH];
	lstrcpy(TARGET_WINDOW_TITLE, L"Calculator"); //tested on Win10 Calculator app, but should work on anything with this title

	wcout << endl << "TEST 2: find children of \"" << TARGET_WINDOW_TITLE << "\" window, if it exists" << endl;

	//populate the list of open windows (we dont have to retrieve it this time because we already have the pointer)
	WindowEnumerator::getInstance()->topLevelWindowList();

	//find a window titled TARGET_WINDOW_TITLE

	//find a window with matching title
	HWND targetWindow = NULL;
	for (unsigned int i = 0; i < windowList->size(); i++)
	{
		if (lstrcmp(TARGET_WINDOW_TITLE, windowList->at(i).title) == 0)
		{
			targetWindow = windowList->at(i).handle;
			break;
		}
	}

	//confirm results
	if (targetWindow == NULL)
	{
		wcout << "Could not find a window with title " << TARGET_WINDOW_TITLE << endl;
	}
	else 
	{
		//we found the target window.  Enumerate its children and list them.
		wcout << "Children of " << TARGET_WINDOW_TITLE << ":" << endl;

		//populate the list of children (we dont have to retrieve it this time because we already have the pointer)
		WindowEnumerator::getInstance()->childWindowList(targetWindow);

		//print info of each
		if (windowList->size() == 0)
			wcout << "<none>" << endl;
		for (unsigned int i = 0; i < windowList->size(); i++)
		{
			wcout << windowList->at(i).handle << ": " << windowList->at(i).title << endl;
		}
	}

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();

	//END TEST 2

	return 0;
}



