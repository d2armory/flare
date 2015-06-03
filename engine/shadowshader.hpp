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
	void Bind(Model* model, int index);
	void Populate(Model* model, int index);
	void Unbind(Model* model, int index);
	
	GLuint programLocation;
	
	GLuint locMvpTransform;
	
};