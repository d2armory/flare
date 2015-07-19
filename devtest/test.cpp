// source 2 file dump

#include <stdio.h>
#include <stdlib.h>
#include "../glm/glm.hpp"
#include "../glm/gtc/quaternion.hpp"
#include "../glm/gtx/quaternion.hpp"

#include "../mdl/dmxHeader.h"
#include "../engine/kvreader2.hpp"

#include "../engine/half.hpp"
#include <cfloat>

int main(int argc, char** argv)
{
	const int pointerSize = sizeof(void*);
	if(pointerSize!=4){
        printf("Pointer size: %d | Expected: 4\n",pointerSize);
        return 1;
    } 
	
	//emcc test.cpp ../engine/kvreader2.cpp -O2 -o test.js --embed-file bin/
	
	//const char* fileName = "axe/axe.vmdl_c";
	//const char* fileName = "axe/asset_sequences_c588a788.vagrp_c";
	char* fileName = argv[1];
	
	//const char* fileName = "axe/c588a788/loadout.vanim_c";
	//const char* fileName = "";
	
	FILE* fp = fopen(fileName,"rb");
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	printf("File: %s | Size: %X bytes\n",fileName,fileSize);
	char* fileData = (char*) malloc(fileSize);
	//char* fileData = (char*) ((((unsigned int) origFileData)));	// safe align pad, might not needed
	int frrt = fread(fileData,fileSize,fileSize,fp);
	if(frrt==0) {
		printf("File read failed!\n");
		return 1;
	}
	printf("File data read into memory (0x%X)\n",(unsigned int) fileData);
	fclose(fp);
	
	// Parsing
	KeyValue* root = KVReader2::Parse(fileData);
	
	// Dump all
    printf("========Dump Start=======\n");
	KVReader2::Dump(root, (unsigned int) fileData);
	printf("========Dump End=========\n");
    
	// Clean up
	KVReader2::Clean(root);
	
	free(fileData);
    printf("Done\n");
	return 0;
}