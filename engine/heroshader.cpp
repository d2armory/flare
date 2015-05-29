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
	
	char* vShaderStr = FileLoader::ReadFile(vShaderFilename);
	char* fShaderStr = FileLoader::ReadFile(fShaderFilename);
	
	//printf("%s\n",vShaderStr);

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint programObject;
	GLint linked;

	// Load the vertex/fragment shaders
	vertexShader = Shader::LoadShader ( GL_VERTEX_SHADER, (const char*) vShaderStr );
	fragmentShader = Shader::LoadShader ( GL_FRAGMENT_SHADER, (const char*) fShaderStr );
	
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
	
	// get these from camera
	glm::mat4 v = glm::translate(glm::mat4(1), glm::vec3(0,-100,-200));
	glUniformMatrix4fv(locViewTransform, 1, GL_FALSE, &v[0][0]);
	glm::mat4 p = glm::perspective (45.0f, 1.5f, 0.01f, 1000.0f);
	glUniformMatrix4fv(locProjTransform, 1, GL_FALSE, &p[0][0]);
	
	if(m->material != 0)
	{
		m->material->Bind();
	}
	
	const GLint samplers[4] = {0,1,2,3}; // we've bound our textures in textures 0 and 1.
	glUniform1iv( locTexture, 4, samplers );
}

void HeroShader::Unbind(Model* m)
{
	
	if(m->material != 0)
	{
		m->material->Unbind();
	}
	
	// Use the program object
	glUseProgram ( 0 );
}