#include "ShaderStructs.hlsli"

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	matrix world;
	matrix view;
	matrix projection;
}

VertexToPixelNormalSupport main( VertexShaderInput input )
{
	//Normailize the normal field of input.
	input.normal = normalize(input.normal);

	// Set up output struct
	VertexToPixelNormalSupport output;

	matrix wvp = mul(projection, mul(view, world));

	output.position = mul(wvp, float4(input.position, 1.0f));

	
	//Apply world matrix rotation to the normal to ensure proper rotation
	//Do this by casting to a 3x3 matrix! Cuts off the 4th column/row
	output.normal = normalize(mul((float3x3)world, input.normal));
	
	//Apply same ^ to tangent
	output.tangent = normalize(mul((float3x3)world, input.tangent));

	//Get world position of pixel
	output.worldPos = mul(world, float4(input.position, 1.0f)).xyz;

	// Pass the color through 
	// - The values will be interpolated per-pixel by the rasterizer
	// - We don't need to alter it here, but we do need to send it to the pixel shader
	output.color = colorTint;

	//Pass uv and normals in
	output.uv = input.uv;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}