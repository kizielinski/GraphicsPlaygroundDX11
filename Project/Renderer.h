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
	Renderer(
		Microsoft::WRL::ComPtr<ID3D11Device> _device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
		Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV,
		unsigned int _windowWidth,
		unsigned int _windowHeight,
		SimplePixelShader* _pShader,
		SimpleVertexShader* _vShader,
		SkyMap* _sky,
		const std::vector<Entity*>& _entities,
		const std::vector<Light>& _lights
	);
	~Renderer();

	void Update(float deltaTime, float totalTime);
	void Order();
	void Render(float deltaTime, float totalTime, Camera* cam, EntityWindow* eW, HWND windowHandle);
	void SetCurrentIndex(int index);
	void AddSkyBox(SkyMap* sM);
	void AlterPosition(EntityPosition entityPos);
	int EntitiesListSize();
	int ReturnCurrentEntityIndex();
	Entity ReturnCurrentEntity();

	void PreResize();

	void PostResize(
		unsigned int windowWidth,
		unsigned int windowHeight,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV
	);
		
private:
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
	const std::vector<Entity*>& entities;
	const std::vector<Light>& lights;

	SimplePixelShader* pixelShader;
	SimpleVertexShader* vertexShader;

	void DrawPointLights(Camera* cam);
	void SetUpLights(Camera* cam);
	void LoadLighting();

};

