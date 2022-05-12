#pragma once
#include "Entity.h"
#include "Lights.h"
#include "SkyMap.h"
#include "Emitter.h"
#include "EntityWindow.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include <wrl/client.h>

using namespace DirectX;

class Renderer
{
public:
	Renderer(
		Microsoft::WRL::ComPtr<ID3D11Device> _device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
		Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerOptions,
		unsigned int _windowWidth,
		unsigned int _windowHeight,
		SimplePixelShader* _pShader,
		SimplePixelShader* _finalCombinePS,
		SimplePixelShader* _finalOutputPS,
		SimplePixelShader* _refractionPS,
		SimpleVertexShader* _vShader,
		SimpleVertexShader* _fsVS,
		SkyMap* _sky,
		const std::vector<Entity*>& _entities,
		const std::vector<Light>& _lights,
		const std::vector<Emitter*>& _emitters
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

	void CreateRenderTarget(
		unsigned int width,
		unsigned int height,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSceneColorSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSceneAmbientColorSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSceneNormalSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSceneDepthSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetFluidSRV();

	void RenderWindow();

private:
	//Info from Game
	SkyMap* mySkyBox;
	int currentIndex;
	Light light;
	const std::vector<Entity*>& entities;
	const std::vector<Light>& lights;
	const std::vector<Emitter*>& emitters;

	DirectX::XMFLOAT3 ambientColor;

	//Core DX resources renderer needs access to
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

	//Other DiriectX Resources
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV;

	//Multiple Render Targets Resource fields
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneColorRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneAmbientColorRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneNormalRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneDepthRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> refracRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> finalRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneColorSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneAmbientColorSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneDepthSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> refracSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> finalSRV;
	
	//Fluid Simulation
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> fluidSRV;

	// Particle states
	Microsoft::WRL::ComPtr<ID3D11BlendState> particleBlendAdditive;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> particleDepthState;

	//Refraction
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> refractionSilhouetteDepthState;
	bool useRefracSil;
	bool refracNormalMap;
	float indexOfRefraction;
	float refracScale;

	//Window Dims
	unsigned int windowWidth;
	unsigned int windowHeight;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;

	SimplePixelShader* pixelShader;
	SimplePixelShader* finalCombinePS;
	SimplePixelShader* finalOutputPS;
	SimplePixelShader* refractionPS;
	SimpleVertexShader* vertexShader;
	SimpleVertexShader* fullScreenVS;

	void DrawPointLights(Camera* cam);
	void SetUpLights(Camera* cam);
	void LoadLighting();
};

