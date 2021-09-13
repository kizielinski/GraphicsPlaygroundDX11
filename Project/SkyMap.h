#pragma once
#include "Camera.h"
#include "Mesh.h"
#include "SimpleShader.h"
#include <wrl/client.h>

class SkyMap
{
public:
	SkyMap();
	SkyMap(Mesh* _mesh, Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerOptions, Microsoft::WRL::ComPtr<ID3D11Device> _device, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>, SimplePixelShader* _pSS, SimpleVertexShader* _vSS);
	~SkyMap();
	SimplePixelShader* GetPixelShader();
	SimpleVertexShader* GetVertexShader();
	void SkyDraw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> device, Camera* cam);
private:

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthBufferType;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> raterizerOption;

	Mesh* skyMesh;
	SimplePixelShader* pixelSkyShader;
	SimpleVertexShader* vertexSkyShader;
};

