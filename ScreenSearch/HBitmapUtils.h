#pragma once

#include <Windows.h>

//this is a collection of bitmap utility functions used elsewhere in ScreenSearch

//performs a screencap of window src, stores it in a new HBITMAP, and saves it at dest.  Returns true on success and false on failure.  
//requires that dest is NULL to prevent overwriting existing bitmaps
bool bitmapFromWindow(HWND src, HBITMAP* dest); 

//loads an image from file, stores it in a new HBITMAP, and saves it at dest.  Returns true on success and false on failure.  
//requires that dest is NULL to prevent overwriting existing bitmaps
bool bitmapFromFile(LPCWSTR src, HBITMAP* dest);

//saves the given bitmap to a .bmp file of the given name.  if showPrompt is true, ask the user if they want
//to see the file after it is saved.
bool bitmapToFile(HBITMAP* src, LPCWSTR dest, bool showPrompt = false);