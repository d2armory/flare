#include "heroshader.hpp"

HeroShader::HeroShader()
{
	locModelTransform = 0;
	locViewTransform = 0;
	locProjTransform = 0;
	locTexture = 0;
}

void HeroShader::Load()
{
	char vShaderFilename[] = "assets/shader.vert";
	char fShaderFilename[] = "assets/shader.frag";
	
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
	glBindAttribLocation ( programObject, 1, "vNormal" );
	glBindAttribLocation ( programObject, 2, "vUV" );
	glBindAttribLocation ( programObject, 3, "vBoneCount" );
	glBindAttribLocation ( programObject, 4, "vBone1" );
	glBindAttribLocation ( programObject, 5, "vBone2" );
	glBindAttribLocation ( programObject, 6, "vBone3" );
	glBindAttribLocation ( programObject, 7, "vBoneweight1" );
	glBindAttribLocation ( programObject, 8, "vBoneweight2" );
	glBindAttribLocation ( programObject, 9, "vBoneweight3" );
	glBindAttribLocation ( programObject, 10, "vTangent" );

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
	programLocation = programObject;
	
	locModelTransform = glGetUniformLocation(programObject, "modelTransform");
	locViewTransform = glGetUniformLocation(programObject, "viewTransform");
	locProjTransform = glGetUniformLocation(programObject, "projTransform");
	
	locMvTransform = glGetUniformLocation(programObject, "mvTransform");
	locMvpTransform = glGetUniformLocation(programObject, "mvpTransform");
	locNTransform = glGetUniformLocation(programObject, "nTransform");
	
	locDepthBiasMvpTransform = glGetUniformLocation(programLocation, "depthBiasMvpTransform");
	locLightDir = glGetUniformLocation(programObject, "lightDir");
	locTexture = glGetUniformLocation(programObject, "texture");
	locDrawShadow = glGetUniformLocation(programObject, "drawShadow");
}

void HeroShader::Bind(Model* m)
{
	// Use the program object
	glUseProgram ( programLocation );
	//printf("Shader at %d binded\n",programLocation);
	
}

void HeroShader::Populate(Model* m)
{
	
	// debug flag: draw scene in light PoV
	bool renderInLightSpace = false;
	
	// normal MVP and related matrix binding
	glUniformMatrix4fv(locModelTransform, 1, GL_FALSE, &m->modelTransform[0][0]);
	glm::mat4 v = glm::lookAt(Scene::camPosition, Scene::camTarget, glm::vec3(0,1,0));
	glm::mat4 p = glm::perspective (Scene::fov, Scene::screenWidth/Scene::screenHeight, Scene::nearZ, Scene::farZ);
	if(!renderInLightSpace) 
	{
		glUniformMatrix4fv(locViewTransform, 1, GL_FALSE, &v[0][0]);
		glUniformMatrix4fv(locProjTransform, 1, GL_FALSE, &p[0][0]);
		glm::mat4 mv = v * m->modelTransform;
		glm::mat4 mvp = p * mv;
		glm::mat3 nt = glm::mat3(mv);
		glUniformMatrix4fv(locMvTransform, 1, GL_FALSE, &mv[0][0]);
		glUniformMatrix4fv(locMvpTransform, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix3fv(locNTransform, 1, GL_FALSE, &nt[0][0]);
	}
	
	// Light
	glm::vec3 lightDir = glm::normalize(Scene::lightDir);//glm::vec3(-1.0,-10.0,-1.0);
	lightDir = glm::mat3(v) * lightDir;
	if(!renderInLightSpace) glUniform3fv(locLightDir, 1, &lightDir[0] );
	
	// Shadowmap Transform
	glm::vec3 lightInvDir = lightDir * -1.0f;
	glm::mat4 depthProjectionMatrix = glm::ortho<float>(-Scene::shadowMapCoverage,Scene::shadowMapCoverage,-Scene::shadowMapCoverage,Scene::shadowMapCoverage,-Scene::shadowMapCoverage,Scene::shadowMapCoverage);
	glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 depthMvp = depthProjectionMatrix * depthViewMatrix * m->modelTransform;
	glm::mat4 biasMatrix(1);
	// glm matrix is left to right
	//biasMatrix = glm::scale(biasMatrix,glm::vec3(Scene::screenWidth/1024.0f,Scene::screenHeight/1024.0f,1.0f));
	biasMatrix = glm::translate(biasMatrix,glm::vec3(0.5f,0.5f,0.5f));
	biasMatrix = glm::scale(biasMatrix,glm::vec3(0.5f,0.5f,0.5f));
	glm::mat4 depthBiasMvp = biasMatrix * depthMvp;
	glUniformMatrix4fv(locDepthBiasMvpTransform, 1, GL_FALSE, &depthBiasMvp[0][0]);
	int drawShadow = Scene::enableShadow;
	glUniform1iv(locDrawShadow, 1, &drawShadow);
	
	// debug mode code
	lightDir = glm::vec3(0,0,-1);//glm::mat3(v) * lightDir;
	if(renderInLightSpace) 
	{
		glUniform3fv(locLightDir, 1, &lightDir[0] );
		glUniformMatrix4fv(locViewTransform, 1, GL_FALSE, &depthViewMatrix[0][0]);
		glUniformMatrix4fv(locProjTransform, 1, GL_FALSE, &depthProjectionMatrix[0][0]);
		glm::mat4 mv = depthViewMatrix * m->modelTransform;
		glm::mat4 mvp = depthProjectionMatrix * mv;
		glm::mat3 nt = glm::mat3(mv);
		glUniformMatrix4fv(locMvTransform, 1, GL_FALSE, &mv[0][0]);
		glUniformMatrix4fv(locMvpTransform, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix3fv(locNTransform, 1, GL_FALSE, &nt[0][0]);
	}
	
	// texture binding
	if(m->material != 0)
	{
		m->material->Bind();
	}
	// bind shadowmap
	if(Scene::enableShadow)
	{
		glActiveTexture(GL_TEXTURE0 + 4);
		glBindTexture(GL_TEXTURE_2D, Scene::shadowDepthTexture);
	}
	const GLint samplers[5] = {0,1,2,3,4}; // we've bound our textures in textures 0 and 1.
	glUniform1iv( locTexture, 5, samplers );
}

void HeroShader::Unbind(Model* m)
{
	
	// unbind texture
	if(m->material != 0)
	{
		m->material->Unbind();
	}
	//unbind shadow map
	if(Scene::enableShadow)
	{
		glActiveTexture(GL_TEXTURE0 + 4);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	// unbind shader
	glUseProgram ( 0 );
}