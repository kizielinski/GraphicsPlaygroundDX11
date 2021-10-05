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

	Material(
		SimplePixelShader* pixelShader, 
		SimpleVertexShader* vertexShader, 
		DirectX::XMFLOAT4 colorTint, 
		float specularIntensity, 
		ID3D11ShaderResourceView* _textureSRV, 
		ID3D11ShaderResourceView* _normalMapSRV, 
		ID3D11ShaderResourceView* _roughMapSRV, 
		ID3D11ShaderResourceView* _metalMapSRV, 
		ID3D11SamplerState* _sampler,
		ID3D11SamplerState* _clampSampler);
	~Material();
	SimplePixelShader* GetPixelShader();
	SimpleVertexShader* GetVertexShader();
	DirectX::XMFLOAT4 GetColorTint();
	float GetSpecularIntensity();
	ID3D11SamplerState* GetSampleState();
	ID3D11SamplerState* GetClampSampleState();
	void SetColorTint(DirectX::XMFLOAT4 newTint);
	void InsertNewTexture(ID3D11ShaderResourceView* inputTexture, ID3D11ShaderResourceView* textureToChange);
	void ClearMaterial();
	ID3D11ShaderResourceView* BaseTexture();
	ID3D11ShaderResourceView* NormalTexture();
	ID3D11ShaderResourceView* RoughTexture();
	ID3D11ShaderResourceView* MetalTexture();
	void CustomTextureSet(
		ID3D11ShaderResourceView* albedo,
		ID3D11ShaderResourceView* normal,
		ID3D11ShaderResourceView* metal,
		ID3D11ShaderResourceView* roughness);


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
	ID3D11SamplerState* clampSampler;
};

