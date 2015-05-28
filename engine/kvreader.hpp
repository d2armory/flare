
#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

class PCST
{
	
public:

	PCST* parent;
	PCST* child;
	PCST* sibling;
	char* key;
	char* value;
	unsigned char keylength;
	unsigned char valuelength;
	unsigned char depth;
	unsigned char unused;
	
	PCST()
	{
		key = 0;
		keylength = 0;
		value = 0;
		valuelength = 0;
		parent = 0;
		sibling = 0;
		child = 0;
		depth = 0;
	};
	
	~PCST()
	{
		if(key != 0) delete key;
		if(value != 0) delete value;
	};
};

class KVReader
{
	
public:

	static PCST* Parse(char* data, int fSize);
	static void Clean(PCST* pData);
	
};