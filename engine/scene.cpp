#include "scene.hpp"

// need texture code
#include "texture.hpp"

RenderStep Scene::currentStep = RS_SCENE;

bool Scene::supportShadow = true;
bool Scene::supportTextureCompression = true;
bool Scene::supportVAO = true;
bool Scene::supportFragDepth = true;
bool Scene::supportFragTangent = true;

bool Scene::enableShadow = true;
bool Scene::enableTextureCompression = true;
bool Scene::enableVAO = true;
bool Scene::enableFragDepth = true;
bool Scene::enableFragTangent = true;

GLuint Scene::shadowFrameBuffer = 0;
GLuint Scene::shadowDepthTexture = 0;
GLuint Scene::shadowColorTexture = 0;

GLuint Scene::finalRenderFrameBuffer = 0;
GLuint Scene::finalRenderDepthTexture = 0;
GLuint Scene::finalRenderColorTexture = 0;
GLuint Scene::finalRenderShaderProgram = 0;
GLuint Scene::finalRenderLocTexture = 0;
GLuint Scene::finalRenderVBO = 0;

float Scene::shadowMapCoverage = 200.0f;

glm::vec3 Scene::camPosition = glm::vec3(0.0f,100.0f,250.0f);
glm::vec3 Scene::camTarget = glm::vec3(0.0f,100.0f,0.0f);
float Scene::fov = 45.0f;
float Scene::screenHeight = 640.0f;
float Scene::screenWidth = 960.0f;
float Scene::nearZ = 0.01f;
float Scene::farZ = 1000.0f;

glm::vec3 Scene::lightDir = glm::vec3(-1.0,-1.0,-1.0);

Texture* Scene::defaultDiffuse;
Texture* Scene::defaultNormal;
Texture* Scene::defaultMask1;
Texture* Scene::defaultMask2;

void Scene::InitFeatures()
{
	printf("Checking webGL extensions\n");
	Scene::supportShadow = emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(),"WEBGL_depth_texture");
	Scene::supportTextureCompression = emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(),"WEBGL_compressed_texture_s3tc");
	Scene::supportVAO = emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(),"OES_vertex_array_object");
	Scene::supportFragDepth = emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(),"EXT_frag_depth");
	Scene::supportFragTangent = emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(),"OES_standard_derivatives");
	
	printf("- Shadow Map (WEBGL_depth_texture): %d\n",Scene::supportShadow);
	printf("- Compressed Texture (WEBGL_compressed_texture_s3tc): %d\n",Scene::supportTextureCompression);
	printf("- Vertex Array Object (OES_vertex_array_object): %d\n",Scene::supportVAO);
	printf("- Fragment depth test (EXT_frag_depth): %d\n",Scene::supportFragDepth);
	printf("- Tangent calculation (OES_standard_derivatives): %d\n",Scene::supportFragTangent);
	
	Scene::enableShadow = Scene::supportShadow;
	Scene::enableTextureCompression = Scene::supportTextureCompression;
	Scene::enableVAO = Scene::supportVAO;
	Scene::enableFragDepth = Scene::supportFragDepth;
	Scene::enableFragTangent = Scene::supportFragTangent;
}

void Scene::InitDefaultTextures()
{
	Manager::add(Scene::defaultDiffuse = new Texture("assets/button_blank_64_grey_psd_53f54ea1.vtex_c"));
	Manager::add(Scene::defaultNormal = new Texture("assets/flatnormal_normal_psd_760f0359.vtex_c"));
	Manager::add(Scene::defaultMask1 = new Texture("assets/blankmasks1_selfillummask_tga_ac873b92.vtex_c"));
	Manager::add(Scene::defaultMask2 = new Texture("assets/blankmasks2_rimmask_tga_2fbf6342.vtex_c"));
}

void Scene::InitFinalRender()
{
	
	// frame buffer
	
	finalRenderFrameBuffer = 0;
	glGenFramebuffers(1, &finalRenderFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, finalRenderFrameBuffer);

	glGenTextures(1, &finalRenderColorTexture);
	glBindTexture(GL_TEXTURE_2D, finalRenderColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, floor(Scene::screenWidth), floor(Scene::screenHeight), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	glGenTextures(1, &finalRenderDepthTexture);
	glBindTexture(GL_TEXTURE_2D, finalRenderDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT, floor(Scene::screenWidth), floor(Scene::screenHeight), 0,GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowDepthTexture, 0);
	glFramebufferTexture2D(	GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, finalRenderDepthTexture, 0);
	glFramebufferTexture2D(	GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalRenderColorTexture, 0);
	
	
	//glDrawBuffer(GL_NONE); // No color buffer is drawn to.
	
	// Always check that our framebuffer is ok
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	printf("Shadow frameBuffer error!\n");
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// shader
	
	char vShaderFilename[] = "assets/finalshader.vert";
	char fShaderFilename[] = "assets/finalshader.frag";
	
	unsigned int vShaderLength = 0;
	unsigned int fShaderLength = 0;
	char* vShaderStr = FileLoader::ReadFile(vShaderFilename, vShaderLength);
	char* fShaderStr = FileLoader::ReadFile(fShaderFilename, fShaderLength);
	
	//printf("%s\n",vShaderStr);

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint programObject;
	GLint linked;

	// Load the vertex/fragment shaders
	vertexShader = Shader::LoadShader ( GL_VERTEX_SHADER, (const char*) vShaderStr, vShaderLength);
	fragmentShader = Shader::LoadShader ( GL_FRAGMENT_SHADER, (const char*) fShaderStr, fShaderLength);
	
	free(vShaderStr);
	free(fShaderStr);

	// Create the program object
	programObject = glCreateProgram ( );
	
	if ( programObject == 0 )
		return;

	glAttachShader ( programObject, vertexShader );
	glAttachShader ( programObject, fragmentShader );

	// Bind vPosition to attribute 0   
	glBindAttribLocation ( programObject, 0, "vPosition" );

	// Link the program
	glLinkProgram ( programObject );

	// Check the link status
	glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

	if ( !linked ) 
	{
		GLint infoLen = 0;

		glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
		
		if ( infoLen > 1 )
		{
			char* infoLog = (char*) malloc (sizeof(char) * infoLen );

			glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
			printf("Error linking program:\n%s\n",infoLog);           
			
			free ( infoLog );
		}

		glDeleteProgram ( programObject );
		return;// GL_FALSE;
	}

	// Store the program object
	finalRenderShaderProgram = programObject;
	finalRenderLocTexture = glGetUniformLocation(programObject, "texture");
	
	// VBO data
	float vbod[12] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 0.0f
	};
	
	glGenBuffers(1, (GLuint*) &finalRenderVBO);
	glBindBuffer(GL_ARRAY_BUFFER, finalRenderVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12 , &vbod, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Scene::FinalRender()
{
	glUseProgram(finalRenderShaderProgram);
	glBindBuffer(GL_ARRAY_BUFFER, finalRenderVBO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, finalRenderColorTexture);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*) 0);
	glEnableVertexAttribArray(0);
	const GLint samplers[1] = {0};
	glUniform1iv( finalRenderLocTexture, 1, samplers );
	glDrawArrays( GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void Scene::InitShadowmap()
{
	
	if(!Scene::supportShadow) return;
	
	shadowFrameBuffer = 0;
	glGenFramebuffers(1, &shadowFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);

	glGenTextures(1, &shadowColorTexture);
	glBindTexture(GL_TEXTURE_2D, shadowColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, floor(Scene::screenWidth), floor(Scene::screenHeight), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	glGenTextures(1, &shadowDepthTexture);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT, floor(Scene::screenWidth), floor(Scene::screenHeight), 0,GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowDepthTexture, 0);
	glFramebufferTexture2D(	GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture, 0);
	glFramebufferTexture2D(	GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowColorTexture, 0);
	
	
	//glDrawBuffer(GL_NONE); // No color buffer is drawn to.
	
	// Always check that our framebuffer is ok
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	printf("Shadow frameBuffer error!\n");
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}