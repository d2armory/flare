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

const char* testName = "materials/models/heroes/axe/axe_body_color.vtf";

typedef struct
{
	// Handle to a program object
	GLuint programObject;
	
	GLuint modelTransform;
	GLuint viewTransform;
	GLuint projTransform;
	
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
			esLogMessage ( "Error compiling shader %s :\n%s\n",shaderSrc, infoLog );            
			
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
		return GL_FALSE;
	}

	// Store the program object
	userData->programObject = programObject;
	
	userData->modelTransform = glGetUniformLocation(programObject, "modelTransform");
	userData->viewTransform = glGetUniformLocation(programObject, "viewTransform");
	userData->projTransform = glGetUniformLocation(programObject, "projTransform");
	
	userData->textureLocation = glGetUniformLocation(programObject, "texture");
	userData->deg = 0;
	
	//printf("%d\n",userData->rotateLocation);

	glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
	
	return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//

ESContext esContext;
UserData  userData;

struct timeval t1, t2;
struct timezone tz;
float deltatime;
float totaltime = 0.0f;
unsigned int frames = 0;

bool axe_data = false;
bool bounty_data = false;

float total = 0;

Model* mx;

void Update ( ESContext *esContext, float deltaTime )
{
	Manager::Update();
	
	if(totaltime > 1 && !bounty_data)
	{
		bounty_data = true;
		Texture* t1 = new Texture(testName);
		Manager::add(t1);
		//Texture* t2 = new Texture("materials/models/heroes/axe/axe_armor_normal.vtf");
		//Manager::add(t2);
		//Texture* t3 = new Texture("materials/models/heroes/axe/axe_armor_masks1.vtf");
		//Manager::add(t3);
		//Texture* t4 = new Texture("materials/models/heroes/axe/axe_armor_masks2.vtf");
		//Manager::add(t4);
		Model* m1 = new Model("models/heroes/axe/axe.mdl");
		Manager::add(m1);
		mx = m1;
	}
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
	
	/* GLfloat vVertices[] = {  
		-0.5f,  0.5f, 0.0f, 
		-0.5f, -0.5f, 0.0f,
		0.5f,  -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f, 
		0.5f,  -0.5f, 0.0f, 
		0.5f,  0.5f, 0.0f,
	}; */
	
	vvdVertexFormat vVer[6];
	vVer[0].m_vecPosition = glm::vec3(-0.5f,  0.5f, 0.0f);
	vVer[1].m_vecPosition = glm::vec3(-0.5f, -0.5f, 0.0f);
	vVer[2].m_vecPosition = glm::vec3(0.5f,  -0.5f, 0.0f);
	vVer[3].m_vecPosition = glm::vec3(-0.5f,  0.5f, 0.0f);
	vVer[4].m_vecPosition = glm::vec3(0.5f,  -0.5f, 0.0f);
	vVer[5].m_vecPosition = glm::vec3(0.5f,  0.5f, 0.0f);
	
	vVer[0].m_vecTexCoord = glm::vec2(0.0f, 1.0f);
	vVer[1].m_vecTexCoord = glm::vec2(0.0f, 0.0f);
	vVer[2].m_vecTexCoord = glm::vec2(1.0f, 0.0f);
	vVer[3].m_vecTexCoord = glm::vec2(0.0f, 1.0f);
	vVer[4].m_vecTexCoord = glm::vec2(1.0f, 0.0f);
	vVer[5].m_vecTexCoord = glm::vec2(1.0f, 1.0f);

	/* if(!axe_data){
		axe_data = true;
		for(int i=0;i<sizeof(vvdVertexFormat)*6;i++)
		{
			printf("%2X ",*(((char*)(vVer))+i));
			if(i%48==47) printf("\n");
		}	
	} */
	

	// No clientside arrays, so do this in a webgl-friendly manner
	//GLuint vertexPosObject;
	//glGenBuffers(1, &vertexPosObject);
	//glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vvdVertexFormat) * 6, vVer, GL_STATIC_DRAW);
	
	if(mx->state == FS_READY)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mx->vbo[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mx->vbo[1]);
	}
	
	// Set the viewport
	glViewport ( 0, 0, esContext->width, esContext->height );
	
	// Clear the color buffer
	glClear ( GL_COLOR_BUFFER_BIT );

	// Use the program object
	glUseProgram ( userData->programObject );

	// Load the vertex data
	//glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
	//glVertexAttribPointer(0 /* ? */, 3, GL_FLOAT, 0, 0, 0);
	//glEnableVertexAttribArray(0);
	if(mx->state == FS_READY)
	{
		mx->SetVAO();
	}
	
	//mx->Draw();
	
	//glm::mat4 m0 = glm::rotate(glm::mat4(1),(float) M_PI,glm::vec3(1,0,0));
	glm::mat4 m = glm::rotate(glm::mat4(1),userData->deg,glm::vec3(0,1,0));
	glUniformMatrix4fv(userData->modelTransform, 1, GL_FALSE, &m[0][0]);
	
	glm::mat4 v = glm::translate(glm::mat4(1), glm::vec3(0,-100,-100));
	glUniformMatrix4fv(userData->viewTransform, 1, GL_FALSE, &v[0][0]);
	
	glm::mat4 p = glm::perspective (30.0f, 1.0f, 0.01f, 1000.0f);
	glUniformMatrix4fv(userData->projTransform, 1, GL_FALSE, &p[0][0]);
	
	userData->deg += M_PI/90 / 5;
	
	//printf("%f \n",userData->deg);

	Texture* t = Manager::find(testName);

	if(t) t->Bind(0);
	
	glUniform1i(userData->textureLocation, 0);

	//mx->Draw();

	//glDrawArrays ( GL_TRIANGLES, 0, 6 );
	if(mx->state == FS_READY)
	{
		//glDrawArrays ( GL_TRIANGLES, 1899, 300 );
		
		glDrawElements(GL_TRIANGLES, mx->elementLength, GL_UNSIGNED_SHORT, 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	
	if(t) t->Unbind(0);
}

void mainloop()
{
	
	gettimeofday(&t2, &tz);
	deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
	t1 = t2;

	esMainLoop(&esContext);

	totaltime += deltatime;
	frames++;
	
	const float fpsRefresh = 15.0f;
	
	if (totaltime >  fpsRefresh)
	{
		printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames/totaltime);
		totaltime -= fpsRefresh;
		frames = 0;
	}
	
	/* if(!axe_data) 
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
	} */
	
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
	
	//FileLoader::Load("models/heroes/axe/axe.mdl");
	
	emscripten_set_main_loop(mainloop, -1,0);
	
	
}