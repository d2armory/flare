
#include "texture.hpp"

Texture::Texture(const char* fileName)
{
	strncpy(this->fileName,fileName,TEXTURE_NAME_LENGTH);
	std::string fnStr(fileName);
	std::hash<std::string> hasher;
	this->fnHash = (unsigned int) hasher(fnStr);
	state = FS_UNINIT;
}
Texture::~Texture()
{
	
}

void Texture::Update()
{
	if(state == FS_UNINIT)
	{
		state = FS_LOADING;
		if(!FileLoader::FileExist(fileName))
		{
			FileLoader::Load(fileName);
		}
	}
	if(state == FS_LOADING)
	{
		if(FileLoader::FileExist(fileName))
		{
			state = FS_READY;
			printf("Texture %s ready\n",fileName);
		}
	}
}
void Texture::Draw()
{
	
}