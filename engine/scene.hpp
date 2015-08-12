#pragma once

#include "../common.h"
#include <stdio.h>
#include "../glm/glm.hpp"
#include "manager.hpp"
#include <html5.h>

enum RenderStep
{
	RS_SCENE,
	RS_SHADOW
};

// forward dec
class Texture;

class Scene
{
public:

	static void InitFeatures();
	static void InitDefaultTextures();
	static void InitFinalRender();
	static void InitShadowmap();
	static void FinalRender();

	static RenderStep currentStep;
	
	static bool enableShadow;
	static bool enableTextureCompression;
	static bool enableVAO;
	static bool enableFragDepth;
	static bool enableFragTangent;
	static bool enableFloatTexture;
	
	static bool supportShadow;
	static bool supportTextureCompression;
	static bool supportVAO;
	static bool supportFragDepth;
	static bool supportFragTangent;
	static bool supportFloatTexture;
	
	static GLuint shadowFrameBuffer;
	static GLuint shadowDepthTexture;
	static GLuint shadowColorTexture;
	
	static GLuint finalRenderFrameBuffer;
	static GLuint finalRenderDepthTexture;
	static GLuint finalRenderColorTexture;
	static GLuint finalRenderShaderProgram;
	static GLuint finalRenderLocTexture;
	static GLuint finalRenderVBO;
	
	static glm::vec3 lightDir;
	static float shadowMapCoverage;
	
	static glm::vec3 camXAxis;
	static glm::vec3 camRotationSpeed;
	static glm::vec3 camRotationAcc;
	
	static glm::vec3 camPosition;
	static glm::vec3 camTarget;
	
	static float fov;
	static float screenHeight;
	static float screenWidth;
	static float nearZ;
	static float farZ;
	
	static float shadowMapHeight;
	static float shadowMapWidth;
	
	static Texture* defaultDiffuse;
	static Texture* defaultNormal;
	static Texture* defaultMask1;
	static Texture* defaultMask2;
	
};