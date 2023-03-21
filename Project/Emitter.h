#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include "SimpleShader.h"
#include "Camera.h"
#include "DXCore.h"
#include "DataStruct.h"

//Alignment matters here
struct Particle {
	float EmitTime;
	DirectX::XMFLOAT3 StartPos;
	float ParticleType;
	DirectX::XMFLOAT3 Accel;
	float Variable;
	DirectX::XMFLOAT3 Velocity;
};

class Emitter
{

public:

	Emitter(
		int maxParticles,
		int particlesPerSecond,
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
	);
	~Emitter();

	void Update(float dt, float currentTime);
	void Draw(Camera* camera, float currentTime);
	//etc

	EmitterData ReturnData();
	
private:

	//Our particles
	int maxParticles;
	unsigned int* indices;
	Particle* particles;
	float particleType;
	DirectX::XMFLOAT3 posOffset;
	DirectX::XMFLOAT3 acceleration;
	DirectX::XMFLOAT3 velocity;

	//Emission trackers
	int emissionRate;
	float emissionDelay;
	float timeBetweenParticleEmissions;

	//Track life of particles
	float lifetime;
	int firstDeadParticleIndex, firstLivingParticleIndex, particlesAliveCount;
	
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleSRV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;

	SimpleVertexShader* vs;
	SimplePixelShader* ps;
	
	void UpdateSingleParticle(float currentTime, int indexToUpdate);
	void EmitParticle(float currentTime);
};