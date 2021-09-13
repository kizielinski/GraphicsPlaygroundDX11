#include "ShaderStructs.hlsli"
cbuffer ExternalData : register(b0)
{
	Light light;
	Light upward;
	Light diagonal;
	Light grey;
	Light defaultLight;
	float3 ambientColor;
	float specularIntensity;
	float3 camPosition;
}

//Defines texture variable for all texture resources
Texture2D albedoTexture : register(t0); // <- T for textures.
Texture2D normalMapTexture : register(t1); //<- 2nd texture for processing normal map
Texture2D roughMapTexture : register(t2); //<- 3rd texture for processing rough map
Texture2D metalMapTexture : register(t3); //<- 4th texture for processing metal map
SamplerState basicSampler : register(s0); // <- S for sampler register

float4 main(VertexToPixelNormalSupport input) : SV_TARGET
{
	//Texture and normalMap
	float3 albedo = pow(albedoTexture.Sample(basicSampler, input.uv).rgb, 2.2f);

	float roughness = roughMapTexture.Sample(basicSampler, input.uv).r;
	float metalness = metalMapTexture.Sample(basicSampler, input.uv).r;

	//Specular color determination
	float3 specularColor = lerp(F0_NON_METAL.rrr, albedo.rgb, metalness);

	float3 unpackedNormal = normalMapTexture.Sample(basicSampler, input.uv).rgb * 2 - 1;

	float3 Normal = normalize(input.normal); //Should be normalized, possible error by NMVS
	float3 Tangent = normalize(input.tangent); //Should be normalized, possible error by NMVS

	//Gram-Schmidt orthogonalization for the tangent value
	Tangent = normalize(Tangent - Normal * dot(Tangent, Normal));

	//Create the bitangent value
	float3 BiTangent = cross(Tangent, Normal);

	//Create the final matrix
	float3x3 TBNMatrix = float3x3(Tangent, BiTangent, Normal);

	//Apply changes to normal
	input.normal = mul(unpackedNormal, TBNMatrix);

	//Test white light
	float3 whiteLight = FinalValueCalculation(input.normal, input.worldPos, camPosition, defaultLight, input.color.rgb, specularIntensity, roughness, metalness, specularColor);;
	
	//Finalize color to output
	float3 finalLight = whiteLight;//(whiteLight + ambientColor) * albedo;

	return float4(pow(finalLight, 1.0f / 2.2f), 1);
}