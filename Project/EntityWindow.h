#pragma once
#include "UIWindow.h"
#include "DataStruct.h"

enum WindowState {
	WaitingForInput,
	ApplyingData,
	DeleteCurrentEntity,
	ApplyingSky,
	SavingScene,
	CreateNewEntity,
	AddChild,
	RemoveChild,
};

class EntityWindow : public UIWindow
{

public:

	EntityWindow(HWND _handle = 0, UIWindowCreation* windowParam = nullptr);
	~EntityWindow();
	void DisplayWindow(int windowWidth, int windowHeight);
	void BrowseButton(std::string browseName);
	void Enabled(bool value);
	void SetData();
	void SetCurrentEntity(EntityDef entityData, GraphicData graphicData, EntityPosition lastPosition);
	bool GetKeyLock();
	void ApplySky();
	void ReleaseKeyLock();

	WindowState GetState();

	//Organization of various ImGui elements
	void ObjectInspector();
	void BasicData(int width, int height);
	void BrowserInput();
	std::wstring ReturnSkyPath();
	GraphicData ReturnData();
	EntityDef ReturnEntityData();
	EntityPosition ReturnTranslation();
	void AssignTranslation(float x, float y, float z);
	

private:

	WindowState state;
	std::string meshPath;
	std::wstring albedoPath;
	std::wstring normalPath;
	std::wstring roughPath;
	std::wstring metalPath;
	std::wstring skyMapPath;
	EntityDef myEntityData;
	GraphicData currentGraphicData;
	EntityPosition newPosition;
	bool keyLock;
	float translationOffset[3];
	ImGuiIO io;

	void AddChildButton();
	void RemoveChildButton();
	void ApplyButton();
	void RemoveEntityButton();
	void NewEntityButton();
	void SaveSceneButton();
};

