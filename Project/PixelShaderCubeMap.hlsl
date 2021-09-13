
#include "ShaderStructs.hlsli"

TextureCube cubeMap : register(t0);
SamplerState basicSampler : register(s0);

float4 main(VertexToPixelSky input) : SV_TARGET
{
	return cubeMap.Sample(basicSampler, input.sampleDir);
}