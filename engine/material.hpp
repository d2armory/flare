#pragma once

#include "enum.hpp"
#include "texture.hpp"

#define MATERIAL_NAME_LENGTH 128

class Material
{

public:

	Material();
	~Material();

	// function
	void Update();
	void Draw();

	char fileName[MATERIAL_NAME_LENGTH];

	// no mdlMaterial, need to parse ascii files
	FILE_STATE state;
	
	Texture* textureDiffuse;
	Texture* textureNormal;
	Texture* textureMask1;
	Texture* textureMask2;
	Texture* textureEnvmap;
	
	// manager
	Material* nextMaterial;

private:

	
};