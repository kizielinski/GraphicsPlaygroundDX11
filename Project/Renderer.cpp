 #include "Renderer.h"

Renderer::Renderer()
{
	device = nullptr;
	context = nullptr;
	swapChain = nullptr;
	backBufferRTV = nullptr;
	depthBufferDSV = nullptr;

	myEntities = vector<Entity*>();
	lights = vector<Light>();

	windowWidth = 100;
	windowHeight = 100;

	mySkyBox = nullptr;
	currentIndex = -1;
}

Renderer::Renderer(
	Microsoft::WRL::ComPtr<ID3D11Device> _device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
	Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain,
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTC,
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV,
	unsigned int _windowWidth,
	unsigned int _windowHeight,
	//std::vector<Entity*> _entities,
	std::vector<Light> _lights,
	SimplePixelShader* pShader,
	SimpleVertexShader* vShader)
	//: entities(_entities), lights(_lights) <-----This syntax didn't help because I also needed to intialize the default constructor which was the problem
{
	device = _device;
	context = _context;
	swapChain = _swapChain;
	backBufferRTV = _backBufferRTC;
	depthBufferDSV = _depthBufferDSV;
	windowWidth = _windowWidth;
	windowHeight = _windowHeight;
	myEntities = vector<Entity*>();
	lights = _lights;
	currentIndex = -1;
	mySkyBox = nullptr;

	pixelShader = pShader;
	vertexShader = vShader;

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
	
	DrawPointLights(cam);

	for (int i = 0; i < myEntities.size(); i++)
	{
		myEntities[i]->DrawEntity(context, cam);
	}

	mySkyBox->SkyDraw(context.Get(), cam);

	eW->DisplayWindow(windowHandle, windowWidth, windowHeight);

	swapChain->Present(0, 0);
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());

	/*for (int i = 0; i < entities.size(); i++)
	{
		entities[i]->DrawEntity(context, cam);
	}

	sky->SkyDraw(context, cam);*/
}

//Makes sure that the entities is always live updated
//This was in place before the renderer assignment
//Could figure out the const vector<Entity*> constructor syntax so I left it as is for now
void Renderer::SetEntities(vector<Entity*> _myEntities)
{
	//Changed to use the const value of entities
	myEntities.clear();
	myEntities = _myEntities; //entities;
	currentIndex = int(_myEntities.size()) - 1;//entities.size() - 1;
}

void Renderer::AddSkyBox(SkyMap* sM)
{
	mySkyBox = sM;
}

void Renderer::AlterPosition(EntityPosition entityPos)
{
	myEntities[currentIndex]->SetPositionDataStruct(entityPos);
	myEntities[currentIndex]->GetTransform()->SetPosition(entityPos.X, entityPos.Y, entityPos.Z);
}

int Renderer::EntitiesListSize()
{
	return int(myEntities.size());
}

void Renderer::RemoveEntity(int index)
{
	Entity* e = myEntities[index];
	myEntities.erase(myEntities.begin() + index);
	delete e;
	e = nullptr;

	//Have to realign all of the entities with their new indice locations.
	for (int i = 0; i < myEntities.size(); i++)
	{
		EntityDef temp;
		temp = myEntities[i]->GetDataStruct();
		temp.index = i;
		myEntities[i]->SetDataStruct(temp);
	}
	DecrementCurrentEntity();
}

//Keeping track of the current index (also don't let it fall below 0)
//Allows to return current selected entity.
void Renderer::IncrementCurrentEntity()
{
	currentIndex++;
}

void Renderer::DecrementCurrentEntity()
{
	currentIndex--;
	if (currentIndex < 0)
	{
		currentIndex = 0;
	}
}

int Renderer::ReturnCurrentEntityIndex()
{
	return currentIndex;
}

Entity Renderer::ReturnCurrentEntity()
{
	return *myEntities[currentIndex];
}

void Renderer::PostResize(unsigned int _windowWidth, unsigned int _windowHeight, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV, Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV)
{
	windowWidth = _windowWidth;
	windowHeight = windowHeight;
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
	light.intensity = 9.0f;
	light.direction = DirectX::XMFLOAT3(0, 0, 0);
	light.position = DirectX::XMFLOAT3(2, 4, 0);
	light.lightType = 1;

	ambientColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
}
