#pragma once

#include "../common.h"
#include "../mdl/common.h"
#include "enum.hpp"
#include "manager.hpp"
#include "material.hpp"
#include "heroshader.hpp"
#include "shadowshader.hpp"
#include "scene.hpp"
#include "modelanimation.hpp"
#include <string>
#include <math.h>
#include "half.hpp"

#define MODEL_NAME_LENGTH 128
#define MODEL_STRIP_COUNT 4

// forward dec
class KeyValue;
class ModelAnimation;

struct PackedNorm {
	int z:8;
	int y:8;
	int x:8;
	int a:8;
};

struct VertEx
{
	glm::vec3 normal;
	//glm::vec3 tangent;
	glm::vec2 uv;
};

struct ModelDrawCall
{
	GLuint VAO;	// vertex array object
	GLuint vertexVBO; // main vertex buffer object
	GLuint vertexVBOex;	// extension vertex buffer object (for runtime processed data)
	GLuint meshVBO;	// strip data
	
	unsigned int vertexCount;
	unsigned int indexCount;
	
	unsigned int vertexSize;
	unsigned int indexSize;
	
	unsigned int vtOffset_Pos;
	unsigned int vtOffset_Norm;
	unsigned int vtOffset_UV;
	unsigned int vtOffset_bIndex;
	unsigned int vtOffset_bWeight;
	
	Material* material;
	
	unsigned int boneCount;
	unsigned int* meshBoneList;
	unsigned int* meshBoneIndex;
};

class Model
{
	
public:

	Model(const char* fileName);
	~Model();

	// function
	void Update(ESContext *esContext, float deltaTime);
	void Draw(ESContext *esContext);
	
	// use on every draw call because no vao
	void SetVAO(int i);

	// TODO: move to private and make accessor
	
	glm::vec4 position;
	glm::vec4 rotation;	// euler
	
	glm::mat4 modelTransform;
	
	FILE_STATE state;
	
	// data
	char fileName[MODEL_NAME_LENGTH];
	char meshFileName[MODEL_NAME_LENGTH];
	char vagrpFileName[MODEL_NAME_LENGTH];
	FILE_STATE mdlState;
	FILE_STATE meshState;
	FILE_STATE vagrpState;
	
	char* vmdlData;
	char* vmeshData;
	char* vanimData;
	
	KeyValue* mdlRoot;
	KeyValue* meshRoot;
	
	unsigned int subModelCount;
	ModelDrawCall** subModel;
	
	int numBone;
	int* boneMap;	// only when using other model anim
	glm::mat4* boneTransform;
	glm::mat4* invBoneTransform;
	
	GLuint boneTransformTexture;	// 4*numBone Float Texture (numBone ceil to 2^n)
	
	// animation
	ModelAnimation* anim;
	bool useAnimation;
	Model* animParent;	// use animation from which model, compute mapping on this too
	
	HeroShader* shader;
	ShadowShader* shaderShadow;
	
	// manager
	Model* nextModel;
	
	void SetAnimationParent(Model* mdlanimp);

private:

	static void ExtractAnimValue( int frame, mdlAnimValue *panimvalue, float scale, float &v1);
	
};