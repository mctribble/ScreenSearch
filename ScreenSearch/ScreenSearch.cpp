// ScreenSearch.cpp : Application entry point
// ScreenSeach by Christopher Hake (mctribble)
// locates various ojects in images either captured at runtime or imported from files

#include <fcntl.h>
#include "HBitmapUtils.h"
#include <iostream>
#include <io.h>
#include "windowEnumerator.h"
#include "ScreenHighlighter.h"
#include <gdiplus.h>

#pragma comment(lib,"gdiplus.lib")

using namespace std;
using namespace Gdiplus;

//predeclarations
void testDriver();

int main(int argc, wchar_t* argv[])
{
	//ensure wcout will actually support unicode (http://stackoverflow.com/a/19258509)
	//WARNING: this seems to break cout, though there isnt much reason to use it anyway in a unicode app
	_setmode(_fileno(stdout), _O_U16TEXT);

	//init GDI+
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	testDriver(); //for now, just run the test driver.  Later this will be a menu option

	//shut down GDI+
	GdiplusShutdown(gdiplusToken);

	return 0;
}

//tests the system
void testDriver()
{
	//used by multiple tests in this driver
	HWND targetWindow = NULL;
	vector<WindowData>* windowList = WindowEnumerator::getInstance()->getWindowList();

	//TEST 1: List top-level windows

	wcout << "TEST 1: fetch top level windows" << endl;

	//populate and fetch the list of open windows
	WindowEnumerator::getInstance()->topLevelWindowList();

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

	//declaring this way is a slight memory waste, but ensures the window title set here will actually fit the space alloted for titles in WindowData.
	wchar_t TEST2_TARGET_WINDOW_TITLE[WindowData::MAX_TITLE_LENGTH];
	lstrcpy(TEST2_TARGET_WINDOW_TITLE, L"Calculator"); //tested on Win10 Calculator app, but should work on anything with this title

	wcout << endl << "TEST 2: find children of \"" << TEST2_TARGET_WINDOW_TITLE << "\" window, if it exists" << endl;

	//populate the list of open windows 
	WindowEnumerator::getInstance()->topLevelWindowList();

	//find a window with matching title
	targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(TEST2_TARGET_WINDOW_TITLE);

	//if a window was not found, prompt user and wait until it exists.
	if (targetWindow == NULL)
	{
		wcout << "Could not find a window with title " << TEST2_TARGET_WINDOW_TITLE << ".  Please create one to continue." << endl; 
		while (targetWindow == NULL)
		{
			Sleep(1000);
			WindowEnumerator::getInstance()->topLevelWindowList();
			targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(TEST2_TARGET_WINDOW_TITLE);
		}
	}
	
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
	

	//END TEST 2: List children of given window

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();

	//TEST 3: highlight children of target window
		
	//declaring this way is a slight memory waste, but ensures the window title set here will actually fit the space alloted for titles in WindowData.
	wchar_t TEST3_TARGET_WINDOW_TITLE[WindowData::MAX_TITLE_LENGTH];
	lstrcpy(TEST3_TARGET_WINDOW_TITLE, L"Calculator"); //tested on Win10 Calculator app, but should work on anything with this title that has children

	wcout << endl << "TEST 3: highlight children of \"" << TEST3_TARGET_WINDOW_TITLE << "\" window" << endl;

	//populate the list of open windows (we dont have to retrieve it this time because we already have the pointer)
	WindowEnumerator::getInstance()->topLevelWindowList();

	//find a window with matching title
	targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(TEST3_TARGET_WINDOW_TITLE);

	//if a window was not found, prompt user and wait until it exists.
	if (targetWindow == NULL)
	{
		wcout << "Could not find a window with title " << TEST3_TARGET_WINDOW_TITLE << ".  Please create one to continue." << endl;
		while (targetWindow == NULL)
		{
			Sleep(1000);
			WindowEnumerator::getInstance()->topLevelWindowList();
			targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(TEST3_TARGET_WINDOW_TITLE);
		}
	}

	//repopulate window list with children of the target window.  (note: commenting this out results in red boxes EVERYWHERE)
	WindowEnumerator::getInstance()->childWindowList(targetWindow);

	//repeatedly draw highlights until escape is pressed
	wcout << "Highlighting child windows.  Hold escape to clear highlights and continue." << endl;
	while ((GetKeyState(VK_ESCAPE) & 0x80) == 0) //0x80 is the "high bit" of the key state, and indicates whether the key is pressed
	{
		//draw a highlight for each child window
		for (unsigned int i = 0; i < windowList->size(); i++)
		{
			ScreenHighlighter::getInstance()->highlight(windowList->at(i).handle); //draw highlight over the window, if it is visible
		}

		Sleep(100); //dont hog system resources to do nothing but draw rectangles over and over
	}

	//clear highlights
	ScreenHighlighter::getInstance()->clearWindowHighlights();

	//END TEST 3: highlight children of target window
	
	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();

	//TEST 4: save screencap of a window
	
	//declaring this way is a slight memory waste, but ensures the window title set here will actually fit the space alloted for titles in WindowData.
	wchar_t TEST4_TARGET_WINDOW_TITLE[WindowData::MAX_TITLE_LENGTH];
	lstrcpy(TEST4_TARGET_WINDOW_TITLE, L"Calculator"); //tested on Win10 Calculator app, but should work on anything with this title

	wcout << endl << "TEST 4: capture screenshot of \"" << TEST4_TARGET_WINDOW_TITLE << "\" window and save it to a file" << endl;

	//populate the list of open windows (we dont have to retrieve it this time because we already have the pointer)
	WindowEnumerator::getInstance()->topLevelWindowList();

	//find a window with matching title
	targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(TEST4_TARGET_WINDOW_TITLE);

	//if a window was not found, prompt user and wait until it exists.
	if (targetWindow == NULL)
	{
		wcout << "Could not find a window with title " << TEST4_TARGET_WINDOW_TITLE << ".  Please create one to continue." << endl;
		while (targetWindow == NULL)
		{
			Sleep(1000);
			WindowEnumerator::getInstance()->topLevelWindowList();
			targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(TEST4_TARGET_WINDOW_TITLE);
		}
	}

	//get screencap
	HBITMAP test4Bitmap = NULL;
	if (bitmapFromWindow(targetWindow, &test4Bitmap) == false)
	{
		wcout << "failed to perform screen capture!" << endl;
	}
	else
	{
		//save it
		bitmapToFile(&test4Bitmap, L"test4.bmp");
	}

	//cleanup
	DeleteObject(test4Bitmap);

	//END TEST 4: save screencap of a window
	
	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();

	//TEST 5: load an image and save it as something else

	//declaring this way is a slight memory waste, but ensures the window title set here will actually fit the space alloted for titles in WindowData.
	wchar_t TEST5_IN[20] = L"test4.bmp";
	wchar_t TEST5_OUT[20] = L"test5.png";

	wcout << endl << "TEST 5: load " << TEST5_IN << " and save it to " << TEST5_OUT << endl;
	HBITMAP test5Bitmap = NULL;
	if (bitmapFromFile(TEST5_IN, &test5Bitmap) == false)
	{
		wcout << "Load failed!" << endl;
	}
	else 
	{
		if (bitmapToFile(&test5Bitmap, TEST5_OUT) == false)
		{
			wcout << "Save failed!" << endl;
		}
	}

	//END TEST 5: load an image and save it as something else

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();
}

