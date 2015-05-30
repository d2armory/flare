// mdl file information dump

#include <stdio.h>
#include <stdlib.h>
#include "mdl/common.h"
#include "glm/glm.hpp"
#include "engine/kvreader.hpp"

void PrintNode(PCST* cur)
{
	if(cur==0) return;
	for(int i=0;i<cur->depth;i++)
	{
		printf("==");
	}
	if(cur->key!=0)
	{
		printf("= '%s' :",cur->key);
	}
	else 
	{
		printf("=  nil");
	}
	if(cur->value)
	{
		printf(" '%s'",cur->value);
	}
	printf("\n");
	
	PrintNode(cur->child);
	PrintNode(cur->sibling);
}

int main(void)
{
	
	printf("Validating pointer size\n");
	
	const int pointerSizeExpected = 4;
	const int pointerSize = sizeof(void*);
	
	printf("- Expected: %d, Runtime: %d\n",pointerSizeExpected,pointerSize);
	
	if(pointerSize!=pointerSizeExpected) return 1;
	
	printf("Validating mdlHeader size\n");
	
	const int mdlHeaderSizeExpected = 408;
	const int mdlHeaderSize = sizeof(mdlHeader);
	
	printf("- Expected: %d, Runtime: %d\n",mdlHeaderSizeExpected,mdlHeaderSize);
	
	if(mdlHeaderSize!=mdlHeaderSizeExpected) return 1;
	
	printf("Openning mdl file\n");
	
	const char* fileName = "testbin/axe.mdl";
	
	FILE* fp = fopen(fileName,"rb");
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	printf("File size: %d bytes\n",fileSize);
	
	char* fileData = (char*) malloc(fileSize);
	int frrt = fread(fileData,fileSize,fileSize,fp);
	
	if(frrt==0) {
		printf("File read failed!\n");
		return 1;
	}
	
	printf("File data read into memory (0x%X)\n",(unsigned int) fileData);
	
	fclose(fp);
	
	printf("Apply header on data\n");
	
	mdlHeader* mh = (mdlHeader*) fileData;
	
	char* mhId = (char*) &mh->id;
	
	printf("- ID: %c%c%c%c (0x%X)\n",*mhId,*(mhId+1),*(mhId+2),*(mhId+3),mh->id);
	printf("- Version: %d\n",mh->version);
	printf("- Checksum: 0x%X\n",mh->checksum);
	printf("- Name: %s\n",mh->name);
	printf("- Length: %d\n",mh->length);
	
	
	const bool dumpBones = false;
	const bool dumpAnim = true;
	const bool dumpTextures = false;
	
	
	if(dumpBones)
	{
		printf("Retrieving bone data\n");
		
		printf("- Num bones: %d\n",mh->numbones);
		
		for(int i=0;i<mh->numbones;i++)
		{
			mdlBone* bone = mh->pBone(i);
			printf("--- %i: %s",i,bone->pszName());
			if(bone->parent != -1)
			{
				printf(" - %s (%i)",mh->pBone(bone->parent)->pszName(),bone->parent);
			}
			printf("\n");
		}
	}
	
	if(dumpTextures)
	{
		printf("Retrieving texture list\n");
		
		printf("- Num textures: %d\n",mh->numtextures);
		
		for(int i=0;i<mh->numtextures;i++)
		{
			mdlTexture* texture = mh->pTexture(i);
			printf("--- %i: %s\n",i,texture->pszName());
		}
	}
	
	
	if(dumpAnim)
	{
		printf("Retrieving animation list\n");
		
		printf("- Num anims: %d\n",mh->numlocalanim);
		
		for(int i=0;i<mh->numlocalanim;i++)
		{
			mdlAnimDesc* animDesc = mh->pLocalAnimdesc(i);
			printf("--- %i: %s\n",i,animDesc->pszName());
			printf("----- frames: %i\n",animDesc->numframes);
			printf("----- fps: %i\n",animDesc->numframes);
			printf("----- flags: %X\n",animDesc->flags);
			printf("----- animblock: %i\n",animDesc->animblock);
			printf("----- animindex: %i\n",animDesc->animindex);
			mdlAnim* anim = 0;
			int ac = 0;
			if(i==3)
			{
				anim = (mdlAnim*) (((char*) animDesc) + animDesc->animindex);
				ac = 0;
				while(anim!=0)
				{
					printf("------- anim %i\n",ac++);
					printf("--------- bone: %i\n",anim->bone);
					printf("--------- flag: %i\n",anim->flags);
					printf("--------- offset: %i\n",anim->nextoffset);
					anim = anim->pNext();
				}
				
			}
			printf("----- nummovements: %i\n",animDesc->nummovements);
			printf("----- movementindex: %i\n",animDesc->movementindex);
			printf("----- sectionindex: %i\n",animDesc->sectionindex);
			printf("----- sectionframes: %i\n",animDesc->sectionframes);
			if(i==3)
			{
				for(int s=0;s<animDesc->sectionframes;s++)
				{
					mdlAnimSection* section = animDesc->pSection(s);
					printf("------- section %i\n",s);
					printf("--------- animblock: %i\n",section->animblock);
					printf("--------- animindex: %i\n",section->animindex);
					//anim = (mdlAnim*) (((char*) section) + section->animindex);
					//ac = 0;
					//while(anim!=0)
					//{
					//	printf("----------- anim %i\n",ac++);
					//	printf("------------- bone: %i\n",anim->bone);
					//	printf("------------- flag: %i\n",anim->flags);
					//	printf("------------- offset: %i\n",anim->nextoffset);
					//	anim = anim->pNext();
					//}
				}
			}
		}
		
		printf("- External animblock num: %d\n",mh->numanimblocks);
		printf("- Animblock filename: %s\n",mh->pszAnimBlockName());
	}
	
	free(fileData);
	
	
	/* printf("Openning vtx file\n");
	
	const char* fileName = "testbin/axe.dx90.vtx";
	
	FILE* fp = fopen(fileName,"rb");
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	printf("File size: %d bytes\n",fileSize);
	
	char* fileData = (char*) malloc(fileSize);
	int frrt = fread(fileData,fileSize,fileSize,fp);
	
	if(frrt==0) {
		printf("File read failed!\n");
		return 1;
	}
	
	printf("File data read into memory (0x%X)\n",(unsigned int) fileData);
	
	vtxHeader* mData = (vtxHeader*) fileData;
	printf("meshes: %s\n",fileName);
	printf("- version: %d\n",mData->version);
	printf("- num lods: %d\n",mData->numLODs);
	printf("- num bodyparts: %d\n",mData->numBodyParts);
	
	int elementLength = 0;
	
	// assume 1 body parts
	if(mData->numBodyParts==1)
	{
		vtxBodyPart* bp = (vtxBodyPart*) (((char*)(mData)) + mData->bodyPartOffset);
		printf("- num models: %d\n",bp->numModels);
		// assume 1 models
		if(bp->numModels==1)
		{
			vtxModelHeader* mh = (vtxModelHeader*) (((char*)(bp)) + bp->modelOffset);
			for(int l=0;l<mh->numLODs;l++)
			{
				vtxModelLOD* mlod = ((vtxModelLOD*) (((char*)(mh)) + mh->lodOffset)) + l;
				printf("--- lod %d: mesh = %d\n",l,mlod->numMeshes);
				if(l==0)
				{
					for(int m=0;m<mlod->numMeshes;m++)
					{
						vtxMesh* msh = ((vtxMesh*) (((char*)(mlod)) + mlod->meshOffset)) + m;
						printf("----- mesh %d: strip group = %d\n",m,msh->numStripGroups);
						for(int sg=0;sg<msh->numStripGroups;sg++)
						{
							vtxStripGroup* stripgr = ((vtxStripGroup*) (((char*)(msh)) + msh->stripGroupHeaderOffset)) + sg;
							
							printf("------- strip group %d\n",sg);
							printf("--------- verts: %d\n",stripgr->numVerts);
							printf("--------- indices: %d\n",stripgr->numIndices);
							printf("--------- strips: %d\n",stripgr->numStrips);
							printf("--------- flag: 0x%X\n",stripgr->flags);
							
							if(true || (stripgr->flags & 0x02) != 0)
							{
								//for(int s=0;s<stripgr->numStrips;s++)
								//{
								vtxStrip* strip = ((vtxStrip*) (((char*)(stripgr)) + stripgr->stripOffset));
								//printf("--------- strip %d\n",s);
								printf("----------- verts: %d\n",strip->numVerts);
								printf("----------- indices: %d\n",strip->numIndices);
								printf("--------- flag: 0x%X\n",strip->flags);
								//}
								
								if(elementLength>0) continue;
								
								elementLength = stripgr->numIndices;
								unsigned short* elementBuffer = new unsigned short[stripgr->numIndices];
								
								vtxVertex* vertexArr = ((vtxVertex*) (((char*)(stripgr)) + stripgr->vertOffset));
								//printf("%X\n",stripgr->vertOffset);
								
								//vertexArr = ((vtxVertex*) (((char*)(stripgr)) + strip->vertOffset));
								//printf("%X\n",vertexArr);
								unsigned short* indexArr = (unsigned short*) (((char*) (stripgr)) + stripgr->indexOffset);
								
								//for(int v=0;v<elementLength;v++)
								//{
								//	printf("%hu ",indexArr[v]);
								//}
								//printf("\n");
								
								//for(int v=0;v<strip->numVerts;v++)
								//{
								//	printf("%hu ",vertexArr[v].origMeshVertID);
								//}
								//printf("\n"); 
								
								for(int v=0;v<elementLength;v++)
								{
									unsigned short idx = indexArr[v];
									//printf("%hu ",idx);
									//if(v%32==31) printf("\n");
									elementBuffer[v] = vertexArr[idx].origMeshVertID;
									
									//if(elementBuffer[idx] >= vertexCount) 
									//{
									//	printf("%d\n",elementBuffer[idx]);
									//	elementBuffer[idx] = vertexCount - 1;
									//}
									
									//printf("%d: %hu - %hu - %hu\n",v,indexArr[v],vertexArr[v].origMeshVertID,elementBuffer[v]);
								}
								
								for(int v=0;v<elementLength;v++)
								{
									printf("%hu ",elementBuffer[v]);
								}
								printf("\n");
								
								
								
								//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo[1]);
								//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * stripgr->numIndices, elementBuffer, GL_STATIC_DRAW);
								//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
								
								delete elementBuffer;
							}
							else
							{
								printf("----------- non-hw skin grp, skip\n");
							}
							
						}
					}
				}
				else
				{
					printf("----- skipping non-0 lod\n");
				}
			}
		}
		else
		{
			printf("- unsporrted num models count\n");
		}
	}
	else
	{
		printf("- unsporrted num bodyparts count\n");
	}
	
	free(fileData);
	*/
	
	/*printf("Openning vtm file\n");
	
	const char* fileName = "testbin/axe_armor_color.vmt";
	
	FILE* fp = fopen(fileName,"rb");
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	printf("File size: %d bytes\n",fileSize);
	
	char* fileData = (char*) malloc(fileSize);
	int frrt = fread(fileData,fileSize,fileSize,fp);
	
	if(frrt==0) {
		printf("File read failed!\n");
		return 1;
	}
	
	printf("File data read into memory (0x%X)\n",(unsigned int) fileData);
	
	PCST* root = KVReader::Parse(fileData, fileSize);
	
	PCST* cur = root;
	
	PrintNode(root);
	
	KVReader::Clean(root);
	
	free(fileData);*/
	
	return 0;
}