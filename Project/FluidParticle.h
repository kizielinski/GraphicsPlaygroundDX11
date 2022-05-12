#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

#pragma once
class FluidParticle
{
	DirectX::XMFLOAT2 velocity;
	DirectX::XMFLOAT2 scalarPressure;
	DirectX::XMFLOAT2 cellLocation;
};

