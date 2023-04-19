//Kyle Zielinski
//LastEdited: 11/2/2021
//Compilation of all the types of rendering and handles various shaders/buffers for entities.
//Bugs: 1 (Line 129) SkyMap and RenderTarget[0] overlap eachother and cause gamma color increase issues.

#include "Renderer.h"

Renderer::Renderer(
	Microsoft::WRL::ComPtr<ID3D11Device> _device, 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context, 
	Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain, 
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV, 
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV,
	Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerOptions,
	unsigned int _windowWidth, 
	unsigned int _windowHeight, 
	SimplePixelShader* _pShader, 
	SimplePixelShader* _finalCombinePS,
	SimplePixelShader* _finalOutputPS,
	SimplePixelShader* _refractionPS,
	SimpleVertexShader* _vShader, 
	SimpleVertexShader* _fsVS,
	SkyMap* _sky,
	const std::vector<Entity*>& _entities, 
	const std::unordered_map<std::string, LightObject>& _lights,
	const std::vector<Emitter*>& _emitters) : entities(_entities), lights(_lights), emitters(_emitters)
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
	finalCombinePS = _finalCombinePS;
	finalOutputPS = _finalOutputPS;
	vertexShader = _vShader;
	fullScreenVS = _fsVS;
	refractionPS = _refractionPS;
	samplerOptions = _samplerOptions;

	//Default values
	refracScale = 0.1f;
	useRefracSil = false;
	indexOfRefraction = 0.5f;
	refracNormalMap = true;

	CreateRenderTarget(windowWidth, windowHeight, sceneColorRTV, sceneColorSRV);
	CreateRenderTarget(windowWidth, windowHeight, sceneAmbientColorRTV, sceneAmbientColorSRV);
	CreateRenderTarget(windowWidth, windowHeight, sceneNormalRTV, sceneNormalSRV);
	CreateRenderTarget(windowWidth, windowHeight, sceneDepthRTV, sceneDepthSRV);
	CreateRenderTarget(windowWidth, windowHeight, finalRTV, finalSRV);
	CreateRenderTarget(windowWidth, windowHeight, refracRTV, refracSRV);

	LoadLighting();

	// Set up render states for particles (since all emitters might use similar ones)
	D3D11_DEPTH_STENCIL_DESC particleDepthDesc = {};
	particleDepthDesc.DepthEnable = true; // READ from depth buffer
	particleDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // No depth WRITING
	particleDepthDesc.DepthFunc = D3D11_COMPARISON_LESS; // Standard depth comparison
	device->CreateDepthStencilState(&particleDepthDesc, particleDepthState.GetAddressOf());

	// Additive blend state for particles (Not every emitter is necessarily additively blended!)
	D3D11_BLEND_DESC additiveBlendDesc = {};
	additiveBlendDesc.RenderTarget[0].BlendEnable = true;
	additiveBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // Add both colors
	additiveBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // Add both alpha values

	//100% of all these values.
	additiveBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	additiveBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	additiveBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	additiveBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	additiveBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&additiveBlendDesc, particleBlendAdditive.GetAddressOf());
}

Renderer::~Renderer()
{
	pixelShader = nullptr;
	vertexShader = nullptr;
	finalOutputPS = nullptr;
	finalCombinePS = nullptr;
	fullScreenVS = nullptr;
	refractionPS = nullptr;
	
	mySkyBox = nullptr;
}

void Renderer::Order()
{
}

// Render all lights and game objects in one go.
void Renderer::Render(float deltaTime, float totalTime, Camera* cam, EntityWindow* entityWindow, ImageWindow* noise, HWND windowHandle)
{
	SimplePixelShader* sPixelShader;

	// Once per frame, you're resetting the window
	// Background color (Black in this case) for clearing
	const float color[4] = { 0, 0, 0, 0 };

	// Clear my RTVs
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearRenderTargetView(sceneColorRTV.Get(), color);
	context->ClearRenderTargetView(sceneAmbientColorRTV.Get(), color);
	context->ClearRenderTargetView(sceneNormalRTV.Get(), color);
	context->ClearRenderTargetView(sceneDepthRTV.Get(), color);
	context->ClearRenderTargetView(finalRTV.Get(), color);
	context->ClearRenderTargetView(refracRTV.Get(), color);
	context->ClearDepthStencilView(
		depthBufferDSV.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	// Setup RTVs
	ID3D11RenderTargetView* renderTargets[4] = {};
	renderTargets[0] = sceneColorRTV.Get();
	renderTargets[1] = sceneAmbientColorRTV.Get();
	renderTargets[2] = sceneNormalRTV.Get();
	renderTargets[3] = sceneDepthRTV.Get();

	context->OMSetRenderTargets(4, renderTargets, depthBufferDSV.Get());
	
	DrawPointLights(cam);

	// Collect all refractive entities for after the final texture combination.
	std::vector<Entity*> refractiveEntities;

	for(auto gameEntity : entities)
	{
		// Checks if entity is refractive
		if (gameEntity->GetMaterial()->IsRefractive())
		{
			refractiveEntities.push_back(gameEntity);
			continue; 
		}
		else 
		{
			// Handles normal entity rendering using IBL lighting
			sPixelShader = gameEntity->GetMaterial()->GetPixelShader();
			sPixelShader->SetInt("SpecIBLTotalMipLevels", mySkyBox->ReturnCalculatedMipLevels());
			sPixelShader->SetShaderResourceView("BrdfLookUpMap", mySkyBox->ReturnLookUpTexture());
			sPixelShader->SetShaderResourceView("IrradianceIBLMap", mySkyBox->ReturnIrradianceCubeMap());
			sPixelShader->SetShaderResourceView("SpecularIBLMap", mySkyBox->ReturnConvolvedSpecularCubeMap());
			gameEntity->DrawEntity(context, cam);
		}
	}

	/* [IMPORTANT]
	 * Sky Draw gets clumped with RenderTarget[0] not sure how to stop this from happening
	 * Tried to separate it manually by setting RenderTarget[0] = 0; and calling OMSetRenderTargets(...),
	 * but it didn't have any effect. Only bug in this version of Renderer.cpp.
	 */
	mySkyBox->SkyDraw(context.Get(), cam);

	fullScreenVS->SetShader();

	// Final Combine
	{
		renderTargets[0] = finalRTV.Get();
		context->OMSetRenderTargets(1, renderTargets, 0);
		finalCombinePS->SetShader();
		finalCombinePS->SetShaderResourceView("finalTextureColor", sceneColorSRV.Get());
		finalCombinePS->SetShaderResourceView("finalTextureAmbient", sceneAmbientColorSRV.Get());
		finalCombinePS->SetSamplerState("basicSampler", samplerOptions.Get());

		//Fullscreen triangle render
		context->Draw(3, 0);
	}
	
	//Final total composite texture
	{
		renderTargets[0] = backBufferRTV.Get();
		context->OMSetRenderTargets(1, renderTargets, 0);
		finalOutputPS->SetShader();
		finalOutputPS->SetShaderResourceView("Pixels", finalSRV.Get());
		finalCombinePS->SetSamplerState("basicSampler", samplerOptions.Get());
		//Fullscreen triangle render
		context->Draw(3, 0);
	}

	// Handle Refraction calculation (basic no Silhouettes yet)
	{
		// No refractive silhouttes, fix rendering class organization first.
		// if (useRefracSilhouette) {...} 

		// Loop and draw refractive objects
		{
			renderTargets[0] = backBufferRTV.Get();
			context->OMSetRenderTargets(1, renderTargets, depthBufferDSV.Get());

			for (auto refracGE : refractiveEntities)
			{
				// Material* material = refracGE->GetMaterial().get(); <--------This creates a dangling pointer
				SimplePixelShader* prevPS = refracGE->GetMaterial().get()->GetPixelShader(); 
				vertexShader->SetShader();
				refracGE->GetMaterial().get()->SetPixelShader(refractionPS);

				// Material Prep? Idk if I need this
				refracGE->GetMaterial().get()->PrepMaterialForDraw(refracGE->GetTransform(), cam);

				// Setup the basic VertexShader stuff.
				refractionPS->SetSamplerState("basicSampler", samplerOptions);

				// Setup Refraction data
				refractionPS->SetMatrix4x4("viewMatrix", cam->GetViewMatrix());
				refractionPS->SetMatrix4x4("projMatrix", cam->GetProjectionMatrix());
				refractionPS->SetFloat2("screenSize", XMFLOAT2((float)windowWidth, (float)windowHeight));
				refractionPS->SetInt("useRefracSil", useRefracSil);
				refractionPS->SetInt("refracFromNormalMap", refracNormalMap);
				refractionPS->SetFloat3("camPos", cam->GetPosition());
				refractionPS->SetFloat("indexOfRefraction", indexOfRefraction );
				refractionPS->SetFloat("refractionScale", refracScale);
				refractionPS->CopyBufferData("perObject");

				// Set textures to use
				refractionPS->SetShaderResourceView("ScreenPixels", finalSRV.Get());
				refractionPS->SetShaderResourceView("NormalTexture", refracGE->GetMaterial()->NormalTexture());
				refractionPS->SetShaderResourceView("RefractionSilhouette", refracSRV.Get());
				refractionPS->SetShaderResourceView("EnvironmentMap", mySkyBox->ReturnSkyMapSRV());
				
				/* Reset "per frame" buffers <-Not implemented yet so not using this for now
				 * context->VSSetConstantBuffers(0, 1, vsPerFrameConstantBuffer.GetAddressOf());
				 * context->PSSetConstantBuffers(0, 1, psPerFrameConstantBuffer.GetAddressOf());
				 */
				refracGE->GetMesh()->DrawUsingBuffs(context);

				// Reset back to original pixelShader
				refracGE->GetMaterial().get()->SetPixelShader(prevPS);

				context->Draw(3, 0);
			}
		}
	}

	//Render particles
	{
		renderTargets[0] = backBufferRTV.Get();
		context->OMSetRenderTargets(1, renderTargets, depthBufferDSV.Get());

		context->OMSetBlendState(particleBlendAdditive.Get(), 0, 0xFFFFFFFF);
		context->OMSetDepthStencilState(particleDepthState.Get(), 0);

		//Draw all emitters
		for (auto& e : emitters)
		{
			e->Draw(cam, totalTime);
		}

		context->OMSetBlendState(0, 0, 0xFFFFFFFF);
		context->OMSetDepthStencilState(0, 0);
	}
	
	noise->DisplaySettings(640, 640, "Noise Window");
	noise->DisplayWindow(400, 500);

	RenderImGUI(entityWindow);
	//All ImGui Begin()/End() calls should be before this call
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

//Assigns new skyMap
void Renderer::AddSkyBox(SkyMap* sM)
{
	mySkyBox = sM;
}

//Alters physical position of the entity from Entity Window information
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

Entity* Renderer::ReturnCurrentEntity()
{
	return entities[currentIndex];
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
	sceneAmbientColorRTV.Reset();
	sceneAmbientColorSRV.Reset();
	sceneNormalRTV.Reset();
	sceneNormalSRV.Reset();
	sceneDepthSRV.Reset();
	sceneDepthSRV.Reset();

	// Recreate using the new window size
	CreateRenderTarget(windowWidth, windowHeight, sceneColorRTV, sceneColorSRV); 
	CreateRenderTarget(windowWidth, windowHeight, sceneAmbientColorRTV, sceneAmbientColorSRV);
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
	int iterator = 0;
	Light shaderLights[64];
	for (auto light : lights)
	{
		if (iterator < lights.size())
		{
			shaderLights[iterator] = light.second.GetLight();
			iterator++;
		}
	}

	/*pixelShader->SetData(
		"light",
		&light,
		sizeof(Light)
	);*/

	pixelShader->SetData(
		"lightList",
		&shaderLights,
		sizeof(Light) * lights.size()
	);

	pixelShader->SetFloat3("ambientColor", ambientColor);

	//pixelShader->SetFloat("specularIntensity", baseMaterial->GetSpecularIntensity()); //Rework to grab base material
	pixelShader->SetFloat("specularIntensity", 1.0f);

	pixelShader->SetFloat3("camPosition", cam->GetPosition());
}

//Load some default light values
void Renderer::LoadLighting()
{
	//New light intialization
	light.color = DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f);
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
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::GetSceneAmbientColorSRV()
{return sceneAmbientColorSRV;}
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::GetSceneNormalSRV() 
{ return sceneNormalSRV; }
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::GetSceneDepthSRV()
{ return sceneDepthSRV; }
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::GetFluidSRV()
{ return fluidSRV; }

void Renderer::RenderImGUI(EntityWindow* eW)
{
	eW->DisplayWindow(windowWidth, windowHeight);

	iW.SetSRVs({ 
	GetSceneColorSRV().Get(),
	GetSceneAmbientColorSRV().Get(),
	GetSceneNormalSRV().Get(),
	GetSceneDepthSRV().Get() });
	iW.DisplaySettings(windowWidth, windowHeight, "M-RTV Window");
	iW.DisplayWindow(480, 600);

	int x = 0; 

	iW.SetSRVs({ GetFluidSRV().Get() });
	iW.DisplaySettings(windowWidth, windowHeight, "Fluids Window");
	iW.DisplayWindow(480, 600);

}