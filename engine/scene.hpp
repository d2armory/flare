#pragma once

#include "../es/esUtil.h"
#include <stdio.h>
#include "../glm/glm.hpp"

enum RenderStep
{
	RS_SCENE,
	RS_SHADOW
};

class Scene
{
public:
	static RenderStep currentStep;
	
	static GLuint shadowFrameBuffer;
	static GLuint shadowDepthTexture;
	static void InitShadowmap();
	
	static glm::vec3 lightDir;
	
};