#pragma once

#include "../common.h"
#include "../mdl/common.h"
#include "enum.hpp"
#include "manager.hpp"
#include "material.hpp"
#include "heroshader.hpp"
#include "shadowshader.hpp"
#include "scene.hpp"
#include <string>
#include <math.h>


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
	glm::vec4 rotation;	// euler
	
	glm::mat4 modelTransform;
	
	FILE_STATE state;
	
	// data
	char fileName[MODEL_NAME_LENGTH];
	char meshFileName[MODEL_NAME_LENGTH];
	char vertexFileName[MODEL_NAME_LENGTH];
	FILE_STATE mdlState;
	FILE_STATE meshState;
	FILE_STATE vertexState;
	
	mdlHeader* data;	// kept for animation data
	vvdHeader* vData;
	vtxHeader* mData;
	
	int vertexCount;
	
	GLuint vao;
	GLuint vertexVBO[2];
	int tangentOffset;
	
	int numStrip;
	GLuint meshVBO[MODEL_STRIP_COUNT];
	int elementLength[MODEL_STRIP_COUNT];
	int meshBoneCount[MODEL_STRIP_COUNT];
	unsigned int* meshBoneList[MODEL_STRIP_COUNT];	// to use in data preparation, 53 int for each strip (map to bonePos/boneRot)
	unsigned int* meshBoneIndex[MODEL_STRIP_COUNT]; // to use in vert shader, 128 int for each strip
	
	int numBone;
	glm::vec3* bonePos;	// array of position and rotation (in quat)
	glm::quat* boneRot;
	glm::mat4* boneTransform;
	
	// animation
	bool posePrepared;
	bool useAnimation;
	
	int curFrame;
	float frameTime;
	
	HeroShader* shader;
	ShadowShader* shaderShadow;
	Material* material;
	
	// manager
	Model* nextModel;

private:

	static void ExtractAnimValue( int frame, mdlAnimValue *panimvalue, float scale, float &v1);
	
};