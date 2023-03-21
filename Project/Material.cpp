//Kyle Zielinski
//3/02/2021
//Class implementation of a material object with getters and setters.
#include "Material.h"
#include <memory>
#include <DirectXMath.h>

using namespace DirectX;
using namespace std;

Material::Material(SimplePixelShader* pixelShader, SimpleVertexShader* vertexShader, DirectX::XMFLOAT4 colorTint, float specularIntensity, bool _isRefractive, ID3D11ShaderResourceView* _textureSRV, ID3D11ShaderResourceView* _normalMapSRV, ID3D11ShaderResourceView* _roughMapSRV, ID3D11ShaderResourceView* _metalMapSRV, ID3D11SamplerState* _sampler, ID3D11SamplerState* _clampSampler)
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
    isRefractive = _isRefractive;
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
    textureSRV->Release();
    normalMapSRV->Release();
    roughMapSRV->Release();
    metalMapSRV->Release();
}

SimplePixelShader* Material::GetPixelShader()
{
    return pShader;
}

SimpleVertexShader* Material::GetVertexShader()
{
    return vShader;
}

void Material::SetPixelShader(SimplePixelShader* ps)
{
    pShader = ps;
}

void Material::SetVertexShader(SimpleVertexShader* vs)
{
    vShader = vs;
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

bool Material::IsRefractive()
{
    return isRefractive;
}

void Material::SetRefractive(bool value)
{
    isRefractive = value;
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

void Material::PrepMaterialForDraw(Transform* transform, Camera* cam)
{
    vShader->SetShader();
    pShader->SetShader();

   //Only Setting vertex shader data
   vShader->SetMatrix4x4("world", transform->GetWorldMatrix());
   vShader->SetMatrix4x4("worldInverseTranspose", transform->GetWorldITMatrix());
   vShader->SetMatrix4x4("view", cam->GetViewMatrix());
   vShader->SetMatrix4x4("projection", cam->GetProjectionMatrix());
   XMFLOAT2 uvScale = XMFLOAT2(2, 2);
   vShader->SetFloat2("uvScale", uvScale);
   vShader->CopyAllBufferData();
}

void Material::InsertNewTexture(ID3D11ShaderResourceView* inputTexture, ID3D11ShaderResourceView* textureToChange)
{
    textureToChange->Release();
    textureToChange = nullptr;
    textureToChange = inputTexture;
}

void Material::CustomTextureSet(
    Microsoft::WRL::ComPtr<ID3D11Device> device,
    int srvIndex,
    const uint8_t R,
    const uint8_t G,
    const uint8_t B,
    const uint8_t A)
{
    uint32_t s_pixel = R | (G << 8) | (B << 16) | (A << 24);
    D3D11_SUBRESOURCE_DATA initData = { &s_pixel, sizeof(uint32_t), 0 };

    //texture description
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = desc.Height = 1;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    //Figured out the issue, had desc.CPUAcessFlag as writable since i stole it from
    //one of the other shaders. Do NOT do that if you want hr to succeed. Nothing will 
    //be written to the comptr texture if the Cpu can write as part of the access flag.

    //create texture
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = NULL;
    HRESULT hr = device->CreateTexture2D(&desc, &initData, texture.GetAddressOf());

    //Need to use HRESULT to get the output of device...no other way around it.
    if (SUCCEEDED(hr))
    {
        //Format srv description
        D3D11_SHADER_RESOURCE_VIEW_DESC newSRVDesc = {};
        newSRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        newSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        newSRVDesc.Texture2D.MipLevels = 1;
        newSRVDesc.Texture2D.MostDetailedMip = 0;

        switch (srvIndex)
        {
        case 0: hr = device->CreateShaderResourceView(texture.Get(), &newSRVDesc, &textureSRV);
            break;
        case 1: hr = device->CreateShaderResourceView(texture.Get(), &newSRVDesc, &normalMapSRV);
            break;
        case 2: hr = device->CreateShaderResourceView(texture.Get(), &newSRVDesc, &metalMapSRV);
            break;
        case 3: hr = device->CreateShaderResourceView(texture.Get(), &newSRVDesc, &roughMapSRV);
            break;
        }
    }
    if (FAILED(hr))
    {
        std::cout << hr << "Error in making Custom Texture!" << std::endl;
    }

      
}

