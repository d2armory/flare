#pragma once

#include "../es/esUtil.h"
#include <stdio.h>
#include "../glm/glm.hpp"
#include <html5.h>

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
	
	static int drawShadow;
	static glm::vec3 lightDir;
	static float shadowMapCoverage;
	
	static glm::vec3 camPosition;
	static glm::vec3 camTarget;
	
	static float fov;
	static float screenHeight;
	static float screenWidth;
	static float nearZ;
	static float farZ;
	
};