#pragma once

#include "../common.h"
#include "../mdl/common.h"
#include "../es/esUtil.h"
#include "enum.hpp"
#include "manager.hpp"
#include "material.hpp"
#include "heroshader.hpp"
#include "shadowshader.hpp"
#include "scene.hpp"
#include <string>


#define MODEL_NAME_LENGTH 128
#define MODEL_STRIP_COUNT 4

class Model
{
	
public:

	Model(const char* fileName);
	~Model();

	// function
	void Update(ESContext *esContext, float deltaTime);
	void Draw(ESContext *esContext);
	
	// use on every draw call because no vao
	void SetVAO();

	// TODO: move to private and make accessor
	
	glm::vec4 position;
	glm::vec4 rotation;
	
	glm::mat4 modelTransform;
	
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
	// no vao in es2.0 :(
	//GLuint vao;
	GLuint vertexVBO[2];
	int numStrip;
	GLuint meshVBO[MODEL_STRIP_COUNT];
	int elementLength[MODEL_STRIP_COUNT];
	int tangentOffset;
	
	HeroShader* shader;
	ShadowShader* shaderShadow;
	Material* material;
	
	// manager
	Model* nextModel;

private:

	
	
};