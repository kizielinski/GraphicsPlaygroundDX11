#pragma once

#include <d3d11.h>
#include <stdint.h>
#include <wrl/client.h>
#include <iostream>
#include <cmath>

struct Primes {
	const unsigned int A = 0b10011110001101110111100110110001;
	const unsigned int B = 0b10000101111010111100101001110111;
	const unsigned int C = 0b11000010101100101010111000111101;
	const unsigned int D = 0b00100111110101001110101100101111;
	const unsigned int E = 0b00010110010101100110011110110001;
};

class HashNoise
{
public:
	HashNoise(Microsoft::WRL::ComPtr<ID3D11Device> dev, INT32 _seed);
	~HashNoise();

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GenerateHashNoiseImage();

private:
	Primes primes;
	INT32 seed;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	uint32_t accumulator;
	uint32_t avalanche;
	uint32_t* hashes;
	uint32_t resolution = 50;
	float invResolution = 1 / resolution;
	uint32_t length;
	void HashEat(int data);
	void ExecuteNoise(int iteration);
	float ReturnDecimal(float value);
	static uint32_t RotateLeft(uint32_t data, int steps);
};

