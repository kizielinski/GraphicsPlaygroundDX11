//Kyle Zielinski 
//3-15-2021
//Struct for directionalLight in c++
#pragma once
#include <DirectXMath.h>

#define LIGHT_DIRECTIONAL	0
#define LIGHT_POINT		    1
//#define LIGHT_SPOT			2

struct Light
{
	DirectX::XMFLOAT3 color;
	float intensity;
	DirectX::XMFLOAT3 direction;
	int lightType; 
	DirectX::XMFLOAT3 position;
	float range;
};
