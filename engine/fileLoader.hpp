#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <string>

class FileLoader
{
	
public:

	static void PrepareDirectory(const char* fileName);

	static void Load(const char* fileName);
	static void LoadCallbackSuccess(const char* fileName);
	static void LoadCallbackFail(const char* fileName);
	
	static bool FileExist(const char* fileName);
	static char* ReadFile(const char* fileName);
	
};