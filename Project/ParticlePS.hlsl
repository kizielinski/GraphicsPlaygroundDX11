
struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD0;
	float  particleType : PSIZE; //Just to pass particle type over as a float
	float  age : PSIZE;//Just to pass age.
};


Texture2D Texture         : register(t0);
SamplerState BasicSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{

	float3 outputColor = float3(0, 0, 0);

	switch (input.particleType)
	{
	case 0:
		outputColor += float3(Texture.Sample(BasicSampler, input.uv).r, 0, 0);
		break;
	case 1:
		outputColor += float3(Texture.Sample(BasicSampler, input.uv).r, 0, Texture.Sample(BasicSampler, input.uv).b);
		break;
	case 2:
		outputColor += float3(0, Texture.Sample(BasicSampler, input.uv).g, 0);
		break;
	case 3:
		outputColor += float3(0, 0, Texture.Sample(BasicSampler, input.uv).g);
		break;
	case 4:
		outputColor += float3(Texture.Sample(BasicSampler, input.uv).rb * input.age, 0);
		break;
	}


	return float4(outputColor, 1);
}