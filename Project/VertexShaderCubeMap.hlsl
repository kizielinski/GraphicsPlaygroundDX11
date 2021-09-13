#include "ShaderStructs.hlsli"

cbuffer ExternalData : register(b0)
{
	matrix view;
	matrix projection;
}

VertexToPixelSky main(VertexShaderInput input)
{
	VertexToPixelSky output;
	matrix viewCopy = view;
	
	//0-out the translation values
	viewCopy._14 = 0;
	viewCopy._24 = 0;
	viewCopy._34 = 0;
	
	//Calc the output position from this zero'd view matrix and projection matrix = viewProj
	viewCopy = mul(projection, viewCopy);

	//Set output position equal to product of the viewProj matrix and the intput position
	output.position = mul(viewCopy, float4(input.position, 1.0f));

	//Make sure output depth will be 1.0
	output.position.z = output.position.w;

	//Sample direction is input position, since it's offset from vertex origin
	output.sampleDir = input.position;

	return output;
}