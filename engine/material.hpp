#pragma once

#include "enum.hpp"
#include "texture.hpp"
#include "manager.hpp"
#include "kvreader.hpp"
#include "kvreader2.hpp"

class Texture;

#define MATERIAL_NAME_LENGTH 128

class Material
{

public:

	Material(const char* fileName);
	~Material();

	// function
	void Update();
	void Bind();
	void Unbind();

	char fileName[MATERIAL_NAME_LENGTH];

	// no mdlMaterial, need to parse ascii files
	FILE_STATE state;
	
	Texture* textureDiffuse;
	Texture* textureNormal;
	Texture* textureMask1;
	Texture* textureMask2;
	Texture* textureDetail;
	
	// manager
	Material* nextMaterial;

private:

	
};