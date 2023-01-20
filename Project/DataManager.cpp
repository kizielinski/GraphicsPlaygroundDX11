//Kyle Zielinski
//12/8/2022
//Basic Scene Storage

#include "DataManager.h"

DataManager::DataManager(
	EntityWindow _eWindow,
	const std::vector<Entity*>& _entities,
	const std::unordered_map<std::string, LightObject>& _lights,
	const std::vector<Emitter*>& _emitters) : entities(_entities), lights(_lights), emitters(_emitters)
{
	_eWindow = eWindow;
}

DataManager::~DataManager()
{
}

//wstring To_WString(string s) 
//{
//	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(s);
//}
//
//string To_String(wstring ws)
//{
//	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(ws);
//}

void DataManager::ConvertSceneToData()
{
	sceneData.skyMesh = eWindow.ReturnSkyPath();

	//Store Entity Data
	int index = 0;
	for (auto e : entities)
	{
		StoredEntity entity;
		entity.storedEP =  e->GetPositionDataStruct();
		entity.storedED =  e->GetDataStruct();
		entity.storedGD =  e->GetGraphicDataStruct();

		if (index < entities.size() || index < 255)
		{
			sceneData.entities[index] = entity;
		}
		index++;
	}

	//for (auto em : emitters)
	//{
	//	sceneData.storedEmitters = em->ReturnData();
	//}
	//
	//for (auto l : lights)
	//{
	//	dataToWrite += To_WString(l.first);
	//}

	WriteData();
	ReadData();
}

void DataManager::ConvertDataToScene()
{

}

void DataManager::WriteData()
{
	ofstream writeFile("sceneName.dat", ios::out | ios::binary);
	if (!writeFile)
	{
		cout << "Cannot write file!" << endl;
	}

	for (int i = 0; i < entities.size(); i++)
	{
		writeFile.write((char *) &sceneData.entities[i], sizeof(StoredEntity));
	}
	writeFile.close();
}

void DataManager::ReadData()
{
	ifstream readFile("sceneName.dat");
	if (!readFile)
	{
		cout << "Cannot read File!" << endl;
	}

	for (int i = 0; i < (sizeof(sceneData.entities) / sizeof(StoredEntity)); i++)
	{
		readFile.read((char*)&sceneDataInput.entities[i], sizeof(StoredEntity));
	}
	
	if (!readFile.good())
	{
		cout << "Error while reading file!" << endl;
	}

	int x = 0;
	sceneDataInput.entities->storedEP.X = x;

}
