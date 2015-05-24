
#include "model.hpp"

Model::Model()
{
	
}

Model::~Model()
{
	if(data) delete data;
	//if(vData) delete vData;
}

void Model::Update()
{
	if(state==FS_UNINIT)
	{
		state = FS_LOADING;
	}
	if(state==FS_LOADING)
	{
		
	}
}

void Model::Draw()
{
	
}