
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

			// SOURCE 2 format
			// binary key-value
			
			KeyValue* root = KVReader2::Parse(vmtData);
			Texture* tt = 0;
			KeyValue* txtParams = root->Find("m_textureParams");
			for(int i=0;i<txtParams->childCount;i++)
			{
				KeyValue* txt = txtParams->Get(i);
				if(strcmp("g_tColor",txt->Find("m_name")->AsName())==0)
				{
					std::string txtFilename = std::string(txt->Find("m_pValue")->AsHandle());
					txtFilename = txtFilename + "_c";
					const char* txtFNC = txtFilename.c_str();
					printf("Use %s as diffuse\n",txtFNC);
					if((textureDiffuse = Manager::find(txtFNC))==0) Manager::add(textureDiffuse = new Texture(txtFNC));
				}
				else if(strcmp("g_tNormal",txt->Find("m_name")->AsName())==0)
				{
					std::string txtFilename = std::string(txt->Find("m_pValue")->AsHandle());
					txtFilename = txtFilename + "_c";
					const char* txtFNC = txtFilename.c_str();
					printf("Use %s as normal\n",txtFNC);
					if((textureNormal = Manager::find(txtFNC))==0) Manager::add(textureNormal = new Texture(txtFNC));
				}
				else if(strcmp("g_tMasks1",txt->Find("m_name")->AsName())==0 || strcmp("g_tFresnelWarp",txt->Find("m_name")->AsName())==0)
				{
					std::string txtFilename = std::string(txt->Find("m_pValue")->AsHandle());
					txtFilename = txtFilename + "_c";
					const char* txtFNC = txtFilename.c_str();
					printf("Use %s as mask 1\n",txtFNC);
					if((textureMask1 = Manager::find(txtFNC))==0) Manager::add(textureMask1 = new Texture(txtFNC));
				}
				else if(strcmp("g_tMasks2",txt->Find("m_name")->AsName())==0 || strcmp("g_tDetail2",txt->Find("m_name")->AsName())==0)
				{
					std::string txtFilename = std::string(txt->Find("m_pValue")->AsHandle());
					txtFilename = txtFilename + "_c";
					const char* txtFNC = txtFilename.c_str();
					printf("Use %s as mask 2\n",txtFNC);
					if((textureMask2 = Manager::find(txtFNC))==0) Manager::add(textureMask2 = new Texture(txtFNC));
				}
			}
			
			// TODO: add other parameter such as specular scale
			
			KVReader2::Clean(root);
			
			free(vmtData);
			
		}
	}
}

void Material::Bind()
{
	if(state == FS_READY)
	{
		//printf("Binding mat %X %X %X %X\n",textureDiffuse,textureNormal,textureMask1,textureMask2);
		if(textureDiffuse && textureDiffuse->state == FS_READY) textureDiffuse->Bind(0);
		else Scene::defaultDiffuse->Bind(0);
		if(textureNormal && textureNormal->state == FS_READY) textureNormal->Bind(1);
		else Scene::defaultNormal->Bind(1);
		if(textureMask1 && textureMask1->state == FS_READY) textureMask1->Bind(2);
		else Scene::defaultMask1->Bind(2);
		if(textureMask2 && textureMask2->state == FS_READY) textureMask2->Bind(3);
		else Scene::defaultMask2->Bind(3);
	}
	else
	{
		Scene::defaultDiffuse->Bind(0);
		Scene::defaultNormal->Bind(1);
		Scene::defaultMask1->Bind(2);
		Scene::defaultMask2->Bind(3);
	}
}

void Material::SetUniform(GLuint locHqNormal)
{
	// HQ normal map
	int hqn = 1;
	glUniform1iv(locHqNormal, 1, &hqn);
}

void Material::Unbind()
{
	if(state == FS_READY)
	{
		if(textureDiffuse && textureDiffuse->state == FS_READY) textureDiffuse->Unbind(0);
		else Scene::defaultDiffuse->Unbind(0);
		if(textureNormal && textureNormal->state == FS_READY) textureNormal->Unbind(1);
		else Scene::defaultNormal->Unbind(1);
		if(textureMask1 && textureMask1->state == FS_READY) textureMask1->Unbind(2);
		else Scene::defaultMask1->Unbind(2);
		if(textureMask2 && textureMask2->state == FS_READY) textureMask2->Unbind(3);
		else Scene::defaultMask2->Unbind(3);
	}
	else
	{
		Scene::defaultDiffuse->Unbind(0);
		Scene::defaultNormal->Unbind(1);
		Scene::defaultMask1->Unbind(2);
		Scene::defaultMask2->Unbind(3);
	}
}