
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
	
	translucent = false;
	blendType = 0;
	
	ambientScale = 1;
	specExponent = 1;
	specScale = 1;
	rimScale = 1;
	cloakIntensity = 0;
	
	useMask1 = 0;
	useMask2 = 0;
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

			printf("Material %s\n",fileName);

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
			
			KeyValue* intParams = root->Find("m_intParams");
			for(int i=0;i<intParams->childCount;i++)
			{
				KeyValue* txt = intParams->Get(i);
				if(strcmp("F_TRANSLUCENT",txt->Find("m_name")->AsName())==0)
				{
					int val = txt->Find("m_nValue")->AsInt();
					translucent = (val>0)?true:false;
				}
				else if(strcmp("F_ADDITIVE_BLEND",txt->Find("m_name")->AsName())==0)
				{
					int val = txt->Find("m_nValue")->AsInt();
					blendType = (val==1)?1:0;
				}
				else if(strcmp("F_MASKS_1",txt->Find("m_name")->AsName())==0)
				{
					int val = txt->Find("m_nValue")->AsInt();
					useMask1 = (val==1)?1:0;
				}
				else if(strcmp("F_MASKS_2",txt->Find("m_name")->AsName())==0)
				{
					int val = txt->Find("m_nValue")->AsInt();
					useMask2 = (val==1)?1:0;
				}
			}
			
			KeyValue* floatParams = root->Find("m_floatParams");
			for(int i=0;i<floatParams->childCount;i++)
			{
				KeyValue* txt = floatParams->Get(i);
				printf("%s: %f\n",txt->Find("m_name")->AsName(),txt->Find("m_flValue")->AsFloat());
				if(strcmp("g_flAmbientScale",txt->Find("m_name")->AsName())==0)
				{
					float val = txt->Find("m_flValue")->AsFloat();
					ambientScale = val;
				}
				else if(strcmp("g_flSpecularExponent",txt->Find("m_name")->AsName())==0)
				{
					float val = txt->Find("m_flValue")->AsFloat();
					specExponent = val;
				}
				else if(strcmp("g_flSpecularScale",txt->Find("m_name")->AsName())==0)
				{
					float val = txt->Find("m_flValue")->AsFloat();
					specScale = val;
				}
				else if(strcmp("g_flRimLightScale",txt->Find("m_name")->AsName())==0)
				{
					float val = txt->Find("m_flValue")->AsFloat();
					rimScale = val;
				}
				else if(strcmp("g_flCloakIntensity",txt->Find("m_name")->AsName())==0)
				{
					float val = txt->Find("m_flValue")->AsFloat();
					cloakIntensity = val;
				}
			}
			
			printf("translucent : %d, cloak intense : %f\n",translucent?1:0,cloakIntensity);
			
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
		
		//if(translucent) glDisable(GL_DEPTH_TEST);
	}
	else
	{
		Scene::defaultDiffuse->Bind(0);
		Scene::defaultNormal->Bind(1);
		Scene::defaultMask1->Bind(2);
		Scene::defaultMask2->Bind(3);
	}
}

void Material::SetUniform(MaterialShaderLocation msl)
{
	// HQ normal map
	int hqn = 1;
	glUniform1iv(msl.locHqNormal, 1, &hqn);
	
	int itl = (translucent)?1:0;
	glUniform1iv(msl.locIsTranslucent, 1, &itl);
	glUniform1iv(msl.locBlendType, 1, &blendType);
	glUniform1iv(msl.locUseMask1, 1, &useMask1);
	glUniform1iv(msl.locUseMask2, 1, &useMask2);
	
	glUniform1fv(msl.locAmbientScale,1,&ambientScale);
	glUniform1fv(msl.locSpecExponent,1,&specExponent);
	glUniform1fv(msl.locSpecScale,1,&specScale);
	glUniform1fv(msl.locRimScale,1,&rimScale);
	glUniform1fv(msl.locCloakIntensity,1,&cloakIntensity);
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
		
		//if(translucent) glEnable(GL_DEPTH_TEST);
	}
	else
	{
		Scene::defaultDiffuse->Unbind(0);
		Scene::defaultNormal->Unbind(1);
		Scene::defaultMask1->Unbind(2);
		Scene::defaultMask2->Unbind(3);
	}
}