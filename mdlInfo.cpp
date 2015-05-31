// mdl file information dump

#include <stdio.h>
#include <stdlib.h>
#include "mdl/common.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
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

void ExtractAnimValue( int frame, mdlAnimValue *panimvalue, float scale, float &v1)//, float &v2 )
{

	if ( !panimvalue )
	{
		v1 = 0;
		return;
	}

	int k = frame;

	while (panimvalue->num.total <= k)
	{
		k -= panimvalue->num.total;
		panimvalue += panimvalue->num.valid + 1;
		if ( panimvalue->num.total == 0 )
		{
			//Assert( 0 ); // running off the end of the animation stream is bad
			v1 = 0;
			return;
		}
	}
	if (panimvalue->num.valid > k)
	{
		v1 = panimvalue[k+1].value * scale;
	}
	else
	{
		// get last valid data block
		v1 = panimvalue[panimvalue->num.valid].value * scale;
	}

	/* int k = frame;

	// find the data list that has the frame
	while (panimvalue->num.total <= k)
	{
		k -= panimvalue->num.total;
		panimvalue += panimvalue->num.valid + 1;
	}
	if (panimvalue->num.valid > k)
	{
		// has valid animation data
		v1 = panimvalue[k+1].value * scale;

		if (panimvalue->num.valid > k + 1)
		{
			// has valid animation blend data
			v2 = panimvalue[k+2].value * scale;
		}
		else
		{
			if (panimvalue->num.total > k + 1)
			{
				// data repeats, no blend
				v2 = v1;
			}
			else
			{
				// pull blend from first data block in next list
				v2 = panimvalue[panimvalue->num.valid+2].value * scale;
			}
		}
	}
	else
	{
		// get last valid data block
		v1 = panimvalue[panimvalue->num.valid].value * scale;
		if (panimvalue->num.total > k + 1)
		{
			// data repeats, no blend
			v2 = v1;
		}
		else
		{
			// pull blend from first data block in next list
			v2 = panimvalue[panimvalue->num.valid + 2].value * scale;
		}
	} */
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
	
	char* origFileData = (char*) malloc(fileSize+4);
	char* fileData = (char*) ((((unsigned int) origFileData) + 3) & (~0x3));
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
		
		/* printf("- Num anims: %d\n",mh->numlocalanim);
		
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
		*/
		
		//printf("- External animblock num: %d\n",mh->numanimblocks);
		//printf("- Animblock filename: %s\n",mh->pszAnimBlockName());
		
		printf("- Num seq: %d\n",mh->numlocalseq);
		
		//for(int i=0;i<mh->numlocalseq;i++)
		//{
		int i=3;
			mdlSeqDesc* seqDesc = mh->pLocalSeqdesc(i);
			printf("--- %i: %s\n",i,seqDesc->pszLabel());
			//printf("----- grp0: %i\n",seqDesc->groupsize[0]);
			//printf("----- grp1: %i\n",seqDesc->groupsize[1]);
			//printf("----- frames: %i\n",seqDesc->numframes);
			//printf("----- fps: %i\n",seqDesc->numframes);
			//printf("----- flags: %X\n",seqDesc->flags);
			//printf("----- animblock: %i\n",seqDesc->animblock);
			//printf("----- animindex: %i\n",seqDesc->animindex);
			if(seqDesc->groupsize[0]==1 && seqDesc->groupsize[1]==1)
			{
				mdlAnimDesc* animDesc = mh->pLocalAnimdesc(seqDesc->anim(0,0));
				printf("----- anim %s\n",animDesc->pszName());
				printf("------- frames: %i\n",animDesc->numframes);
				printf("------- fps: %i\n",animDesc->numframes);
				printf("------- flags: %X\n",animDesc->flags);
				for(int f=0;f<animDesc->numframes&&f<=3;f++)
				{
					if((animDesc->flags & 0x0020) == 0)
					{
						
						printf("--------- frame %d\n",f);
						
						int frame = f;
						int* piFrame = &frame;
						
						mdlAnim* anim = 0;
						
						int block = animDesc->animblock;
						int index = animDesc->animindex;
						int section = 0;
					
						if (animDesc->sectionframes != 0)
						{
							if (animDesc->numframes > animDesc->sectionframes && *piFrame == animDesc->numframes - 1)
							{
								// last frame on long anims is stored separately
								*piFrame = 0;
								section = (animDesc->numframes / animDesc->sectionframes) + 1;
							}
							else
							{
								section = *piFrame / animDesc->sectionframes;
								*piFrame -= section * animDesc->sectionframes;
							}
					
							block = animDesc->pSection( section )->animblock;
							index = animDesc->pSection( section )->animindex;
						}
					
						if(block==0)
						{
							anim = (mdlAnim*) (((char*) animDesc) + index);
						}
						else
						{
							printf("what?\n");
						}
						
						//for(int b=0;b<mh->numbones;b++)
						//{
						//	mdlBone* bone = mh->pBone(b);
						//	float boneWeight = seqDesc->weight(b);
						//}
						for(int b=0;b<100&&anim!=0;b++)
						{
							mdlBone* bone = mh->pBone(b);
							printf("----------- anim %d: bone %d, flag 0x%2X\n",b,anim->bone,anim->flags);
							if(anim->flags & STUDIO_ANIM_RAWPOS) printf("------------- RAW_POS: %f, %f, %f\n",anim->pPos()->data[0].unpack(),anim->pPos()->data[1].unpack(),anim->pPos()->data[2].unpack());
							if(anim->flags & STUDIO_ANIM_RAWROT)
							{
								Quaternion48* q48 = anim->pQuat48();
								glm::quat q = q48->unpack();
								glm::vec3 euler = glm::eulerAngles(q);
								printf("------------- RAW_ROT: %f, %f, %f\n",euler[0],euler[1],euler[2]);
							}
							if(anim->flags & STUDIO_ANIM_ANIMPOS)
							{
								printf("------------- ANM_POS: %hu, %hu, %hu\n",anim->pPosV()->pAnimvalue(0)->value,anim->pPosV()->pAnimvalue(1)->value,anim->pPosV()->pAnimvalue(2)->value);
							}
							if(anim->flags & STUDIO_ANIM_ANIMROT)
							{
								printf("------------- ANM_ROT: %hu, %hu, %hu\n",anim->pRotV()->pAnimvalue(0)->value,anim->pRotV()->pAnimvalue(1)->value,anim->pRotV()->pAnimvalue(2)->value);
								glm::vec3 baseRotScale = bone->rotscale;
								glm::vec3 angle1(0,0,0);	// euler angle
								//glm::vec3 angle2(0,0,0);
								ExtractAnimValue( frame, anim->pRotV()->pAnimvalue( 0 ), baseRotScale[0], angle1[0]);//, angle2[0] );
								ExtractAnimValue( frame, anim->pRotV()->pAnimvalue( 1 ), baseRotScale[1], angle1[1]);//, angle2[1] );
								ExtractAnimValue( frame, anim->pRotV()->pAnimvalue( 2 ), baseRotScale[2], angle1[2]);//, angle2[2] );
								// add base rotation here here
								
								//angle1[0] = angle1[0] * bone->rotscale[0] + bone->rot[0];
								//angle1[1] = angle1[1] * bone->rotscale[1] + bone->rot[1];
								//angle1[2] = angle1[2] * bone->rotscale[2] + bone->rot[2];
								
								//angle2[0] = angle2[0] * bone->rotscale[0] + bone->rot[0];
								//angle2[1] = angle2[1] * bone->rotscale[1] + bone->rot[1];
								//angle2[2] = angle2[2] * bone->rotscale[2] + bone->rot[2];
								
								angle1 += bone->rot;
								//angle2 += bone->rot;
								
								//if(glm::all(glm::equal(angle1,angle2)))
								//{
									printf("--------------- EULERS: %f, %f, %f\n",angle1[0],angle1[1],angle1[2]);
								//}
								//else
								//{
								//	printf("--------------- EULERS1: %f, %f, %f\n",angle1[0],angle1[1],angle1[2]);
								//	printf("--------------- EULERS2: %f, %f, %f\n",angle2[0],angle2[1],angle2[2]);
								//}
							}
							if(anim->flags & STUDIO_ANIM_RAWROT2)
							{
								Quaternion64* q64 = anim->pQuat64();
								glm::quat q = q64->unpack();
								glm::vec3 euler = glm::eulerAngles(q);
								printf("------------- RAW_ROT: %f, %f, %f\n",euler[0],euler[1],euler[2]);
							}
							anim = anim->pNext();
						}
					}
					
				}
				
			}
		//}
	}
	
	free(origFileData);
	
	
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