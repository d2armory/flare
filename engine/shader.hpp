#pragma once

//#include "model.hpp"
#include "../common.h"
#include <stdlib.h>
#include <stdio.h>

class Shader
{
	
public:
	static GLuint LoadShader ( GLenum type, const char *shaderSrc, unsigned int length );
};