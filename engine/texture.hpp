#pragma once

#include "enum.hpp"
#include "../es/esUtil.h"
#include "fileLoader.hpp"
#include <cstring>

#define TEXTURE_NAME_LENGTH 128

class Texture
{

public:

	Texture(const char* fileName);
	~Texture();

	// function
	void Update();
	void Draw();

	unsigned int fnHash;
	char fileName[TEXTURE_NAME_LENGTH];

	FILE_STATE state;
	// mdlTexture is temporary
	
	GLuint textureId;
	
	// manager
	Texture* nextTexture;
	Texture* childLeft;
	Texture* childRight;

private:

	
};