 #include "Renderer.h"



Renderer::Renderer(
	Microsoft::WRL::ComPtr<ID3D11Device> _device, 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context, 
	Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain, 
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV, 
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV, 
	unsigned int _windowWidth, 
	unsigned int _windowHeight, 
	SimplePixelShader* _pShader, 
	SimpleVertexShader* _vShader, 
	SkyMap* _sky,
	const std::vector<Entity*>& _entities, 
	const std::vector<Light>& _lights) : entities(_entities), lights(_lights)
{
	device = _device;
	context = _context;
	swapChain = _swapChain;
	backBufferRTV = _backBufferRTV;
	depthBufferDSV = _depthBufferDSV;
	windowWidth = _windowWidth;
	windowHeight = _windowHeight;
	currentIndex = -1;
	mySkyBox = _sky;

	pixelShader = _pShader;
	vertexShader = _vShader;

	CreateRenderTarget(windowWidth, windowHeight, sceneColorRTV, sceneColorSRV);
	CreateRenderTarget(windowWidth, windowHeight, sceneNormalRTV, sceneNormalSRV);
	CreateRenderTarget(windowWidth, windowHeight, sceneDepthRTV, sceneDepthSRV);

	LoadLighting();
}

Renderer::~Renderer()
{
	pixelShader = nullptr;
	vertexShader = nullptr;
}

void Renderer::Update(float deltaTime, float totalTime)
{
}

void Renderer::Order()
{
}

//Render all lights and objects in one go.
void Renderer::Render(float deltaTime, float totalTime, Camera* cam, EntityWindow* eW, HWND windowHandle)
{
	SimplePixelShader* temp;
	//Once per frame, you're resetting the window
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearRenderTargetView(sceneColorRTV.Get(), color);
	context->ClearRenderTargetView(sceneNormalRTV.Get(), color);
	context->ClearRenderTargetView(sceneDepthRTV.Get(), color);
	context->ClearDepthStencilView(
		depthBufferDSV.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	//Setup RTVs
	ID3D11RenderTargetView* renderTargets[3] = {};
	renderTargets[0] = sceneColorRTV.Get();
	renderTargets[1] = sceneNormalRTV.Get();
	renderTargets[2] = sceneDepthRTV.Get();

	context->OMSetRenderTargets(3, renderTargets, depthBufferDSV.Get());

	DrawPointLights(cam);

	for (int i = 0; i < entities.size(); i++)
	{
		temp = entities[i]->GetMaterial()->GetPixelShader();
		temp->SetInt("SpecIBLTotalMipLevels", mySkyBox->ReturnCalculatedMipLevels());
		temp->SetShaderResourceView("BrdfLookUpMap", mySkyBox->ReturnLookUpTexture());
		temp->SetShaderResourceView("IrradianceIBLMap", mySkyBox->ReturnIrradianceCubeMap());
		temp->SetShaderResourceView("SpecularIBLMap", mySkyBox->ReturnConvolvedSpecularCubeMap());
		entities[i]->DrawEntity(context, cam);
	}

	mySkyBox->SkyDraw(context.Get(), cam);

	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), 0);

	eW->DisplayWindow(windowHandle, windowWidth, windowHeight);

	RenderWindow();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	swapChain->Present(0, 0);
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
}

//Makes sure that the entities is always live updated
//This was in place before the renderer assignment
//Could figure out the const vector<Entity*> constructor syntax so I left it as is for now
void Renderer::SetCurrentIndex(int index)
{
	currentIndex = index;
}

void Renderer::AddSkyBox(SkyMap* sM)
{
	mySkyBox = sM;
}

void Renderer::AlterPosition(EntityPosition entityPos)
{
	entities[currentIndex]->SetPositionDataStruct(entityPos);
	entities[currentIndex]->GetTransform()->SetPosition(entityPos.X, entityPos.Y, entityPos.Z);
}

int Renderer::EntitiesListSize()
{
	return int(entities.size());
}

int Renderer::ReturnCurrentEntityIndex()
{
	return currentIndex;
}

Entity Renderer::ReturnCurrentEntity()
{
	return *entities[currentIndex];
}

void Renderer::PreResize()
{
	backBufferRTV.Reset();
	depthBufferDSV.Reset();
}

void Renderer::PostResize(unsigned int _windowWidth, unsigned int _windowHeight, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV, Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV)
{
	windowWidth = _windowWidth;
	windowHeight = _windowHeight;
	backBufferRTV = _backBufferRTV;
	depthBufferDSV = _depthBufferDSV;

	// Release all of the renderer-specific render targets
	sceneColorRTV.Reset();
	sceneColorSRV.Reset();
	sceneNormalRTV.Reset();
	sceneNormalSRV.Reset();
	sceneDepthSRV.Reset();
	sceneDepthSRV.Reset();

	// Recreate using the new window size
	CreateRenderTarget(windowWidth, windowHeight, sceneColorRTV, sceneColorSRV);
	CreateRenderTarget(windowWidth, windowHeight, sceneNormalRTV, sceneNormalSRV);
	CreateRenderTarget(windowWidth, windowHeight, sceneDepthRTV, sceneDepthSRV);
}

void Renderer::DrawPointLights(Camera* cam)
{
	//Lighting (have to rework PixelShader to use an array of lights rather than individual ones)
	SetUpLights(cam);
	pixelShader->CopyAllBufferData();
}

void Renderer::SetUpLights(Camera* cam)
{
	pixelShader->SetData(
		"light",
		&light,
		sizeof(Light)
	);

	/*pixelShader->SetData(
		"lightList",
		&lights,
		sizeof(Light) * lights.size()
	);*/

	pixelShader->SetFloat3("ambientColor", ambientColor);

	//pixelShader->SetFloat("specularIntensity", baseMaterial->GetSpecularIntensity()); //Need to figure out a way to grab base material
	pixelShader->SetFloat("specularIntensity", 1.0f);

	pixelShader->SetFloat3("camPosition", cam->GetPosition());
}

//Load some default light values
void Renderer::LoadLighting()
{
	//New light intialization
	light.color = DirectX::XMFLOAT3(0.9f, 0.9f, 0.9f);
	light.intensity = 1.0f;
	light.direction = DirectX::XMFLOAT3(1, 0, 0);
	light.position = DirectX::XMFLOAT3(0, 0, 0);
	light.lightType = 0;

	ambientColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
}

void Renderer::CreateRenderTarget(
	unsigned int width,
	unsigned int height,
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv)
{
	// Make the texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D> rtTexture;

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // Need both!
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //We'll keep this standard 
	texDesc.MipLevels = 1; //No mipmap chain
	texDesc.MiscFlags = 0;
	texDesc.SampleDesc.Count = 1; // Can't be zero
	device->CreateTexture2D(&texDesc, 0, rtTexture.GetAddressOf());

	// Make the render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;  // This points to a Texture2D
	rtvDesc.Texture2D.MipSlice = 0;                         // Which mip are we rendering into?
	rtvDesc.Format = texDesc.Format;						// Same format as texture
	device->CreateRenderTargetView(rtTexture.Get(), &rtvDesc, rtv.GetAddressOf());

	// Create the shader resource view using default options 
	//Texture, Null Desc, and srv ComPtr   
	device->CreateShaderResourceView(rtTexture.Get(), 0, srv.GetAddressOf()); 
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::GetSceneColorSRV() 
{ return sceneColorSRV; }
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::GetSceneNormalSRV() 
{ return sceneNormalSRV; }
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::GetSceneDepthSRV()
{ return sceneDepthSRV; }

void Renderer::RenderWindow() 
{
	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
	ImGui::Begin("Render Window");
	ImGui::Image(this->GetSceneColorSRV().Get(), ImVec2(windowWidth / 4, windowHeight / 4));
	ImGui::Image(this->GetSceneNormalSRV().Get(), ImVec2(windowWidth / 4, windowHeight / 4));
	ImGui::Image(this->GetSceneDepthSRV().Get(), ImVec2(windowWidth / 4, windowHeight / 4));
	ImGui::End();
}