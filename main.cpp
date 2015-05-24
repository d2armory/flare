//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// Hello_Triangle.c
//
//    This is a simple example that draws a single triangle with
//    a minimal vertex/fragment shader.  The purpose of this 
//    example is to demonstrate the basic concepts of 
//    OpenGL ES 2.0 rendering.
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <emscripten.h>

#include "es/esUtil.h"
#include <sys/time.h>

#include "engine/common.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

const char* testName = "materials/models/heroes/chen/chen_color.vtf";

typedef struct
{
	// Handle to a program object
	GLuint programObject;
	GLuint rotateLocation;
	GLuint textureLocation;
	float deg;

} UserData;

///
// Create a shader object, load the shader source, and
// compile the shader.
//

GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
	GLuint shader;
	GLint compiled;
	
	// Create the shader object
	shader = glCreateShader ( type );

	if ( shader == 0 )
		return 0;

	// Load the shader source
	glShaderSource ( shader, 1, &shaderSrc, NULL );
	
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

			glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
			esLogMessage ( "Error compiling shader:\n%s\n", infoLog );            
			
			free ( infoLog );
		}

		glDeleteShader ( shader );
		return 0;
	}

	return shader;

}

///
// Initialize the shader and program object
//

int Init ( ESContext *esContext )
{
	
	Manager::Init();
	
	esContext->userData = (char*) malloc(sizeof(UserData));

	UserData *userData = (UserData*) esContext->userData;
	
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
		return 0;

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
			esLogMessage ( "Error linking program:\n%s\n", infoLog );            
			
			free ( infoLog );
		}

		glDeleteProgram ( programObject );
		return GL_FALSE;
	}

	// Store the program object
	userData->programObject = programObject;
	userData->rotateLocation = glGetUniformLocation(programObject, "modelTransform");
	userData->textureLocation = glGetUniformLocation(programObject, "texture");
	userData->deg = 0;
	
	printf("%d\n",userData->rotateLocation);

	glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
	
	
	Texture* t1 = new Texture(testName);
	Manager::add(t1);
	//Texture* t2 = new Texture("materials/models/heroes/axe/axe_armor_normal.vtf");
	//Manager::add(t2);
	//Texture* t3 = new Texture("materials/models/heroes/axe/axe_armor_masks1.vtf");
	//Manager::add(t3);
	//Texture* t4 = new Texture("materials/models/heroes/axe/axe_armor_masks2.vtf");
	//Manager::add(t4);
	
	return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//

void Update ( ESContext *esContext, float deltaTime )
{
	Manager::Update();
}

void Draw ( ESContext *esContext )
{
	UserData *userData = (UserData*) esContext->userData;
	/* GLfloat vVertices[] = {  
		0.0f,  0.5f, 0.0f, 
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f 
	};
	*/
	
	GLfloat vVertices[] = {  
		-0.5f,  0.5f, 0.0f, 
		-0.5f, -0.5f, 0.0f,
		0.5f,  -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f, 
		0.5f,  -0.5f, 0.0f, 
		0.5f,  0.5f, 0.0f,
	};

	// No clientside arrays, so do this in a webgl-friendly manner
	GLuint vertexPosObject;
	glGenBuffers(1, &vertexPosObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
	glBufferData(GL_ARRAY_BUFFER, 18*4, vVertices, GL_STATIC_DRAW);
	
	// Set the viewport
	glViewport ( 0, 0, esContext->width, esContext->height );
	
	// Clear the color buffer
	glClear ( GL_COLOR_BUFFER_BIT );

	// Use the program object
	glUseProgram ( userData->programObject );

	// Load the vertex data
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
	glVertexAttribPointer(0 /* ? */, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(0);
	
	glm::mat4 m = glm::rotate(glm::mat4(1),userData->deg,glm::vec3(0,0,1));
	glUniformMatrix4fv(userData->rotateLocation, 1, GL_FALSE, &m[0][0]);
	
	userData->deg += M_PI/90 / 5;
	
	//printf("%f \n",userData->deg);

	Manager::find(testName)->Bind(0);
	
	glUniform1i(userData->textureLocation, 0);

	glDrawArrays ( GL_TRIANGLES, 0, 6 );
	
	Manager::find(testName)->Unbind(0);
}

ESContext esContext;
UserData  userData;

struct timeval t1, t2;
struct timezone tz;
float deltatime;
float totaltime = 0.0f;
unsigned int frames = 0;

bool axe_data = false;

void mainloop()
{
	
	gettimeofday(&t2, &tz);
	deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
	t1 = t2;

	esMainLoop(&esContext);

	totaltime += deltatime;
	frames++;
	
	const float fpsRefresh = 5.0f;
	
	if (totaltime >  fpsRefresh)
	{
		printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames/totaltime);
		totaltime -= fpsRefresh;
		frames = 0;
	}
	
	if(!axe_data) 
	{
		bool ex = FileLoader::FileExist("models/heroes/axe/axe.mdl");
		if(ex)
		{
			axe_data = true;
			char* fileData = FileLoader::ReadFile("models/heroes/axe/axe.mdl");

			printf("Apply header on data\n");

			mdlHeader* mh = (mdlHeader*) fileData;
			
			char* mhId = (char*) &mh->id;
			
			printf("- ID: %c%c%c%c (0x%X)\n",*mhId,*(mhId+1),*(mhId+2),*(mhId+3),mh->id);
			printf("- Version: %d\n",mh->version);
			printf("- Checksum: 0x%X\n",mh->checksum);
			printf("- Name: %s\n",mh->name);
			printf("- Length: %d\n",mh->length);
			
			free(fileData);
		}
	}
	
}

int main ( int argc, char *argv[] )
{
	 
	 glm::vec4 v(1,2,3,4);
	 v += glm::vec4(-10,10,100,-100);
	 
	 printf("%f %f %f %f\n",v.x,v.y,v.z,v.w); 
	 
	 glm::mat4 m = glm::rotate( glm::mat4(1.0f), (float) M_PI / 2 , glm::vec3(-1.0f, 0.0f, 0.0f));
	 
	 v = v * m;
	 
	 printf("%f %f %f %f\n",v.x,v.y,v.z,v.w); 

	esInitContext ( &esContext );
	esContext.userData = &userData;

	esCreateWindow ( &esContext, "Hello Triangle", 640, 640, ES_WINDOW_RGB );

	if ( !Init ( &esContext ) )
		return 0;

	esRegisterUpdateFunc ( &esContext, Update );

	esRegisterDrawFunc ( &esContext, Draw );

	//esMainLoop ( &esContext );
	
	gettimeofday ( &t1 , &tz );
	
	FileLoader::Load("models/heroes/axe/axe.mdl");
	
	emscripten_set_main_loop(mainloop, -1,0);
	
	
}