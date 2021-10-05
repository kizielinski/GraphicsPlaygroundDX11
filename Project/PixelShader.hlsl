//Kyle Zielinski 
//3-15-2021
//PixelShader.hlsl file which handles how the lighting will be rendered.
#include "ShaderStructs.hlsli"

#define MAX_LIGHTS 50

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

float4 main(VertexToPixelMain input) : SV_TARGET
{
	//float3 unpackedNormal = normalMapTexture.Sample(basicSampler, input.uv).rgb * 2 - 1;
	//float3 Normal = normalize(input.normal); //Should be normalized, possible error by NMVS
	//float3 Tangent = normalize(input.tangent); //Should be normalized, possible error by NMVS
	//Better than the above ^
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	input.normal = ComputeNormalMap(normalMapTexture, basicSampler, input.uv, input.normal, input.tangent);

	float3 albedo = pow(albedoTexture.Sample(basicSampler, input.uv).rgb, 2.2f);
	float roughness = roughMapTexture.Sample(basicSampler, input.uv).r;
	float metalness = metalMapTexture.Sample(basicSampler, input.uv).r;
	//float metalness = 0;
	//float roughness = 1;
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

	finalLight += whiteLight;
	/*for (int i = 0; i < 1; i++)
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
	}*/
	
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
	
	finalLight += fullIndirect;
	//finalLight = albedoTexture.Sample(basicSampler, input.uv).rgb;
	//finalLight = normalMapTexture.Sample(basicSampler, input.uv).rgb;
	//finalLight = roughMapTexture.Sample(basicSampler, input.uv).rgb;
	//finalLight = metalMapTexture.Sample(basicSampler, input.uv).rgb;
	//finalLight = input.normal;
	//finalLight = BrdfLookUpMap.Sample(basicSampler, input.uv);

	//Gamma Correction
	return float4(pow(finalLight, 1.0f / 2.2f), 1);
}