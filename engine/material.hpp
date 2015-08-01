#pragma once

#include "enum.hpp"
#include "texture.hpp"
#include "manager.hpp"
#include "kvreader.hpp"
#include "kvreader2.hpp"

class Texture;

#define MATERIAL_NAME_LENGTH 128

struct MaterialShaderLocation
{
public:
	GLuint locHqNormal;
	
	GLuint locUseMask1;
	GLuint locUseMask2;
	
	GLuint locIsTranslucent;
	GLuint locBlendType;
	
	GLuint locAmbientScale;
	GLuint locSpecExponent;
	GLuint locSpecScale;
	GLuint locRimScale;
	GLuint locCloakIntensity;
};

class Material
{

public:

	Material(const char* fileName);
	~Material();

	// function
	void Update();
	void Bind();
	void SetUniform(MaterialShaderLocation msl);
	void Unbind();

	char fileName[MATERIAL_NAME_LENGTH];

	// no mdlMaterial, need to parse ascii files
	FILE_STATE state;
	
	Texture* textureDiffuse;
	Texture* textureNormal;
	Texture* textureMask1;
	Texture* textureMask2;
	Texture* textureDetail;
	
	// material properties
	
	int useMask1;
	int useMask2;
	
	bool translucent;
	int blendType;	// 1 = additive blend
	
	float ambientScale;
	float specExponent;
	float specScale;
	float rimScale;
	float cloakIntensity;
	
	// manager
	Material* nextMaterial;

private:

	
};