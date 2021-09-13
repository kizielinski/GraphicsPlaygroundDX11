#include "SkyMap.h"

//Default constructor
SkyMap::SkyMap()
{
	skyMesh = nullptr;
	samplerOptions = nullptr;
	cubeSRV = nullptr;
	depthBufferType = nullptr;
	raterizerOption = nullptr;
	
	pixelSkyShader = nullptr;
	vertexSkyShader = nullptr;
}

//Constructor that sets up all necessary values for a valid skyMap
SkyMap::SkyMap(Mesh* _mesh, Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerOptions, Microsoft::WRL::ComPtr<ID3D11Device> _device, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _mapTexture, SimplePixelShader* _pSS, SimpleVertexShader* _vSS)
{
	_samplerOptions.CopyTo(samplerOptions.GetAddressOf());
	_mapTexture.CopyTo(cubeSRV.GetAddressOf());
	skyMesh = _mesh;
	//Intialize samplerState
	D3D11_RASTERIZER_DESC rasterizerDescription = {};
	rasterizerDescription.FillMode = D3D11_FILL_SOLID;
	rasterizerDescription.CullMode = D3D11_CULL_FRONT;
	_device->CreateRasterizerState(&rasterizerDescription, raterizerOption.GetAddressOf());

	//Initialize depthStencilState
	D3D11_DEPTH_STENCIL_DESC depthStencilDescription = {};
	depthStencilDescription.DepthEnable = true;
	depthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	_device->CreateDepthStencilState(&depthStencilDescription, depthBufferType.GetAddressOf());

	pixelSkyShader = _pSS;
	vertexSkyShader = _vSS;
}

SkyMap::~SkyMap()
{
	delete skyMesh;
	skyMesh = nullptr;
	////Might not need
	//delete pixelSkyShader;
	//delete vertexSkyShader;
	////!!!!!!
	/*pixelSkyShader = nullptr;
	vertexSkyShader = nullptr;*/
}

SimplePixelShader* SkyMap::GetPixelShader()
{
	return pixelSkyShader;
}

SimpleVertexShader* SkyMap::GetVertexShader()
{
	return vertexSkyShader;
}

void SkyMap::SkyDraw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* cam)
{
	context->RSSetState(raterizerOption.Get());
	context->OMSetDepthStencilState(depthBufferType.Get(), 0);

	GetPixelShader()->SetShader();
	GetVertexShader()->SetShader();
	
	pixelSkyShader->SetShaderResourceView("cubeMap", cubeSRV.Get());
	pixelSkyShader->SetSamplerState("basicSampler", samplerOptions.Get());

	vertexSkyShader->SetMatrix4x4("view", cam->GetViewMatrix());
	vertexSkyShader->SetMatrix4x4("projection", cam->GetProjectionMatrix());

	pixelSkyShader->CopyAllBufferData();
	vertexSkyShader->CopyAllBufferData();

	skyMesh->DrawUsingBuffs(context);
	
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}
