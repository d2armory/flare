#include "manager.hpp"

Model* Manager::headModel = 0;
Material* Manager::headMaterial = 0;
Texture* Manager::headTexture = 0;	// list
Texture* Manager::rootTexture = 0;	// tree

void Manager::add(Model* m)
{
	m->nextModel = headModel;
	headModel = m;
}
void Manager::remove(Model* m)
{
	// only allow removal of model at moment
	Model* cur = headModel;
	if(cur==m)
	{
		headModel = cur->nextModel;
		m->~Model();
		free(m);
	}
	else
	{
		while(cur!=0)
		{
			if(cur->nextModel == m)
			{
				cur->nextModel = m->nextModel;
				m->~Model();
				free(m);
				break;
			}
			cur = cur->nextModel;
		}
	}
}

Model* Manager::createModel(const char* fileName)
{
	
	// new Model() won't work when called from embind for some reason
	// so I call malloc myself
	
	void* mem = malloc(sizeof(Model));
	Model* m = new (mem) Model(fileName);
	add(m);
	return m;
}

// need optimization here
Model* Manager::findModel(const char* fileName)
{
	Model* cur = headModel;
	while(cur!=0)
	{
		if(strncmp(cur->fileName,fileName,128) == 0)
		{
			return cur;
		}
		cur = cur->nextModel;
	}
	return 0;
}

void Manager::add(Material* m)
{
	m->nextMaterial = headMaterial;
	headMaterial = m;
}
void Manager::remove(Material* m)
{
	// should we?
}

void Manager::add(Texture* m)
{
	m->nextTexture = headTexture;
	headTexture = m;
	
	// tree traversal
	if(rootTexture==0)
	{
		rootTexture = m;
	}
	else
	{
		Texture* cur = rootTexture;
		while(true)
		{
			if(m->fnHash >= cur->fnHash)
			{
				if(cur->childRight!=0)
				{
					cur = cur->childRight;
				}
				else
				{
					cur->childRight = m;
					break;
				}
			}
			else
			{
				if(cur->childLeft!=0)
				{
					cur = cur->childLeft;
				}
				else
				{
					cur->childLeft = m;
					break;
				}
			}
		}
	}
}
void Manager::remove(Texture* m)
{
	// well..
}
Texture* Manager::find(const char* fileName)
{
	std::string fnStr(fileName);
	std::hash<std::string> hasher;
	unsigned int hash = (unsigned int) hasher(fnStr);
	
	//printf("Finding %s\n",fileName);
	
	Texture* ret = 0;
	
	Texture* cur = rootTexture;
	while(cur!=0)
	{
		//printf("Current = %s\n",cur->fileName);
		if(hash == cur->fnHash)
		{
			std::string curFnStr(cur->fileName);
			if(curFnStr.compare(fnStr)!=0)
			{
				// key collision
				cur = cur->childRight;
				//printf("Key collision\n");
			}
			else 
			{
				ret = cur;
				//printf("Found\n");
				break;
			}
		}
		else if(hash > cur->fnHash)
		{
			//printf("Go right\n");
			cur = cur->childRight;
		}
		else if(hash < cur->fnHash)
		{
			//printf("Go left\n");
			cur = cur->childLeft;
		}
	}
	
	return ret;
}

void Manager::Init()
{
	// do nothing currently
}

void Manager::Update(ESContext *esContext, float deltaTime)
{
	
	// prevent frame from hanging too long when load finished simultaneously
	const float maxUpdateTime = 0.1f;
	
	// this algo always process 1 update of loading of each type
	
	struct timeval before;
	struct timeval after;
	
	bool isLoading;
	float total = 0;
	
	Texture* t = headTexture;
	while(t!=0)
	{
		gettimeofday(&before,0);
		isLoading = t->state == FS_LOADING;
		/* */
		t->Update();
		t = t->nextTexture;
		/* */
		gettimeofday(&after,0);
		total += (float)(after.tv_sec - before.tv_sec + (after.tv_usec - before.tv_usec) * 1e-6);
		if(total > maxUpdateTime) break;
	}
	Material* mt = headMaterial;
	while(mt!=0)
	{
		gettimeofday(&before,0);
		isLoading = t->state == FS_LOADING;
		/* */
		//TODO: material update too, for detail map animation
		mt->Update();
		mt = mt->nextMaterial;
		/* */
		gettimeofday(&after,0);
		total += (float)(after.tv_sec - before.tv_sec + (after.tv_usec - before.tv_usec) * 1e-6);
		if(total > maxUpdateTime) break;
	}
	Model* m = headModel;
	while(m!=0)
	{
		gettimeofday(&before,0);
		isLoading = t->state == FS_LOADING;
		/* */
		m->Update(esContext, deltaTime);
		m = m->nextModel;
		/* */
		gettimeofday(&after,0);
		total += (float)(after.tv_sec - before.tv_sec + (after.tv_usec - before.tv_usec) * 1e-6);
		if(total > maxUpdateTime) break;
	}
}

void Manager::Uninit()
{
	Texture* t = headTexture;
	while(t!=0)
	{
		Texture* tc = t->nextTexture;
		delete t;
		t = tc;
	}
	Material* mt = headMaterial;
	while(mt!=0)
	{
		Material* mtc = mt->nextMaterial;
		delete mt;
		mt = mtc;
	}
	Model* m = headModel;
	while(m!=0)
	{
		Model* mc = m->nextModel;
		delete m;
		m = mc;
	}
}
