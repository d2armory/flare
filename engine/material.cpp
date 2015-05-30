
#include "material.hpp"

Material::Material(const char* fileName)
{
	strncpy(this->fileName,fileName,MATERIAL_NAME_LENGTH);
	state = FS_UNINIT;
	nextMaterial = 0;
	textureDiffuse = 0;
	textureNormal = 0;
	textureMask1 = 0;
	textureMask2 = 0;
	textureDetail = 0;
}

Material::~Material()
{
	
}

void Material::Update()
{
	if(state == FS_UNINIT)
	{
		state = FS_LOADING;
		FileLoader::Load(fileName);
	}
	else if(state == FS_LOADING)
	{
		if(FileLoader::FileExist(fileName))
		{
			state = FS_READY;
			
			unsigned int fSize = 0;
			char* vmtData = FileLoader::ReadFile(fileName,fSize);
			
			PCST* pData = KVReader::Parse(vmtData, fSize);
			
			// use data here
			Texture* tt = 0;
			// pData->sibling is shader name
			PCST* node = pData->sibling->child;
			while(node != 0)
			{
				if(node->key != 0)
				{
		
					if(strcmp(node->key,"$basetexture") == 0)
					{
						// might want to make function of preparing txt filename
						std::string txtFilename = std::string(node->value);
						txtFilename = "materials/" + txtFilename;
						if(txtFilename.find(".vtf")==std::string::npos) txtFilename = txtFilename + ".vtf";
						const char* txtFNC = txtFilename.c_str();
						printf("Use %s as diffuse\n",txtFNC);
						if((textureDiffuse = Manager::find(txtFNC))==0) Manager::add(textureDiffuse = new Texture(txtFNC));
						//printf("mat %X %X %X %X\n",textureDiffuse,textureNormal,textureMask1,textureMask2);
					}
					else if(strcmp(node->key,"$normalmap") == 0)
					{
						std::string txtFilename = std::string(node->value);
						txtFilename = "materials/" + txtFilename;
						if(txtFilename.find(".vtf")==std::string::npos) txtFilename = txtFilename + ".vtf";
						const char* txtFNC = txtFilename.c_str();
						printf("Use %s as normal\n",txtFNC);
						if((textureNormal = Manager::find(txtFNC))==0) Manager::add(textureNormal = new Texture(txtFNC));
					}
					else if(strcmp(node->key,"$maskmap1") == 0)
					{
						std::string txtFilename = std::string(node->value);
						txtFilename = "materials/" + txtFilename;
						if(txtFilename.find(".vtf")==std::string::npos) txtFilename = txtFilename + ".vtf";
						const char* txtFNC = txtFilename.c_str();
						printf("Use %s as mask 1\n",txtFNC);
						if((textureMask1 = Manager::find(txtFNC))==0) Manager::add(textureMask1 = new Texture(txtFNC));
					}
					else if(strcmp(node->key,"$maskmap2") == 0)
					{
						std::string txtFilename = std::string(node->value);
						txtFilename = "materials/" + txtFilename;
						if(txtFilename.find(".vtf")==std::string::npos) txtFilename = txtFilename + ".vtf";
						const char* txtFNC = txtFilename.c_str();
						printf("Use %s as mask 2\n",txtFNC);
						if((textureMask2 = Manager::find(txtFNC))==0) Manager::add(textureMask2 = new Texture(txtFNC));
					}
				}
				node = node->sibling;
			}
			
			KVReader::Clean(pData);
			
			free(vmtData);
			
		}
	}
}

void Material::Bind()
{
	if(state == FS_READY)
	{
		//printf("Binding mat %X %X %X %X\n",textureDiffuse,textureNormal,textureMask1,textureMask2);
		if(textureDiffuse) textureDiffuse->Bind(0);
		if(textureNormal) textureNormal->Bind(1);
		if(textureMask1) textureMask1->Bind(2);
		if(textureMask2) textureMask2->Bind(3);
	}
}

void Material::Unbind()
{
	if(state == FS_READY)
	{
		if(textureDiffuse) textureDiffuse->Unbind(0);
		if(textureNormal) textureNormal->Unbind(1);
		if(textureMask1) textureMask1->Unbind(2);
		if(textureMask2) textureMask2->Unbind(3);
	}
}