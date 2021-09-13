#pragma once
#include "Entity.h"
#include "SkyMap.h"

class Renderer
{
public:
	Renderer();
	~Renderer();
	void Update(float deltaTime, float totalTime);
	void Order();
	void Draw(float deltaTime, float totalTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* cam);
	void SetEntities(vector<Entity*> _myEntities);
	void AddSkyBox(SkyMap* sM);
	void AlterPosition(EntityPosition entityPos);
	int EntitiesListSize();
	void RemoveEntity(int index);
	void IncrementCurrentEntity();
	void DecrementCurrentEntity();
	int ReturnCurrentEntityIndex();
	Entity ReturnCurrentEntity();

private:
	std::vector<Entity*> myEntities;
	//Skybox
	SkyMap* mySkyBox;
	int currentIndex;
};

