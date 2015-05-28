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
	vertexShader = LoadShader ( GL_VERTEX_SHADER, (const char*) vShaderStr );
	fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, (const char*) fShaderStr );
	
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

void HeroShader::Unbind(Model* m)
{
	// Use the program object
	glUseProgram ( 0 );
}