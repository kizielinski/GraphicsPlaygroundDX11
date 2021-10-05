#pragma once
#include "Camera.h"
#include "Mesh.h"
#include "SimpleShader.h"
#include <wrl/client.h>

class SkyMap
{
public:
	SkyMap();
	SkyMap(
		Mesh* _mesh, 
		Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerOptions, 
		Microsoft::WRL::ComPtr<ID3D11Device> _device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _mapTexture, 
		SimplePixelShader* _pSS, 
		SimpleVertexShader* _vSS,
		SimpleVertexShader* fullscreenVS,
		SimplePixelShader* irradiancePS,
		SimplePixelShader* specularConPS,
		SimplePixelShader* lookUpTexturePS);
	~SkyMap();
	SimplePixelShader* GetPixelShader();
	SimpleVertexShader* GetVertexShader();
	void SkyDraw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> device, Camera* cam);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ReturnIrradianceCubeMap();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ReturnConvolvedSpecularCubeMap();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ReturnLookUpTexture();
	int ReturnCalculatedMipLevels();

	void RefreshSkyMap(
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> newMap,
		SimpleVertexShader* fullscreenVS,
		SimplePixelShader* irradiancePS,
		SimplePixelShader* specularConPS,
		SimplePixelShader* lookUpTexturePS);


private:

	//Default Skymap fields
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthBufferType;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> raterizerOption;

	Mesh* skyMesh;
	SimplePixelShader* pixelSkyShader;
	SimpleVertexShader* vertexSkyShader;

	//IBL enabled Skymap fields/methods
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> irraIBLCubeMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> conSpecIBLCubeMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brdfLookUpTexture;

	int calculatedMipLevels;
	const int mipToSkip = 3;
	const int cubeMapFaceSize = 512;
	const int lookUpTextureSize = 512;

	void IBLCreateIrradianceMap(SimpleVertexShader* fullscreenVS, SimplePixelShader* irradiancePS);
	void IBLCreateConvolvedSpecularMap(SimpleVertexShader* fullscreenVS, SimplePixelShader* specularConPS);
	void IBLCreateBRDFLookUpTexture(SimpleVertexShader* fullscreenVS, SimplePixelShader* lookUpTexturePS);
};