#include "shadowshader.hpp"

ShadowShader::ShadowShader()
{
	locMvpTransform = 0;
}

void ShadowShader::Load()
{
	char vShaderFilename[] = "assets/shadowmap.vert";
	char fShaderFilename[] = "assets/shadowmap.frag";
	
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
	programLocation = programObject;
	
	locMvpTransform = glGetUniformLocation(programObject, "mvpTransform");
}

void ShadowShader::Bind(Model* m, int index)
{
	// Use the program object
	glUseProgram ( programLocation );
	//printf("Shader at %d binded\n",programLocation);
	
}

void ShadowShader::Populate(Model* m, int index)
{
	
	// TODO: send bone data
	
	// get these from camera
	glm::mat4 v = glm::lookAt(Scene::camPosition, Scene::camTarget, glm::vec3(0,1,0));
	
	glm::vec3 lightDir = glm::normalize(Scene::lightDir);//glm::vec3(-1.0,-10.0,-1.0);
	//lightDir = glm::mat3(v) * lightDir;
	glm::vec3 lightInvDir = lightDir * -1.0f;
	
	glm::mat4 depthProjectionMatrix = glm::ortho<float>(-Scene::shadowMapCoverage,Scene::shadowMapCoverage,-Scene::shadowMapCoverage,Scene::shadowMapCoverage,-Scene::shadowMapCoverage,Scene::shadowMapCoverage);
	glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
	
	//glm::mat4 v = glm::translate(glm::mat4(1), glm::vec3(0,-100,-200));
	//glUniformMatrix4fv(locViewTransform, 1, GL_FALSE, &v[0][0]);
	
	//glm::mat4 p = glm::perspective (45.0f, 1.5f, 0.01f, 1000.0f);
	//glUniformMatrix4fv(locProjTransform, 1, GL_FALSE, &p[0][0]);

	glm::mat4 mvp = depthProjectionMatrix * depthViewMatrix * m->modelTransform;
	
	// don't render shadow for translucent material
	if(m->subModel[index]->material != 0 && m->subModel[index]->material->translucent) mvp = glm::mat4(0);
	
	glUniformMatrix4fv(locMvpTransform, 1, GL_FALSE, &mvp[0][0]);
	

}

void ShadowShader::Unbind(Model* m, int index)
{
	// Use the program object
	glUseProgram ( 0 );
}