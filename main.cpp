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

#include "common.h"

///
// Initialize the shader and program object
//

int Init ( ESContext *esContext )
{
	
	Manager::Init();
	
	esContext->userData = (char*) malloc(sizeof(UserData));

	UserData *userData = (UserData*) esContext->userData;
	
	HeroShader* hShader = new HeroShader();
	hShader->Load();
	userData->heroShader = hShader;
	
	userData->deg = 0;
	
	//printf("%d\n",userData->rotateLocation);

	glClearColor (0, 0, 0, 0.0f );
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
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

int modelCount = 5;

Model** mx;

void Update ( ESContext *esContext, float deltaTime )
{
	Manager::Update( esContext, deltaTime );
	
	// model rotation
	UserData *userData = (UserData*) esContext->userData;
	userData->deg += M_PI/90 / 10;
	
	// data loading
	if(totaltime > 0.1f && !bounty_data)
	{
		bounty_data = true;

		char* modelName[5] = {
			"models/heroes/axe/axe.mdl",
			"models/heroes/axe/axe_armor.mdl",
			"models/heroes/axe/axe_belt.mdl",
			"models/heroes/axe/axe_ponytail.mdl",
			"models/heroes/axe/axe_weapon.mdl"
		};

		mx = new Model*[5];

		for(int i=0;i<modelCount;i++)
		{
			mx[i] = new Model(modelName[i]);
			mx[i]->shader = userData->heroShader;
			Manager::add(mx[i]);
		}
		
	}
}

void Draw ( ESContext *esContext )
{

	// Set the viewport
	glViewport ( 0, 0, esContext->width, esContext->height );
	
	// Clear the color buffer
	glClearColor (0, 0, 0, 0.0f );
	glClear ( GL_COLOR_BUFFER_BIT );
	// Clear the depth buffer
	glClear ( GL_DEPTH_BUFFER_BIT );

	// TODO: use scene graph
	for(int m=0;m<modelCount;m++)
	{
		mx[m]->Draw(esContext);
	}
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

	esCreateWindow ( &esContext, "Hello Triangle", 960, 640, ES_WINDOW_RGB | ES_WINDOW_DEPTH );

	if ( !Init ( &esContext ) )
		return 0;

	esRegisterUpdateFunc ( &esContext, Update );

	esRegisterDrawFunc ( &esContext, Draw );

	//esMainLoop ( &esContext );
	
	gettimeofday ( &t1 , &tz );
	
	//FileLoader::Load("models/heroes/axe/axe.mdl");
	
	emscripten_set_main_loop(mainloop, -1,0);
	
	
}