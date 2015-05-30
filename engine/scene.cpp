#include "scene.hpp"

RenderStep Scene::currentStep = RS_SCENE;

bool Scene::supportShadow = true;
bool Scene::supportTextureCompression = true;
bool Scene::supportVAO = true;

bool Scene::enableShadow = true;
bool Scene::enableTextureCompression = true;
bool Scene::enableVAO = true;

GLuint Scene::shadowFrameBuffer = 0;
GLuint Scene::shadowDepthTexture = 0;

float Scene::shadowMapCoverage = 200.0f;

glm::vec3 Scene::camPosition = glm::vec3(0.0f,100.0f,250.0f);
glm::vec3 Scene::camTarget = glm::vec3(0.0f,100.0f,0.0f);
float Scene::fov = 45.0f;
float Scene::screenHeight = 640.0f;
float Scene::screenWidth = 960.0f;
float Scene::nearZ = 0.01f;
float Scene::farZ = 1000.0f;

glm::vec3 Scene::lightDir = glm::vec3(-1.0,-1.0,-1.0);

void Scene::InitFeatures()
{
	printf("Checking webGL extensions\n");
	Scene::supportShadow = emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(),"WEBGL_depth_texture");
	Scene::supportTextureCompression = emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(),"WEBGL_compressed_texture_s3tc");
	Scene::supportVAO = emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(),"OES_vertex_array_object");
	printf("- Shadow Map (WEBGL_depth_texture): %d\n",Scene::supportShadow);
	printf("- Compressed Texture (WEBGL_compressed_texture_s3tc): %d\n",Scene::supportTextureCompression);
	printf("- Vertex Array Object (OES_vertex_array_object): %d\n",Scene::supportVAO);
	
	Scene::enableShadow = Scene::supportShadow;
	Scene::enableTextureCompression = Scene::supportTextureCompression;
	Scene::enableVAO = Scene::supportVAO;
}

void Scene::InitShadowmap()
{
	
	if(!Scene::supportShadow) return;
	
	shadowFrameBuffer = 0;
	glGenFramebuffers(1, &shadowFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
	
	GLuint colorTexture = 0;
	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	glGenTextures(1, &shadowDepthTexture);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT, 1024, 1024, 0,GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowDepthTexture, 0);
	glFramebufferTexture2D(	GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture, 0);
	glFramebufferTexture2D(	GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
	
	
	//glDrawBuffer(GL_NONE); // No color buffer is drawn to.
	
	// Always check that our framebuffer is ok
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	printf("Shadow frameBuffer error!\n");
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}