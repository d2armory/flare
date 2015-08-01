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

int main()
{
	
	// Pointer validation
	
	printf("Validating pointer size\n");
	
	const int pointerSizeExpected = 4;
	const int pointerSize = sizeof(void*);
	
	printf("- Expected: %d, Runtime: %d\n",pointerSizeExpected,pointerSize);
	
	if(pointerSize!=pointerSizeExpected) return 1;
	
	// main code is here
	printf("Openning file\n");
	//const char* fileName = "testbin/axe_bg_default_lod0.vmesh_c";
	const char* fileName = "testbin/alliance_ward.vmat_c";
	
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

	/*	
	const char* fileName2 = "testbin/pedestal_spotlight_bg_default_lod0.vmesh_c";
	FILE* fp2 = fopen(fileName2,"rb");
	fseek(fp2, 0, SEEK_END);
	int fileSize2 = ftell(fp2);
	fseek(fp2, 0, SEEK_SET);
	printf("File size: %d bytes\n",fileSize2);
	char* origFileData2 = (char*) malloc(fileSize2+4);
	char* fileData2 = (char*) ((((unsigned int) origFileData2) + 3) & (~0x3));	// safe align pad, might not needed
	int frrt2 = fread(fileData2,fileSize2,fileSize2,fp2);
	if(frrt2==0) {
		printf("File read failed!\n");
		return 1;
	}
	printf("File data read into memory (0x%X)\n",(unsigned int) fileData2);
	fclose(fp2);*/
	
	// Parsing
	KeyValue* root = KVReader2::Parse(fileData);
	
	// Dump all
	KVReader2::Dump(root, (unsigned int) fileData);
	
	// Clean up
	KVReader2::Clean(root);
	
	free(origFileData);
	return 0;
	
	// Sample Usage
	/* KeyValue* txtParams = root->Find("m_textureParams");
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
	} */
	
	
	KeyValue* animDesc = root->Find("m_frameData")->Get(0);
	int numFrame = animDesc->Find("m_nFrames")->AsInt();
	int framePerBlock = animDesc->Find("m_nFramesPerBlock")->AsInt();
	
	KeyValue* segmentArray = root->Find("m_segmentArray");
	KeyValue* decoder = root->Find("m_decoderArray");
	for(int i=0;i<17;i++)
	{
		KeyValue* segment = segmentArray->Get(i);
		//printf("Segment %d\n",i);
		//printf("- Unique Frame Index: %d\n",segment->Find("m_nUniqueFrameIndex")->AsInt());
		//printf("- Local Channel/Element Masks: %d / %u\n",segment->Find("m_nLocalChannel")->AsInt(),segment->Find("m_nLocalElementMasks")->AsUint());
		char* container = segment->Find("m_container")->Get(0)->value;
		short type1 = *((short*)(container+0)); // data type as in decode array?
		short type2 = *((short*)(container+2));	// size of some kind? may be number of elements?
		short count = *((short*)(container+4));
		short containerSize = *((short*)(container+6));
		const char* keyType = decoder->Get(type1)->Find("m_szName")->AsName();
		printf("%d , Type1: %d , Type2: %d , Count: %d , Size: %d\n",i,type1,type2,count,containerSize);
		printf("--- %s with %d elements per key\n",keyType,type2);
		
		char* containerData = container + 8 + count*2;
		int containerDataSize = containerSize - 8 - count*2;
		
		int perKey = containerDataSize/count;
		int perKeyPerFrame = containerDataSize/count/framePerBlock;	// TODO: change to block specific value
		
		printf("- Left over data: %d / %d per key / %d per key per frame\n",containerDataSize,perKey,perKeyPerFrame);
		if(perKeyPerFrame==0) printf("- Static Data Mode\n");
		
		for(int j=0;j<count;j++)
		{
			printf("--- IDX %d : %d\n",j,*((short*)(container+8+(j*2))));
			int eFrame = framePerBlock;	// TODO: use block specific frame count
			if(perKeyPerFrame==0)
			{
				eFrame = 1;
			}
			for(int k=0;k<eFrame;k++)
			{
				char* fkData = containerData + perKeyPerFrame * count * k + perKeyPerFrame * j;
				if(perKeyPerFrame==0)
				{
					fkData = containerData + perKey * j;
				}
				if(strcmp(keyType,"CCompressedStaticFullVector3")==0)
				{
					float x = *((emscripten_align1_float*) (fkData));
					float y = *((emscripten_align1_float*) (fkData + 4));
					float z = *((emscripten_align1_float*) (fkData + 8));
					printf("----- %d: S.Vect3: %f, %f, %f\n",k,x,y,z);
				}
				else if(strcmp(keyType,"CCompressedStaticVector3")==0)
				{
					// Vector48
					unsigned short ix = *((emscripten_align1_short*) (fkData));
					unsigned short iy = *((emscripten_align1_short*) (fkData + 2));
					unsigned short iz = *((emscripten_align1_short*) (fkData + 4));
					half_float::half* px = (half_float::half*) (&ix);
					half_float::half* py = (half_float::half*) (&iy);
					half_float::half* pz = (half_float::half*) (&iz);
					float x = *px;
					float y = *py;
					float z = *pz;
					printf("----- %d: S.Vect3C: %f, %f, %f\n",k,x,y,z);
				}
				else if(strcmp(keyType,"CCompressedFullVector3")==0)
				{
					float x = *((emscripten_align1_float*) (fkData));
					float y = *((emscripten_align1_float*) (fkData + 4));
					float z = *((emscripten_align1_float*) (fkData + 8));
					printf("----- %d: Vect3: %f, %f, %f\n",k,x,y,z);
				}
				else if(strcmp(keyType,"CCompressedAnimVector3")==0)
				{
					// Vector48
					unsigned short ix = *((emscripten_align1_short*) (fkData));
					unsigned short iy = *((emscripten_align1_short*) (fkData + 2));
					unsigned short iz = *((emscripten_align1_short*) (fkData + 4));
					half_float::half* px = (half_float::half*) (&ix);
					half_float::half* py = (half_float::half*) (&iy);
					half_float::half* pz = (half_float::half*) (&iz);
					float x = *px;
					float y = *py;
					float z = *pz;
					printf("----- %d: Vect3C: %f, %f, %f\n",k,x,y,z);
				}
				else if(strcmp(keyType,"CCompressedStaticQuaternion")==0)
				{
					// Quat48
					printf("----- %d: S.QuatC: \n",k);
				}
				else if(strcmp(keyType,"CCompressedAnimQuaternion")==0)
				{
					// Quat48
					//short ix = *((emscripten_align1_short*) (fkData));
					//short iy = *((emscripten_align1_short*) (fkData + 2));
					//short iz = *((emscripten_align1_short*) (fkData + 4));
					//unsigned short iw = iz & 0x00000001;
					//iz = iz >> 1;
					
					//printf("----- %d: QuatC: %d, %d, %d\n",k,ix,iy,iz);
					
					//float x = ((int)(ix) - 16384) * (1.0 / 16384.0);
					//float y = ((int)(iy) - 16384) * (1.0 / 16384.0);
					//float z = ((int)(iz) - 16384) * (1.0 / 16384.0);
					//float x = ((int)(ix) - 16384) / 32768.0;
					//float y = ((int)(iy) - 16384) / 32768.0;
					//float z = ((int)(iz) - 16384) / 32768.0;
					//float w = sqrtf(1 - x*x - y*y - z*z);
					//if(iw) w = -w; 
					
					unsigned short ix = *((emscripten_align1_short*) (fkData));
					unsigned short iy = *((emscripten_align1_short*) (fkData + 2));
					unsigned short iz = *((emscripten_align1_short*) (fkData + 4));
					//half_float::half* px = (half_float::half*) (&ix);
					//half_float::half* py = (half_float::half*) (&iy);
					//half_float::half* pz = (half_float::half*) (&iz);
					//float x = *px;
					//float y = *py;
					//float z = *pz;
					//float w = 0.0f;
					//float x = ix;
					//float y = iy;
					//float z = iz;
					//float w = 0.0f;
					
					float x = ((float) ((ix & 0x7FFF) - 0x4000)) * 4.3162985E-5;
					float y = ((float) ((iy & 0x7FFF) - 0x4000)) * 4.3162985E-5;
					float z = ((float) ((iz & 0x7FFF) - 0x4000)) * 4.3162985E-5;
					float w = 1 - (x*x) - (y*y) - (z*z);
					if(w>FLT_EPSILON) w = sqrtf(w);
					if((((iz & 0x8000) >> 15) & 1)) w = -w;
					int pos = ((((ix & 0x8000) >> 15) & 1) *100) | ((((iy & 0x8000) >> 15) & 1) *10) | ((((iz & 0x8000) >> 15) & 1) *1);
					
					printf("----- %d: QuatC: %f, %f, %f, %f - %d\n",k,x,y,z,w,pos);
					/*printf("----- %d: QuatC: ",k);
					for(int l=0;l<48;l++)
					{
						printf("%d",(*(fkData + l/8) >> (7 - l%8)) & 1);
						if(l%8==7) printf(" ");
					}
					printf("\n");*/
					
					// Quaternion deceompression from
					// http://svn.gna.org/svn/cal3d/branches/TRY-IMVU_merge/src/cal3d/quaternion.cpp
					//
					// which is from this book 
					// http://www.amazon.com/Game-Programming-GEMS-Gems-Series/dp/1584502339/
					
					/*short s0 = ix;
					short s1 = iy;
					short s2 = iz;
					
					int which = ((s1 & 1) << 1) | (s2 & 1);
					s1 &= 0xfffe;
					s2 &= 0xfffe;
				
					static const float scale = 1.0f / 32767.0f / 1.41421f;
				
					if (which == 3) {
						x = s0 * scale;
						y = s1 * scale;
						z = s2 * scale;
				
						w = 1 - (x*x) - (y*y) - (z*z);
						if (w > FLT_EPSILON)
							w = sqrt(w);
					}
					else if (which == 2) {
						x = s0 * scale;
						y = s1 * scale;
						w = s2 * scale;
				
						z = 1 - (x*x) - (y*y) - (w*w);
						if (z > FLT_EPSILON)
							z = sqrt(z);
					}
					else if (which == 1) {
						x = s0 * scale;
						z = s1 * scale;
						w = s2 * scale;
				
						y = 1 - (x*x) - (z*z) - (w*w);
						if (y > FLT_EPSILON)
							y = sqrt(y);
					}
					else {
						y = s0 * scale;
						z = s1 * scale;
						w = s2 * scale;
				
						x = 1 - (y*y) - (z*z) - (w*w);
						if (x > FLT_EPSILON)
							x = sqrt(x);
					}*/
					
					
					
					//printf("----- %d: QuatC: %f, %f, %f, %f - %d\n",k,x,y,z,w,which);
				}
			}
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