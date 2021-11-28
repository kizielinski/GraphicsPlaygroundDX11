#include "ShaderStructs.hlsli"
cbuffer perObject : register(b2)
{
	// These realistically change once per frame, but we're
	// setting here to make the renderer implementation easier
	matrix viewMatrix;
	matrix projMatrix;
	float2 screenSize;
	int useRefractionSilhouette;
	int refractionFromNormalMap;
	float3 camPos;
	float indexOfRefraction;
	float refractionScale;
};


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 screenPosition	: SV_POSITION;
	float4 color			: COLOR;
	float2 uv				: TEXCOORD;
	float3 normal			: NORMAL;
	float3 worldPos			: POSITION; // The world position of this PIXEL
	float3 tangent			: TANGENT;
};


// Texture-related variables
Texture2D NormalTexture			: register(t0);
Texture2D RoughnessTexture		: register(t1);

// Environment map for reflections
TextureCube EnvironmentMap		: register(t2);

// Refraction requirement
Texture2D ScreenPixels			: register(t3);
Texture2D RefractionSilhouette	: register(t4);

// Samplers
SamplerState BasicSampler		: register(s0);
SamplerState ClampSampler		: register(s1);

// Fresnel term - Schlick approx.
float SimpleFresnel(float3 n, float3 v, float f0)
{
	// Pre-calculations
	float NdotV = saturate(dot(n, v));

	// Final value
	return f0 + (1 - f0) * pow(1 - NdotV, 5);
}

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	// Always re-normalize interpolated direction vectors
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	input.normal = ComputeNormalMap(NormalTexture, BasicSampler, input.uv, input.normal, input.tangent);

	// Calculate requisite reflection vectors
	float3 viewToCam = normalize(camPos - input.worldPos);
	float3 viewRefl = normalize(reflect(-viewToCam, input.normal));

	// The actual screen UV and refraction offset UV
	float2 screenUV = input.screenPosition.xy / screenSize;
	float2 offsetUV = float2(0, 0);

	// Which kind of refraction?
	if (refractionFromNormalMap)
	{
		offsetUV = NormalTexture.Sample(BasicSampler, input.uv).xy * 2 - 1;
		offsetUV.y *= -1; // UV's are upside down compared to world space
	}
	else
	{

		// Calculate the refraction amount in WORLD SPACE
		float3 refrDir = refract(viewToCam, input.normal, indexOfRefraction);

		// Get the refraction XY direction in VIEW SPACE (relative to the camera)
		// We use this as a UV offset when sampling the texture
		offsetUV = mul(viewMatrix, float4(refrDir, 0.0f)).xy;
		offsetUV.x *= -1.0f; // Flip the X to point away from the edge (Y already does this due to view space <-> texture space diff)
	}

	float2 refractedUV = screenUV + offsetUV * refractionScale;

	// Get the depth at the offset and verify its valid
	float silhouette = RefractionSilhouette.Sample(ClampSampler, refractedUV).r;
	if (useRefractionSilhouette && silhouette < 1)
	{
		// Invalid spot for the offset so default to THIS pixel's UV for the "refraction"
		refractedUV = screenUV;
	}

	// Get the color at the (now verified) offset UV
	float3 sceneColor = pow(ScreenPixels.Sample(ClampSampler, refractedUV).rgb, 2.2f); // Un-gamma correct

	// Get reflections
	float3 envSample = EnvironmentMap.Sample(BasicSampler, viewRefl).rgb;

	// Determine the reflectivity based on viewing angle
	// using the Schlick approximation of the Fresnel term
	float fresnel = SimpleFresnel(input.normal, viewToCam, F0_NON_METAL);
	return float4(pow(lerp(sceneColor, envSample, fresnel), 1.0f / 2.2f), 1); // Re-gamma correct after linear interpolation
}