#pragma once

#include "../mdl/common.h"
#include "enum.hpp"
#include "material.hpp"

#define MODEL_NAME_LENGTH 128

class Model
{
	
public:

	Model();
	~Model();

	// function
	void Update();
	void Draw();

	// TODO: move to private and make accessor
	// data
	char fileName[MODEL_NAME_LENGTH];
	FILE_STATE state;
	mdlHeader* data;
	//mdlVertex* vData;
	
	Material* material;
	
	// manager
	Model* nextModel;

private:

	
};