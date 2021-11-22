struct Particle
{
	float EmitTime;
	float3 StartPos;
	float ParticleType;
	float3 Accel;
	float Variable;
	float3 Velocity;
};

cbuffer externalData :register (b0)
{
	matrix view;
	matrix projection;
	float currentTime;
};

StructuredBuffer<Particle> ParticleData : register(t0);

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD0;
	float  particleType : PSIZE; //Just to pass particle type over as a float
	float age           : PSIZE; //Just to pass age to PS
};

VertexToPixel main(uint id : SV_VertexID)
{
	VertexToPixel output;

	uint particleID = id / 4; //Every 4 is 1 particle
	uint cornerID = id % 4; //Calculated corner of the particle's quad

	//Load particle from struct bufffer and calculate age
	Particle p = ParticleData.Load(particleID); //Vertex gets associated particle.
	output.particleType = p.ParticleType;
	float3 pos = p.StartPos;
	float age = currentTime - p.EmitTime;

	//Calculate other particle data here (switch statement?)

	

	switch (p.ParticleType)
	{
	case 0:
		pos = pos + age * float3(0, 2.0f, 0);
		break;
	case 1:
		pos = p.Accel * age * age / 2.0f + p.Velocity * age + p.StartPos;
		break;
	case 2:
		pos = (p.Accel + p.Velocity) * age + pos;
		pos.x = (age* 10) % 3;
		break;
	case 3:
		pos.z = pos.z + p.Accel.z;
		pos.y = (age * -2) % 3;
		pos.x = pos + p.Velocity * age;
		break;
	case 4:
		pos.x = sin(age);
		break;
	}

	//pos = accel * age * age / 2.0f + p.StartVelocity * age + p.StartPos;

	float2 offsets[4];
	offsets[0] = float2(-1.0f, +1.0f); //Top left
	offsets[1] = float2(+1.0f, +1.0f); //Top right
	offsets[2] = float2(+1.0f, -1.0f); //Back right
	offsets[3] = float2(-1.0f, -1.0f); //Back left

	pos += float3(view._11, view._12, view._13) * offsets[cornerID].x; // RIGHT
	pos += float3(view._21, view._22, view._23) * offsets[cornerID].y; // UP

	//Complete the output position by calculating the matrix.
	matrix viewProjection = mul(projection, view);
	output.position = mul(viewProjection, float4(pos, 1.0f));

	//Assign UVs
	float2 uvs[4];
	uvs[0] = float2(0, 0); //Top left
	uvs[1] = float2(1, 0); //Top right
	uvs[2] = float2(1, 1); //Back right
	uvs[3] = float2(0, 1); //Back left
	output.uv = uvs[cornerID];
	output.age = age;

	return output;
}