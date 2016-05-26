// ScreenSearch.cpp : Application entry point
// ScreenSeach by Christopher Hake (mctribble)
// locates various ojects in images either captured at runtime or imported from files

#include <fcntl.h>
#include <iostream>
#include <io.h>
#include "windowEnumerator.h"
#include "ScreenHighlighter.h"

using namespace std;

//predeclarations
void testDriver();

int main(int argc, wchar_t* argv[])
{
	//ensure wcout will actually support unicode (http://stackoverflow.com/a/19258509)
	//WARNING: this seems to break cout, though there isnt much reason to use it anyway in a unicode app
	_setmode(_fileno(stdout), _O_U16TEXT);

	testDriver(); //for now, just run the test driver.  Later this will be a menu option

	return 0;
}

//tests the system
void testDriver()
{
	//TEST 1: List top-level windows

	wcout << "TEST 1: fetch top level windows" << endl;

	//populate and fetch the list of open windows
	WindowEnumerator::getInstance()->topLevelWindowList();
	vector<WindowData>* windowList = WindowEnumerator::getInstance()->getWindowList();

	//print info of each
	for (unsigned int i = 0; i < windowList->size(); i++)
	{
		wcout << windowList->at(i).handle << ": " << windowList->at(i).title << endl;
	}

	//END TEST 1: List top-level windows

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();

	//TEST 2: List children of given window

	//declaring this way is a slight memory waste, but ensures the window title set here will actually fit the space alloted.
	wchar_t TEST2_TARGET_WINDOW_TITLE[WindowData::MAX_TITLE_LENGTH];
	lstrcpy(TEST2_TARGET_WINDOW_TITLE, L"Calculator"); //tested on Win10 Calculator app, but should work on anything with this title

	wcout << endl << "TEST 2: find children of \"" << TEST2_TARGET_WINDOW_TITLE << "\" window, if it exists" << endl;

	//populate the list of open windows (we dont have to retrieve it this time because we already have the pointer)
	WindowEnumerator::getInstance()->topLevelWindowList();

	//find a window with matching title
	HWND targetWindow = NULL;
	for (unsigned int i = 0; i < windowList->size(); i++)
	{
		if (lstrcmp(TEST2_TARGET_WINDOW_TITLE, windowList->at(i).title) == 0)
		{
			targetWindow = windowList->at(i).handle;
			break;
		}
	}

	//confirm results
	if (targetWindow == NULL)
	{
		wcout << "Could not find a window with title " << TEST2_TARGET_WINDOW_TITLE << endl;
	}
	else
	{
		//we found the target window.  Enumerate its children and list them.
		wcout << "Children of " << TEST2_TARGET_WINDOW_TITLE << ":" << endl;

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

	//END TEST 2: List children of given window

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();

	//TEST 3: highlight children of window from test 2
		
	wcout << endl << "TEST 3: highlight child windows found in test 2" << endl;

	//only perform test 3 if test 2 succeeded, since it relies on the results of test 2
	if ((targetWindow == NULL) || (windowList->size() == 0))
	{
		wcout << "Could not perform test: Test 2 did not find a window with children to highlight." << endl;
	}
	else
	{
		//repeatedly draw highlights until escape is pressed
		wcout << "Highlighting child windows.  Hold escape to clear highlights and continue." << endl;
		while ((GetKeyState(VK_ESCAPE) & 0x80) == 0) //0x80 is the "high bit" of the key state, and indicates whether the key is pressed
		{
			//draw a highlight for each child window
			for (unsigned int i = 0; i < windowList->size(); i++)
			{
				ScreenHighlighter::getInstance()->highlight(windowList->at(i).handle); //draw highlight over the window, if it is visible
			}

			Sleep(500); //dont hog system resources to do nothing but draw rectangles over and over
		}

		//clear highlights for the parent window
		ScreenHighlighter::getInstance()->clearWindowHighlights(targetWindow);
	}

	//END TEST 3: highlight children of given window
	
	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();

}

