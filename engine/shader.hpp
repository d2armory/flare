#pragma once

//#include "model.hpp"
#include "../es/esUtil.h"
#include <stdlib.h>
#include <stdio.h>

class Shader
{
	
public:
	static GLuint LoadShader ( GLenum type, const char *shaderSrc );
};