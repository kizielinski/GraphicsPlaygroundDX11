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
	myEntities = _entities;
	currentIndex = -1;
	mySkyBox = _sky;

	pixelShader = _pShader;
	vertexShader = _vShader;

	LoadLighting();
}

Renderer::~Renderer()
{
	for (int i = 0; i < myEntities.size(); i++)
	{
		myEntities[i] = nullptr;
	}

	delete mySkyBox;
	mySkyBox = nullptr;

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
	//Once per frame, you're resetting the window
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearDepthStencilView(
		depthBufferDSV.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);
	
	pixelShader->SetInt("SpecIBLTotalMipLevels", mySkyBox->ReturnCalculatedMipLevels());
	pixelShader->SetShaderResourceView("BrdfLookUpMap", mySkyBox->ReturnLookUpTexture());
	pixelShader->SetShaderResourceView("IrradianceIBLMap", mySkyBox->ReturnIrradianceCubeMap());
	pixelShader->SetShaderResourceView("SpecularIBLMap", mySkyBox->ReturnConvolvedSpecularCubeMap());

	DrawPointLights(cam);

	for (int i = 0; i < entities.size(); i++)
	{
		entities[i]->DrawEntity(context, cam);
	}

	mySkyBox->SkyDraw(context.Get(), cam);

	eW->DisplayWindow(windowHandle, windowWidth, windowHeight);

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

void Renderer::PostResize(unsigned int _windowWidth, unsigned int _windowHeight, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV, Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV)
{
	windowWidth = _windowWidth;
	windowHeight = _windowHeight;
	backBufferRTV = _backBufferRTV;
	depthBufferDSV = _depthBufferDSV;
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
	light.color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	light.intensity = 1.0f;
	light.direction = DirectX::XMFLOAT3(1, 0, 0);
	light.position = DirectX::XMFLOAT3(2, 4, 0);
	light.lightType = 0;

	ambientColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
}