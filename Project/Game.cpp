//Kyle Zielinski
//2/04/2021
//Drawing Meshes: Test Triangle, Rectangle, Pentagon, and a Pinwheel.
//This class allows for the creation of mesh objects and drawing them to a Direct X window via vertex/indice buffers sent to the gpu.

#include "Game.h"
#include "Vertex.h"

//Temp
#include <iostream>

//For basic image loading need to include header 
#include "WICTextureLoader.h"
//For cubemap loading for our skybox
#include "DDSTextureLoader.h"
// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1900,			   // Width of the window's client area
		1000,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
	
	//Create camera
	camera = new Camera(0, 0, -5, (float)width/height);

	//Intialize all variables to remove Warnings
	pixelShader = nullptr;
	vertexShader = nullptr;
	pixelSkyShader = nullptr;
	vertexSkyShader = nullptr;
	specularIntensity = 1;
	entityCounter = 0;
	lightCounter = 0;

	

	light = Light();
	upward = Light();
	grey = Light();
	diagonal = Light();
	defaultLight = Light();
	ambientColor = { 0, 0, 0 };
	
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	//Shutdown ImGUi
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Note: Since we're using smart pointers (ComPtr),
	// we don't need to explicitly clean up those DirectX objects
	// - If we weren't using smart pointers, we'd need
	//   to call Release() on each DirectX object created in Game
	/*for (int i = 0; i < myMeshes.size(); i++)
	{
		delete myMeshes[i];
	}*/

	delete camera;
	delete vertexShader;
	delete pixelShader;	
	delete pixelSkyShader;
	delete vertexSkyShader;
	delete tempEntity;
	
	camera = nullptr;
	vertexShader = nullptr;
	pixelShader = nullptr;
	vertexSkyShader = nullptr;
	pixelSkyShader = nullptr;
	tempEntity = nullptr;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	//Setup ImgGUI
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui::StyleColorsDark();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; //Enable keyboard options

	//Intialize Window
	testWindow = UIWindow();
	entityWindow = EntityWindow();

	//Helper method to load textures
	LoadDefaultTextures();

	//Initialize sampler state
	D3D11_SAMPLER_DESC sampDescription = {};
	sampDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDescription.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDescription.MaxAnisotropy = 8;
	sampDescription.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampDescription, sampler.GetAddressOf());

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	LoadLighting();
	LoadShaders();

	baseData.meshPath = "../../Assets/sphere.obj";
	baseData.albedoPath = L"../../Assets/Sun/Sun.jpg";
	baseData.normalPath = L"../../Assets/Sun/SunN2.png";
	baseData.roughPath = L"../../Assets/Sun/SunR.png";
	baseData.metalPath = L"../../Assets/Sun/SunM.png";

	CreateBasicGeometry();

	wstring baseSky = L"../../Assets/CubeMapTextures/SpaceMap.dds";
	LoadCubeMap(baseSky);
	
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create a new buffer with desired sizes.
	//Set up transform
	/*transform.SetPosition(0.5f, 0, 0);
	transform.SetScale(0.5f, 0.5f, 0.5f);
	transform.SetRotation(0, 0, XM_PI);*/
}

void Game::LoadLighting() 
{
	specularIntensity = 1.0f;

	ambientColor = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);

	//New light intialization
	light.color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	light.intensity = 9.0f;
	light.direction = DirectX::XMFLOAT3(1, 0, 0);
	light.position = DirectX::XMFLOAT3(2, 0, 0);
	light.lightType = 0;

	//Upwards
	upward.color = DirectX::XMFLOAT3(1.0f, 0.5f, 0);
	upward.intensity = 0.3f;
	upward.direction = DirectX::XMFLOAT3(0, 1, 0);
	upward.position = DirectX::XMFLOAT3(0, 0, 0);
	upward.lightType = 0;

	//Diagonal
	diagonal.color = DirectX::XMFLOAT3(1.0f, 0.01f, 0.01f);
	diagonal.intensity = 6.0f;
	diagonal.direction = DirectX::XMFLOAT3(0, 0, 0);
	diagonal.position = DirectX::XMFLOAT3(2, 3, 0);
	diagonal.lightType = 1;

	//GreyLight
	grey.color = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f);
	grey.intensity = 0.6f;
	grey.direction = DirectX::XMFLOAT3(-0.2f, 0.0f, -0.5f);
	grey.position = DirectX::XMFLOAT3(0, 0, 0);
	grey.lightType = 0;

	//DefaultLight
	defaultLight.color = DirectX::XMFLOAT3(0.4f, 0.4f, 0.4f);
	defaultLight.intensity = 0.5f;
	defaultLight.direction = DirectX::XMFLOAT3(-0.2f, -1.0f, 0.5f);
	defaultLight.position = DirectX::XMFLOAT3(0, 0, 0);
	defaultLight.lightType = 0;
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	pixelShader = new SimplePixelShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShader.cso").c_str());	
	pixelSkyShader = new SimplePixelShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShaderCubeMap.cso").c_str());
	vertexShader = new SimpleVertexShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShader.cso").c_str());	
	vertexSkyShader = new SimpleVertexShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShaderCubeMap.cso").c_str());
}

//Depricated
void Game::LoadDefaultTextures()
{
	////Load textures?
	//CreateWICTextureFromFile(
	//	device.Get(), //Allows creation of resource
	//	GetFullPathTo_Wide(L"../../Assets/Textures/cobbleA.png").c_str(),
	//	0, // The texture gets made, but we don't need the pointer
	//	cobbleA.GetAddressOf()
	//);

	//CreateWICTextureFromFile(
	//	device.Get(), //Allows creation of resource
	//	GetFullPathTo_Wide(L"../../Assets/Textures/cobbleN.png").c_str(),
	//	0, // The texture gets made, but we don't need the pointer
	//	cobbleN.GetAddressOf()
	//);

	//CreateWICTextureFromFile(
	//	device.Get(), //Allows creation of resource
	//	GetFullPathTo_Wide(L"../../Assets/Textures/cobbleR.png").c_str(),
	//	0, // The texture gets made, but we don't need the pointer
	//	cobbleR.GetAddressOf()
	//);

	//CreateWICTextureFromFile(
	//	device.Get(), //Allows creation of resource
	//	GetFullPathTo_Wide(L"../../Assets/Textures/cobbleM.png").c_str(),
	//	0, // The texture gets made, but we don't need the pointer
	//	cobbleM.GetAddressOf()
	//);

	////Load textures?
	//CreateWICTextureFromFile(
	//	device.Get(), //Allows creation of resource
	//	GetFullPathTo_Wide(L"../../Assets/Textures/bronzeA.png").c_str(),
	//	0, // The texture gets made, but we don't need the pointer
	//	bronzeA.GetAddressOf()
	//);

	//CreateWICTextureFromFile(
	//	device.Get(), //Allows creation of resource
	//	GetFullPathTo_Wide(L"../../Assets/Textures/bronzeN.png").c_str(),
	//	0, // The texture gets made, but we don't need the pointer
	//	bronzeN.GetAddressOf()
	//);

	//CreateWICTextureFromFile(
	//	device.Get(), //Allows creation of resource
	//	GetFullPathTo_Wide(L"../../Assets/Textures/bronzeR.png").c_str(),
	//	0, // The texture gets made, but we don't need the pointer
	//	bronzeR.GetAddressOf()
	//);

	//CreateWICTextureFromFile(
	//	device.Get(), //Allows creation of resource
	//	GetFullPathTo_Wide(L"../../Assets/Textures/bronzeM.png").c_str(),
	//	0, // The texture gets made, but we don't need the pointer
	//	bronzeM.GetAddressOf()
	//);
}

void Game::LoadTextures(GraphicData newData)
{
	inputAlbedo.Detach();
	inputNormal.Detach();
	inputRough.Detach();
	inputMetal.Detach();

	CreateWICTextureFromFile(
		device.Get(), //Allows creation of resource
		GetFullPathTo_Wide(newData.albedoPath).c_str(),
		0, // The texture gets made, but we don't need the pointer
		inputAlbedo.GetAddressOf()
	);

	CreateWICTextureFromFile(
		device.Get(), //Allows creation of resource
		GetFullPathTo_Wide(newData.normalPath).c_str(),
		0, // The texture gets made, but we don't need the pointer
		inputNormal.GetAddressOf()
	);

	CreateWICTextureFromFile(
		device.Get(), //Allows creation of resource
		GetFullPathTo_Wide(newData.roughPath).c_str(),
		0, // The texture gets made, but we don't need the pointer
		inputRough.GetAddressOf()
	);

	CreateWICTextureFromFile(
		device.Get(), //Allows creation of resource
		GetFullPathTo_Wide(newData.metalPath).c_str(),
		0, // The texture gets made, but we don't need the pointer
		inputMetal.GetAddressOf()
	);
}

void Game::LoadCubeMap(wstring customSky)
{
	CreateDDSTextureFromFile(
		device.Get(),
		GetFullPathTo_Wide(customSky).c_str(),
		0,
		spaceMapSRV.GetAddressOf()
	);

	Mesh* mesh1 = new Mesh(GetFullPathTo("../../Assets/cube.obj").c_str(), device);
	currentRender.AddSkyBox(new SkyMap(mesh1, sampler, device, spaceMapSRV, pixelSkyShader, vertexSkyShader));
}

// --------------------------------------------------------
// Compressed down to a few initial lines.
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	//Setup the Sun
	CreateEntity(baseData);
	
	//Setup the Earth
	baseData.meshPath = "../../Assets/sphere.obj";
	baseData.albedoPath = L"../../Assets/Earth/Earth.png";
	baseData.normalPath = L"../../Assets/Earth/EarthN.png";
    baseData.roughPath = L"../../Assets/Earth/EarthR.png";
	baseData.metalPath = L"../../Assets/Earth/EarthM.png";
	CreateEntity(baseData);
	liveEntities[0]->GetTransform()->AddChild(liveEntities[1]->GetTransform());
	EntityPosition earthPosition = { -3, 0, 0 };
	liveEntities[1]->SetPositionDataStruct(earthPosition);
	liveEntities[1]->GetTransform()->SetPosition(earthPosition.X, earthPosition.Y, earthPosition.Z);
	entityWindow.SetCurrentEntity(
		liveEntities[1]->GetDataStruct(),
		liveEntities[1]->GetGraphicDataStruct(),
		liveEntities[1]->GetPositionDataStruct()
	);

	//Setup the Moon
	baseData.meshPath = "../../Assets/sphere.obj";
	baseData.albedoPath = L"../../Assets/Moon/Moon.jpg";
	baseData.normalPath = L"../../Assets/Moon/MoonN.png";
	baseData.roughPath = L"../../Assets/Moon/MoonR.png";
	baseData.metalPath = L"../../Assets/Moon/MoonM.png";
	CreateEntity(baseData);
	liveEntities[1]->GetTransform()->AddChild(liveEntities[2]->GetTransform());
	EntityPosition moonPosition = { -1.0f, 0.4f, 0 };
	liveEntities[2]->SetPositionDataStruct(moonPosition);
	liveEntities[2]->GetTransform()->SetPosition(moonPosition.X, moonPosition.Y, moonPosition.Z);
	liveEntities[2]->GetTransform()->SetScale(0.2f, 0.2f, 0.2f);
	entityWindow.SetCurrentEntity(
		liveEntities[2]->GetDataStruct(),
		liveEntities[2]->GetGraphicDataStruct(),
		liveEntities[2]->GetPositionDataStruct()
	);

	//Setup the Ship
	baseData.meshPath = "../../Assets/Ship/shipModel.obj";
	baseData.albedoPath = L"../../Assets/Ship/ship.png";
	baseData.normalPath = L"../../Assets/Ship/shipnN.png";
	baseData.roughPath = L"../../Assets/Ship/shipR.png";
	baseData.metalPath = L"../../Assets/Ship/shipM.png";
	CreateEntity(baseData);
	liveEntities[2]->GetTransform()->AddChild(liveEntities[3]->GetTransform());
	EntityPosition shipPosition = { -2.0f, 0.5f, 0 };
	liveEntities[3]->SetPositionDataStruct(shipPosition);
	liveEntities[3]->GetTransform()->SetPosition(shipPosition.X, shipPosition.Y, shipPosition.Z);
	liveEntities[3]->GetTransform()->SetScale(0.1f, 0.1f, 0.1f);
	entityWindow.SetCurrentEntity(
		liveEntities[3]->GetDataStruct(),
		liveEntities[3]->GetGraphicDataStruct(),
		liveEntities[3]->GetPositionDataStruct()
	);

	//Setup the Ship
	baseData.meshPath = "../../Assets/Ship/shipModel.obj";
	baseData.albedoPath = L"../../Assets/Ship/ship2.png";
	baseData.normalPath = L"../../Assets/Ship/shipnN.png";
	baseData.roughPath = L"../../Assets/Ship/shipR.png";
	baseData.metalPath = L"../../Assets/Ship/shipM.png";
	CreateEntity(baseData);
	liveEntities[1]->GetTransform()->AddChild(liveEntities[4]->GetTransform());
	EntityPosition ship2Position = { -1.0f, 0.5f, 0.7f };
	liveEntities[4]->SetPositionDataStruct(ship2Position);
	liveEntities[4]->GetTransform()->SetScale(0.01f, 0.01f, 0.01f);
	entityWindow.SetCurrentEntity(
		liveEntities[4]->GetDataStruct(),
		liveEntities[4]->GetGraphicDataStruct(),
		liveEntities[4]->GetPositionDataStruct()
	);
}

void Game::CreateEntity(GraphicData newData)
{
	std::shared_ptr<Mesh> newMesh = std::make_shared<Mesh>(GetFullPathTo(newData.meshPath).c_str(), device);
	myMeshes.push_back(newMesh.get());

	LoadTextures(newData);
	baseMaterial = std::make_shared<Material>(pixelShader, vertexShader, defaultTint, specularIntensity, inputAlbedo.Get(), inputNormal.Get(), inputRough.Get(), inputMetal.Get(), sampler.Get());
	
	EntityDef newDef;
	newDef.index = currentRender.EntitiesListSize();
	newDef.shadows = 1;
	newDef.transparency = 0;

	EntityPosition currentPos;
	currentPos.X = 0;
	currentPos.Y = 0;
	currentPos.Z = 0;

	tempEntity = new Entity(newMesh.get(), baseMaterial.get(), newDef, newData);
	liveEntities.push_back(tempEntity);
	currentRender.SetEntities(liveEntities);
	entityWindow.SetCurrentEntity(newDef, newData, currentPos);
	entityCounter++;
	std::cout << currentRender.EntitiesListSize() << std::endl;
}

void Game::EstablishNewEntityData(GraphicData newData)
{
	LoadTextures(newData);
	std::shared_ptr<Mesh> tempMesh = std::make_shared<Mesh>(Mesh(GetFullPathTo(newData.meshPath).c_str(), device));
	std::shared_ptr<Material> tempMaterial = std::make_shared<Material>(
		//Shaders
		pixelShader, vertexShader, defaultTint, specularIntensity, 
		//New textures
		inputAlbedo.Get(), inputNormal.Get(), inputRough.Get(), inputMetal.Get(), 
		//Sampler
		sampler.Get());

	tempEntity->AssignMesh(tempMesh);
	tempEntity->AssignMaterial(tempMaterial);
}

void Game::EstablishNewLightData()
{
	
}

//Updates the data values stored within the GUI from live data in the renderer class
void Game::UpdateGUIWindow()
{
	entityWindow.SetCurrentEntity(
		currentRender.ReturnCurrentEntity().GetDataStruct(),
		currentRender.ReturnCurrentEntity().GetGraphicDataStruct(),
		currentRender.ReturnCurrentEntity().GetPositionDataStruct()
	);
}

void Game::SetUpLights()
{
	pixelShader->SetData(
		"light",
		&light,
		sizeof(Light)
	);

	pixelShader->SetData(
		"upward",
		&upward,
		sizeof(Light)
	);

	pixelShader->SetData(
		"diagonal",
		&diagonal,
		sizeof(Light)
	);

	pixelShader->SetData(
		"grey",
		&grey,
		sizeof(Light)
	);

	pixelShader->SetFloat3("ambientColor", ambientColor);

	pixelShader->SetFloat("specularIntensity", baseMaterial->GetSpecularIntensity());

	pixelShader->SetFloat3("camPosition", camera->GetPosition());
}

void Game::SetUpLightsNormal()
{
	/*pixelShader->SetFloat3("ambientColor", ambientColor);
	pixelShader->SetFloat("specularIntensity", baseMaterial->GetSpecularIntensity());

	pixelShader->SetData(
		"defaultLight",
		&defaultLight,
		sizeof(Light)
	);

	pixelShader->SetFloat3("camPosition", camera->GetPosition());*/
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	//Update Camera for new window shape
	camera->UpdateProjectionMatrix((float)width / height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		//TODO: Move this into a custom Input Class (didn't realize this wasn't already in one) 
		//Open/Close UI
		if(GetAsyncKeyState('W') & 0x8000)
		{
			entityWindow.Enabled(false);
		}
		else if(GetAsyncKeyState('S') & 0x8000)
		{
			entityWindow.Enabled(true);
		}

		if (GetAsyncKeyState('A') & 0x8000)
		{
			if ((entityCounter > 1 && entityWindow.ReturnEntityData().index > 0) && entityWindow.GetKeyLock())
			{			
				currentRender.DecrementCurrentEntity();
				UpdateGUIWindow();
			}
		}

		if (GetAsyncKeyState('D') & 0x8000)
		{
			if ((entityWindow.ReturnEntityData().index < currentRender.EntitiesListSize()-1) && entityWindow.GetKeyLock())
			{	
				currentRender.IncrementCurrentEntity();
				UpdateGUIWindow();
			}
		}

		if (!(GetAsyncKeyState('A') & 0x8000) && !(GetAsyncKeyState('D') & 0x8000))
		{
			entityWindow.ReleaseKeyLock();
		}
	}

	if (entityWindow.MakeNewEntity())
	{
		CreateEntity(entityWindow.ReturnData());
		entityWindow.NewEntityFinished();
	}
	if (entityWindow.CanApplyData())
	{
		GraphicData newData = entityWindow.ReturnData();
		EstablishNewEntityData(newData);
		entityWindow.DisableNewData();
	}
	if (entityWindow.CanDeleteEntity() && entityCounter > 1)
	{
		currentRender.RemoveEntity(entityWindow.ReturnEntityData().index);
		myMeshes.erase(myMeshes.begin() + entityWindow.ReturnEntityData().index);
		entityWindow.EntityDeletionComplete();
		UpdateGUIWindow();
		entityCounter--;
	}

	if (entityWindow.CanAddChild() && currentIndex <= liveEntities.size() - 2)
	{
		int temp = currentIndex;
		liveEntities[currentIndex]->GetTransform()->AddChild(liveEntities[++temp]->GetTransform());
	}

	if (entityWindow.CanRemoveChild() && currentIndex >= 0)
	{
		if (liveEntities[currentIndex]->GetTransform()->GetChildCount() > 0)
		{
			liveEntities[currentIndex]->GetTransform()->RemoveChild(
				liveEntities[currentIndex]->GetTransform()->GetChild(0)
			);
		}
	}

	if (entityWindow.CanApplySky())
	{
		LoadCubeMap(entityWindow.ReturnSkyPath());
		entityWindow.SkyApplied();
	}
	
	currentIndex = currentRender.ReturnCurrentEntityIndex();

	//Update position of object

	currentRender.AlterPosition(entityWindow.ReturnTranslation());
	
	liveEntities[0]->GetTransform()->Rotate(0, 0.003f, 0);
	liveEntities[1]->GetTransform()->Rotate(0, 0.001f, 0);
	liveEntities[2]->GetTransform()->Rotate(0.0003, 0.0003f, 0);
	liveEntities[3]->GetTransform()->Rotate(0.001f, 0.002f , 0);
	float temp = (XMScalarCos(1.6f)+1)/8;

	liveEntities[3]->GetTransform()->SetScale(temp, temp, temp);
	//currentRender.Update(deltaTime, totalTime);
	camera->Update(deltaTime, this->hWnd);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	//Once per frame, you're resetting the window
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearDepthStencilView(
		depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	//Setup light
	//At some point move this into a renderer class
	/*________*/
	SetUpLights();
	pixelShader->CopyAllBufferData();
	
	currentRender.Draw(deltaTime, totalTime, context, camera);
	entityWindow.DisplayWindow(hWnd, DXCore::width, DXCore::height);
	
	swapChain->Present(0, 0);
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}