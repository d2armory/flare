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
	GLuint locDrawShadow;
	GLuint locHqNormal;
	
	GLuint locBoneIndex;
	//GLuint locBonePos;
	//GLuint locBoneRot;
	GLuint locBoneTransform;
	
};