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
			esLogMessage ( "Error linking program:\n%s\n", infoLog );            
			
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
	locDepthBiasMvpTransform = glGetUniformLocation(programLocation, "depthBiasMvpTransform");
	locLightDir = glGetUniformLocation(programObject, "lightDir");
	locTexture = glGetUniformLocation(programObject, "texture");
}

void HeroShader::Bind(Model* m)
{
	// Use the program object
	glUseProgram ( programLocation );
	//printf("Shader at %d binded\n",programLocation);
	
}

void HeroShader::Populate(Model* m)
{
	
	//UserData *userData = (UserData*) esContext->userData;
	//glm::mat4 m0 = glm::rotate(glm::mat4(1),(float) M_PI,glm::vec3(1,0,0));
	//glm::mat4 m1 = glm::rotate(glm::mat4(1),userData->deg,glm::vec3(0,1,0));
	//glm::mat4 m2 = glm::translate(m1, glm::vec3(0,0,0));
	
	glUniformMatrix4fv(locModelTransform, 1, GL_FALSE, &m->modelTransform[0][0]);
	
	glm::mat4 v = glm::translate(glm::mat4(1), glm::vec3(0,-100,-250));
	glm::mat4 p = glm::perspective (45.0f, 1.5f, 0.01f, 1000.0f);
	
	// shadow map
	glm::vec3 lightDir = glm::normalize(Scene::lightDir);//glm::vec3(-1.0,-10.0,-1.0);
	lightDir = glm::mat3(v) * lightDir;
	glUniform3fv(locLightDir, 1, &lightDir[0] );
	glm::vec3 lightInvDir = lightDir * -1.0f;
	
	glm::mat4 depthProjectionMatrix = glm::ortho<float>(-200,200,-200,200,-200,200);
	glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 depthMvp = depthProjectionMatrix * depthViewMatrix * m->modelTransform;
	glm::mat4 biasMatrix(1);
	// use screen size here?
	biasMatrix = glm::scale(biasMatrix,glm::vec3(960.0f/1024.0f,640.0f/1024.0f,1.0f));
	biasMatrix = glm::translate(biasMatrix,glm::vec3(0.5f,0.5f,0.5f));
	biasMatrix = glm::scale(biasMatrix,glm::vec3(0.5f,0.5f,0.5f));
	glm::mat4 depthBiasMvp = biasMatrix * depthMvp;
	glUniformMatrix4fv(locDepthBiasMvpTransform, 1, GL_FALSE, &depthBiasMvp[0][0]);
	
	// get these from camera
	
	//lightDir = glm::normalize(Scene::lightDir);//glm::vec3(-1.0,-10.0,-1.0);
	//lightDir = glm::vec3(0,0,-1);//glm::mat3(v) * lightDir;
	//glUniform3fv(locLightDir, 1, &lightDir[0] );
	glUniformMatrix4fv(locViewTransform, 1, GL_FALSE, &v[0][0]);
	glUniformMatrix4fv(locProjTransform, 1, GL_FALSE, &p[0][0]);
	
	if(m->material != 0)
	{
		m->material->Bind();
	}
	
	// bind shadowmap
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, Scene::shadowDepthTexture);
	
	const GLint samplers[5] = {0,1,2,3,4}; // we've bound our textures in textures 0 and 1.
	glUniform1iv( locTexture, 5, samplers );
}

void HeroShader::Unbind(Model* m)
{
	
	if(m->material != 0)
	{
		m->material->Unbind();
	}
	
	//unbind shadow map
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Use the program object
	glUseProgram ( 0 );
}