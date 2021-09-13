//Kyle Zielinski
//3/02/2021
//Class header for a material object that holds data regaring colortint, pixelshaders, and vertexshaders.
#pragma once

#include "SimpleShader.h"
#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>

using namespace std;

class Material
{
public:

	Material(SimplePixelShader* pixelShader, SimpleVertexShader* vertexShader, DirectX::XMFLOAT4 colorTint, float specularIntensity, ID3D11ShaderResourceView* _textureSRV, ID3D11ShaderResourceView* _normalMapSRV, ID3D11ShaderResourceView* _roughMapSRV, ID3D11ShaderResourceView* _metalMapSRV, ID3D11SamplerState* _sampler);
	~Material();
	SimplePixelShader* GetPixelShader();
	SimpleVertexShader* GetVertexShader();
	DirectX::XMFLOAT4 GetColorTint();
	float GetSpecularIntensity();
	ID3D11ShaderResourceView* GetTextureSRV();
	ID3D11ShaderResourceView* GetNormalMapSRV();
	ID3D11SamplerState* GetSampleState();
	void SetColorTint(DirectX::XMFLOAT4 newTint);
	void InsertNewTexture(ID3D11ShaderResourceView* inputTexture, ID3D11ShaderResourceView* textureToChange);
	void ClearMaterial();
	ID3D11ShaderResourceView* BaseTexture();
	ID3D11ShaderResourceView* NormalTexture();
	ID3D11ShaderResourceView* RoughTexture();
	ID3D11ShaderResourceView* MetalTexture();

private:
	DirectX::XMFLOAT4 cTint;
	float sIntensity;
	SimplePixelShader* pShader;
	SimpleVertexShader* vShader;

	ID3D11ShaderResourceView* textureSRV;
	ID3D11ShaderResourceView* normalMapSRV;
	ID3D11ShaderResourceView* roughMapSRV;
	ID3D11ShaderResourceView* metalMapSRV;
	ID3D11SamplerState* sampler;
};

