#pragma once

#include "model.hpp"
#include "shader.hpp"
#include "scene.hpp"

// forward dec
class Shader;
class Model;

class HeroShader
{
	
public:

	HeroShader();

	bool Load();
	void Bind(Model* model, int index);
	void Populate(Model* model, int index);
	void Unbind(Model* model, int index);
	
	GLuint programLocation;
	
//	GLuint locModelTransform;
//	GLuint locViewTransform;
//	GLuint locProjTransform;
	GLuint locDepthBiasMvpTransform;
	GLuint locMvTransform;
	GLuint locMvpTransform;
	GLuint locNTransform;
	
	GLuint locLightDir;
	GLuint locTexture;
	GLuint locBoneTexture;
	GLuint locUseBoneWeight;
	
	GLuint locDrawShadow;
	
	GLuint locHqNormal;
	GLuint locIsTranslucent;
	GLuint locBlendType;
	GLuint locUseMask1;
	GLuint locUseMask2;
	GLuint locAmbientScale;
	GLuint locSpecExponent;
	GLuint locSpecScale;
	GLuint locRimScale;
	GLuint locCloakIntensity;
	
	GLuint locBoneIndex;
	GLuint locBoneTransform;
	//GLuint locBonePos;
	//GLuint locBoneRot;
	//GLuint locBoneTransform1;
	//GLuint locBoneTransform2;
	//GLuint locBoneTransform3;
	
};