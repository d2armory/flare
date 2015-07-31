#pragma once

#include "../common.h"
#include "../mdl/common.h"
#include "enum.hpp"
#include "manager.hpp"
#include <string>
#include <math.h>
#include "half.hpp"
#include "../glm/glm.hpp"
#include "../glm/gtc/quaternion.hpp"
#include "../glm/gtx/quaternion.hpp"
#include "../glm/gtx/compatibility.hpp"

#define MODEL_ANIMATION_NAME_LENGTH 128

// forward dec
class KeyValue;
class Model;

struct BoneData
{
	glm::vec3 pos;
	glm::quat rot;
};

class ModelAnimation
{
public:
	ModelAnimation(const char* fileName);
	~ModelAnimation();
	
	void Update(ESContext *esContext, float deltaTime);
	void Draw(ESContext *esContext, Model* model);	// need model to compute inverse bind pose, and bone name mapping
	
	FILE_STATE state;
	char fileName[MODEL_ANIMATION_NAME_LENGTH];
	
	char* fileData;
	KeyValue* animRoot;
	
	Model* parent;
	
	int boneCount;
	int frameCount;
	float fps;
	float curTime;
	float speed;
	
	BoneData* frameC;	// current frame
	BoneData* frameN;	// next frame
	BoneData* frameB;	// blended frame
	
	void ExtractFrame(BoneData*& output, int frame);
	void ExtractDataFullVector(BoneData& output, const char* data);
	void ExtractDataHalfVector(BoneData& output, const char* data);
	void ExtractDataQuaternion(BoneData& output, const char* data);
	
};