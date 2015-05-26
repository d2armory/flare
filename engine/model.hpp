#pragma once

#include "../mdl/common.h"
#include "../es/esUtil.h"
#include "enum.hpp"
#include "material.hpp"
#include <string>

#define MODEL_NAME_LENGTH 128
#define MODEL_VBO_COUNT 2

class Model
{
	
public:

	Model(const char* fileName);
	~Model();

	// function
	void Update();
	void Draw();
	
	// use on every draw call because no vao
	void SetVAO();

	// TODO: move to private and make accessor
	
	FILE_STATE state;
	
	// data
	char fileName[MODEL_NAME_LENGTH];
	char meshFileName[MODEL_NAME_LENGTH];
	char vertexFileName[MODEL_NAME_LENGTH];
	FILE_STATE mdlState;
	FILE_STATE meshState;
	FILE_STATE vertexState;
	
	mdlHeader* data;
	vvdHeader* vData;
	vtxHeader* mData;
	
	int vertexCount;
	int elementLength;
	// no vao in es2.0 :(
	//GLuint vao;
	GLuint vbo[MODEL_VBO_COUNT];
	int tangentOffset;
	
	Material* material;
	
	// manager
	Model* nextModel;

private:

	
	
};