// ScreenSearch.cpp : Application entry point
// ScreenSeach by Christopher Hake (mctribble)
// locates various ojects in images either captured at runtime or imported from files

#include <iostream>
#include "windowEnumerator.h"

using namespace std;

int main(int argc, wchar_t* argv[])
{
	//populate and fetch the list of open windows
	WindowEnumerator::getInstance()->updateWindowList();
	vector<WindowData>* windowList = WindowEnumerator::getInstance()->getWindowList();

	//print info of each
	for (unsigned int i = 0; i < windowList->size(); i++)
	{
		wcout << windowList->at(i).handle << ": " << windowList->at(i).title << endl;
	}

	//prompt for keypress before exiting
	cout << endl << "Press any key to exit." << endl;
	cin.get();

	return 0;
}



