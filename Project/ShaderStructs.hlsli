#include "PBRInclude.hlsli"

#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__


struct DirectionalLight
{
	float3 Color       : COLOR;
	float Intensity    : BLENDWEIGHT;
	float3 Direction   : DIR;
};

struct DirectionalLightInput
{
	float3 color          : COLOR;
	float intensity       : BLENDWEIGHT;
	float3 direction      : POSITION;
};

struct Light
{
	float3 color;
	float intensity;
	float3 direction;
	int lightType;
	float3 position;
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
struct VertexToPixel
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

struct VertexToPixelNormalSupport
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

float3 DirectionToLight(float3 direction)
{
	return normalize(-1 * direction);
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

float3 FinalValueCalculation(float3 normal, float3 worldPos, float3 camPos, Light tempLight, float3 surfaceColor, float specularIntensity, float roughness, float metalness, float3 specColor)
{
	float3 finalColor = { 255, 0, 0 };
	float3 spec;
	float3 diffuse;
	int lightType = tempLight.lightType;

	float diffPreBalance;
	float3 balancedDiff;

	switch (lightType)
	{
		//Directional Light
	case 0:
		//Diffuse calculation for our first light
		//diffuse = Diffuse(normal, DirectionToLight(tempLight.direction), tempLight);
		diffPreBalance = DiffusePBR(normal, tempLight.direction);
		/*float3 toLight = normalize(-tempLight.direction);
		float3 toCam = normalize(camPos - worldPos);
		diffPreBalance = DiffusePBR(normal, toLight);*/

		//Define specular light
		//spec = Specular(tempLight.direction, worldPos, camPos, normal, specularIntensity);
		spec = MicrofacetBRDF(normal, DirectionToLight(tempLight.direction), camPos, roughness, specColor);
		//spec = MicrofacetBRDF(normal, toLight, toCam, roughness, specColor);
		//spec *= any(diffPreBalance);

		balancedDiff = DiffuseEnergyConserve(diffPreBalance, spec, metalness);

		//Calculate final color
		//finalColor = (directionDiffuseAmount * surfaceColor * tempLight.color * tempLight.intensity) + (ambientColor * surfaceColor);
		//finalColor = surfaceColor * (diffuse + spec);
		
		//PBR implementation
		//finalColor = surfaceColor;
		//finalColor = (balancedDiff * surfaceColor + spec) * tempLight.intensity * tempLight.color;
		finalColor = (balancedDiff*surfaceColor+spec)*tempLight.intensity * tempLight.color;
		break;

		//Point Light
	case 1:
		//Calculate direction
		float3 pointLightDirection = tempLight.position - worldPos;

		//Diffuse calculation
		//diffuse = Diffuse(normal, DirectionToLight(pointLightDirection), tempLight);
		diffPreBalance = DiffusePBR(normal, DirectionToLight(pointLightDirection));

		//Change from Phong BRDF to Cook-Torrence BRDF
		//spec = Specular(DirectionToLight(pointLightDirection), worldPos, camPos, normal, specularIntensity);
		spec = MicrofacetBRDF(normal, DirectionToLight(pointLightDirection), ViewVector(camPos, worldPos), roughness, specColor);
		spec *= any(diffPreBalance);

		balancedDiff = DiffuseEnergyConserve(diffPreBalance, spec, metalness);

		//Calculate final color
		finalColor = (balancedDiff * surfaceColor + spec) * tempLight.intensity * tempLight.color;
		break;
	}

	return finalColor;
}

#endif