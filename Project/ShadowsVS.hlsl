
#include "ShaderStructs.hlsli"

//Cbuffer where the data will be passed into from C++ side.
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
}

VertexToPixel_Shadow main(VertexShaderInput input)
{
	VertexToPixel_Shadow output;
	
	//Calculate output position for our shadow
	//                             3            2      1
	matrix worldViewProj = mul(projection, mul(view, world));
	output.screenPosition = mul(worldViewProj, float4(input.position, 1.0f));
	return output;
}