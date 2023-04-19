#include "HashNoise.h"

HashNoise::HashNoise(Microsoft::WRL::ComPtr<ID3D11Device> dev, INT32 _seed)
{
    seed = _seed;
    length = resolution * resolution;
    hashes = new uint32_t[length];

    resolution = 32;
    invResolution = 1.0f / resolution;
    device = dev;

    accumulator = (uint32_t)seed + primes.E;
    avalanche = accumulator;
    avalanche ^= avalanche >> 15;
    avalanche *= primes.B;
    avalanche ^=  avalanche >> 13;
    avalanche *= primes.C;
    avalanche ^= avalanche >> 16;
}

HashNoise::~HashNoise()
{
    delete hashes;
    hashes = nullptr;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> HashNoise::GenerateHashNoiseImage()
{
    const uint8_t R = 10;
    const uint8_t G = 255;
    const uint8_t B = 10;
    const uint8_t A = 255;

    for (int i = 0; i < length; i++)
    {
        ExecuteNoise(i);
    }

    uint32_t s_pixel = R | (G << 8) | (B << 16) | (A << 24);

    //uint32_t pixels[50];

    //for (auto p : pixels)
    //{
    //    p = R | (G << 8) | (B << 16) | (A << 24);
    //}

    D3D11_SUBRESOURCE_DATA initData;
    initData = { 
        &(hashes[0]), //or &pixels
        sizeof(int32_t)*32,
        0 };

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = resolution;
    desc.Height = resolution;
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

        hr = device->CreateShaderResourceView(texture.Get(), &newSRVDesc, &textureSRV);
    }
    if (FAILED(hr))
    {
        std::cout << hr << "Error in making Hash Noisemap!" << std::endl;
    }

    return textureSRV;
}

float HashNoise::ReturnDecimal(float value)
{
    return value - floor(value);
}

uint32_t HashNoise::RotateLeft(uint32_t data, int steps)
{
    return (data << steps) | (data >> 32 - steps);
}

void HashNoise::ExecuteNoise(int iteration)
{
    //Weyl Sequence
    //hashes[iteration] = (uint32_t)(ReturnDecimal(iteration * 0.381f) * 256.0f);

    //Gradient
    int v = (int)floor(invResolution * iteration + 0.00001f);
    int u = iteration - resolution * v;
    v -= resolution / 2;
    //hashes[iteration] = (uint32_t)(ReturnDecimal(u * v * 0.381f) * 256.0f);

    //Eating method
    uint32_t hash = uint32_t(avalanche);
    std::cout << "Before Dinner: " << hash << std::endl;
    HashEat(u);
    HashEat(v);
    hash = accumulator;
    std::cout << "After Dinner: " << hash << std::endl;
    hashes[iteration] = hash;
    std::cout << "After Dinner: " << hashes[iteration] << std::endl;
   
}

void HashNoise::HashEat(int data)
{
    accumulator = RotateLeft(accumulator + (uint32_t)data * primes.E, 11) * primes.A;
}
