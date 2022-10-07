//Kyle Zielinski 
//3-15-2021
//PixelShader.hlsl file which handles how the lighting will be rendered.
#include "ShaderStructs.hlsli"

#define MAX_LIGHTS 64

cbuffer ExternalData : register(b0)
{
	Light light;
	Light lightList[MAX_LIGHTS];
	int lightCount;
	float3 ambientColor;
	float specularIntensity;
	float3 camPosition;
	int SpecIBLTotalMipLevels;
}

struct PS_Output
{ 
	float4 colorNoAmbient 	 : SV_TARGET0;
	float4 ambientColor 	 : SV_TARGET1;
	float4 normals			 : SV_TARGET2;
	float4 depths			 : SV_TARGET3;
};

//Defines texture variable for all texture resources
Texture2D albedoTexture : register(t0); // <- T for textures.
Texture2D normalMapTexture : register(t1); //<- 2nd texture for processing normal map
Texture2D roughMapTexture : register(t2); //<- 3rd texture for processing rough map
Texture2D metalMapTexture : register(t3); //<- 4th texture for processing metal map

//IBL Textures
Texture2D BrdfLookUpMap : register(t4);
TextureCube IrradianceIBLMap : register(t5);
TextureCube SpecularIBLMap : register(t6);

SamplerState basicSampler : register(s0); // <- S for sampler register
SamplerState clampSampler : register(s1);

PS_Output main(VertexToPixelMain input) : SV_TARGET
{
	//Normal/Tangent calculations
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	input.normal = ComputeNormalMap(normalMapTexture, basicSampler, input.uv, input.normal, input.tangent);

	float3 albedo = pow(albedoTexture.Sample(basicSampler, input.uv).rgb, 2.2f);
	float roughness = roughMapTexture.Sample(basicSampler, input.uv).r;
	float metalness = metalMapTexture.Sample(basicSampler, input.uv).r;
	//float metalness = 1;
	//float roughness = 0;
	//Specular color determination
	float3 specularColor = lerp(F0_NON_METAL.rrr, albedo.rgb, metalness);

	//Test white light
	float3 whiteLight = FinalValueCalculation(
		input.normal, 
		input.worldPos, 
		camPosition, 
		light, 
		albedo, 
		specularIntensity, 
		roughness, 
		metalness, 
		specularColor);

	float3 finalLight = float3(0, 0, 0);
	//finalLight = whiteLight;
	finalLight = whiteLight;
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		finalLight += FinalValueCalculation(
			input.normal,
			input.worldPos,
			camPosition,
			lightList[i],
			albedo,
			specularIntensity,
			roughness,
			metalness,
			specularColor);

		/*finalLight.x = finalLight.x / 3;
		finalLight.y = finalLight.y / 3;
		finalLight.z = finalLight.z / 3;*/
	}
	
	//!!!!!!!!!!!!!!!!!!! IBL Calculations !!!!!!!!!!!!!!!!!!!!
	
	//Reflection vector calculation
	float3 viewToCam = normalize(camPosition - input.worldPos);
	float3 viewRefl = normalize(reflect(-viewToCam, input.normal));
	float NdotV = saturate(dot(input.normal, viewToCam));

	//Indirect Lighting
	float3 indirectDiffuse = IndirectDiffuse(IrradianceIBLMap, basicSampler, input.normal);
	float3 indirectSpecular = IndirectSpecular(
		SpecularIBLMap,
		SpecIBLTotalMipLevels,
		BrdfLookUpMap,
		clampSampler,
		viewRefl,
		NdotV,
		roughness,
		specularColor);

	float3 balancedDiff = DiffuseEnergyConserve(indirectDiffuse, indirectSpecular, metalness);
	float3 fullIndirect = indirectSpecular + balancedDiff * albedo.rgb;

	//! !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	//finalLight += fullIndirect;
	//Allows for debugging of specific textures
	//finalLight = albedoTexture.Sample(basicSampler, input.uv).rgb;
	//finalLight = normalMapTexture.Sample(basicSampler, input.uv).rgb;
	//finalLight = roughMapTexture.Sample(basicSampler, input.uv).rgb;
	//finalLight = metalMapTexture.Sample(basicSampler, input.uv).rgb;

	//Take everything and insert it into the output struct
	//float4(pow(finalLight, 1.0f / 2.2f), 1);
	PS_Output output; 
	output.colorNoAmbient = float4(finalLight, 1);
	output.ambientColor = float4(fullIndirect, 1);
	output.normals = float4(input.normal * 0.5f + 0.5f, 1);
	output.depths = input.position.z;

	//Gamma Correction
	//return float4(pow(finalLight, 1.0f / 2.2f), 1);
	return output;
}