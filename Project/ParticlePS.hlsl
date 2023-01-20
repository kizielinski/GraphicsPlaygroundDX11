
struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD0;
	float  particleType : PSIZE; //Just to pass particle type over as a float
	float  age : PSIZE; //Just to pass age.
};

Texture2D Texture         : register(t0);
SamplerState BasicSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	//Sample only once here
	//If we were to sample by age, flickering occurs which isn't optimal for 
	//our particle system.
	return Texture.Sample(BasicSampler, input.uv);
}