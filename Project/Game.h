//Kyle Zielinski
//2/04/2021
//Header file for game window, stored mesh fields here using smart pointers.
#pragma once
#include "DXCore.h"
#include "Mesh.h"
#include "Material.h"
#include "SimpleShader.h"
#include "Lights.h"
#include "Renderer.h"
#include <memory>
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "BufferStructs.h"
#include "Transform.h"
#include <vector>
#include "Camera.h"
#include "UIWindow.h"
#include "EntityWindow.h"
#include "imgGUI/imgui.h"
#include "imgGUI/imgui_impl_win32.h"
#include "imgGUI/imgui_impl_dx11.h"


class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	int counter = 0;
	int valueToRotate;

private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadLighting();
	void LoadShaders();
	void LoadDefaultTextures(); //Default objects we've been using all semester.
	void LoadTextures(GraphicData newData);
	void LoadCubeMap(wstring customSky);
	void SetUpLights();
	void SetUpLightsNormal();
	void CreateBasicGeometry();
	void CreateEntity(GraphicData newData);
	void EstablishNewEntityData(GraphicData newData);
	void EstablishNewLightData();
	void UpdateGUIWindow();

	
	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//    Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr
	
	// Shaders and shader-related constructs
	//Replaced with Simple Shader variation
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	//Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;	
	SimplePixelShader* pixelShader;
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelSkyShader;
	SimpleVertexShader* vertexSkyShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	//Texture Resources
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> spaceMapSRV; 
	
	//PBR Textures - List depricated, these textures are dynamic now.
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleA; 
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleN; 
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleR; 
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleM; 
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeA;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeN;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeR;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeM;
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> inputAlbedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> inputNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> inputRough;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> inputMetal;

	//Sampler, Rasterizer, Depth-Stencil States (Min: One per program)
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler; 
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencil;
	
	//BaseTexture storage
	GraphicData baseData;

	//Vector for entity testing
	std::vector<Mesh*> myMeshes;

	//Materials
	std::shared_ptr<Material> baseMaterial;
	//std::shared_ptr<Material> secondaryMaterial;
	//std::shared_ptr<Material> tertiaryMaterial;
	
	//TestWindow
	UIWindow testWindow;
	EntityWindow entityWindow;
	Entity* tempEntity;
	vector<Entity*> liveEntities;
	int currentIndex;

	//Renderer
	Renderer currentRender;
	//Counter to keep track objects in scene
	int entityCounter;
	int lightCounter;

	// A simple transform for testing
	Transform transform;

	//Rise up Gaurdian
	Light light;
	Light upward;
	Light diagonal;
	Light grey;
	Light defaultLight;
	float specularIntensity;

	//Defaults
	DirectX::XMFLOAT4 defaultTint;
	DirectX::XMFLOAT3 ambientColor;

	//Camera for our 3D Scene
	Camera* camera;
};

