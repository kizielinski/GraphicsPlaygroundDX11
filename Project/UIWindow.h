#pragma once
#include <string>
#include "imgGUI/imgui_impl_dx11.h"
#include "imgGUI/imgui_impl_win32.h"
#include <ShObjIdl_core.h>
#include <Windows.h>
#include <iostream>
#include <codecvt>

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
	UIWindow();
	~UIWindow();
	virtual void displayWindow(HWND windowHandle); //All ImGui draw calls here.
protected:
	std::string name; //User name of object
	std::string currentList; //Entity or Lights
	int index; //Number in list;
	bool isEnabled;
	bool readyToApplyData;
private:
};

