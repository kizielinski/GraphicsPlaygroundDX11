//Kyle Zielinski 
//11-2-2021
//PixelShaderFinal.hlsl file which handles the final return value for the pixels.
#include "ShaderStructs.hlsli"

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv       : TEXCOORD0;
};

//Defines texture variable for all texture resources
Texture2D finalTextureColor : register(t0); // <- T for textures.
Texture2D finalTextureAmbient : register(t1); // <- T for textures.

SamplerState basicSampler : register(s0); // <- S for sampler register

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 color = finalTextureColor.Sample(basicSampler, input.uv).rgb;
	float3 colorAmbient = finalTextureAmbient.Sample(basicSampler, input.uv).rgb;
	
	return float4(pow(colorAmbient + color, 1.0f / 2.2f), 1);
}