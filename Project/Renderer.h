#pragma once
#include "Entity.h"
#include "Lights.h"
#include "SkyMap.h"
#include "EntityWindow.h"
#include "SimpleShader.h"
#include <wrl/client.h>

class Renderer
{
public:
	Renderer();
	Renderer(
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV,
		unsigned int windowWidth,
		unsigned int windowHeight,
		//std::vector<Entity*> entities,
		std::vector<Light> lights,
		SimplePixelShader* pShader,
		SimpleVertexShader* vShader
	);
	~Renderer();
	void Update(float deltaTime, float totalTime);
	void Order();
	void Render(float deltaTime, float totalTime, Camera* cam, EntityWindow* eW, HWND windowHandle);
	void SetEntities(vector<Entity*> _myEntities); //Uses const value of entities now rather than passing in the current vector
	//void SetEntities();
	void AddSkyBox(SkyMap* sM);
	void AlterPosition(EntityPosition entityPos);
	int EntitiesListSize();
	void RemoveEntity(int index);
	void IncrementCurrentEntity();
	void DecrementCurrentEntity();
	int ReturnCurrentEntityIndex();
	Entity ReturnCurrentEntity();

	void PostResize(
		unsigned int windowWidth,
		unsigned int windowHeight,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV
	);
		
private:
	std::vector<Entity*> myEntities;
	//Skybox
	SkyMap* mySkyBox;
	int currentIndex;
	Light light;
	DirectX::XMFLOAT3 ambientColor;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV;
	unsigned int windowWidth;
	unsigned int windowHeight;
	//const std::vector<Entity*> entities;
	//const std::vector<Light> lights;
	std::vector<Light> lights;

	SimplePixelShader* pixelShader;
	SimpleVertexShader* vertexShader;

	void DrawPointLights(Camera* cam);
	void SetUpLights(Camera* cam);
	void LoadLighting();

};

