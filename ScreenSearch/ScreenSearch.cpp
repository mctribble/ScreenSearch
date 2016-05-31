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
#include "OpenCVUtils.h"

#pragma comment(lib,"gdiplus.lib")

using namespace std;
using namespace Gdiplus;

//predeclarations
void testDriver();
void test1(HWND targetWindow, vector<WindowData>* windowList);
void test2(HWND targetWindow, vector<WindowData>* windowList);
void test3(HWND targetWindow, vector<WindowData>* windowList);
void test4(HWND targetWindow, vector<WindowData>* windowList);
void test5(HWND targetWindow, vector<WindowData>* windowList);
void test6(HWND targetWindow, vector<WindowData>* windowList);
void test7(HWND targetWindow, vector<WindowData>* windowList);
void test8(HWND targetWindow, vector<WindowData>* windowList);

int main(int argc, wchar_t* argv[])
{
	//ensure wcout will actually support unicode (http://stackoverflow.com/a/19258509)
	//WARNING: this seems to break cout, though there isnt much reason to use it anyway in a unicode app
	_setmode(_fileno(stdout), _O_U16TEXT);

	//init GDI+
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	//used by various menu items
	HWND targetWindow = NULL;
	vector<WindowData>* windowList = WindowEnumerator::getInstance()->getWindowList();

	//menu
	int choice = -1;
	while (choice != 0)
	{
		wcout << L"Welcome to Screensearch!" << endl << endl
			<< L"0: quit" << endl
			<< L"1: List top-level windows" << endl
			<< L"2: List child windows of a specific top-level window" << endl
			<< L"3: highlight all windows (warning: this flickers and lags significantly!)" << endl  //not really practical for anything.  I just had this happen as a bug and thought it would be funny to keep it as an option
			<< L"4: highlight child windows of a specific top-level window" << endl
			<< L"5: save screenshot of a specific top-level window" << endl
			<< L"6: provide an image file and output another showing the contours" << endl
			<< L"7: output an image showing the contours of a given top-level window" << endl
			<< L"8: run the test driver" << endl;

		cin.clear();
		cin >> choice;


		//perform chosen item
		switch (choice)
		{
			case 0: //quit
			{
				wcout << L"goodbye." << endl;
				break;
			}
			case 1: //List top - level windows
			{
				//populate and fetch the list of open windows
				WindowEnumerator::getInstance()->topLevelWindowList();

				//print info of each
				for (unsigned int i = 0; i < windowList->size(); i++)
				{
					wcout << windowList->at(i).handle << ": " << windowList->at(i).title << endl;
				}
				break;
			}
			case 2: //List child windows of a specific top - level window
			{
				wchar_t targetWindowTitle[WindowData::MAX_TITLE_LENGTH];
				wcout << L"Which window?" << endl;
				wcin >> targetWindowTitle;

				//populate the list of open windows 
				WindowEnumerator::getInstance()->topLevelWindowList();

				//find a window with matching title
				targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(targetWindowTitle);

				//if a window was not found, prompt user and wait until it exists.
				if (targetWindow == NULL)
				{
					wcout << "Could not find a window with title " << targetWindowTitle << "." << endl;
					break;
				}

				//we found the target window.  Enumerate its children and list them.
				wcout << "Children of " << targetWindowTitle << ":" << endl;

				//populate the list of children 
				WindowEnumerator::getInstance()->childWindowList(targetWindow);

				//print info of each
				if (windowList->size() == 0)
					wcout << "<none>" << endl;
				for (unsigned int i = 0; i < windowList->size(); i++)
				{
					wcout << windowList->at(i).handle << ": " << windowList->at(i).title << endl;
				}

				break;
			}
			case 3: //highlight all windows(warning : this tends to flicker significantly)
			{
				//repopulate window list with children of the desktop window.  
				WindowEnumerator::getInstance()->childWindowList(GetDesktopWindow());

				//repeatedly draw highlights until escape is pressed
				wcout << L"Highlighting windows.  Hold escape to clear highlights and continue." << endl;
				while ((GetKeyState(VK_ESCAPE) & 0x80) == 0) //0x80 is the "high bit" of the key state, and indicates whether the key is pressed
				{
					//draw a highlight for each child window
					for (unsigned int i = 0; i < windowList->size(); i++)
					{
						ScreenHighlighter::getInstance()->highlight(windowList->at(i).handle); //draw highlight over the window, if it is visible
					}

					Sleep(10); //dont hog system resources to do nothing but draw rectangles over and over
				}

				//clear highlights
				ScreenHighlighter::getInstance()->clearWindowHighlights();
				break;
			}
			case 4: //highlight child windows of a specific top - level window
			{
				wchar_t targetWindowTitle[WindowData::MAX_TITLE_LENGTH];
				wcout << L"Which window?" << endl;
				wcin >> targetWindowTitle;

				//populate the list of open windows (we dont have to retrieve it this time because we already have the pointer)
				WindowEnumerator::getInstance()->topLevelWindowList();

				//find a window with matching title
				targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(targetWindowTitle);

				//if a window was not found, tell user and bail
				if (targetWindow == NULL)
				{
					wcout << "Could not find a window with title " << targetWindowTitle << "." << endl;
					break;
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

				break;
			}
			case 5: //save screenshot of a specific top - level window
			{
				wchar_t targetWindowTitle[WindowData::MAX_TITLE_LENGTH];
				wcout << L"Which window?" << endl;
				wcin >> targetWindowTitle;

				//populate the list of open windows (we dont have to retrieve it this time because we already have the pointer)
				WindowEnumerator::getInstance()->topLevelWindowList();

				//find a window with matching title
				targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(targetWindowTitle);

				//if a window was not found, prompt user and wait until it exists.
				if (targetWindow == NULL)
				{
					wcout << "Could not find a window with title " << targetWindowTitle << "." << endl;
					break;
				}

				//get screencap
				HBITMAP screenshotBitmap = NULL;
				if (bitmapFromWindow(targetWindow, &screenshotBitmap) == false)
				{
					wcout << "failed to perform screen capture!" << endl;
					break;
				}
				else
				{
					//save it and offer to show the user
					bitmapToFile(&screenshotBitmap, L"test4.bmp", true);
				}

				//cleanup
				DeleteObject(screenshotBitmap);

				break;
			}
			case 6: //provide an image file and output another showing the contours
			{
				char inputFileName[20];	//name of the sample image
				wcout << L"input file name?" << endl;
				cin >> inputFileName;

				int		contourThreshold = 10;	//sensitivity of the edge detection step.  lower numbers find more edges (max 255).  This search is very sensitive because the sample page has light gray on a white background.
				wcout << L"What threshold? (lower numbers are more sensitive.  range 0-255)" << endl;
				cin >> contourThreshold;

				double	minSize = 500.0; //culls contours smaller than this
				wcout << L"Minimum size? (larger values less sensitive)." << endl;
				cin >> minSize;

				char outputFileName[20];	//name of the output image
				wcout << L"output file name?" << endl;
				cin >> outputFileName;

				//delegate to opencvutils
				cv::Mat result = findCountoursFromFile(inputFileName, contourThreshold, minSize);

				//save result to file
				cv::imwrite(outputFileName, result);

				//offer to show result to the user
				//ask user until they respond in a valid way or the input stream closes
				char response = '0';
				do
				{
					wcout << "Saved file " << outputFileName << " to disk.  Would you like to open it? [y/n]" << endl;
					cin >> response;
				} while (!cin.fail() && response != 'y' && response != 'Y' && response != 'n' && response != 'N');

				//if yes, open file with the default program
				if (response == 'y' || response == 'Y')
					ShellExecuteA(0, 0, outputFileName, 0, 0, SW_SHOW);

				break;
			}
			case 7: //output an image showing the contours of a given top - level window
			{
				wchar_t targetWindowTitle[WindowData::MAX_TITLE_LENGTH];
				wcout << L"Which window?" << endl;
				wcin >> targetWindowTitle;

				int		contourThreshold = 10;	//sensitivity of the edge detection step.  lower numbers find more edges (max 255).  This search is very sensitive because the sample page has light gray on a white background.
				wcout << L"What threshold? (lower numbers are more sensitive.  range 0-255)" << endl;
				cin >> contourThreshold;

				double	minSize = 500.0; //culls contours smaller than this
				wcout << L"Minimum size? (larger values less sensitive)." << endl;
				cin >> minSize;

				char outputFileName[20];	//name of the output image
				wcout << L"output file name?" << endl;
				cin >> outputFileName;

				//populate the list of open windows (we dont have to retrieve it this time because we already have the pointer)
				WindowEnumerator::getInstance()->topLevelWindowList();

				//find a window with matching title
				targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(targetWindowTitle);

				//if a window was not found, prompt user and wait until it exists.
				if (targetWindow == NULL)
				{
					wcout << "Could not find a window with title " << targetWindowTitle << "." << endl;
					break;
				}

				//get screencap
				HBITMAP screenshotBitmap = NULL;
				if (bitmapFromWindow(targetWindow, &screenshotBitmap) == false)
				{
					wcout << "failed to perform screen capture!" << endl;
					break;
				}
				else
				{
					//silently save it to a temporary file
					bitmapToFile(&screenshotBitmap, L"temp.bmp", false);

					//use the temp. file as an input for contour detection
					cv::Mat result = findCountoursFromFile("temp.bmp", contourThreshold, minSize);

					//save result to file
					cv::imwrite(outputFileName, result);

					//delete our temp image
					remove("temp.bmp");

					//offer to show result to the user
					//ask user until they respond in a valid way or the input stream closes
					char response = '0';
					do
					{
						wcout << "Saved file " << outputFileName << " to disk.  Would you like to open it? [y/n]" << endl;
						cin >> response;
					} while (!cin.fail() && response != 'y' && response != 'Y' && response != 'n' && response != 'N');

					//if yes, open file with the default program
					if (response == 'y' || response == 'Y')
						ShellExecuteA(0, 0, outputFileName, 0, 0, SW_SHOW);
				}

				//cleanup
				DeleteObject(screenshotBitmap);

				break;
			}
			case 8: //run the test driver
			{
				testDriver();
				break;
			}
		}
	}

	//shut down GDI+
	GdiplusShutdown(gdiplusToken);

	return 0;
}

//tests the system
void testDriver()
{
	//used by multiple tests in this driver.  
	//each driver initializes them on their own to do what they need, so this is just to save allocations
	HWND targetWindow = NULL;
	vector<WindowData>* windowList = WindowEnumerator::getInstance()->getWindowList();

	//tests.  each one prompts for input after.
	//each test is self-contained so they can be commented out without affecting the others

	//test1(targetWindow, windowList); //TEST 1: List top-level windows
	//test2(targetWindow, windowList); //TEST 2: List children of Calculator Window
	//test3(targetWindow, windowList); //TEST 3: highlight children of Calculator window
	//test4(targetWindow, windowList); //TEST 4: save screencap of Calculator window
	//test5(targetWindow, windowList); //TEST 5: load the screencap from test 4 and save it in another format
	//test6(targetWindow, windowList); //TEST 6: find contours of a sample image
	//test7(targetWindow, windowList); //TEST 7: find contours of window screencap (much less sensitive than test 6, but does not cull as aggressively)
	//test8(targetWindow, windowList); //TEST 8: find sample object in sample image  // <<this was postponed because it requires building the nonfree opencv-contrib.  Redirected efforts to tesseract OCR for now>>
}

//TEST 1: List top-level windows
void test1(HWND targetWindow, vector<WindowData>* windowList)
{
	wcout << "TEST 1: fetch top level windows" << endl;

	//populate and fetch the list of open windows
	WindowEnumerator::getInstance()->topLevelWindowList();//TEST 6: find contours of a sample image

	//print info of each
	for (unsigned int i = 0; i < windowList->size(); i++)
	{
		wcout << windowList->at(i).handle << ": " << windowList->at(i).title << endl;
	}

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();
}

//TEST 2: List children of given window
void test2(HWND targetWindow, vector<WindowData>* windowList)
{
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

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();
}

//TEST 3: highlight children of target window
void test3(HWND targetWindow, vector<WindowData>* windowList)
{
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

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();
}

//TEST 4: save screencap of a window
void test4(HWND targetWindow, vector<WindowData>* windowList)
{
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
		//save it and offer to show the user
		bitmapToFile(&test4Bitmap, L"test4.bmp", true);
	}

	//cleanup
	DeleteObject(test4Bitmap);

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();
}

//TEST 5: load an image and save it as something else
void test5(HWND targetWindow, vector<WindowData>* windowList)
{
	//filenames for the test
	wchar_t TEST5_IN[20] = L"test4.bmp";
	wchar_t TEST5_OUT[20] = L"test5.png";

	wcout << endl << "TEST 5: load " << TEST5_IN << " and save it to " << TEST5_OUT << endl;
	HBITMAP test5Bitmap = NULL;
	//load
	if (bitmapFromFile(TEST5_IN, &test5Bitmap) == false)
	{
		wcout << "Load failed!" << endl;
	}
	else
	{
		//save and offer to show it to the user after
		if (bitmapToFile(&test5Bitmap, TEST5_OUT, true) == false)
		{
			wcout << "Save failed!" << endl;
		}
	}

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();
}

//TEST 6: find contours of a sample image
void test6(HWND targetWindow, vector<WindowData>* windowList)
{
	wcout << L"TEST 6: find contours of a sample image" << endl;

	char			TEST6_IN[20] = "javaPageSample.bmp";	//name of the sample image
	const int		TEST6_THRESHOLD = 10;					//sensitivity of the edge detection step.  lower numbers find more edges (max 255).  This search is very sensitive because the sample page has light gray on a white background.
	const double	TEST6_MIN_SIZE = 500.0;					//culls contours smaller than this
	char			TEST6_OUT[20] = "test6.bmp";			//name of the output file image (char)

	//delegate to opencvutils
	cv::Mat result = findCountoursFromFile(TEST6_IN, TEST6_THRESHOLD, TEST6_MIN_SIZE);
	
	//save result to file
	cv::imwrite(TEST6_OUT, result);

	//offer to show result to the user
	//ask user until they respond in a valid way or the input stream closes
	char response = '0';
	do
	{
		wcout << "Saved file " << TEST6_OUT << " to disk.  Would you like to open it? [y/n]" << endl;
		cin >> response;
	} while (!cin.fail() && response != 'y' && response != 'Y' && response != 'n' && response != 'N');

	//if yes, open file with the default program
	if (response == 'y' || response == 'Y')
		ShellExecuteA(0, 0, TEST6_OUT, 0, 0, SW_SHOW);

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();
}

//TEST 7: find contours of window screencap (much less sensitive than test 6, but does not cull as aggressively)
void test7(HWND targetWindow, vector<WindowData>* windowList)
{
	wchar_t TEST7_TARGET_WINDOW_TITLE[WindowData::MAX_TITLE_LENGTH];
	lstrcpy(TEST7_TARGET_WINDOW_TITLE, L"Calculator");		//tested on Win10 Calculator app, but should work on anything with this title
	const int		TEST7_THRESHOLD = 50;					//sensitivity of the edge detection step.  lower numbers find more edges (max 255).  This search is very sensitive because the sample page has light gray on a white background.
	const double	TEST7_MIN_SIZE = 50.0;					//culls contours smaller than this
	char			TEST7_OUT[20] = "test7.bmp";			//name of the output file image (char)

	wcout << endl << "TEST 7: capture screenshot of \"" << TEST7_TARGET_WINDOW_TITLE << "\" window, search it for contours, and save the result to a file" << endl;

	//populate the list of open windows (we dont have to retrieve it this time because we already have the pointer)
	WindowEnumerator::getInstance()->topLevelWindowList();

	//find a window with matching title
	targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(TEST7_TARGET_WINDOW_TITLE);

	//if a window was not found, prompt user and wait until it exists.
	if (targetWindow == NULL)
	{
		wcout << "Could not find a window with title " << TEST7_TARGET_WINDOW_TITLE << ".  Please create one to continue." << endl;
		while (targetWindow == NULL)
		{
			Sleep(1000);
			WindowEnumerator::getInstance()->topLevelWindowList();
			targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(TEST7_TARGET_WINDOW_TITLE);
		}
	}

	//get screencap
	HBITMAP test7Bitmap = NULL;
	if (bitmapFromWindow(targetWindow, &test7Bitmap) == false)
	{
		wcout << "failed to perform screen capture!" << endl;
	}
	else
	{
		//silently save it to a temporary file
		bitmapToFile(&test7Bitmap, L"temp.bmp", false);

		//use the temp. file as an input for contour detection
		cv::Mat result = findCountoursFromFile("temp.bmp", TEST7_THRESHOLD, TEST7_MIN_SIZE);

		//save result to file
		cv::imwrite(TEST7_OUT, result);

		//delete our temp image
		remove("temp.bmp");

		//offer to show result to the user
		//ask user until they respond in a valid way or the input stream closes
		char response = '0';
		do
		{
			wcout << "Saved file " << TEST7_OUT << " to disk.  Would you like to open it? [y/n]" << endl;
			cin >> response;
		} while (!cin.fail() && response != 'y' && response != 'Y' && response != 'n' && response != 'N');

		//if yes, open file with the default program
		if (response == 'y' || response == 'Y')
			ShellExecuteA(0, 0, TEST7_OUT, 0, 0, SW_SHOW);
	}

	//cleanup
	DeleteObject(test7Bitmap);

	//prompt for keypress before continuing
	wcout << endl << "Press enter to continue." << endl;
	cin.get();
}

//// <<this was postponed because it requires building the nonfree opencv-contrib.  Redirected efforts to tesseract OCR for now>>
////TEST 8: find sample object in sample image.  show how the match is done as well.
//void test8(HWND targetWindow, vector<WindowData>* windowList)
//{
//	char	TEST8_IN_SAMPLE[25] = "homographySample.bmp";		//name of the sample object image
//	char	TEST8_IN_SCENE[25] = "homographyScene.bmp";		//name of the sample image to search
//	char	TEST8_OUT[20] = "test8.bmp";		//name of the output file image (char)
//
//	wcout << L"finding the object depected in " << TEST8_IN_SAMPLE << L" in the image " << TEST8_IN_SCENE << L" and showing the match points.";
//
//	//the test is performed in grayscale
//	cv::Mat sampleMat = cv::imread(TEST8_IN_SAMPLE, cv::IMREAD_GRAYSCALE);
//	cv::Mat sceneMat = cv::imread(TEST8_IN_SCENE, cv::IMREAD_GRAYSCALE);
//
//	//validate materials
//	if (!sampleMat.data || !sceneMat.data)
//	{
//		wcerr << L"Failed to load images!";
//		return;
//	}
//
//	//perform search
//	cv::Mat result = findObjectInImage(sampleMat, sceneMat, true);
//
//	//save result to file
//	cv::imwrite(TEST8_OUT, result);
//
//	//offer to show result to the user
//	//ask user until they respond in a valid way or the input stream closes
//	char response = '0';
//	do
//	{
//		wcout << L"Saved file " << TEST8_OUT << L" to disk.  Would you like to open it? [y/n]" << endl;
//		cin >> response;
//	} while (!cin.fail() && response != 'y' && response != 'Y' && response != 'n' && response != 'N');
//
//	//if yes, open file with the default program
//	if (response == 'y' || response == 'Y')
//		ShellExecuteA(0, 0, TEST8_OUT, 0, 0, SW_SHOW);
//
//	//prompt for keypress before continuing
//	wcout << endl << "Press enter to continue." << endl;
//	cin.get();
//}