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

	void Load();
	void Bind(Model* model);
	void Populate(Model* model);
	void Unbind(Model* model);
	
	GLuint programLocation;
	
	GLuint locModelTransform;
	GLuint locViewTransform;
	GLuint locProjTransform;
	GLuint locDepthBiasMvpTransform;
	
	GLuint locLightDir;
	GLuint locTexture;
	
};