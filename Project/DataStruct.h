#pragma once
struct GraphicData 
{
	std::string meshPath;
	std::wstring albedoPath;
	std::wstring normalPath;
	std::wstring roughPath;
	std::wstring metalPath;
};

struct EntityDef
{
	int index;
	bool transparency;
	bool shadows;
};

struct EntityPosition
{
	float X;
	float Y;
	float Z;
};