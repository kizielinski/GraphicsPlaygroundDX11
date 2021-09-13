 #include "Renderer.h"

Renderer::Renderer()
{
	mySkyBox = nullptr;
	currentIndex = -1;
}

Renderer::~Renderer()
{
	for (int i = 0; i < myEntities.size(); i++)
	{
		myEntities[i] = nullptr;
	}

	delete mySkyBox;
	mySkyBox = nullptr;
}

void Renderer::Update(float deltaTime, float totalTime)
{
}

void Renderer::Order()
{
}

void Renderer::Draw(float deltaTime, float totalTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* cam)
{
	for (int i = 0; i < myEntities.size(); i++)
	{
		myEntities[i]->DrawEntity(context, cam);
	}

	mySkyBox->SkyDraw(context.Get(), cam);
}

void Renderer::SetEntities(vector<Entity*> _myEntities)
{
	myEntities.clear();
	myEntities = _myEntities;
	currentIndex = _myEntities.size() - 1;
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
