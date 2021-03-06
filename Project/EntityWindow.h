#pragma once
#include "UIWindow.h"
#include "DataStruct.h"

class EntityWindow : public UIWindow
{
public:
	EntityWindow();
	~EntityWindow();
	void DisplayWindow(HWND windowHandle, int width, int height);
	void BrowseButton(std::string browseName);
	void Enabled(bool value);
	void DisableNewData();
	void SetData();
	void SetCurrentEntity(EntityDef entityData, GraphicData graphicData, EntityPosition lastPosition);
	bool CanApplyData();
	bool CanDeleteEntity();
	bool CanApplySky();
	bool GetKeyLock();
	bool MakeNewEntity();
	void ApplySky();
	void EntityDeletionComplete();
	void SkyApplied();
	bool CanAddChild();
	bool CanRemoveChild();
	void NewEntityFinished();
	void ReleaseKeyLock();

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
	std::string meshPath;
	std::wstring albedoPath;
	std::wstring normalPath;
	std::wstring roughPath;
	std::wstring metalPath;
	std::wstring skyMapPath;
	EntityDef myEntityData;
	GraphicData currentGraphicData;
	EntityPosition newPosition;
	bool newEntity;
	bool deleteEntity;
	bool updateSky;
	bool keyLock;
	bool addChild;
	bool removeChild;
	float translationOffset[3];
	ImGuiIO io;

	void AddChildButton();
	void RemoveChildButton();
	void ApplyButton();
	void RemoveEntityButton();
	void NewEntityButton();
};

