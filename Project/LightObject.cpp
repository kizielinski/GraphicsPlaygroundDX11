#include "LightObject.h"

LightObject::LightObject()
{
	light = Light();
}

LightObject::LightObject(DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 direction, DirectX::XMFLOAT3 position, float intensity, float lightType, float range)
{
	light.color = color;
	light.direction = direction;
	light.position = position;
	light.intensity = intensity;
	light.lightType = lightType;
	light.range = range;
}

LightObject::LightObject(Light newLight)
{
	light = newLight;
}

LightObject::~LightObject()
{
}

Light LightObject::GetLight()
{
	return light;
}

void LightObject::SetLight(Light newLight)
{
	light = newLight;
}

void LightObject::SetColor(DirectX::XMFLOAT3 color)
{
	light.color = color;
}

void LightObject::SetIntensity(float intensity)
{
	light.intensity = intensity;
}

void LightObject::SetDirection(DirectX::XMFLOAT3 direction)
{
	light.direction = direction;
}

void LightObject::SetLightType(float lightType)
{
	light.lightType = lightType;
}

void LightObject::SetPosition(DirectX::XMFLOAT3 position)
{
	light.position = position;
}

void LightObject::SetRange(float range)
{
	light.range = range;
}