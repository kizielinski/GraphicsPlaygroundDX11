//Kyle Zielinski
//3/02/2021
//Class header for a material object that holds data regaring colortint, pixelshaders, and vertexshaders.
#pragma once

#include "SimpleShader.h"
#include "DXCore.h"
#include "Camera.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include <iostream>

using namespace std;

class Material
{
public:

	Material(
		SimplePixelShader* pixelShader, 
		SimpleVertexShader* vertexShader, 
		DirectX::XMFLOAT4 colorTint, 
		float specularIntensity,
		bool isRefractive,
		ID3D11ShaderResourceView* _textureSRV, 
		ID3D11ShaderResourceView* _normalMapSRV, 
		ID3D11ShaderResourceView* _roughMapSRV, 
		ID3D11ShaderResourceView* _metalMapSRV, 
		ID3D11SamplerState* _sampler,
		ID3D11SamplerState* _clampSampler);
	~Material();
	SimplePixelShader* GetPixelShader();
	SimpleVertexShader* GetVertexShader();
	void SetPixelShader(SimplePixelShader* ps);
	void SetVertexShader(SimpleVertexShader* vs);
	DirectX::XMFLOAT4 GetColorTint();
	float GetSpecularIntensity();
	ID3D11SamplerState* GetSampleState();
	ID3D11SamplerState* GetClampSampleState();
	void SetColorTint(DirectX::XMFLOAT4 newTint);
	void PrepMaterialForDraw(Transform* transform, Camera* cam);
	void InsertNewTexture(ID3D11ShaderResourceView* inputTexture, ID3D11ShaderResourceView* textureToChange);
	void ClearMaterial();
	ID3D11ShaderResourceView* BaseTexture();
	ID3D11ShaderResourceView* NormalTexture();
	ID3D11ShaderResourceView* RoughTexture();
	ID3D11ShaderResourceView* MetalTexture();
	void CustomTextureSet(
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		int srvIndex,
		const uint8_t R,
		const uint8_t G,
		const uint8_t B,
		const uint8_t A);
	bool IsRefractive();
	void SetRefractive(bool value);

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

	//Refraction
	bool isRefractive;
};

