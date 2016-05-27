#pragma once
#include <Windows.h>

//this singleton class is responsible for drawing highlights on the screen to indicate identified objects/regions
class ScreenHighlighter
{
public:
	//allow access to the class instance, creating it if it does not exist
	static ScreenHighlighter* getInstance()
	{
		static ScreenHighlighter instance;
		return &instance;
	}

	//each of these highlights a region of the screen by drawing an overlay
	void highlight(int left, int top, int right, int bottom);	//rect, by coords
	void highlight(HWND hWnd);									//rect, by window handle

	void clearWindowHighlights(); //clears all highlights
private:
	//private constructor/destructor since this is a singleton
	ScreenHighlighter();
	~ScreenHighlighter();

	HDC screenContext; //device context for the entire screen
};

