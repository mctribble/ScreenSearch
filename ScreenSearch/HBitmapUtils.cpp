#include <exception>
#include <windows.h>
#include <gdiplus.h>
#include <gdiplusimagecodec.h>
#include "HBitmapUtils.h"
#include <stdio.h>
#include <iostream>

#pragma comment(lib,"gdiplus.lib")

using namespace std;
using namespace Gdiplus;

//gets the class ID for the given encoder (from https://msdn.microsoft.com/en-us/library/ms533843%28v=vs.85%29.aspx)
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;  //array of encoders

	//get info about how the array will be structured
	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	//allocate array
	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	//populate array
	GetImageEncoders(num, size, pImageCodecInfo);

	//find the desired encoder
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

//performs a screencap of window src, stores it in a new HBITMAP, and saves it at dest.  Returns true on success and false on failure.  
//requires that dest is NULL to prevent overwriting existing bitmaps
bool bitmapFromWindow(HWND src, HBITMAP* dest)
{
	//requires that dest is NULL to prevent overwriting existing bitmaps
	if (*dest != NULL)
		return false;

	//window dimensions
	RECT windowRect;										//screen coordinates of the window
	GetWindowRect(src, &windowRect);
	int windowWidth = windowRect.right - windowRect.left;	//window width
	int windowHeight = windowRect.bottom - windowRect.top;	//window height

	//client area dimensions
	RECT clientRect;
	GetClientRect(src, &clientRect);
	int clientAreaWidth = clientRect.right - clientRect.left;
	int clientAreaHeight = clientRect.bottom - clientRect.top;

	//device contexts
	HDC srcContext	= GetDC(src);						//context of the window to capture
	HDC capContext	= CreateCompatibleDC(srcContext);	//used to hold intermediate result from printWindow()
	HDC destContext = CreateCompatibleDC(srcContext);	//used for final output

	//create the bitmaps
	HBITMAP capBitmap = CreateCompatibleBitmap(srcContext, windowWidth, windowHeight);
	*dest = CreateCompatibleBitmap(srcContext, clientAreaWidth, clientAreaHeight);

	//plug our new bitmaps into their contexts so we can draw to them
	SelectObject(capContext,  capBitmap); 
	SelectObject(destContext, *dest);

	//copy data.  BitBlt() is the standard method, but this returned black for windows owned by other processes, so I use PrintWindow() instead.
	//PrintWindow() is intended for use with printers, but works even if the window owned by another process and/or obscured by other windows. 
	//hidden or minimized windows may not be captured properly. (TODO: work around this?)
	//oddly, the PW_RENDERFULLCONTENT flag used here is not documented on MSDN, but it is referenced elsewhere.  it is required to properly capture some windows.
	PrintWindow(src, capContext, PW_RENDERFULLCONTENT);

	////PrintWindow() creates a black border, so we have to process the image a second time to crop these out
	int cropX = (windowWidth - clientAreaWidth) / 2;
	BitBlt(destContext, 0, 0, clientAreaWidth, clientAreaHeight, capContext, cropX, 0, SRCCOPY);

	//cleanup
	ReleaseDC(src, srcContext);
	DeleteObject(capBitmap);
	DeleteDC(capContext);
	DeleteDC(destContext);

	return true;
}

//loads an image from file, stores it in a new HBITMAP, and saves it at dest.  Returns true on success and false on failure.  
//requires that dest is NULL to prevent overwriting existing bitmaps
bool bitmapFromFile(LPCWSTR src, HBITMAP* dest)
{
	throw new exception("Not Implemented.");
	return false;
}

//saves the given bitmap to a .bmp file
bool bitmapToFile(HBITMAP* src, LPCWSTR dest)
{
	//error check: filename string must be at least 5 characters to be valid
	int destLength = lstrlen(dest);
	if (destLength < 5)
	{
		wcerr << "file name too short" << endl;
		return false;
	}

	//pick an encoder based on file name
	WCHAR	encoderTarget[15] = L"";
	if (lstrcmp(dest + destLength - 4, L".bmp") == 0) //if file ends in .bmp, use encoder image/bmp.
	{
		lstrcpy(encoderTarget, L"image/bmp");
	}
	else if (lstrcmp(dest + destLength - 4, L".png") == 0) //if file ends in .png, use encoder image/png.
	{
		//use encoder image/png
		lstrcpy(encoderTarget, L"image/png");
	}
	else
	{
		//unsupported format
		wcerr << "bitmapToFile() does not currently support that format" << endl;
		return false;
	}

	//attempt to find chosen encoder
	CLSID encoderID;
	GetEncoderClsid(encoderTarget, &encoderID);

	//get Bitmap from HBITMAP
	Bitmap* bmp = Bitmap::FromHBITMAP(*src, NULL);

	//we can finally save the image
	bmp->Save(dest, &encoderID);
	
	return true;
}
