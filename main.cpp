
/*

	Flare DotA Viewer Engine
	
	Author: KennyZero (@bongikairu)

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <emscripten.h>

#include "gl.h"
#include "common.h"
#include "engine/common.hpp"

// Application Context
ESContext* context;

// Time
struct timeval t1, t2;
struct timezone tz;
float deltatime;
float totaltime = 0.0f;
unsigned int frames = 0;

// App data (move to userdata later)
bool axe_data = false;
bool bounty_data = false;
float total = 0;
int modelCount = 6;
Model** mx = 0;

int Init ( ESContext *esContext )
{
	
	// Start time reference
	gettimeofday ( &t1 , &tz );
	
	// Init
	Manager::Init();
	Scene::InitFeatures();
	Scene::InitDefaultTextures();
	
	// Passable data
	esContext->userData = (char*) malloc(sizeof(UserData));
	UserData *userData = (UserData*) esContext->userData;
	userData->deg = 0;
	
	// Shader Init
	// - For hero
	HeroShader* hShader = new HeroShader();
	hShader->Load();
	userData->heroShader = hShader;
	// - For shadowmap
	ShadowShader* sShader = new ShadowShader();
	sShader->Load();
	userData->shadowShader = sShader;
	
	// Canvas transparency
	glClearColor (0, 0, 0, 0.0f );
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	
	// Back Face Culling
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	// Final Render map
	Scene::InitFinalRender();
	
	// Create shadowmap buffer and texture
	Scene::InitShadowmap();
	
	return GL_TRUE;
}

void Update ( ESContext *esContext, float deltaTime )
{
	Manager::Update( esContext, deltaTime );
	
	// model rotation
	UserData *userData = (UserData*) esContext->userData;
	userData->deg += M_PI/90 / 10;
	
	Scene::lightDir = glm::vec3(-1.0,-2.0,-1.0);
	
	// data loading
	if(totaltime > 0.1f && !bounty_data)
	{
		bounty_data = true;

		const char* modelName[6] = {
			"models/heroes/axe/axe.mdl",
			"models/heroes/axe/axe_armor.mdl",
			"models/heroes/axe/axe_belt.mdl",
			"models/heroes/axe/axe_ponytail.mdl",
			"models/heroes/axe/axe_weapon.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_backpack.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_bandana.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_bweapon.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_lweapon.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_rweapon.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_pads.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_shuriken.mdl"
			//"models/heroes/tidehunter/tidehunter.mdl",
			//"models/heroes/tidehunter/tidehunter_anchor.mdl",
			//"models/heroes/tidehunter/tidehunter_belt.mdl",
			//"models/heroes/tidehunter/tidehunter_bracer.mdl",
			//"models/heroes/tidehunter/tidehunter_fish.mdl"
			////"models/heroes/tidehunter/tidehunter_hook.mdl"
			//"models/heroes/enigma/enigma.mdl"
			"models/heroes/pedestal/pedestal_1_small.mdl"
		};

		mx = new Model*[modelCount];

		for(int i=0;i<modelCount;i++)
		{
			mx[i] = new Model(modelName[i]);
			mx[i]->shader = userData->heroShader;
			mx[i]->shaderShadow = userData->shadowShader;
			Manager::add(mx[i]);
		}
		
		Model* pedes = mx[modelCount-1];
		pedes->rotation[0] = - M_PI / 2.0f;
		//pedes->position[1] = -10.0f;
	}
	
	if(bounty_data)
	{
		for(int i=0;i<modelCount;i++)
		{
			mx[i]->rotation[1] = userData->deg;
		}
	}
}

void Draw ( ESContext *esContext )
{

	// Reset the viewport
	glViewport(0, 0, floor(Scene::screenWidth), floor(Scene::screenHeight));

	// if set to true, draw shadowmap
	bool showShadowmap = false;
	
	if(!showShadowmap)
	{
		// Normal draw mode
		// Generate shadowmap
		Scene::currentStep = RS_SHADOW;
		glBindFramebuffer(GL_FRAMEBUFFER, Scene::shadowFrameBuffer);
		//glColorMask(false, false, false, false);
		glClearColor (1.0f, 1.0f, 1.0f, 1.0f );
		glClear ( GL_COLOR_BUFFER_BIT );
		glClear ( GL_DEPTH_BUFFER_BIT );
		if(mx!=0)
		{
			// TODO: use scene graph
			for(int m=0;m<modelCount;m++)
			{
				mx[m]->Draw(esContext);
			}
		}
		// Draw real scene
		Scene::currentStep = RS_SCENE;
		glBindFramebuffer(GL_FRAMEBUFFER, Scene::finalRenderFrameBuffer);
		glColorMask(true, true, true, true);
		glClearColor (0, 0, 0, 0.0f );
		glClear ( GL_COLOR_BUFFER_BIT );
		glClear ( GL_DEPTH_BUFFER_BIT );
		if(mx!=0)
		{
			// TODO: use scene graph
			for(int m=0;m<modelCount;m++)
			{
				mx[m]->Draw(esContext);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glColorMask(true, true, true, true);
		glClearColor (0, 0, 0, 0.0f );
		glClear ( GL_COLOR_BUFFER_BIT );
		glClear ( GL_DEPTH_BUFFER_BIT );
		Scene::FinalRender();
	}
	else
	{
		// Shadow map only mode
		Scene::currentStep = RS_SHADOW;
		glBindFramebuffer(GL_FRAMEBUFFER, Scene::finalRenderFrameBuffer);
		glColorMask(true, true, true, true);
		glClearColor (1.0f, 1.0f, 1.0f, 1.0f );
		glClear ( GL_COLOR_BUFFER_BIT );
		glClear ( GL_DEPTH_BUFFER_BIT );
		if(mx!=0)
		{
			// TODO: use scene graph
			for(int m=0;m<modelCount;m++)
			{
				mx[m]->Draw(esContext);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glColorMask(true, true, true, true);
		glClearColor (0, 0, 0, 0.0f );
		glClear ( GL_COLOR_BUFFER_BIT );
		glClear ( GL_DEPTH_BUFFER_BIT );
		Scene::FinalRender();
	}
}

void mainloop()
{
	
	gettimeofday(&t2, &tz);
	deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
	t1 = t2;

	if (context->updateFunc != NULL)
	{
		context->updateFunc(context, deltatime);
	}
	if (context->drawFunc != NULL)
	{
		context->drawFunc(context);
	}

	totaltime += deltatime;
	frames++;
	
	const float fpsRefresh = 5.0f;
	
	if (totaltime >  fpsRefresh)
	{
		printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames/totaltime);
		totaltime -= fpsRefresh;
		frames = 0;
	}
	
	glfwSwapBuffers(context->m_window);
	glfwPollEvents();
	
}

void errorCallback(int error, const char *description) {
	printf("GLFW Error %d: %s\n",error,description);
}

int main ( int argc, char *argv[] )
{
	
	printf("Starting Flare DotA Model Viewer Engine ..\n");
	
	// don't change it after you set it here
	Scene::screenWidth = 960.0f;
	Scene::screenHeight = 640.0f;
	
	context = new ESContext();
	context->userData = new UserData();
	
	glfwSetErrorCallback(errorCallback);
	
	if( !glfwInit() )
	{
		printf("Unable to initialize GLFW (%s,%d)\n",__FILE__,__LINE__);
		return 1;
	}

	context->m_window = glfwCreateWindow(floor(Scene::screenWidth), floor(Scene::screenHeight), "Flare", 0, 0);
	//context->m_window = glfwOpenWindow(floor(Scene::screenWidth), floor(Scene::screenHeight), 0,0,0,0,16,0, GLFW_WINDOW);

	if(!context->m_window )
	{
		glfwTerminate();
		printf("Unable to create GLFW window (%s,%d)\n",__FILE__,__LINE__);
		return 1;
	}

	glfwMakeContextCurrent(context->m_window);

	if ( glewInit() != GLEW_OK )
	{
		printf("Unable to initialize GLeW (%s,%d)\n",__FILE__,__LINE__);
		return 1;
	}

	// HID Callback
	//glfwSetKeyCallback(m_window,key_callback);
	//glfwSetCursorPosCallback(m_window, cursor_pos_callback);
	//glfwSetFramebufferSizeCallback(m_window,framebuffer_size_callback);

	glViewport(0, 0, floor(Scene::screenWidth), floor(Scene::screenHeight));

	// bind draw and update
	context->updateFunc = Update;
	context->drawFunc = Draw;
	
	Init(context);
	
	// set main loop function
	emscripten_set_main_loop(mainloop, -1,0);
	
	return 0;
	
}