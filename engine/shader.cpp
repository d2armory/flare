#include "shader.hpp"

GLuint Shader::LoadShader( GLenum type, const char *shaderSrc )
{
	GLuint shader;
	GLint compiled;
	
	// Create the shader object
	shader = glCreateShader ( type );

	if ( shader == 0 )
		return 0;

	// Load the shader source
	glShaderSource ( shader, 1, &shaderSrc, 0 );
	
	// Compile the shader
	glCompileShader ( shader );

	// Check the compile status
	glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

	if ( !compiled ) 
	{
		GLint infoLen = 0;

		glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
		
		if ( infoLen > 1 )
		{
			char* infoLog = (char*) malloc (sizeof(char) * infoLen );

			glGetShaderInfoLog ( shader, infoLen, 0, infoLog );
			esLogMessage ( "Error compiling shader %s :\n%s\n",shaderSrc, infoLog );            
			
			free ( infoLog );
		}

		glDeleteShader ( shader );
		return 0;
	}

	return shader;
}