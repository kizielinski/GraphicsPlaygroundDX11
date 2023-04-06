#include "Transform.h"
#include <string>
#include "Material.h"
#include "Mesh.h"

using namespace std;

//Rock and Stone!
struct Properties {

	string name;
	float toughness;
	float veinSize = -1; //Default is no max vein size
	int boundsSize = 1;
};

class Block
{

public:

	Block();
	~Block();

private:

	Transform myTransform;
	Properties myProperties;
	//Material mat;
	//Mesh mesh; 
};

