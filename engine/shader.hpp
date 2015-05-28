#pragma once

//#include "model.hpp"
#include "../es/esUtil.h"
#include <stdlib.h>

// forward dec
class Model;

class Shader
{
	
public:
	
	virtual void Load() {};
	virtual void Bind(Model* model) {};
	virtual void Unbind(Model* model) {};
	
	GLuint LoadShader ( GLenum type, const char *shaderSrc );
};