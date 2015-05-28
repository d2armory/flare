#pragma once

#include <string>
#include <functional>

// forward dec
class Model;
class Material;

#include "model.hpp"
#include "material.hpp"
#include "texture.hpp"

class Manager
{

public:
	
	static void add(Model* m);
	static void remove(Model* m);
	
	static void add(Material* m);
	static void remove(Material* m);
	
	static void add(Texture* m);
	static void remove(Texture* m);
	static Texture* find(const char* fileName);
	
	static void Init();
	static void Update(ESContext *esContext, float deltaTime);
	static void Uninit();
	
	static Model* headModel;
	static Material* headMaterial;
	static Texture* headTexture;	// list
	static Texture* rootTexture;	// tree
	
};