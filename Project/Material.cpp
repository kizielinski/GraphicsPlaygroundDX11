//Kyle Zielinski
//3/02/2021
//Class implementation of a material object with getters and setters.
#include "Material.h"
#include <memory>
using namespace std;

Material::Material(SimplePixelShader* pixelShader, SimpleVertexShader* vertexShader, DirectX::XMFLOAT4 colorTint, float specularIntensity, ID3D11ShaderResourceView* _textureSRV, ID3D11ShaderResourceView* _normalMapSRV, ID3D11ShaderResourceView* _roughMapSRV, ID3D11ShaderResourceView* _metalMapSRV, ID3D11SamplerState* _sampler, ID3D11SamplerState* _clampSampler)
{
    pShader = pixelShader;
    vShader = vertexShader;
    cTint = colorTint;
    sIntensity = specularIntensity;
    textureSRV = _textureSRV;
    normalMapSRV = _normalMapSRV;
    roughMapSRV = _roughMapSRV;
    metalMapSRV = _metalMapSRV;
    sampler = _sampler;
    clampSampler = _clampSampler;
}

Material::~Material()
{
    pShader = nullptr;
    vShader = nullptr;
    sampler = nullptr;
    clampSampler = nullptr;
    textureSRV   = nullptr;
    normalMapSRV = nullptr;
    roughMapSRV  = nullptr;
    metalMapSRV  = nullptr;   
}

void Material::ClearMaterial()
{
    /*textureSRV->Release();
    normalMapSRV->Release();
    roughMapSRV->Release();
    metalMapSRV->Release();*/
}

SimplePixelShader* Material::GetPixelShader()
{
    return pShader;
}

SimpleVertexShader* Material::GetVertexShader()
{
    return vShader;
}

DirectX::XMFLOAT4 Material::GetColorTint()
{
    return cTint;
}

float Material::GetSpecularIntensity()
{
    return sIntensity;
}

ID3D11ShaderResourceView* Material::BaseTexture()
{
    return textureSRV;
}

ID3D11ShaderResourceView* Material::NormalTexture()
{
    return normalMapSRV;
}

ID3D11ShaderResourceView* Material::RoughTexture()
{
    return roughMapSRV;
}

ID3D11ShaderResourceView* Material::MetalTexture()
{
    return metalMapSRV;
}

ID3D11SamplerState* Material::GetSampleState()
{
    return sampler;
}

ID3D11SamplerState* Material::GetClampSampleState()
{
    return clampSampler;
}


void Material::SetColorTint(DirectX::XMFLOAT4 newTint)
{
    cTint = newTint;
}

void Material::InsertNewTexture(ID3D11ShaderResourceView* inputTexture, ID3D11ShaderResourceView* textureToChange)
{
    textureToChange->Release();
    textureToChange = nullptr;
    textureToChange = inputTexture;
}

void Material::CustomTextureSet(
    ID3D11ShaderResourceView* albedo,
    ID3D11ShaderResourceView* normal,
    ID3D11ShaderResourceView* metal,
    ID3D11ShaderResourceView* roughness)
{
    textureSRV = albedo;
    normalMapSRV = normal;
    metalMapSRV = metal;
    roughMapSRV = roughness;
}

