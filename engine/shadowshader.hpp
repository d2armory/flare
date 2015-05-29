#pragma once

#include "model.hpp"
#include "shader.hpp"

// forward dec
class Shader;
class Model;

class ShadowShader
{
	
public:

	ShadowShader();

	void Load();
	void Bind(Model* model);
	void Populate(Model* model);
	void Unbind(Model* model);
	
	GLuint programLocation;
	
	GLuint locMvpTransform;
	
};