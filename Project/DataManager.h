#pragma once

#include<iostream>
#include<fstream>
#include <codecvt>
#include <locale>
#include "Renderer.h"

class DataManager
{
public:
	DataManager(
		EntityWindow _eWindow,
		const std::vector<Entity*>& _entities,
		const std::unordered_map<std::string, LightObject>& _lights,
		const std::vector<Emitter*>& _emitters
		);
	~DataManager();

	void ConvertSceneToData();
	void ConvertDataToScene();

private:

	void WriteData();
	void ReadData(); 
	
	struct StoredEntity 
	{
		EntityPosition storedEP;
		EntityDef storedED;
		GraphicData storedGD;
	};

	struct DataObject
	{
		__int64 size;
		wstring skyMesh;
		StoredEntity entities[256];
	};

	EntityWindow eWindow;
	const std::vector<Entity*>& entities;
	const std::vector<Emitter*>& emitters;
	const std::unordered_map<std::string, LightObject>& lights;

	DataObject sceneData;
	DataObject sceneDataInput;

};

