//Kyle Zielinski
//2/04/2021
//Drawing Meshes: Test Triangle, Rectangle, Pentagon, and a Pinwheel.
//This class allows for the creation of mesh objects and drawing them to a Direct X window via vertex/indice buffers sent to the gpu.

#include "Game.h"
#include "Vertex.h"

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
	camera = new Camera(0, 0, -5, (float)width / height);

	//Intialize all variables to remove Warnings
	pixelShader = nullptr;
	vertexShader = nullptr;
	pixelSkyShader = nullptr;
	vertexSkyShader = nullptr;
	fullscreenVS = nullptr;
	irradiancePS = nullptr;
	specularConvoledPS = nullptr;
	lookUpTexturePS = nullptr;
	finalCombinePS = nullptr;
	finalOutputPS = nullptr;
	refractionPS = nullptr;
	newSRV = nullptr;
	locationSRV = nullptr;
	sky = nullptr;
	tempEntity = nullptr;
	currentRender = nullptr;
	specularIntensity = 1;
	entityCounter = 0;
	lightCounter = 0;
	currentIndex = -1;

	spaceSceneEnabled = false;

	light = Light();
	upward = Light();
	grey = Light();
	diagonal = Light();
	defaultLight = Light();
	defaultTint = { 0, 0, 0, 0 };
	ambientColor = { 0, 0, 0 };
	valueToRotate = 0;

	//dm = new DataManager(entityWindow, liveEntities, lights, emitters);
	dm = nullptr;
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

	for (int i = 0; i < liveEntities.size(); i++)
	{
		delete liveEntities[i];
	}

	for (int i = 0; i < emitters.size(); i++)
	{
		delete emitters[i];
	}

	liveEntities.clear();
	myMeshes.clear();
	staticColors.clear();
	lights.clear();
	emitters.clear();

	inputAlbedo.Detach();
	inputNormal.Detach();
	inputRough.Detach();
	inputMetal.Detach();
	
	delete camera;
	delete currentRender;
	delete sky;
	delete dm;
	
	camera = nullptr;

	//Base
	delete vertexShader;
	delete pixelShader;
	delete vertexSkyShader;
	delete pixelSkyShader;
	
	//IBL
	delete fullscreenVS;
	delete irradiancePS;
	delete specularConvoledPS;
	delete lookUpTexturePS;

	//Other
	delete particlePS;
	delete particleVS;
	delete finalCombinePS;
	delete finalOutputPS ;
	delete refractionPS;


	//Base
	vertexShader = nullptr;
	pixelShader = nullptr;
	vertexSkyShader = nullptr;
	pixelSkyShader = nullptr;

	//IBL
	fullscreenVS = nullptr;
	irradiancePS = nullptr;
	specularConvoledPS = nullptr;
	lookUpTexturePS = nullptr;

	//Other
	particlePS = nullptr;
	particleVS = nullptr;
	finalCombinePS = nullptr;
	finalOutputPS = nullptr;
	refractionPS = nullptr;

	currentRender = nullptr;
	tempEntity = nullptr;
	sky = nullptr;
	dm = nullptr;
	currentRender = nullptr;

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

	entityWindowDef.windowSize = ImVec2(500, 400);

	//Intialize Window
	entityWindow = EntityWindow(hWnd, &entityWindowDef);

	//Initialize sampler state
	D3D11_SAMPLER_DESC sampDescription = {};
	sampDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescription.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDescription.MaxAnisotropy = 8;
	sampDescription.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampDescription, sampler.GetAddressOf());

	//Setup clampSampler at the same time, cheap and easy
	sampDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	device->CreateSamplerState(&sampDescription, clampSampler.GetAddressOf());

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	LoadLighting();
	LoadShaders();
	LoadCubeMap(baseSky);

	//My implementation takes into account each newly created object, so the renderer must be initialized fist and then updated with each new entity.
	currentRender = new Renderer(
		device, context, swapChain, backBufferRTV, depthStencilView, sampler, width, height, 
		pixelShader, finalCombinePS, finalOutputPS, refractionPS, vertexShader, fullscreenVS,
		sky, liveEntities, lights, emitters
	);
	CreateBasicGeometry();

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Game::LoadLighting() 
{
	specularIntensity = 1.0f;

	ambientColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

	lights.emplace("defaultLight", LightObject(
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), //Color
		DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), //Direction
		DirectX::XMFLOAT3(2.0f, 0.0f, 0.0f), //Position
		8.0f,
		1
	));
	lightCounter++;

	////Upwards
	//lights.emplace("upwardsLight", LightObject(
	//	DirectX::XMFLOAT3(0.0f, 0.8f, 0.0f),
	//	DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f),
	//	DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
	//	1.0f,
	//	0
	//));
	//lightCounter++;

	////Diagonal
	//lights.emplace("diagonalLight", LightObject(
	//	DirectX::XMFLOAT3(1.0f, 0.01f, 0.01f),
	//	DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
	//	DirectX::XMFLOAT3(2.0f, 3.0f, 0.0f),
	//	1.0f,
	//	0
	//));
	//lightCounter++;

	////BlueLight
	//lights.emplace("blueLight", LightObject(
	//	DirectX::XMFLOAT3(0.01f, 0.01f, 1.0f),
	//	DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f),
	//	DirectX::XMFLOAT3(-2.0f, 0.0f, -3.0f),
	//	1.0f,
	//	0
	//));
	//lightCounter++;

	////RedLight
	//lights.emplace("redLight", LightObject(
	//	DirectX::XMFLOAT3(1.0f, 0.01f, 0.01f),
	//	DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f),
	//	DirectX::XMFLOAT3(2.0f, 0.0f, -3.0f),
	//	1.0f,
	//	0
	//));
	//lightCounter++;

	//for (int l = lightCounter; l < 64; l++)
	//{
	//	std::string name = "randomLight" + std::to_string(lightCounter);
	//	lights.emplace(name, LightObject(
	//		DirectX::XMFLOAT3(GenRandFloat(), GenRandFloat(), GenRandFloat()),
	//		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
	//		DirectX::XMFLOAT3(GenRandNegAndPos() * 20, GenRandNegAndPos() * 20, GenRandNegAndPos() * 20),
	//		GenRandFloat(),
	//		1
	//	));
	//	lightCounter++;
	//}
}

//Gen rand float for colors 0.0f to 1.0f
float Game::GenRandFloat()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

//Gen rand number from -1.0f to 1.0f
float Game::GenRandNegAndPos()
{
	return -1 + (((float)rand() / (float)RAND_MAX) * 2);
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

	fullscreenVS = new SimpleVertexShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"FullScreenVS.cso").c_str());
	irradiancePS = new SimplePixelShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"IBLIrradianceMapPS.cso").c_str());
	specularConvoledPS = new SimplePixelShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"IBLSpecularConvolutionsPS.cso").c_str());
	lookUpTexturePS = new SimplePixelShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"IBLBrdfLookUpTablePS.cso").c_str());

	//Used to combine all our textures at the end of rendering, before presentation.
	finalCombinePS = new SimplePixelShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"FinalCombinePS.cso").c_str());
	finalOutputPS = new SimplePixelShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"SimpleTexturePS.cso").c_str());

	//Used for objects with refraction as part of their render
	refractionPS = new SimplePixelShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"RefractionPS.cso").c_str());

	//Some shaders for some particle action!
	particlePS = new SimplePixelShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"ParticlePS.cso").c_str());
	particleVS = new SimpleVertexShader(
		device.Get(), context.Get(), GetFullPathTo_Wide(L"ParticleVS.cso").c_str());
}

void Game::RemoveEntity(int index)
{
	Entity* e = liveEntities[index];
	liveEntities.erase(liveEntities.begin() + index);
	delete e;
	e = nullptr;

	//Have to infrom the entities with their new indice locations.
	for (int i = 0; i < liveEntities.size(); i++)
	{
		EntityDef temp;
		temp = liveEntities[i]->GetDataStruct();
		temp.index = i;
		liveEntities[i]->SetDataStruct(temp);
	}
	DecrementCurrentEntity();
}

//Keeps track of the current index (also don't let it fall below 0)
//Allows to return current selected entity.
void Game::IncrementCurrentEntity()
{
	currentIndex++;
	if (currentIndex > liveEntities.size())
	{
		currentIndex = (int)liveEntities.size() - 1;
	}
	currentRender->SetCurrentIndex(currentIndex);
}

void Game::DecrementCurrentEntity()
{
	currentIndex--;
	if (currentIndex < 0)
	{
		currentIndex = 0;
	}
	currentRender->SetCurrentIndex(currentIndex);
}

//Compute shader texture generation. (Make this a helper functions)
void Game::CreateComputeShaderTexture()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> noiseTexture;
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = noiseTextureSize;
	texDesc.Height = noiseTextureSize;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&texDesc, 0, noiseTexture.GetAddressOf());

	// Create the SRV using a default description (passing in null below)
	device->CreateShaderResourceView(noiseTexture.Get(), 0, &computeTextureSRV);

	// Create the UAV that treats this resource as a 2D texture
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = texDesc.Format;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(noiseTexture.Get(), &uavDesc, &computeTextureUAV);
}

//Creates our textures from hard memory.
void Game::LoadTextures(GraphicData newData)
{
	//inputAlbedo.Detach();
	//inputNormal.Detach();
	//inputRough.Detach();
	//inputMetal.Detach();

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

//Load Particle Texture
void Game::LoadEmitterTexture(std::wstring texturePath)
{
	CreateWICTextureFromFile(
		device.Get(), //Allows creation of resource
		GetFullPathTo_Wide(texturePath).c_str(),
		0, // The texture gets made, but we don't need the pointer
		&tempParticleTextureSRV
	);
}

//Load CubeMap Texture
void Game::LoadCubeMap(wstring customSky)
{
	CreateDDSTextureFromFile(
		device.Get(),
		GetFullPathTo_Wide(customSky).c_str(),
		0,
		skyMapSRV.GetAddressOf()
	);

	Mesh* mesh1 = new Mesh(GetFullPathTo("../../Assets/cube.obj").c_str(), device); 
	sky = new SkyMap(mesh1, sampler, device, context, skyMapSRV, pixelSkyShader, vertexSkyShader, fullscreenVS, irradiancePS, specularConvoledPS, lookUpTexturePS);
}

void Game::ChangeCubeMap(wstring newSky)
{
	CreateDDSTextureFromFile(
		device.Get(),
		GetFullPathTo_Wide(newSky).c_str(),
		0,
		skyMapSRV.GetAddressOf()
	);

	sky->RefreshSkyMap(skyMapSRV, fullscreenVS, irradiancePS, specularConvoledPS, lookUpTexturePS);
}

void Game::CreateSpaceScene()
{
	spaceSceneEnabled = true;

	////Setup the Sun
	baseData.meshPath = "../../Assets/sphere.obj";
	baseData.albedoPath = L"../../Assets/Sun/Sun.jpg";
	baseData.normalPath = L"../../Assets/Sun/SunN2.png";
	baseData.roughPath = L"../../Assets/Sun/SunR.png";
	baseData.metalPath = L"../../Assets/Sun/SunM.png";

	CreateEntity(baseData, false);

	//Setup the Earth
	baseData.meshPath = "../../Assets/sphere.obj";
	baseData.albedoPath = L"../../Assets/Earth/Earth.png";
	baseData.normalPath = L"../../Assets/Earth/EarthN.png";
	baseData.roughPath = L"../../Assets/Earth/EarthR.png";
	baseData.metalPath = L"../../Assets/Earth/EarthM.png";
	CreateEntity(baseData, false);
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
	CreateEntity(baseData, false);
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
	CreateEntity(baseData, false);
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
	CreateEntity(baseData, false);
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

void Game::CreateIBLScene()
{
	baseData.meshPath = "../../Assets/sphere.obj";
	//baseData.albedoPath = L"../../Assets/particles/minimush.png";
	baseData.albedoPath = L"../../Assets/defaultTextures/defaultAlbedo.png";
	baseData.normalPath = L"../../Assets/defaultTextures/default_normal.jpg";
	baseData.roughPath = L"../../Assets/defaultTextures/defaultRoughness.png";
	baseData.metalPath = L"../../Assets/defaultTextures/defaultMetal_nonmetal.png";
	EntityPosition entityPosition = { 0, 0, 0 };

#pragma region MetalObjects

	//EntityOne
	CreateEntity(baseData, false);
	//CustomTextureFunction (Device, SRVIndexLocation, R-Value, G-Value, B-Value, A-Value)
	entityPosition = { -5, 1, 4 };
	liveEntities[0]->SetPositionDataStruct(entityPosition);
	liveEntities[0]->GetMaterial()->CustomTextureSet(device, 0, 255, 255, 255, 255); //Abledo
	liveEntities[0]->GetMaterial()->CustomTextureSet(device, 1, 127, 127, 255, 255); //Normal
	liveEntities[0]->GetMaterial()->CustomTextureSet(device, 2, 255, 255, 255, 255); //Metal
	liveEntities[0]->GetMaterial()->CustomTextureSet(device, 3, 0, 0, 0, 255); //Rough

	//EntityTwo
	CreateEntity(baseData, false);
	//CustomTextureFunction (Device, SRVIndexLocation, R-Value, G-Value, B-Value, A-Value)
	entityPosition = { 0, 1, 4 };
	liveEntities[1]->SetPositionDataStruct(entityPosition);
	liveEntities[1]->GetMaterial()->CustomTextureSet(device, 0, 255, 255, 255, 255); //Abledo
	liveEntities[1]->GetMaterial()->CustomTextureSet(device, 1, 127, 127, 255, 255); //Normal
	liveEntities[1]->GetMaterial()->CustomTextureSet(device, 2, 255, 255, 255, 255); //Metal
	liveEntities[1]->GetMaterial()->CustomTextureSet(device, 3, 63, 63, 63, 255); //Rough

	//EntityThree
	CreateEntity(baseData, false);
	//CustomTextureFunction (Device, SRVIndexLocation, R-Value, G-Value, B-Value, A-Value)
	entityPosition = { 5, 1, 4 };
	liveEntities[2]->SetPositionDataStruct(entityPosition);
	liveEntities[2]->GetMaterial()->CustomTextureSet(device, 0, 255, 255, 255, 255); //Abledo
	liveEntities[2]->GetMaterial()->CustomTextureSet(device, 1, 127, 127, 255, 255); //Normal
	liveEntities[2]->GetMaterial()->CustomTextureSet(device, 2, 255, 255, 255, 255); //Metal
	liveEntities[2]->GetMaterial()->CustomTextureSet(device, 3, 127, 127, 127, 255); //Rough

#pragma endregion

#pragma region PlasticObjects

	//EntityFour
	CreateEntity(baseData, false);
	//CustomTextureFunction (Device, SRVIndexLocation, R-Value, G-Value, B-Value, A-Value)
	entityPosition = { -5, -1, 4 };
	liveEntities[3]->SetPositionDataStruct(entityPosition);
	liveEntities[3]->GetMaterial()->CustomTextureSet(device, 0, 125, 125, 125, 255); //Abledo
	liveEntities[3]->GetMaterial()->CustomTextureSet(device, 1, 127, 127, 255, 255); //Normal
	liveEntities[3]->GetMaterial()->CustomTextureSet(device, 2, 0, 0, 0, 255); //Metal
	liveEntities[3]->GetMaterial()->CustomTextureSet(device, 3, 0, 0, 0, 255); //Rough

	//EntityFive
	CreateEntity(baseData, false);
	//CustomTextureFunction (Device, SRVIndexLocation, R-Value, G-Value, B-Value, A-Value)
	entityPosition = { 0, -1, 4 };
	liveEntities[4]->SetPositionDataStruct(entityPosition);
	liveEntities[4]->GetMaterial()->CustomTextureSet(device, 0, 125, 125, 125, 255); //Abledo
	liveEntities[4]->GetMaterial()->CustomTextureSet(device, 1, 127, 127, 255, 255); //Normal
	liveEntities[4]->GetMaterial()->CustomTextureSet(device, 2, 0, 0, 0, 255); //Metal
	liveEntities[4]->GetMaterial()->CustomTextureSet(device, 3, 63, 63, 63, 255); //Rough

	//EntitySix
	CreateEntity(baseData, false);
	//CustomTextureFunction (Device, SRVIndexLocation, R-Value, G-Value, B-Value, A-Value)
	entityPosition = { 5, -1, 4 };
	liveEntities[5]->SetPositionDataStruct(entityPosition);
	liveEntities[5]->GetMaterial()->CustomTextureSet(device, 0, 125, 125, 125, 255); //Abledo
	liveEntities[5]->GetMaterial()->CustomTextureSet(device, 1, 127, 127, 255, 255); //Normal
	liveEntities[5]->GetMaterial()->CustomTextureSet(device, 2, 0, 0, 0, 255); //Metal
	liveEntities[5]->GetMaterial()->CustomTextureSet(device, 3, 127, 127, 127, 255); //Rough
#pragma endregion

#pragma region RefractiveObjects?

	//Entity Seven
	CreateEntity(baseData, true);
	//CustomTextureFunction (Device, SRVIndexLocation, R-Value, G-Value, B-Value, A-Value)
	entityPosition = { 10, -1, 4 };
	liveEntities[6]->SetPositionDataStruct(entityPosition);
	liveEntities[6]->GetMaterial()->CustomTextureSet(device, 0, 255, 255, 255, 255); //Abledo
	liveEntities[6]->GetMaterial()->CustomTextureSet(device, 1, 127, 127, 255, 255); //Normal
	liveEntities[6]->GetMaterial()->CustomTextureSet(device, 2, 0, 0, 0, 255); //Metal
	liveEntities[6]->GetMaterial()->CustomTextureSet(device, 3, 127, 127, 127, 255); //Rough

	entityWindow.AssignTranslation(entityPosition.X, entityPosition.Y, entityPosition.Z);
#pragma endregion

#pragma region Emitters
	LoadEmitterTexture(L"../../Assets/particles/circle_04.png");
	Emitter* em = new Emitter(50, 1, 5, particleVS, particlePS, device, context, tempParticleTextureSRV,
		XMFLOAT3(-50, 0, 0),
		XMFLOAT3(-1, 0, 0),
		XMFLOAT3(-0.5f, 0, 0),
		1);
	emitters.push_back(em);

	LoadEmitterTexture(L"../../Assets/particles/smoke_10.png");
	em = new Emitter(300, 6, 5, particleVS, particlePS, device, context, tempParticleTextureSRV,
		XMFLOAT3(-40, 0, 0),
		XMFLOAT3(0, 0, 0),
		XMFLOAT3(0, 0, 0),
		0);
	emitters.push_back(em);

	LoadEmitterTexture(L"../../Assets/particles/spark_01.png");
	em = new Emitter(400, 8, 5, particleVS, particlePS, device, context, tempParticleTextureSRV,
		XMFLOAT3(-30, 0, 0),
		XMFLOAT3(1, 2, 0),
		XMFLOAT3(0, 2, 0),
		2);
	emitters.push_back(em);

	LoadEmitterTexture(L"../../Assets/particles/light_01.png");
	em = new Emitter(100, 4, 12, particleVS, particlePS, device, context, tempParticleTextureSRV,
		XMFLOAT3(-90, 1, 0),
		XMFLOAT3(8, 0, 4),
		XMFLOAT3(0.7f, 0, 0),
		3);
	emitters.push_back(em);

	LoadEmitterTexture(L"../../Assets/particles/star_07.png");
	em = new Emitter(10, 1, 30, particleVS, particlePS, device, context, tempParticleTextureSRV,
		XMFLOAT3(-80, -6, 0),
		XMFLOAT3(3, 3, 3),
		XMFLOAT3(1, 1, 1),
		4);
	emitters.push_back(em);

	em = nullptr;
#pragma endregion


	baseData.meshPath = "../../Assets/cube.obj";
	//baseData.albedoPath = L"../../Assets/particles/minimush.png";
	baseData.albedoPath = L"../../Assets/defaultTextures/roughGameTextures/game_stone.png";
	baseData.normalPath = L"../../Assets/defaultTextures/roughGameTextures/free_stone.png";
	baseData.roughPath = L"../../Assets/defaultTextures/roughGameTextures/height_map_sample1.png";
	baseData.metalPath = L"../../Assets/defaultTextures/defaultMetal_nonmetal.png";
	entityPosition = { 0, 0, 0 };

	CreateEntity(baseData, false);
	entityPosition = { -5, -5, -5 };
	liveEntities[7]->SetPositionDataStruct(entityPosition);
	liveEntities[7]->GetMaterial()->CustomTextureSet(device, 2, 0, 0, 0, 255); //Metal
	liveEntities[7]->GetMaterial()->CustomTextureSet(device, 3, 127, 127, 127, 255); //Rough
}

void Game::CreateBasicGeometry()
{
	//CreateSpaceScene();
	CreateIBLScene();
}

void Game::CreateEntity(GraphicData newData, bool isRefractive)
{
	std::shared_ptr<Mesh> newMesh = std::make_shared<Mesh>(GetFullPathTo(newData.meshPath).c_str(), device);
	myMeshes.push_back(newMesh.get());

	LoadTextures(newData);
	baseMaterial = std::make_shared<Material>(pixelShader, vertexShader, defaultTint, specularIntensity, isRefractive, inputAlbedo.Get(), inputNormal.Get(), inputRough.Get(), inputMetal.Get(), sampler.Get(), clampSampler.Get());
	
	EntityDef newDef;
	newDef.index = (int)liveEntities.size();
	newDef.shadows = 1;
	newDef.transparency = 0;

	EntityPosition currentPos;
	currentPos.X = 0;
	currentPos.Y = 0;
	currentPos.Z = 0;

	tempEntity = new Entity(newMesh.get(), baseMaterial.get(), newDef, newData);
	liveEntities.push_back(tempEntity);
	IncrementCurrentEntity();
	entityWindow.SetCurrentEntity(newDef, newData, currentPos);
	entityCounter++;
	std::cout << currentRender->EntitiesListSize() << std::endl;
}

void Game::EstablishNewEntityData(GraphicData newData)
{
	LoadTextures(newData);
	std::shared_ptr<Mesh> tempMesh = std::make_shared<Mesh>(Mesh(GetFullPathTo(newData.meshPath).c_str(), device));
	std::shared_ptr<Material> tempMaterial = std::make_shared<Material>(
		//Shaders
		pixelShader, vertexShader, defaultTint, specularIntensity, false,
		//New textures
		inputAlbedo.Get(), inputNormal.Get(), inputRough.Get(), inputMetal.Get(), 
		//Sampler
		sampler.Get(), clampSampler.Get());

	//Make sure to acccess correct Entity
	tempEntity = liveEntities[currentIndex];

	//Set new data for mesh, material, and displayWindow.
	tempEntity->AssignMesh(tempMesh);
	tempEntity->AssignMaterial(tempMaterial);
	tempEntity->SetGraphicDataStruct(newData);
}

void Game::EstablishNewLightData()
{
	
}

//Updates the data values stored within the GUI from live data in the renderer class
void Game::UpdateGUIWindow()
{
	entityWindow.SetCurrentEntity(
		currentRender->ReturnCurrentEntity()->GetDataStruct(),
		currentRender->ReturnCurrentEntity()->GetGraphicDataStruct(),
		currentRender->ReturnCurrentEntity()->GetPositionDataStruct()
	);
}

// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	currentRender->PreResize();

	// Handle base-level DX resize stuff
	DXCore::OnResize();

	currentRender->PostResize(DXCore::width, DXCore::height, backBufferRTV, depthStencilView);

	//Update Camera for new window shape
	camera->UpdateProjectionMatrix((float)width / height);
}

void Game::HandleUIActions()
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	if (ImGui::GetIO().WantCaptureMouse)
	{
		//TODO: Move this into a custom Input Class (didn't realize this wasn't already in one) 
		//Open/Close UI
		if (GetAsyncKeyState('W') & 0x8000)
		{
			entityWindow.Enabled(false);
		}
		else if (GetAsyncKeyState('S') & 0x8000)
		{
			entityWindow.Enabled(true);
		}

		if (GetAsyncKeyState('A') & 0x8000)
		{
			if ((entityCounter > 1 && entityWindow.ReturnEntityData().index > 0) && entityWindow.GetKeyLock())
			{
				DecrementCurrentEntity();
				UpdateGUIWindow();
			}
		}

		if (GetAsyncKeyState('D') & 0x8000)
		{
			if ((entityWindow.ReturnEntityData().index < currentRender->EntitiesListSize() - 1) && entityWindow.GetKeyLock())
			{
				IncrementCurrentEntity();
				UpdateGUIWindow();
			}
		}

		if (!(GetAsyncKeyState('A') & 0x8000) && !(GetAsyncKeyState('D') & 0x8000))
		{
			entityWindow.ReleaseKeyLock();
		}
	}

	switch (entityWindow.GetState())
	{
	case WindowState::CreateNewEntity:
		CreateEntity(entityWindow.ReturnData(), false);
		break;

	case WindowState::ApplyingData: {
		GraphicData newData = entityWindow.ReturnData();
		EstablishNewEntityData(newData);
		UpdateGUIWindow(); 
		}
		break;

	case WindowState::DeleteCurrentEntity:
		if (entityCounter >= 1)
		{
			RemoveEntity(entityWindow.ReturnEntityData().index);
			myMeshes.erase(myMeshes.begin() + entityWindow.ReturnEntityData().index);
			UpdateGUIWindow();
			entityCounter--;
		}
		break;

	case WindowState::AddChild:
		if (currentIndex <= liveEntities.size() - 2)
		{
			int temp = currentIndex;
			liveEntities[currentIndex]->GetTransform()->AddChild(liveEntities[++temp]->GetTransform());
		}
		break;

		// FIX: Doesn't handle at any iterator value of the entties vector
	case WindowState::RemoveChild:
		if(currentIndex == 0 && liveEntities[currentIndex]->GetTransform()->GetChildCount() > 0)
		{
			liveEntities[currentIndex]->GetTransform()->RemoveChild(
				liveEntities[currentIndex]->GetTransform()->GetChild(0)
			);
		}
		break;

	case WindowState::ApplyingSky: {
		ChangeCubeMap(entityWindow.ReturnSkyPath());
		}
		break;

	case WindowState::SavingScene:
		dm->ConvertSceneToData();
		break;
	default:
		break;
	}

	currentIndex = currentRender->ReturnCurrentEntityIndex();

	//Update position of object
	currentRender->AlterPosition(entityWindow.ReturnTranslation());
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	HandleUIActions();

	if (spaceSceneEnabled)
	{
		liveEntities[0]->GetTransform()->Rotate(0, 0.003f, 0);
		liveEntities[1]->GetTransform()->Rotate(0, 0.001f, 0);
		liveEntities[2]->GetTransform()->Rotate(0.0003f, 0.0003f, 0);
		liveEntities[3]->GetTransform()->Rotate(0.001f, 0.002f, 0);
		float temp = (XMScalarCos(1.6f) + 1) / 8;

		liveEntities[3]->GetTransform()->SetScale(temp, temp, temp);
	}
	//currentRender.Update(deltaTime, totalTime);
	camera->Update(deltaTime, this->hWnd);

	for (auto& em : emitters)
	{
		em->Update(deltaTime, totalTime);
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	//Start ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	currentRender->Render(deltaTime, totalTime, camera, &entityWindow, &windows, hWnd);
}