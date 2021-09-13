//Kyle Zielinski 
//3-15-2021
//Struct for directionalLight in c++
#pragma once
#include <DirectXMath.h>

struct Light
{
	DirectX::XMFLOAT3 color;
	float intensity;
	DirectX::XMFLOAT3 direction;
	int lightType; 
	DirectX::XMFLOAT3 position;
};
