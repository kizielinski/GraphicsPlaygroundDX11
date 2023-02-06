#include "PBRInclude.hlsli"

#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__

#define LIGHT_DIRECTIONAL	0
#define LIGHT_POINT		    1
#define LIGHT_SPOT			2

struct Light
{
	float3 color;
	float intensity;
	float3 direction;
	int lightType;
	float3 position;
	float range;
};

struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 position		: POSITION;     // XYZ position
	float2 uv		    : TEXCOORD;     // RG uv coordinate
	float3 normal       : NORMAL;       // normal coordinate
	float3 tangent      : TANGENT;
};

// Struct representing the data we're sending down the pipeline
struct VertexToPixelMain
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float4 color		: COLOR;
	float2 uv		    : TEXCOORD;     // RG uv coordinate
	float3 normal       : NORMAL;       // Normal coordinate
	float3 worldPos     : POSITION;
	float3 tangent      : TANGENT;
};

struct VertexToPixelSky
{
	//type   Name        Semantic
	float4 position  : SV_POSITION;
	float3 sampleDir : DIRECTION;
};

// VStoPS struct for shadow map creation
struct VertexToPixel_Shadow
{
	float4 screenPosition	: SV_POSITION;
};

float3 RawNormalMapData(Texture2D map, SamplerState samp, float2 uv)
{
	return map.Sample(samp, uv).rgb * 2.0f - 1.0f;
}

float3 ComputeNormalMap(Texture2D map, SamplerState samp, float2 uv, float3 normal, float3 tangent)
{
	//Sample and unpack the normal
	float3 normalFromRawData = RawNormalMapData(map, samp, uv);

	float3 Normal = normal;
	//Gram-Schmidt orthogonalization for the tangent value
	float3 Tangent = normalize(tangent - Normal * dot(tangent, Normal));
	float3 BiTangent = cross(Tangent, Normal); //Create the bitangent value

	//Create the final matrix
	float3x3 TBNMatrix = float3x3(Tangent, BiTangent, Normal);

	//Apply changes to normal, and do some more normal things within this totally normal function ^_^
	return normalize(mul(normalFromRawData, TBNMatrix));
}

//Diffuse total Light calculation
float3 Diffuse(float3 normal, float3 dirToLight, Light tempLight)
{
	//N dot L calculation
	float result = dot(normal, dirToLight);

	//Saturate
	float3 directionDiffuse = saturate(result); //i.e. the direction diffuse amount

	//Multiply direction diffuse amount by appropriate color values
	float3 finalDiffuse = directionDiffuse * tempLight.color * tempLight.intensity;
	return finalDiffuse;
}

//Calculate Specular value
float Specular(float3 direction, float3 worldPos, float3 camPos, float3 normal, float specExponent)
{
	//Get view direction
	float3 view = normalize(camPos - worldPos);

	//Normalize light direction
	float3 incomingLightDirection = normalize(direction);

	//Calc reflection
	float3 reflection = reflect(incomingLightDirection, normal);

	return pow(saturate(dot(reflection, view)), specExponent);
}

float3 ViewVector(float3 camPos, float3 worldPos)
{
	float3 view = normalize(camPos - worldPos);
	return view;
}

//Range function for light fades
float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.position, worldPos);

	//Ranged-based attentuation
	float attenuation = saturate(1.0f - (dist * dist / (light.range * light.range)));

	//Softer value
	return attenuation * attenuation;
}

float3 DirectionalLightPBR(float3 normal, float3 worldPos, float3 camPos, Light tempLight, float3 surfaceColor, float roughness, float metalness, float3 specColor)
{
	//Calc Direction Vectors
	float3 toLight = normalize(-tempLight.direction);
	float3 toCam = normalize(camPos - worldPos);

	//Diffuse calculation for our first light
	float diffPreBalance = DiffusePBR(normal, toLight);

	float3 spec = MicrofacetBRDF(normal, toLight, toCam, roughness, specColor);
	spec *= any(diffPreBalance);

	float3 balancedDiff = DiffuseEnergyConserve(diffPreBalance, spec, metalness);

	float3 finalColor = { 0, 0, 0 };
	finalColor = (balancedDiff * surfaceColor + spec) * tempLight.intensity * tempLight.color;

	return finalColor;
}

float3 PointLightPBR(float3 normal, float3 worldPos, float3 camPos, Light tempLight, float3 surfaceColor, float roughness, float metalness, float3 specColor)
{
	//Calculate direction vectors
	float3 toPointLight = normalize(tempLight.position - worldPos);
	float3 toPointCam = normalize(camPos - worldPos);

	float attenuation = Attenuate(tempLight, worldPos);

	float diffPreBalance = DiffusePBR(normal, toPointLight);

	//Change from Phong BRDF to Cook-Torrence BRDF
	float3 spec = MicrofacetBRDF(normal, toPointLight, toPointCam, roughness, specColor);
	spec *= any(diffPreBalance);

	float3 balancedDiff = DiffuseEnergyConserve(diffPreBalance, spec, metalness);

	//Calculate final color
	float3 finalColor = { 0, 0, 0 };

	finalColor = (balancedDiff * surfaceColor + spec) * attenuation * tempLight.intensity * tempLight.color;
	//finalColor = float3(0.8, 0.3, 0.1);

	return finalColor;
}

float3 SpotLightPBR(float3 normal, float3 worldPos, float3 camPos, Light tempLight, float3 surfaceColor, float roughness, float metalness, float3 specColor)
{

	float3 toLight = normalize(tempLight.position - worldPos);
	float3 toPointCam = normalize(camPos - worldPos);

	float attenuation = Attenuate(tempLight, worldPos);

	float diffPreBalance = DiffusePBR(normal, toLight);

	//Change from Phong BRDF to Cook-Torrence BRDF
	float3 spec = MicrofacetBRDF(normal, toLight, toPointCam, roughness, specColor);
	spec *= any(diffPreBalance);

	float3 balancedDiff = DiffuseEnergyConserve(diffPreBalance, spec, metalness);

	//Calculate final color
	float3 finalColor = { 255, 1, 1 };
	finalColor = (balancedDiff * surfaceColor + spec) * attenuation * tempLight.intensity * tempLight.color;

	//Calculation for the spotlight falloff
	float penumbra = pow(saturate(dot(-toLight, tempLight.direction)), tempLight.range);

	//Combine with point light calculation and return
	return finalColor * penumbra;
}

//Function to calculate our desired light values in the scene
float3 AddLightCalculation(float3 normal, float3 worldPos, float3 camPos, Light tempLight, float3 surfaceColor, float specularIntensity, float roughness, float metalness, float3 specColor)
{

	float3 finalColor = { 0, 0, 0 };
	float3 spec;
	float3 diffuse;
	float attenuation;
	int lightType = tempLight.lightType;

	float diffPreBalance;
	float3 balancedDiff;

	switch (lightType)
	{
		//Directional Light
	case LIGHT_DIRECTIONAL:
		finalColor = DirectionalLightPBR(normal, worldPos, camPos, tempLight, surfaceColor, roughness, metalness, specColor);
		break;

		//Point Light
	case LIGHT_POINT:
		finalColor = PointLightPBR(normal, worldPos, camPos, tempLight, surfaceColor, roughness, metalness, specColor);

		break;

	case LIGHT_SPOT:
		finalColor = SpotLightPBR(normal, worldPos, camPos, tempLight, surfaceColor, roughness, metalness, specColor);
		break;
	}

	return finalColor;
}
#endif