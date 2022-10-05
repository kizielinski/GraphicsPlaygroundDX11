//KZ
//10/5/2022
//Simple Light class to adjust struct data easily for Lights.

#include "Lights.h"

#pragma once
class LightObject
{
public:
	LightObject();
	LightObject(
		DirectX::XMFLOAT3 color,
		DirectX::XMFLOAT3 direction,
		DirectX::XMFLOAT3 position,
		float intensity,
		float lightType
	);
	LightObject(Light newLight);
	~LightObject();

	Light GetLight();
	void SetLight(Light newLight);

	void SetColor(DirectX::XMFLOAT3 color);
	void SetIntensity(float intensity);
	void SetDirection(DirectX::XMFLOAT3 direction);
	void SetLightType(float lightType);
	void SetPosition(DirectX::XMFLOAT3 position);

private:
	Light light;
};

