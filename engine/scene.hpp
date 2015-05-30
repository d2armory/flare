#pragma once

#include "../common.h"
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

	static void InitFeatures();
	static void InitShadowmap();

	static RenderStep currentStep;
	
	static bool enableShadow;
	static bool enableTextureCompression;
	static bool enableVAO;
	
	static bool supportShadow;
	static bool supportTextureCompression;
	static bool supportVAO;
	
	static GLuint shadowFrameBuffer;
	static GLuint shadowDepthTexture;
	
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