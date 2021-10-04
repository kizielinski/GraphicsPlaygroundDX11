#include "SkyMap.h"

//Default constructor
SkyMap::SkyMap()
{
	skyMesh = nullptr;
	samplerOptions = nullptr;
	cubeSRV = nullptr;
	depthBufferType = nullptr;
	raterizerOption = nullptr;
	
	pixelSkyShader = nullptr;
	vertexSkyShader = nullptr;
}

//Constructor that sets up all necessary values for a valid skyMap
SkyMap::SkyMap(
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
	SimplePixelShader* lookUpTexturePS) : device(_device), context(_context)
{
	_samplerOptions.CopyTo(samplerOptions.GetAddressOf());
	_mapTexture.CopyTo(cubeSRV.GetAddressOf());
	skyMesh = _mesh;
	//Intialize samplerState
	D3D11_RASTERIZER_DESC rasterizerDescription = {};
	rasterizerDescription.FillMode = D3D11_FILL_SOLID;
	rasterizerDescription.CullMode = D3D11_CULL_FRONT;
	_device->CreateRasterizerState(&rasterizerDescription, raterizerOption.GetAddressOf());

	//Initialize depthStencilState
	D3D11_DEPTH_STENCIL_DESC depthStencilDescription = {};
	depthStencilDescription.DepthEnable = true;
	depthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	_device->CreateDepthStencilState(&depthStencilDescription, depthBufferType.GetAddressOf());

	pixelSkyShader = _pSS;
	vertexSkyShader = _vSS;

	calculatedMipLevels = 0;

	IBLCreateIrradianceMap(fullscreenVS, irradiancePS);
	IBLCreateConvolvedSpecularMap(fullscreenVS, specularConPS);
	IBLCreateBRDFLookUpTexture(fullscreenVS, lookUpTexturePS);
}

SkyMap::~SkyMap()
{
	delete skyMesh;
	skyMesh = nullptr;
	////Might not need
	//delete pixelSkyShader;
	//delete vertexSkyShader;
	////!!!!!!
	/*pixelSkyShader = nullptr;
	vertexSkyShader = nullptr;*/
}

SimplePixelShader* SkyMap::GetPixelShader()
{
	return pixelSkyShader;
}

SimpleVertexShader* SkyMap::GetVertexShader()
{
	return vertexSkyShader;
}

void SkyMap::SkyDraw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* cam)
{
	context->RSSetState(raterizerOption.Get());
	context->OMSetDepthStencilState(depthBufferType.Get(), 0);

	GetPixelShader()->SetShader();
	GetVertexShader()->SetShader();
	
	pixelSkyShader->SetShaderResourceView("cubeMap", cubeSRV.Get());
	pixelSkyShader->SetSamplerState("basicSampler", samplerOptions.Get());

	vertexSkyShader->SetMatrix4x4("view", cam->GetViewMatrix());
	vertexSkyShader->SetMatrix4x4("projection", cam->GetProjectionMatrix());

	pixelSkyShader->CopyAllBufferData();
	vertexSkyShader->CopyAllBufferData();

	skyMesh->DrawUsingBuffs(context);
	
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SkyMap::ReturnIrradianceCubeMap()
{
	return irraIBLCubeMap;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SkyMap::ReturnConvolvedSpecularCubeMap()
{
	return conSpecIBLCubeMap;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SkyMap::ReturnLookUpTexture()
{
	return brdfLookUpTexture;
}

int SkyMap::ReturnCalculatedMipLevels()
{
	return 0;
}

void SkyMap::IBLCreateIrradianceMap(SimpleVertexShader* fullscreenVS, SimplePixelShader* irradiancePS)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> irraMapFinalTexture;

	//Setup the description for the final irradiance cube texture
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = cubeMapFaceSize;
	texDesc.Height = cubeMapFaceSize;
	texDesc.ArraySize = 6;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; //Applies both
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	texDesc.SampleDesc.Count = 1;
	device->CreateTexture2D(&texDesc, 0, irraMapFinalTexture.GetAddressOf());

	//Setup the shader resource view (srv) for the irradiance texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.Format = texDesc.Format;
	device->CreateShaderResourceView(
		irraMapFinalTexture.Get(),
		&srvDesc,
		irraIBLCubeMap.GetAddressOf());

	//Save current render target/depth buffer
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> prevRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> prevDSV;
	context->OMGetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.GetAddressOf());

	//Save current viewport
	unsigned int vpCount = 1;
	D3D11_VIEWPORT prevVP = {};
	context->RSGetViewports(&vpCount, &prevVP);

	D3D11_VIEWPORT vp = {};
	vp.Width = (float)cubeMapFaceSize;
	vp.Height = (float)cubeMapFaceSize;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	context->RSSetViewports(1, &vp);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	fullscreenVS->SetShader();
	irradiancePS->SetShader();
	irradiancePS->SetShaderResourceView("EnviromentMap", cubeSRV.Get());
	irradiancePS->SetSamplerState("BasicSampler", samplerOptions.Get());

	for (int face = 0; face < 6; face++)
	{
		//Make a render target view for this face
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.ArraySize = 1;
		rtvDesc.Texture2DArray.FirstArraySlice = face;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Format = texDesc.Format;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
		device->CreateRenderTargetView(irraMapFinalTexture.Get(), &rtvDesc, rtv.GetAddressOf());

		float black[4] = {};
		context->ClearRenderTargetView(rtv.Get(), black);
		context->OMSetRenderTargets(1, rtv.GetAddressOf(), 0);

		irradiancePS->SetInt("faceIndex", face);
		irradiancePS->SetFloat("sampleStepPhi", 0.025f);
		irradiancePS->SetFloat("sampleStepTheta", 0.025f);
		irradiancePS->CopyAllBufferData();

		//Fullscreen triangle render
		context->Draw(3, 0);

		//Flushes the graphics pipeline.
		//Makes c++ wait for a second, better than having hardware timeout and causing a crash
		context->Flush();
	}

	//Reset everything after loop
	context->OMSetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.Get());
	context->RSSetViewports(1, &prevVP);
}

void SkyMap::IBLCreateConvolvedSpecularMap(SimpleVertexShader* fullscreenVS, SimplePixelShader* specularConPS)
{
	//Calculate mip map levels
	//Smallers levels are undesirable as they have little to no difference between one another. Skip these.
	//Add one (+1) to start skipping at 1x1 level;
	calculatedMipLevels = max((int)(log2(cubeMapFaceSize)) + 1 - mipToSkip, 1);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> conSpecMapFinalTexture;

	//Setup the description for the final irradiance cube texture
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = cubeMapFaceSize;
	texDesc.Height = cubeMapFaceSize;
	texDesc.ArraySize = 6;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; //Applies both
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.MipLevels = calculatedMipLevels;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	texDesc.SampleDesc.Count = 1;
	device->CreateTexture2D(&texDesc, 0, conSpecMapFinalTexture.GetAddressOf());

	//Setup the shader resource view (srv) for the irradiance texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = calculatedMipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.Format = texDesc.Format;
	device->CreateShaderResourceView(
		conSpecMapFinalTexture.Get(),
		&srvDesc,
		conSpecIBLCubeMap.GetAddressOf());

	//Save current render target/depth buffer
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> prevRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> prevDSV;
	context->OMGetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.GetAddressOf());

	//Save current viewport
	unsigned int vpCount = 1;
	D3D11_VIEWPORT prevVP = {};
	context->RSGetViewports(&vpCount, &prevVP);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	fullscreenVS->SetShader();
	specularConPS->SetShader();
	specularConPS->SetShaderResourceView("EnviromentMap", cubeSRV.Get());
	specularConPS->SetSamplerState("BasicSampler", samplerOptions.Get());

	for (int currentMip = 0; currentMip < calculatedMipLevels; currentMip++)
	{
		for (int face = 0; face < 6; face++)
		{
			D3D11_VIEWPORT vp = {};
			vp.Width = (float)pow(2, calculatedMipLevels + mipToSkip - 1 - currentMip);
			vp.Height = vp.Width;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			context->RSSetViewports(1, &vp);

			specularConPS->SetFloat("roughness", currentMip / (float)(calculatedMipLevels-1));
			specularConPS->SetInt("faceIndex", face);
			specularConPS->SetInt("mipLevel", currentMip);
			specularConPS->CopyAllBufferData();

			//Fullscreen triangle render
			context->Draw(3, 0);

			//Flushes the graphics pipeline.
			//Makes c++ wait for a second, better than having hardware timeout and causing a crash
			context->Flush();
		}
	}

	//Reset everything after loop
	context->OMSetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.Get());
	context->RSSetViewports(1, &prevVP);
}

void SkyMap::IBLCreateBRDFLookUpTexture(SimpleVertexShader* fullscreenVS, SimplePixelShader* lookUpTexturePS)
{

	Microsoft::WRL::ComPtr<ID3D11Texture2D> lookUpFinalTexture;

	//Setup the description for the final irradiance cube texture
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = lookUpTextureSize;
	texDesc.Height = lookUpTextureSize;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; //Applies both
	texDesc.Format = DXGI_FORMAT_R16G16_UNORM;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = 0;
	texDesc.SampleDesc.Count = 1;
	device->CreateTexture2D(&texDesc, 0, lookUpFinalTexture.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Format = texDesc.Format;
	device->CreateShaderResourceView(
		lookUpFinalTexture.Get(), &srvDesc, brdfLookUpTexture.GetAddressOf());

	//Save current render target/depth buffer
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> prevRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> prevDSV;
	context->OMGetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.GetAddressOf());

	//Save current viewport
	unsigned int vpCount = 1;
	D3D11_VIEWPORT prevVP = {};
	context->RSGetViewports(&vpCount, &prevVP);

	//Generate new viewport
	D3D11_VIEWPORT vp = {};
	vp.Width = (float)lookUpTextureSize;
	vp.Height = (float)lookUpTextureSize;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	context->RSSetViewports(1, &vp);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	fullscreenVS->SetShader();
	lookUpTexturePS->SetShader();

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Format = texDesc.Format;

	//Fullscreen triangle render
	context->Draw(3, 0);

	//Flushes the graphics pipeline.
	//Makes c++ wait for a second, better than having hardware timeout and causing a crash
	context->Flush();

	//Reset everything after loop
	context->OMSetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.Get());
	context->RSSetViewports(1, &prevVP);
}
