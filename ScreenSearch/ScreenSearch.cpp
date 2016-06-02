// ScreenSearch.cpp : Application entry point
// ScreenSeach by Christopher Hake (mctribble)
// locates various ojects in images either captured at runtime or imported from files
// can also perform OCR with the assistance of tesseract

// dependencies:
// OpenCV 3.1.0 (http://opencv.org/downloads.html)
// prompiled tesseract binaries (included in project, see tesseract-bin\readme.txt)
// sample image files for test driver (included in project, see ScreenSearch\readme.txt)
// windows 8.1 or windows 10
// should be compatible with any compiler, but I used VS2015 community edition
// project repo: https://github.com/mctribble/ScreenSearch

#include <fcntl.h>
#include "HBitmapUtils.h"
#include <iostream>
#include <io.h>
#include <fstream>
#include "windowEnumerator.h"
#include "ScreenHighlighter.h"
#include <gdiplus.h>
#include "OpenCVUtils.h"

#pragma comment(lib,"gdiplus.lib")

using namespace std;
using namespace Gdiplus;

//predeclarations
void testDriver();
void listTopWindows();
void listWindowChildren(WindowData targetWindow);
void highlightWindowChildren(WindowData targetWindow);
WindowData getWindowDataFromTitle(wchar_t* targetWindowTitle);
void saveWindowScreenshot(WindowData targetWindow, wchar_t* fileName, bool showPrompt);
void contoursFromFile(char* inputFileName, int contourThreshold, double minSize, char* outputFileName);
void contoursFromWindow(WindowData targetWindow, int contourThreshold, double minSize, char* outputFileName);
void OCRWordCount(wchar_t* inputFileName, wstring searchString);

//used as max length for arrays throughout the file
const int ARG_ARRAY_LEN = 50;

int main(int argc, wchar_t* argv[])
{
	//ensure wcout will actually support unicode (http://stackoverflow.com/a/19258509)
	//WARNING: this seems to break cout, though there isn't much reason to use it anyway in a unicode project such as this
	_setmode(_fileno(stdout), _O_U16TEXT);

	//init GDI+
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

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
			<< L"8: perform OCR on an image, count ocurrences of a string, and show full text alongside word count" << endl
			<< L"9: run the test driver" << endl;

		cin.clear();
		cin >> choice;
		cin.get(); //clear newline

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
				listTopWindows();
				break;
			}
			case 2: //List child windows of a specific top - level window
			{
				wchar_t targetWindowTitle[WindowData::MAX_TITLE_LENGTH];
				wcout << L"Which window?" << endl;
				wcin.getline(targetWindowTitle, WindowData::MAX_TITLE_LENGTH);

				listWindowChildren(getWindowDataFromTitle(targetWindowTitle));
				break;
			}
			case 3: //highlight all windows(warning : this tends to flicker significantly)
			{
				WindowData desktop;
				desktop.handle = GetDesktopWindow();
				lstrcpy(desktop.title, L"Desktop");
				highlightWindowChildren(desktop);
				break;
			}
			case 4: //highlight child windows of a specific top - level window
			{
				wchar_t targetWindowTitle[WindowData::MAX_TITLE_LENGTH];
				wcout << L"Which window?" << endl;
				wcin.getline(targetWindowTitle, WindowData::MAX_TITLE_LENGTH);

				highlightWindowChildren(getWindowDataFromTitle(targetWindowTitle));
				break;
			}
			case 5: //save screenshot of a specific top - level window
			{
				wchar_t targetWindowTitle[WindowData::MAX_TITLE_LENGTH];
				wcout << L"Which window?" << endl;
				wcin.getline(targetWindowTitle, WindowData::MAX_TITLE_LENGTH);

				wchar_t fileName[ARG_ARRAY_LEN];
				wcout << L"Name of new file?" << endl;
				wcin.getline(fileName, ARG_ARRAY_LEN);

				saveWindowScreenshot(getWindowDataFromTitle(targetWindowTitle), fileName, true);
				break;
			}
			case 6: //provide an image file and output another showing the contours
			{
				char inputFileName[ARG_ARRAY_LEN];	//name of the sample image
				wcout << L"input file name?" << endl;
				cin >> inputFileName;

				int		contourThreshold;	//sensitivity of the edge detection step.  lower numbers find more edges (max 255).
				wcout << L"What threshold? (lower numbers are more sensitive.  range 0-255)" << endl;
				cin >> contourThreshold;

				double	minSize; //culls contours smaller than this
				wcout << L"Minimum size? (larger values less sensitive)." << endl;
				cin >> minSize;

				char outputFileName[ARG_ARRAY_LEN];	//name of the output image
				wcout << L"output file name?" << endl;
				cin >> outputFileName;

				contoursFromFile(inputFileName, contourThreshold, minSize, outputFileName);
				break;
			}
			case 7: //output an image showing the contours of a given top - level window
			{
				wchar_t targetWindowTitle[WindowData::MAX_TITLE_LENGTH];
				wcout << L"Which window?" << endl;
				wcin.getline(targetWindowTitle, WindowData::MAX_TITLE_LENGTH);

				int		contourThreshold;	//sensitivity of the edge detection step.  lower numbers find more edges (max 255).  This search is very sensitive because the sample page has light gray on a white background.
				wcout << L"What threshold? (lower numbers are more sensitive.  range 0-255)" << endl;
				cin >> contourThreshold;

				double	minSize; //culls contours smaller than this
				wcout << L"Minimum size? (larger values less sensitive)." << endl;
				cin >> minSize;

				char outputFileName[ARG_ARRAY_LEN];	//name of the output image
				wcout << L"output file name?" << endl;
				cin >> outputFileName;

				contoursFromWindow(getWindowDataFromTitle(targetWindowTitle), contourThreshold, minSize, outputFileName);
				break;
			}
			case 8: //OCR
			{
				//input vars
				wchar_t inputFileName[ARG_ARRAY_LEN];	//name of the target image
				wcout << L"Scan which image?" << endl;
				wcin.getline(inputFileName, ARG_ARRAY_LEN);
				
				wchar_t searchString[ARG_ARRAY_LEN];
				wcout << L"What do you want to look for in the image?" << endl;
				wcin.getline(searchString, ARG_ARRAY_LEN);

				OCRWordCount(inputFileName, searchString);
				break;
			}
			case 9: //run the test driver
			{
				testDriver();
				break;
			}
		} //end menu

		//prompt for keypress before continuing
		wcout << endl << "Press enter to continue." << endl;
		cin.get();

	}

	//shut down GDI+
	GdiplusShutdown(gdiplusToken);

	return 0;
}

//tests the system by running all features with preset arguments
void testDriver()
{
	wcout << L"Please create a window called \"Calculator\".  (tested with default win10 calculator, but should work on window by the same name)" << endl
		<< L"Press enter when ready." << endl;
	cin.get();
	WindowData calcWindow = getWindowDataFromTitle(L"Calculator");
	if (calcWindow.handle == NULL)
	{
		wcout << L"Window not found." << endl;
		return;
	}

	//wcout << L"TEST 1: List Windows:" << endl;
	//listTopWindows();
	//wcout << endl << L"Press enter to continue." << endl;
	//cin.get();
	//
	//wcout << L"TEST 2: List children of Calculator window:" << endl;
	//listWindowChildren(calcWindow);
	//wcout << endl << L"Press enter to continue." << endl;
	//cin.get();
	//
	//wcout << L"TEST 3: Highlight Calculator child windows:" << endl;
	//highlightWindowChildren(calcWindow);
	//wcout << endl << L"Press enter to continue." << endl;
	//cin.get();
	//
	//wcout << L"TEST 4: Save screenshot of Calculator window (test4.bmp):" << endl;
	//saveWindowScreenshot(calcWindow, L"test4.bmp", true);
	//wcout << endl << L"Press enter to continue." << endl;
	//cin.get();
	//
	//wcout << L"TEST 5: use contoursFromFile to identify areas on the java test page (javaPageSample.bmp -> test5.png):" << endl;
	//contoursFromFile("javaPageSample.bmp", 10, 500.0, "test5.png");
	//wcout << endl << L"Press enter to continue." << endl;
	//cin.get();
	//
	//wcout << L"TEST 6: perform OCR to count how many aardvarks are in aardvarkSample.png:" << endl;
	//OCRWordCount(L"aardvarkSample.png", L"aardvark");
	//wcout << endl << L"Press enter to continue." << endl;
	//cin.get();

	wcout << L"TEST 7: find object pictured in box.png in box_in_scene.png and highlight it in test7.png" << endl;
	cv::Mat mat7 = findObjectInImage(cv::imread("box.png", cv::IMREAD_GRAYSCALE), cv::imread("aardvarkSample.png", cv::IMREAD_GRAYSCALE), false);
	if (mat7.empty())
		wcerr << L"object not found in scene!" << endl;
	else
		matToFile(mat7, "test7.png", true);
	wcout << endl << L"Press enter to continue." << endl;

	wcout << L"Test driver complete." << endl;
}

//List top-level windows to cout
void listTopWindows()
{
	wcout << L"listing top-level windows" << endl;

	//populate and fetch the list of open windows
	WindowEnumerator::getInstance()->populateListWithTopWindows();
	vector<WindowData>* windowList = WindowEnumerator::getInstance()->getWindowList();

	//print info of each
	for (unsigned int i = 0; i < windowList->size(); i++)
	{
		wcout << windowList->at(i).handle << ": " << windowList->at(i).title << endl;
	}
}

//List children of given top-level window to cout
void listWindowChildren(WindowData targetWindow)
{
	wcout << L"listing children of " << targetWindow.title << "." << endl;

	//repopulate window list with children of the target window.
	WindowEnumerator::getInstance()->populateListWithChildWindows(targetWindow.handle);
	vector<WindowData>* windowList = WindowEnumerator::getInstance()->getWindowList();

	//print info of each
	for (unsigned int i = 0; i < windowList->size(); i++)
	{
		wcout << windowList->at(i).handle << ": " << windowList->at(i).title << endl;
	}
}

//highlight children of given window
void highlightWindowChildren(WindowData targetWindow)
{
	wcout << "Highlighting children of window: " << targetWindow.title << endl;

	//repopulate window list with children of the target window.
	WindowEnumerator::getInstance()->populateListWithChildWindows(targetWindow.handle);
	vector<WindowData>* windowList = WindowEnumerator::getInstance()->getWindowList();

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
}

//return WindowData of window with the given title.  if it is not found, the result has a null handle and a title of WindowNotFound
WindowData getWindowDataFromTitle(wchar_t* targetWindowTitle)
{
	//populate window list
	WindowEnumerator::getInstance()->populateListWithTopWindows();

	//find a window with matching title
	HWND targetWindow = NULL;
	targetWindow = WindowEnumerator::getInstance()->findWindowByTitle(targetWindowTitle);
	WindowData result;

	//if the window was not found, tell user and bail
	if (targetWindow == NULL)
	{
		wcout << "Could not find a window with title " << targetWindowTitle << "." << endl;
		result.handle = NULL;
		lstrcpy(result.title, L"WindowNotFound");
		return result;
	}

	//we found the window.  populate data on it
	result.handle = targetWindow;
	DWORD titleFetchRes = GetWindowText(targetWindow, result.title, WindowData::MAX_TITLE_LENGTH); //get the window title
	//error checking
	if (titleFetchRes == 0)
	{
		//GetWindowText failed to give us a title.  put in a placeholder
		lstrcpy(result.title, L"<untitled>");
	}

	return result;
}

//screencap target window and saves it in target file.  supports .bmp and .png.  if showPrompt is true, the user will be asked if they want to see the file
void saveWindowScreenshot(WindowData targetWindow, wchar_t* fileName, bool showPrompt)
{
	wcout << endl << "capture screenshot of \"" << targetWindow.title << "\" window and save it to a file" << endl;

	//get screencap
	HBITMAP screenshotBitmap = NULL;
	if (bitmapFromWindow(targetWindow.handle, &screenshotBitmap) == false)
	{
		wcout << "failed to perform screen capture!" << endl;
	}
	else
	{
		//save it and offer to show the user
		bitmapToFile(&screenshotBitmap, fileName, showPrompt);
	}

	//cleanup
	DeleteObject(screenshotBitmap);
}

//find contours of an image file and highlights them, putting the result into another image
void contoursFromFile(char* inputFileName, int contourThreshold, double minSize, char* outputFileName)
{
	wcout << L"finding contours of " << inputFileName << endl;

	//delegate to opencvutils
	cv::Mat result = findCountoursFromFile(cv::imread(inputFileName, cv::IMREAD_GRAYSCALE), contourThreshold, minSize);

	//save result to file and prompt user
	matToFile(result, outputFileName, true);
}

//screencaps the window and passes it to contoursFromFile
void contoursFromWindow(WindowData targetWindow, int contourThreshold, double minSize, char* outputFileName)
{
	//get screencap and save it to a temp file
	saveWindowScreenshot(targetWindow, L"temp.bmp", false);

	//get contours from it
	contoursFromFile("temp.bmp", contourThreshold, minSize, outputFileName);

	//delete the temp file
	remove("temp.bmp");
}

//run OCR on image and count occurrences of a string
void OCRWordCount(wchar_t* inputFileName, wstring searchString)
{
	const wchar_t OCR_OUT[ARG_ARRAY_LEN] = L"OCR_Result"; //name of the output file, without extension (tesseract automatically saves to .txt, so including it in the argument causes problems)
	const wchar_t OCR_OUT_WITH_EXTENSION[ARG_ARRAY_LEN] = L"OCR_Result.txt"; //same as above, but with the .txt.  keeping both as a constant saves string operations later

	wcout << "Performing OCR to read all text in " << inputFileName << endl;

	//compile args into one string
	wchar_t tesseractArgs[ARG_ARRAY_LEN];
	swprintf_s(tesseractArgs, ARG_ARRAY_LEN, L"%s %s", inputFileName, OCR_OUT);
	wcout << L"running tesseract " << tesseractArgs << endl; 

	//call external binary to perform OCR and wait for completeion
	
	//setup paramaters
	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = L"open";
	ShExecInfo.lpFile = L"tesseract-bin\\tesseract.exe";
	ShExecInfo.lpParameters = tesseractArgs;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;
	
	//run it
	ShellExecuteEx(&ShExecInfo);

	//wait for it
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

	//open result as a UTF-8 file stream
	FILE* file;	
	_wfopen_s(&file, OCR_OUT_WITH_EXTENSION, L"rt, ccs=UTF-8");
	wifstream fileStream(file);

	if (!fileStream)
	{
		wcerr << "Could not open OCR output file." << endl;
		return;
	}

	//read file into a string
	wstring OCROutput;
	while (fileStream.eof() == false) {
		const int LINE_BUFFER_SIZE = 200; //max number of characters that can be on one line
		wchar_t buffer[LINE_BUFFER_SIZE];
		fileStream.getline(buffer, LINE_BUFFER_SIZE);
		OCROutput += buffer;
		OCROutput += '\n'; //retain line breaks
	}

	//print it out for the user
	wcout << L"OCR Results: " << endl << endl << OCROutput << endl;

	//count ocurrences of the search string to make actual use of the data (based on https://www.rosettacode.org/wiki/Count_occurrences_of_a_substring#C.2B.2B)
	int count = 0;
	for (size_t searchPos = 0;																//current position in the string
		 searchPos != wstring::npos;													//keep going until we hit the end of the string
		 searchPos = OCROutput.find(searchString, searchPos + searchString.length()))	//each iteration, advance the search position to just after the next occurrence of the search string
	{
		count++;
	}

	wcout << searchString << L" appears " << count << L" times in the above text." << endl;
}