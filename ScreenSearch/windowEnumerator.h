#pragma once
#include <Windows.h>
#include <vector>

//contains all data windowEnumerator returns about an open window
struct WindowData 
{
	public:
		static const int MAX_TITLE_LENGTH = 50;

		HWND	handle;
		wchar_t	title[MAX_TITLE_LENGTH];
};

//this class is a singleton responsible for polling open windows and providing information about each
class WindowEnumerator
{
	public:
		//allow access to the class instance, creating it if it does not exist
		static WindowEnumerator* getInstance()
		{
			static WindowEnumerator instance;
			return &instance;
		}

		//getter/setters
		std::vector<WindowData>* getWindowList() { return &windowList; }

		//purges and repopulates the window list with all top-level windows.  Should be called prior to searching said list to ensure contents are accurate.
		void topLevelWindowList();

		//like above, but returns all windows that are children of the given window
		void childWindowList(HWND parentWindow);
	private:
		//private constructor/destructor because this is a singleton
		WindowEnumerator() {};
		~WindowEnumerator() {};

		//contains the list of windows
		std::vector<WindowData> windowList;
};

