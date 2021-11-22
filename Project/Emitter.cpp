#include "Emitter.h"

using namespace DirectX;

Emitter::Emitter(
	int maxParticles,
	int emissionRate,
	float lifetime,
	SimpleVertexShader* _vs,
	SimplePixelShader* _ps,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture,
	DirectX::XMFLOAT3 posOffset,
	DirectX::XMFLOAT3 accel,
	DirectX::XMFLOAT3 velocity,
	float particleType
) : //Use assignment syntax
	maxParticles(maxParticles),
	emissionRate(emissionRate), //Particles per second
	lifetime(lifetime),
	vs(_vs),
	ps(_ps),
	context(context),
	texture(texture),
	posOffset(posOffset),
	acceleration(accel),
	velocity(velocity),
	particleType(particleType)
{
	//Basic Data initialization
	emissionDelay = 1.0f / emissionRate;
	timeBetweenParticleEmissions = 0.0f;
	particlesAliveCount = 0;
	firstDeadParticleIndex = 0;
	firstLivingParticleIndex = 0;

	particles = new Particle[maxParticles];
	//ZeroMemory(particles, sizeof(Particle) * maxParticle); //<-This sets the size of the memory to our max number of particles

	//Indices are static so we can just assign them all ahead of time 
	//within a buffer and have them ready to go!
	unsigned int* indices = new unsigned int[(int)maxParticles* 6];
	int indexCount = 0;
	for (int i = 0; i < maxParticles * 4; i += 4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	//Make our static index buffer
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * maxParticles * 6;
	device->CreateBuffer(&ibDesc, &indexData, indexBuffer.GetAddressOf());

	// Make a dynamic buffer to hold all particle data on GPU
	// Note: We'll be overwriting this every frame with new lifetime data
	D3D11_BUFFER_DESC allParticleBufferDesc = {};
	allParticleBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	allParticleBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	allParticleBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	allParticleBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	allParticleBufferDesc.StructureByteStride = sizeof(Particle);
	allParticleBufferDesc.ByteWidth = sizeof(Particle) * maxParticles;
	device->CreateBuffer(&allParticleBufferDesc, 0, particleBuffer.GetAddressOf());

	// Create an SRV that points to a structured buffer of particles
	// so we can grab this data in a vertex shader
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = maxParticles;
	device->CreateShaderResourceView(particleBuffer.Get(), &srvDesc, particleSRV.GetAddressOf());
}

Emitter::~Emitter()
{
	delete[] particles;
}

void Emitter::Update(float dt, float currentTime)
{
	if (particlesAliveCount > 0)
	{
		if (firstLivingParticleIndex < firstDeadParticleIndex) //Determine if we have to wrap aka: 0***0000 and not *00000**
		{
			for (int i = firstLivingParticleIndex; i < firstDeadParticleIndex; i++)
			{
				UpdateSingleParticle(currentTime, i);
			}
		}
		else if (firstDeadParticleIndex < firstLivingParticleIndex) //Determine if wrap occurs aka: *00000** and not 0***0000
		{
			for (int i = firstLivingParticleIndex; i < maxParticles; i++)
			{
				UpdateSingleParticle(currentTime, i);
			}

			for (int i = 0; i < firstDeadParticleIndex; i++)
			{
				UpdateSingleParticle(currentTime, i);
			}
		}
		else //FirstDead and FirstAlive are equal
		{
			for (int i = 0; i < maxParticles; i++)
			{
				UpdateSingleParticle(currentTime, i);
			}
		}
	}

#pragma region PostUpdateParticle
	timeBetweenParticleEmissions += dt;

	while (timeBetweenParticleEmissions > emissionDelay)
	{
		EmitParticle(currentTime);
		timeBetweenParticleEmissions -= emissionDelay;
	}

	//Handle buffer mapping
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(particleBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	if (firstLivingParticleIndex < firstDeadParticleIndex)
	{
		memcpy(
			mapped.pData,
			particles + firstLivingParticleIndex,
			sizeof(Particle) * particlesAliveCount);
	}
	else 
	{
		memcpy(
			mapped.pData,
			particles,
			sizeof(Particle) * firstDeadParticleIndex);

		int temp = maxParticles - firstLivingParticleIndex;
		memcpy(
			(void*)((Particle*)mapped.pData + firstDeadParticleIndex),
			particles + firstLivingParticleIndex,
			sizeof(Particle) * (temp));
	}
	//memcpy(mapped.pData, particles, sizeof(Particle) * maxParticles);
	context->Unmap(particleBuffer.Get(), 0);
#pragma endregion

}

//Setup buffers, shaders, and external data to draw our particles.
void Emitter::Draw(Camera* camera, float currentTime)
{
	//Setup our buffers
	//Not using a vertex buffer, because we construct vertex data on the fly in the shader.
	UINT stride = 0;
	UINT offset = 0;
	ID3D11Buffer* nullBuffer = 0;
	context->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//Assign shaders
	vs->SetShader();
	ps->SetShader();

	//Input data
	vs->SetShaderResourceView("ParticleData", particleSRV);
	ps->SetShaderResourceView("Texture", texture);

	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vs->SetFloat("currentTime", currentTime);
	vs->SetFloat3("accel", acceleration);
	vs->CopyAllBufferData();

	//Particle = 4 vertices = 6 indices;
	context->DrawIndexed(particlesAliveCount * 6, 0, 0);
}

void Emitter::UpdateSingleParticle(float currentTime, int indexToUpdate)
{
	float age = currentTime - particles[indexToUpdate].EmitTime;

	if (age >= lifetime)
	{
		//Particle has deceased, remove it.
		firstLivingParticleIndex++;
		firstLivingParticleIndex %= maxParticles;
		particlesAliveCount--;
	}
}

void Emitter::EmitParticle(float currentTime)
{
	if(particlesAliveCount == maxParticles)
	{ return; }

	int activateIndex = firstDeadParticleIndex;

	particles[activateIndex].EmitTime = currentTime;
	particles[activateIndex].StartPos = posOffset;
	particles[activateIndex].Accel = acceleration;
	particles[activateIndex].Velocity = velocity;
	particles[activateIndex].ParticleType = particleType;
	particles[activateIndex].Variable = 0;


	//Do more particle adjustment here!!!


	//End particle adjustment

	firstDeadParticleIndex++;
	firstDeadParticleIndex %= maxParticles; //handle the wrap

	particlesAliveCount++; //Increment alive counter
}