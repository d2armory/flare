// source 2 file dump

#include <stdio.h>
#include <stdlib.h>
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#include "mdl/dmxHeader.h"
#include "engine/kvreader2.hpp"

int main()
{
	
	// Pointer validation
	
	printf("Validating pointer size\n");
	
	const int pointerSizeExpected = 4;
	const int pointerSize = sizeof(void*);
	
	printf("- Expected: %d, Runtime: %d\n",pointerSizeExpected,pointerSize);
	
	if(pointerSize!=pointerSizeExpected) return 1;
	
	
	// main code is here
	printf("Openning vmat_c file\n");
	const char* fileName = "testbin/axe.vmdl_c";
	FILE* fp = fopen(fileName,"rb");
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	printf("File size: %d bytes\n",fileSize);
	char* origFileData = (char*) malloc(fileSize+4);
	char* fileData = (char*) ((((unsigned int) origFileData) + 3) & (~0x3));	// safe align pad, might not needed
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
	KVReader2::Dump(root);
	
	// Sample Usage
	KeyValue* txtParams = root->Find("m_textureParams");
	for(int i=0;i<txtParams->childCount;i++)
	{
		KeyValue* txt = txtParams->Get(i);
		if(strcmp("g_tColor",txt->Find("m_name")->AsName())==0)
		{
			printf("Diffuse material: %s\n",txt->Find("m_pValue")->AsHandle());
		}
		else if(strcmp("g_tNormal",txt->Find("m_name")->AsName())==0)
		{
			printf("Normal material: %s\n",txt->Find("m_pValue")->AsHandle());
		}
		else if(strcmp("g_tMasks1",txt->Find("m_name")->AsName())==0)
		{
			printf("Mask 1 material: %s\n",txt->Find("m_pValue")->AsHandle());
		}
		else if(strcmp("g_tMasks2",txt->Find("m_name")->AsName())==0)
		{
			printf("Mask 2 material: %s\n",txt->Find("m_pValue")->AsHandle());
		}
	}
	
	// Clean up
	KVReader2::Clean(root);
	
	free(origFileData);
	return 0;
	
	// Old Code
	
	dmxHeader* fileHeader = (dmxHeader*) fileData;
	
	printf("File: %s\n",fileName);
	printf("- size: %d\n",fileHeader->fileSize);
	printf("- sig: 0x%X\n",fileHeader->sig);
	printf("- block offset: 0x%X\n",fileHeader->blockOffset);
	printf("- block count: %d\n",fileHeader->blockCount);
	
	for(int i=0;i<fileHeader->blockCount;i++)
	{
		dmxBlockHeader* blockHeader = fileHeader->block(i);
		printf("-- block %d\n",i);
		printf("--- name: %c%c%c%c\n",blockHeader->name[0],blockHeader->name[1],blockHeader->name[2],blockHeader->name[3]);
		printf("--- data offset: 0x%X\n",blockHeader->dataOffset);
		printf("--- data size: %d\n",blockHeader->dataSize);
		if(blockHeader->name[0] == 'N')
		{
			// assume data block
			ntroHeader* ntro = blockHeader->ntroData();
			printf("---- ntro version %d\n",ntro->version);
			printf("----- struct count: %d\n",ntro->structCount);
			printf("----- struct offset: %d\n",ntro->structOffset);
			
			ntroStruct* sEntry = (ntroStruct*) (((char*) &ntro->structOffset) + ntro->structOffset);
			for(int j=0;j<ntro->structCount;j++)
			{
				ntroStruct* s = sEntry + j;
				printf("------ struct version %d\n",s->version);
				printf("------- id: 0x%X\n",s->id);
				printf("------- baseid: 0x%X\n",s->baseStructId);
				
				const char* name = ((char*) &s->nameOffset) + s->nameOffset;
				printf("------- name: %s\n",name);
				printf("------- field count: %d\n",s->fieldCount);
				
				ntroField* fEntry = (ntroField*) (((char*) &s->fieldOffset) + s->fieldOffset);
				for(int k=0;k<s->fieldCount;k++)
				{
					ntroField* f = fEntry + k;
					printf("-------- field %d\n",k);
					const char* name2 = ((char*) f) + 0 + f->nameOffset;
					printf("--------- name: %s\n",name2);
					printf("--------- disk size: %d\n",f->diskSize);
					printf("--------- disk offset: 0x%X\n",f->diskOffset);
					printf("--------- level: %d\n",f->indirectLevel);
					printf("--------- type: 0x%X\n",f->type);
					printf("--------- typedata: 0x%X\n",f->typeData);
					
					char* indirect =  ((char*) &f->indirectOffset) + f->indirectOffset;
					printf("---------- iByte: 0x%X\n",*indirect);
					if(f->indirectLevel==0x1F)
					{
						printf("----------- %s\n",indirect);
					}
				}
			}
			
			printf("----- enum count: %d\n",ntro->enumCount);
			printf("----- enum offset: %d\n",ntro->enumOffset);
			ntroEnum* eEntry = (ntroEnum*) (((char*) &ntro->enumOffset) + ntro->enumOffset);
			for(int j=0;j<ntro->enumCount;j++)
			{
				ntroEnum* e = eEntry + j;
				printf("------ enum version %d\n",e->version);
				printf("------- id: 0x%X\n",e->id);
				const char* name = ((char*) &e->nameOffset) + e->nameOffset;
				printf("------- name: %s\n",name);
				printf("------- resource count: %d\n",e->resourceCount);
				
				/* ntroField* fEntry = (ntroField*) (((char*) &s->fieldOffset) + s->fieldOffset);
				for(int k=0;k<s->fieldCount;k++)
				{
					ntroField* f = fEntry + k;
					printf("-------- field %d\n",k);
					const char* name2 = ((char*) f) + 0 + f->nameOffset;
					printf("--------- name: %s\n",name2);
					printf("--------- level: %d\n",f->indirectLevel);
					printf("--------- type: 0x%X\n",f->type);
					printf("--------- typedata: 0x%X\n",f->typeData);
					
					char* indirect =  ((char*) &f->indirectOffset) + f->indirectOffset;
					printf("---------- iByte: 0x%X\n",*indirect);
					if(f->indirectLevel==0x1F)
					{
						printf("----------- %s\n",indirect);
					}
				} */
			}
		}
		else if(blockHeader->name[0] == 'D')
		{
			char* data = (char*) blockHeader->data();
			unsigned int sId = *((unsigned int*) data);
			printf("sId: 0x%X\n",sId);
		}
		
		// current material procedure
		// 1. seek to data
		// 2. seek +32 (skip 4:matNameOffset + 4:shaderNameOffset + 8:intParamOffset/count 
		//                 + 8:floatParamOffset/count + 8:vectorParamOffset/count)
		//    we get an offset in first 4 bytes and count in second 4 bytes
		//    (Note: might use nrto spec for this?, it should give us exact location for all files)
		// 3. seek to that offset
		// 4. it's now an array of { int nameOffset; int pad; char externalResourceHash[8]; }
		// 5. search that in RERL block
		// 6. rerl block header is { char externalResourceHash[8]; int nameOffset; int pad; }
		// 7. go straight to that offset and we'll get null-terminated texture name (in vtex extension, need to add _c)
		//
		// Note: can we compare hash using uint64_t or 2 uint32_t ? it should be a lot faster
		// TODO: work on full parser
	}
	
	
	free(origFileData);
	return 0;
}