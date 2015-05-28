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
	static void LoadCallbackSuccess(unsigned int num, void* some, const char* fileName);
	static void LoadCallbackFail(unsigned int num, void* some, int num2);
	
	static bool FileExist(const char* fileName);
	static char* ReadFile(const char* fileName);
	static char* ReadFile(const char* fileName, unsigned int& size);
	
};