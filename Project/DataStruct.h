#pragma once

#include <DirectXMath.h>
struct GraphicData 
{
	std::string meshPath;
	std::wstring albedoPath;
	std::wstring normalPath;
	std::wstring roughPath;
	std::wstring metalPath;
};

struct EntityDef
{
	int index;
	bool transparency;
	bool shadows;
};

struct EntityPosition
{
	float X;
	float Y;
	float Z;
};

struct EmitterData
{
	DirectX::XMFLOAT3 posOffset;
	float particleType;
	DirectX::XMFLOAT3 acceleration;
	float emissionDelay;
	DirectX::XMFLOAT3 velocity;
	float timeBetweenParticles;
	int maxParticles;
	int emissionRate;
};