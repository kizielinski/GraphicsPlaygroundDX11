#pragma once
#include <string>
#include "imgGUI/imgui_impl_dx11.h"
#include "imgGUI/imgui_impl_win32.h"
#include <ShObjIdl_core.h>
#include <Windows.h>
#include <iostream>
#include <codecvt>

//Normal includes
#include <string>
#include <vector>
#include <map>

using namespace std;

class UIWindow
{
	enum {
		Toggle_Window_Off = 119,
		Toggle_Window_On = 115,
		Page_Left = 97,
		Page_Right = 100,
		Center_View = 95,
	};
public:

	struct UIWindowCreation {
		vector<string> buttons;
		string Title;
		map<string, int> name_information;
		ImVec2 windowSize = ImVec2(200, 500);
	};

	UIWindow();
	UIWindow(HWND _handle);
	UIWindow(HWND _handle, UIWindowCreation* inputDat);
	~UIWindow();

	void StartFrame();
	virtual void displayWindow(); //All ImGui draw calls here.


protected:
	std::string name; //User name of object
	std::string currentList; //Entity or Lights
	int index; //Number in list;
	bool isEnabled;
	bool readyToApplyData;
	vector<string> buttonsToDisplay;
	UIWindowCreation* windowToDisplay;
	HWND handle;
	void StringIntSameLine(map<string, int> pairs);

private:
	void Init();
};

