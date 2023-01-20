#include "EntityWindow.h"

EntityWindow::EntityWindow()
{
	name = "Entity List";
	currentList = "Entity List";
	readyToApplyData = false;
	newEntity = false;
	deleteEntity = false;
	updateSky = false;
	addChild = false;
	removeChild = false;
	saveScene = false;
	myEntityData = EntityDef(); //Stop vs complaining
}

EntityWindow::~EntityWindow()
{
}

//Button that opens a dialog window. All Files MUST be from the Assets Directory in: CubeMaps/Textures/Root
void EntityWindow::BrowseButton(std::string browseName)
{
	std::string buttonName = "Browse " + browseName;
	if (ImGui::SmallButton(buttonName.c_str()))
	{
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
			COINIT_DISABLE_OLE1DDE);
		IFileDialog* dialog;
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&dialog));

		if (SUCCEEDED(hr))
		{
			hr = dialog->Show(NULL);
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = dialog->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr))
					{
						//Setup paths
						std::wstring ws(pszFilePath);
						std::wstring texturePath = L"../../";
						std::wstring skyPath = L"../../Assets/CubeMapTextures/";
						std::string newMeshPath = "../../Assets/";

						std::wstring findString = L"Assets";
						size_t pathIndex = ws.find(findString);
						
						texturePath += ws.substr(pathIndex);
						
						switch (browseName[0])
						{
						case 'M':
							hr = pItem->GetDisplayName(SIGDN_PARENTRELATIVE, &pszFilePath);
							if (SUCCEEDED(hr))
							{
								std::wstring objWS(pszFilePath);
								newMeshPath += std::string(objWS.begin(), objWS.end());
								meshPath = newMeshPath;
								std::cout << meshPath << std::endl;
							}
							break;
						case 'A':
							albedoPath = texturePath;
							break;
						case 'N':
							normalPath = texturePath;
							break;
						case 'R':
							roughPath = texturePath;
							break;
						case 'm':
							metalPath = texturePath;
							break;
						case 'S':
							skyPath += ws;
							skyMapPath = skyPath;
							//skyMapPath += L".dds";
							break;
						default:
							break;
						}
						
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			dialog->Release();

		}
		CoUninitialize();
	}
}

void EntityWindow::Enabled(bool value)
{
	isEnabled = value;
}

void EntityWindow::DisableNewData()
{
	readyToApplyData = false;
}

bool EntityWindow::CanApplyData()
{
	return readyToApplyData;
}

bool EntityWindow::CanDeleteEntity()
{
	if (myEntityData.index < 0) { return false; }
	return deleteEntity;
}

bool EntityWindow::CanApplySky()
{
	return updateSky;
}

bool EntityWindow::GetKeyLock()
{
	return keyLock;
}

void EntityWindow::ReleaseKeyLock()
{
	keyLock = true;
}

bool EntityWindow::SaveScene()
{
	return saveScene;
}

void EntityWindow::SceneSaved()
{
	saveScene = false;
}

void EntityWindow::NewEntityButton()
{
	if (ImGui::Button("New Object"))
	{
		newEntity = true;
		SetData();
	}
}

void EntityWindow::SaveSceneButton()
{
	if (ImGui::Button("Save Scene"))
	{
		saveScene = true;
	}
}

void EntityWindow::EntityDeletionComplete()
{
	deleteEntity = false;
}

void EntityWindow::SkyApplied()
{
	updateSky = false;
}

bool EntityWindow::CanAddChild()
{
	return addChild;
}

bool EntityWindow::CanRemoveChild()
{
	return removeChild;
}

bool EntityWindow::MakeNewEntity()
{
	return newEntity;
}

void EntityWindow::AddChildButton()
{
	if (ImGui::Button("Add Child"))
	{
		addChild = true;
	}
	else 
	{
		addChild = false;
	}
}

void EntityWindow::RemoveChildButton()
{
	if (ImGui::Button("Remove Child"))
	{
		removeChild = true;
	}
	else 
	{
		removeChild = false;
	}
}

//Allows for transformations on objects
//TODO: Rotations, Transparency, Lighting Effects
void EntityWindow::ObjectInspector()
{
	translationOffset[0] = newPosition.X;
	translationOffset[1] = newPosition.Y;
	translationOffset[2] = newPosition.Z;
	std::string infoString = std::string(currentList + " Index: ");
	infoString += std::to_string(myEntityData.index);
	ImGui::Text(infoString.c_str());
	ImGui::Text("Position");
	ImGui::SameLine();
	ImGui::DragFloat3("XYZ", translationOffset, 0.1f, -500.0f, 500.0f);
	AssignTranslation(translationOffset[0], translationOffset[1], translationOffset[2]);
}

//Prints all default data for the window 
void EntityWindow::BasicData(int width, int height)
{
	std::string titleString = "Name: " + name;
	ImGui::Begin(titleString.c_str());
	ImGui::Text("Width: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string(width).c_str());
	ImGui::SameLine();
	ImGui::Text("Height: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string(height).c_str());
	ImGui::SameLine();
	ImGui::Text("FPS: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string((int)(io.Framerate)).c_str());
	ImGui::SameLine();
	ImGui::Text("Aspect Ratio: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string((float)height / width).c_str());
}

//Displays Browser options for editing objects
void EntityWindow::BrowserInput()
{
		BrowseButton("Mesh");
		ImGui::SameLine();
		ImGui::TextWrapped(std::string(meshPath.begin(), meshPath.end()).c_str());
		ImGui::Spacing();
		BrowseButton("Albedo");
		ImGui::SameLine();
		ImGui::TextWrapped(std::string(albedoPath.begin(), albedoPath.end()).c_str());
		ImGui::Spacing();
		BrowseButton("Normal");
		ImGui::SameLine();
		ImGui::TextWrapped(std::string(normalPath.begin(), normalPath.end()).c_str());
		ImGui::Spacing();
		BrowseButton("Rough");
		ImGui::SameLine();
		ImGui::TextWrapped(std::string(roughPath.begin(), roughPath.end()).c_str());
		ImGui::Spacing();
		BrowseButton("metal");
		ImGui::SameLine();
		ImGui::TextWrapped(std::string(metalPath.begin(), metalPath.end()).c_str());
		ImGui::Spacing();
		ApplyButton();
}

std::wstring EntityWindow::ReturnSkyPath()
{
	return skyMapPath;
}

void EntityWindow::NewEntityFinished()
{
	newEntity = false;
}

//Set current graphics data
void EntityWindow::SetData()
{
	currentGraphicData.meshPath = meshPath;
	currentGraphicData.albedoPath = albedoPath;
	currentGraphicData.normalPath = normalPath;
	currentGraphicData.roughPath = roughPath;
	currentGraphicData.metalPath = metalPath;
}

//Adjust window to match the current entity.
void EntityWindow::SetCurrentEntity(EntityDef entityData, GraphicData graphicData, EntityPosition lastPosition)
{
	myEntityData = entityData;
	currentGraphicData = graphicData;
	
	meshPath = graphicData.meshPath;
	albedoPath = graphicData.albedoPath;
	normalPath = graphicData.normalPath;
	roughPath = graphicData.roughPath;
	metalPath = graphicData.metalPath;
	
	newPosition = lastPosition;

	keyLock = false;
}

void EntityWindow::ApplyButton()
{
	if (ImGui::SmallButton("Apply Changes"))
	{	
		readyToApplyData = true;
		//Pass in new graphic model/textures
		SetData();
	}
}

void EntityWindow::ApplySky()
{
	if (ImGui::Button("Apply Sky"))
	{	
		updateSky = true;
	}
}

void EntityWindow::RemoveEntityButton()
{
	if (ImGui::Button("Remove Object"))
	{
		deleteEntity = true;
	}
}

GraphicData EntityWindow::ReturnData()
{
	return currentGraphicData;
}

EntityDef EntityWindow::ReturnEntityData()
{
	return myEntityData;
}

void EntityWindow::AssignTranslation(float x, float y, float z)
{
	newPosition.X = x;
	newPosition.Y = y;
	newPosition.Z = z;
}

EntityPosition EntityWindow::ReturnTranslation()
{
	return newPosition;
}

void EntityWindow::DisplayWindow(HWND windowHandle, int width, int height)
{
	io = ImGui::GetIO();
	//Start ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	if (isEnabled)
    {
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Once);

		BasicData(width, height);
		ObjectInspector();
		BrowserInput();

		NewEntityButton();
		ImGui::SameLine();
		RemoveEntityButton();

		AddChildButton();
		ImGui::SameLine();
		RemoveChildButton();

		BrowseButton("SkyMap");
		SaveSceneButton();
		ImGui::SameLine();
		ApplySky();
		ImGui::TextWrapped("Hover Mouse and Press W to close Menu");

		ImGui::End();
	}
	else
	{
		ImGui::SetNextWindowSize(ImVec2(150, 80), ImGuiCond_Once);
		ImGui::Begin("UI Disabled");
		ImGui::TextWrapped("Hover Mouse and Press S to expand Menu");
		ImGui::End();
	}
}
