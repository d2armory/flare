#pragma once

#include "model.hpp"
#include "shader.hpp"

// forward dec
class Shader;
class Model;

class HeroShader : public Shader
{
	
public:

	HeroShader();

	void Load();
	void Bind(Model* model);
	void Unbind(Model* model);
	
	GLuint programLocation;
	
	GLuint locModelTransform;
	GLuint locViewTransform;
	GLuint locProjTransform;
	GLuint locTexture;
	
};