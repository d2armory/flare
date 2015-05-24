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
	// so?
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
	
	Texture* ret = 0;
	
	Texture* cur = rootTexture;
	while(cur!=0)
	{
		if(hash == cur->fnHash)
		{
			std::string curFnStr(cur->fileName);
			if(curFnStr.compare(fnStr)!=0)
			{
				// key collision
				cur = cur->childRight;
			}
			else 
			{
				ret = cur;
			}
		}
		else if(hash > cur->fnHash)
		{
			cur = cur->childRight;
		}
		else if(hash < cur->fnHash)
		{
			cur = cur->childLeft;
		}
	}
	
	return ret;
}

void Manager::Init()
{
	// do nothing currently
}

void Manager::Update()
{
	Texture* t = headTexture;
	while(t!=0)
	{
		t->Update();
		t = t->nextTexture;
	}
	Material* mt = headMaterial;
	while(mt!=0)
	{
		mt->Update();
		mt = mt->nextMaterial;
	}
	Model* m = headModel;
	while(m!=0)
	{
		m->Update();
		m = m->nextModel;
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
