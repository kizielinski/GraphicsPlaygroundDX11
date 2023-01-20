//Kyle Zielinski
//Last Edited: 10/3/2022
//Header file to set up Mesh object
#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"

class Mesh
{
public:
	Mesh(Vertex* vertices, int numVertices, unsigned int* indices, int numIndices, Microsoft::WRL::ComPtr<ID3D11Device> device);
	
	Mesh(const char* objFile, Microsoft::WRL::ComPtr<ID3D11Device> device, bool useASSIMP = true);
	~Mesh();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer(); //Returns vBuff pointer
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer(); //Returns iBuff pointer
	
	int GetIndexCount(); //Returns count of indices in iBuff
	void InitializeBuffers(Vertex* vertices, int numVertices, unsigned int* indices, int numIndices, Microsoft::WRL::ComPtr<ID3D11Device> device, bool calcTangents);
	void DrawUsingBuffs(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

	void StandardMeshLoad(const char* objFile, Microsoft::WRL::ComPtr<ID3D11Device> device);
	void ASSIMPMeshLoad(const char* objFile, Microsoft::WRL::ComPtr<ID3D11Device> device);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vBuff;
	Microsoft::WRL::ComPtr<ID3D11Buffer> iBuff;
	int indexCount; //Num vertices in index buffer;
};