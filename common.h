#pragma once

#include "gl.h"

// GLM Common include
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

// User data to pass around
class HeroShader;
class ShadowShader;
typedef struct
{
	// Handle to a program object
	HeroShader* heroShader;
	ShadowShader* shadowShader;
	// Rotation
	float deg;

} UserData;

// Artifact from EGL Code

#define ESCALLBACK

// Context
typedef struct _escontext
{
	// User data Reference
	void* userData;
	
	// GL window ref
	GLFWwindow* m_window;
	
	// Callbacks
	void (ESCALLBACK *drawFunc) ( struct _escontext * );
	void (ESCALLBACK *keyFunc) ( struct _escontext *, unsigned char, int, int );
	void (ESCALLBACK *updateFunc) ( struct _escontext *, float deltaTime );
} ESContext;