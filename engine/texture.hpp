#pragma once

#include "enum.hpp"
#include "../es/esUtil.h"
#include "fileLoader.hpp"
#include "../mdl/common.h"
#include <cstring>
#include "../squish/squish.h"
#include <html5.h>

#define TEXTURE_NAME_LENGTH 128

class Texture
{

public:

	Texture(const char* fileName);
	~Texture();

	// function
	void Update();
	
	void Bind(int i);
	void Unbind(int i);

	unsigned int fnHash;
	char fileName[TEXTURE_NAME_LENGTH];

	FILE_STATE state;
	// mdlTexture is temporary
	
	bool isCubemap;
	unsigned int txtType;
	GLuint textureId;
	
	// might considering remove/ replace with only header
	char* textureData;
	
	// manager
	Texture* nextTexture;
	Texture* childLeft;
	Texture* childRight;

private:

	
};