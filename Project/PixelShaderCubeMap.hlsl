#include "ShaderStructs.hlsli"

TextureCube cubeMap : register(t0);
SamplerState basicSampler : register(s0);

float4 main(VertexToPixelSky input) : SV_TARGET
{
	return float4(pow(cubeMap.Sample(basicSampler, input.sampleDir).rgb, 8.8f/ 1.0f), 1);
}