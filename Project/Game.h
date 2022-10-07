//Kyle Zielinski
//2/04/2021
//Header file for game window, stored mesh fields here using smart pointers.
#pragma once

#include <memory>
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <iostream>
#include <vector>
#include "DXCore.h"
#include "Mesh.h"
#include "Material.h"
#include "SimpleShader.h"
#include "LightObject.h"
#include "Lights.h"
#include "Renderer.h"
#include "BufferStructs.h"
#include "Transform.h"
#include "Camera.h"
#include "EntityWindow.h"
#include "imgGUI/imgui.h"
#include "imgGUI/imgui_impl_win32.h"
#include "imgGUI/imgui_impl_dx11.h"
#include "Emitter.h"
#include "stdlib.h"


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
	void LoadTextures(GraphicData newData);
	void LoadEmitterTexture(std::wstring texturePath);
	void LoadCubeMap(wstring customSky);
	void ChangeCubeMap(wstring newSky);
	void CreateBasicGeometry();
	void CreateSpaceScene();
	void CreateIBLScene();
	void CreateEntity(GraphicData newData, bool isRefractive);
	void EstablishNewEntityData(GraphicData newData);
	void EstablishNewLightData();
	void UpdateGUIWindow();
	void HandleUIActions();
	void RemoveEntity(int index);
	void IncrementCurrentEntity();
	void DecrementCurrentEntity();
	float GenRandFloat();
	float GenRandNegAndPos();
	
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

	//IBL
	SimpleVertexShader* fullscreenVS;
	SimplePixelShader* irradiancePS;
	SimplePixelShader* specularConvoledPS;
	SimplePixelShader* lookUpTexturePS;

	//Refraction+
	SimplePixelShader* finalCombinePS;
	SimplePixelShader* finalOutputPS;
	SimplePixelShader* refractionPS;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	//Texture Resources
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyMapSRV; 
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> inputAlbedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> inputNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> inputRough;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> inputMetal;

	//Sampler, Rasterizer, Depth-Stencil States (Min: One per program)
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler; 
	Microsoft::WRL::ComPtr<ID3D11SamplerState> clampSampler; 
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencil;
	
	//BaseTexture storage
	GraphicData baseData;

	//Vector for entity testing
	std::vector<Mesh*> myMeshes;

	//Materials
	std::shared_ptr<Material> baseMaterial;
	
	//TestWindow
	UIWindow testWindow;
	EntityWindow entityWindow;
	Entity* tempEntity;
	vector<Entity*> liveEntities;
	//vector<Light> lights;
	int currentIndex;

	SkyMap* sky;

	//Renderer
	Renderer* currentRender;
	//Counter to keep track objects in scene
	int entityCounter;
	int lightCounter;

	// A simple transform for testing
	Transform transform;

	//Rise up Guardian
	std::unordered_map<std::string, LightObject> lights;
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

	//Current Active Scnee (controls transformations etc.)
	bool spaceSceneEnabled;

	vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> staticColors;
	ID3D11ShaderResourceView* newSRV;
	ID3D11ShaderResourceView* locationSRV;

	//Emitters
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tempParticleTextureSRV;
	vector<Emitter*> emitters;
	SimpleVertexShader* particleVS;
	SimplePixelShader* particlePS;

	// Compute shader related resources
	unsigned int noiseTextureSize;
	int noiseInterations;
	float noisePersistance;
	float noiseScale;
	float noiseOffset;
	bool computeShaderActive;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> computeTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> computeTextureUAV;
	void CreateComputeShaderTexture();
};