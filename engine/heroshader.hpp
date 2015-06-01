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
	void Bind(Model* model);
	void Populate(Model* model, int stripGroupIdx);
	void Unbind(Model* model);
	
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
	
	GLuint locBoneIndex;
	//GLuint locBonePos;
	//GLuint locBoneRot;
	GLuint locBoneTransform;
	
};