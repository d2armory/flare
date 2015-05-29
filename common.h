#pragma once

// glm common include
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// user data to pass around
class HeroShader;
class ShadowShader;
typedef struct
{
	// Handle to a program object
	HeroShader* heroShader;
	ShadowShader* shadowShader;
	float deg;

} UserData;