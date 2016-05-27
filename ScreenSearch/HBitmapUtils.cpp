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

//gets the class ID for the given encoder (from)
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

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

	//device contexts
	HDC desktopContext = GetDC(GetDesktopWindow());
	HDC destContext = CreateCompatibleDC(desktopContext);

	//create the bitmap
	*dest = CreateCompatibleBitmap(desktopContext, windowWidth, windowHeight);

	SelectObject(destContext, *dest);

	//copy data.  BitBlt() returned black for windows owned in other processes, so I work around that by using PrintWindow() instead.
	//this works if the window is obscured by other windows, but not if it is hidden or minimized
	//oddly, the PW_RENDERFULLCONTENT flag used here is not referenced on MSDN, but it is referenced on other sites
	PrintWindow(src, destContext, PW_RENDERFULLCONTENT);

	//cleanup
	ReleaseDC(src, desktopContext);
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
