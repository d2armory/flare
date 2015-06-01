
#include "model.hpp"

Model::Model(const char* fileName)
{
	strncpy(this->fileName,fileName,MODEL_NAME_LENGTH);
	std::string sFN(this->fileName);
	unsigned found = sFN.find_last_of(".");
	std::string sFNne = sFN.substr(0,found);
	std::string vFilename = sFNne + ".vvd";
	std::string mFilename = sFNne + ".dx90.vtx";
	strncpy(this->meshFileName,mFilename.c_str(),MODEL_NAME_LENGTH);
	strncpy(this->vertexFileName,vFilename.c_str(),MODEL_NAME_LENGTH);
	state = FS_UNINIT;
	mdlState = FS_UNINIT;
	meshState = FS_UNINIT;
	vertexState = FS_UNINIT;
	numStrip = 0;
	nextModel = 0;
	material = 0;
	shader = 0;
	shaderShadow = 0;
	vao = 0;
	
	//meshBoneCount[0] = 0;
	//meshBoneList[MODEL_STRIP_COUNT];
	for(int i=0;i<MODEL_STRIP_COUNT;i++)
	{
		meshBoneCount[i] = 0;
		meshBoneList[i] = 0;	// to be filled whend data came
		meshBoneIndex[i] = 0;
	}
	
	numBone = 0;
	bonePos = 0;	// init after we know exact number
	boneRot = 0;
	
	posePrepared = false;
	useAnimation = false;
	curFrame = -1;
	frameTime = 99999.0f;
}

Model::~Model()
{
	if(data) delete data;
	if(vData) delete vData;
	if(mData) delete mData;
	if(bonePos) delete bonePos;
	if(boneRot) delete boneRot;
}

void Model::Update(ESContext *esContext, float deltaTime)
{
	if(state==FS_UNINIT)
	{
		state = FS_LOADING;
		mdlState = FS_LOADING;
		meshState = FS_LOADING;
		vertexState = FS_LOADING;
		FileLoader::Load(fileName);
		FileLoader::Load(meshFileName);
		FileLoader::Load(vertexFileName);
	}
	if(state==FS_LOADING)
	{
		if(mdlState == FS_LOADING && FileLoader::FileExist(fileName))
		{
			mdlState = FS_READY;
		}
		if(meshState == FS_LOADING && FileLoader::FileExist(meshFileName))
		{
			meshState = FS_READY;
		}
		if(vertexState == FS_LOADING && FileLoader::FileExist(vertexFileName))
		{
			vertexState = FS_READY;
		}
		if(mdlState == FS_READY && meshState == FS_READY && vertexState == FS_READY)
		{
			state = FS_READY;
			
			mdlHeader* mh =  data = (mdlHeader*) FileLoader::ReadFile(fileName);
			
			char* mhId = (char*) &mh->id;
			
			printf("model: %s\n",fileName);
			//printf("- ID: %c%c%c%c (0x%X)\n",*mhId,*(mhId+1),*(mhId+2),*(mhId+3),mh->id);
			//printf("- Version: %d\n",mh->version);
			//printf("- Checksum: 0x%X\n",mh->checksum);
			//printf("- Name: %s\n",mh->name);
			//printf("- Length: %d\n",mh->length);
			
			// assume 1 material per model
			mdlTexture* texture = mh->pTexture(0);
			//printf("----- %i: %s\n",i,texture->pszName());
			std::string txtFilename = std::string(texture->pszName());
			txtFilename = "materials/" + txtFilename + ".vmt";
			
			// TODO: read actual file
			material = new Material(txtFilename.c_str());
			Manager::add(material);
			
			//printf("- Num bones: %d\n",mh->numbones);
			numBone = mh->numbones;
			bonePos = new glm::vec3[numBone];
			boneRot = new glm::quat[numBone];
			boneTransform = new glm::mat4[numBone];
		
			for(int i=0;i<mh->numbones;i++)
			{
				boneTransform[i] = glm::mat4(1);
				mdlBone* bone = mh->pBone(i);
				printf("--- %i: %s",i,bone->pszName());
				//if(bone->parent != -1)
				//{
				//	printf(" - %s (%i)",mh->pBone(bone->parent)->pszName(),bone->parent);
				//}
				//printf(" - %x",(unsigned int) &(bone->rot));
				//printf("\n");
			}
			
			// VBO gen
			glGenBuffers(2, (GLuint*) &this->vertexVBO);
			glGenBuffers(MODEL_STRIP_COUNT, (GLuint*) &this->meshVBO);
			
			if(this->vertexVBO[0]==0 || this->meshVBO[0]==0)
			{
				printf("Error: unbale to gen buffer for vertex\n");
				return;
			}
			
			// load vertex data
			
			vData = (vvdHeader*) FileLoader::ReadFile(vertexFileName);
			
			//printf("vertices: %s\n",vertexFileName);
			//printf("- id: %d\n",vData->id);
			//printf("- version: %d\n",vData->version);
			//printf("- checksum: %ld\n",vData->checksum);
			//printf("- num lods: %d\n",vData->numLODs);
			vertexCount = 0;
			for(int l=0;l<vData->numLODs;l++)
			{
				vertexCount += vData->numLODVertexes[l];
				//printf("--- lod %d: %d verts\n",l,vData->numLODVertexes[l]);
			}
			// ignore fixup
			//printf("- num fixup: %d\n",vData->numFixups);
			
			char* vertexData = (char*) vData;
			vertexData += vData->vertexDataStart;
			
			glBindBuffer(GL_ARRAY_BUFFER, this->vertexVBO[0]);
			// vertex and tangent
			unsigned int vertexBufferSize = (sizeof(vvdVertexFormat)) * vertexCount;
			glBufferData(GL_ARRAY_BUFFER, vertexBufferSize , vertexData, GL_STATIC_DRAW);
			
			//printf("err = %d\n",glGetError());
			
			//tangentOffset = vData->tangentDataStart - vData->vertexDataStart;
			
			//printf("tOffset = %d , calcSize = %d\n",tangentOffset, sizeof(vvdVertexFormat) * vertexCount);
			
			char* tangentData = (char*) vData;
			tangentData += vData->tangentDataStart;
			
			glBindBuffer(GL_ARRAY_BUFFER, this->vertexVBO[1]);
			// vertex and tangent
			unsigned int tangentBufferSize = sizeof(glm::vec4) * vertexCount;
			glBufferData(GL_ARRAY_BUFFER, tangentBufferSize , tangentData, GL_STATIC_DRAW);
			
			if(Scene::enableVAO)
			{
				glGenVertexArrays(1, &this->vao);
				glBindVertexArray(this->vao);
				this->SetVAO();
				glBindVertexArray(0);
			}
			
			// bind mesh
			mData = (vtxHeader*) FileLoader::ReadFile(meshFileName);
			//printf("meshes: %s\n",meshFileName);
			//printf("- version: %d\n",mData->version);
			//printf("- num lods: %d\n",mData->numLODs);
			//printf("- num bodyparts: %d\n",mData->numBodyParts);
			printf("max bone per strip = %d\n",mData->maxBonesPerStrip);
			
			//elementLength = 0;
			
			// assume 1 body parts
			if(mData->numBodyParts==1)
			{
				vtxBodyPart* bp = (vtxBodyPart*) (((char*)(mData)) + mData->bodyPartOffset);
				//printf("- num models: %d\n",bp->numModels);
				// assume 1 models
				if(bp->numModels==1)
				{
					vtxModelHeader* mh = (vtxModelHeader*) (((char*)(bp)) + bp->modelOffset);
					for(int l=0;l<mh->numLODs;l++)
					{
						vtxModelLOD* mlod = ((vtxModelLOD*) (((char*)(mh)) + mh->lodOffset)) + l;
						//printf("--- lod %d: mesh = %d\n",l,mlod->numMeshes);
						if(l==0)
						{
							for(int m=0;m<mlod->numMeshes;m++)
							{
								vtxMesh* msh = ((vtxMesh*) (((char*)(mlod)) + mlod->meshOffset)) + m;
								//printf("----- mesh %d: strip group = %d\n",m,msh->numStripGroups);
								for(int sg=0;sg<msh->numStripGroups;sg++)
								{
									vtxStripGroup* stripgr = ((vtxStripGroup*) (((char*)(msh)) + msh->stripGroupHeaderOffset)) + sg;
									
									printf("------- strip group %d\n",sg);
									//printf("--------- verts: %d\n",stripgr->numVerts);
									//printf("--------- indices: %d\n",stripgr->numIndices);
									//printf("--------- strips: %d\n",stripgr->numStrips);
									//printf("--------- flag: 0x%X\n",stripgr->flags);
									
									if((stripgr->flags & 0x02) != 0)
									{
										//for(int s=0;s<stripgr->numStrips;s++)
										//{
										//vtxStrip* strip = ((vtxStrip*) (((char*)(stripgr)) + stripgr->stripOffset));
										//printf("--------- strip %d\n",s);
										//printf("----------- verts: %d\n",strip->numVerts);
										//printf("----------- indices: %d\n",strip->numIndices);
										//printf("----------- flag: 0x%X\n",strip->flags);
										//}
										
										//if(elementLength>0) continue;
										
										elementLength[numStrip] = stripgr->numIndices;
										unsigned short* elementBuffer = new unsigned short[stripgr->numIndices];
										
										vtxVertex* vertexArr = ((vtxVertex*) (((char*)(stripgr)) + stripgr->vertOffset));
										//printf("%X\n",stripgr->vertOffset);
										unsigned char* vertexArrC = (unsigned char*) (((unsigned int)(stripgr)) + stripgr->vertOffset);
										
										//vertexArr = ((vtxVertex*) (((char*)(stripgr)) + strip->vertOffset));
										//printf("%X\n",vertexArr);
										unsigned short* indexArr = (unsigned short*) (((unsigned int) (stripgr)) + stripgr->indexOffset);
										unsigned char* indexArrC = (unsigned char*) (((unsigned int) (stripgr)) + stripgr->indexOffset);
										
										//printf("%d\n",((unsigned int)indexArr)-((unsigned int)mData));
										
										/* for(int v=0;v<10;v++)
										{
											printf("%u ",&(indexArr[v]) - ((unsigned int) mData));
										}
										printf("\n"); */
										
										/* for(int v=0;v<stripgr->numVerts;v++)
										{
											printf("%hu ",vertexArr[v].origMeshVertID);
										}
										printf("\n"); */
										
										// bone list for this strip
										bool boneUsage[128];	// 128 bone per model
										for(int b=0;b<128;b++)
										{
											boneUsage[b] = false;
										}
										for(int v=0;v<stripgr->numVerts;v++)
										{
											unsigned char* efp = vertexArrC + (v*9) + 4;
											unsigned char* efp2 = efp + 1;
											unsigned short evp = *efp;
											unsigned short evp2 = *efp2;
											unsigned int vID = (evp2 << 8) + evp;
											vvdVertexFormat* vert = ((vvdVertexFormat*) (((char*) vData) + vData->vertexDataStart)) + vID;
											//unsigned char* b1 = vertexArrC + (v*9) + 6;
											//unsigned char* b2 = b1 + 1;
											//unsigned char* b3 = b1 + 2;
											unsigned char b1 = vert->boneid[0];
											unsigned char b2 = vert->boneid[1];
											unsigned char b3 = vert->boneid[2];
											boneUsage[b1] = true;
											boneUsage[b2] = true;
											boneUsage[b3] = true;
										}
										
										meshBoneCount[numStrip] = 0;
										meshBoneList[numStrip] = new unsigned int[53];
										meshBoneIndex[numStrip] = new unsigned int[128];
										unsigned char bCounter = 0;
										printf("Bone usage: ");
										for(int b=0;b<128;b++)
										{
											if(boneUsage[b])
											{
												meshBoneCount[numStrip]++;
												meshBoneList[numStrip][bCounter] = b;
												meshBoneIndex[numStrip][b] = bCounter;
												bCounter++;
												printf("%d ",b);
											}
											else
											{
												meshBoneIndex[numStrip][b] = 0;	// fallback to root?
											}
										}
										printf("\n");
										
										// if this model has parent, map bone id to parent instead
										
										for(int v=0;v<elementLength[numStrip];v++)
										{
											// need hack to access short correctly on non-aligned byte
											//unsigned short idx = *((unsigned short*) (((unsigned int) indexArr) + (v * 2)));
											unsigned char* fp = indexArrC + (v<<1);
											unsigned char* fp2 = fp + 1;
											unsigned short vp = *fp;
											unsigned short vp2 = *fp2;
											unsigned short idx = (vp2 << 8) + vp;
											//printf("%u ",((unsigned int) (indexArr + v)) - ((unsigned int)mData));
											//printf("%X %X ",vp, vp2);
											//printf("%hu ",idx);
											//if(v%32==31) printf("\n");
											//char* fp = vertexArrC + (idx*9) + 3;
											//char* fp2 = fp + 1;
											unsigned char* efp = vertexArrC + (idx*9) + 4;
											unsigned char* efp2 = efp + 1;
											unsigned short evp = *efp;
											unsigned short evp2 = *efp2;
											elementBuffer[v] = (evp2 << 8) + evp;
											//printf("%hu %hu ",*fp,*fp2);
											
											//unsigned char* b1 = efp2 + 1;
											//unsigned char* b2 = efp2 + 2;
											//unsigned char* b3 = efp2 + 3;
											
											//printf("bone: %d, %d, %d\n",*b1,*b2,*b3);
											
											//if(elementBuffer[idx] >= vertexCount) 
											//{
											//	printf("%d\n",elementBuffer[idx]);
											//	elementBuffer[idx] = vertexCount - 1;
											//}
											
											//printf("%d: %hu - %hu - %hu\n",v,indexArr[v],vertexArr[v].origMeshVertID,elementBuffer[v]);
											
											/* if(v%3==2)
											{
												// swap 2 and 3
												unsigned short tmp = elementBuffer[v-1];
												elementBuffer[v-1] = elementBuffer[v];
												elementBuffer[v] = tmp;
											} */
										}
										
										/* for(int v=0;v<elementLength[numStrip];v++)
										{
											printf("%hu ",elementBuffer[v]);
										}
										printf("\n"); */
										
										//printf("--------- put stripgroup %d to vbo %d : %d unsigned short transfered\n",sg,this->meshVBO[numStrip],stripgr->numIndices);
										
										glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->meshVBO[numStrip]);
										glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * stripgr->numIndices, elementBuffer, GL_STATIC_DRAW);
										glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
										
										numStrip++;
										
										delete elementBuffer;
									}
									else
									{
										//printf("----------- non-hw skin grp, skip\n");
									}
									
								}
							}
						}
						else
						{
							//printf("----- skipping non-0 lod\n");
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
			
			// unbind all
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			//glBindVertexArray(0);
			
		}
	}
	else if(state==FS_READY)
	{
		// handle animation here
		if(useAnimation)
		{
			mdlHeader* mh = data;
			// 3 for loadout
			mdlSeqDesc* seqDesc = mh->pLocalSeqdesc(0);
			mdlAnimDesc* animDesc = mh->pLocalAnimdesc(seqDesc->anim(0,0));
			
			frameTime += deltaTime;
			
			if(frameTime>1.0f / animDesc->fps)
			{
				frameTime = 0.0f;
				curFrame++;
				
				if(curFrame >= animDesc->numframes)
				{
					curFrame = 0;
				}

				if(seqDesc->groupsize[0]==1 && seqDesc->groupsize[1]==1)
				{
					//for(int f=0;f<animDesc->numframes&&f<=3;f++)
					//{
					// TODO: make it animate
					// use frame 1
					int f = curFrame;
					
					// check for data
					if((animDesc->flags & 0x0020) == 0)
					{
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
						
						// pre populate with bone data
						for(int b=0;b<mh->numbones;b++)
						{
							mdlBone* bone = mh->pBone(b);
							//float boneWeight = seqDesc->weight(b);
							bonePos[b] = bone->pos;
							boneRot[b] = bone->quat;
						}
						
						for(int b=0;b<mh->numbones&&anim!=0;b++)
						{
							char boneId = anim->bone;
							mdlBone* bone = mh->pBone(boneId);
							
							//printf("----------- anim %d: bone %d, flag 0x%2X\n",b,anim->bone,anim->flags);
							if(anim->flags & STUDIO_ANIM_RAWPOS) 
							{
								// Raw position data in Vector48 format
								bonePos[boneId] = anim->pPos()->unpack();
							}
							if(anim->flags & STUDIO_ANIM_RAWROT)
							{
								// Raw rotation data in Quat48 format
								Quaternion48* q48 = anim->pQuat48();
								glm::quat q = q48->unpack();
								boneRot[boneId] = q;
								//glm::vec3 euler = glm::eulerAngles(q);
							}
							if(anim->flags & STUDIO_ANIM_RAWROT2)
							{
								// Raw rotation data in Quat64 format
								Quaternion64* q64 = anim->pQuat64();
								glm::quat q = q64->unpack();
								boneRot[boneId] = q;
								//glm::vec3 euler = glm::eulerAngles(q);
							}
							if(anim->flags & STUDIO_ANIM_ANIMPOS)
							{
								glm::vec3 basePosScale = bone->posscale;
								glm::vec3 trans(0,0,0);	// translation vector
								ExtractAnimValue( frame, anim->pPosV()->pAnimvalue( 0 ), basePosScale[0], trans[0]);
								ExtractAnimValue( frame, anim->pPosV()->pAnimvalue( 1 ), basePosScale[1], trans[1]);
								ExtractAnimValue( frame, anim->pPosV()->pAnimvalue( 2 ), basePosScale[2], trans[2]);
								trans += bone->pos;
								bonePos[boneId] = trans;
							}
							if(anim->flags & STUDIO_ANIM_ANIMROT)
							{
								glm::vec3 baseRotScale = bone->rotscale;
								glm::vec3 angle1(0,0,0);	// euler angle
								ExtractAnimValue( frame, anim->pRotV()->pAnimvalue( 0 ), baseRotScale[0], angle1[0]);
								ExtractAnimValue( frame, anim->pRotV()->pAnimvalue( 1 ), baseRotScale[1], angle1[1]);
								ExtractAnimValue( frame, anim->pRotV()->pAnimvalue( 2 ), baseRotScale[2], angle1[2]);
								angle1 += bone->rot;
								angle1 *= 180.0f / M_PI;
								boneRot[boneId] = glm::quat(angle1);
								//if(!posePrepared) printf("--------------- EULERS: %f, %f, %f\n",angle1[0],angle1[1],angle1[2]);
							}
							anim = anim->pNext();
						}
						
						// bone hierachy
						for(int b=0;b<mh->numbones;b++)
						{
							mdlBone* bone = mh->pBone(b);
							//bonePos[b] = bone->pos;
							//boneRot[b] = bone->quat;
							
							//glm::vec3 bOrigPosSw(bone->pos[1],bone->pos[0],bone->pos[2]);
							//glm::vec3 bAnimPosSw(bonePos[b][1],bonePos[b][0],bonePos[b][2]);
							
							glm::vec3 bOrigPosSw(bone->pos);
							glm::vec3 bAnimPosSw(bonePos[b]);
							
							glm::quat bOrigRotSw(bone->quat);
							glm::quat bAnimRotSw(boneRot[b]);
							
							if(b==0)
							{
								float tmp = bOrigPosSw[0];
								bOrigPosSw[0] = bOrigPosSw[1];
								bOrigPosSw[1] = -tmp;
								tmp = bAnimPosSw[0];
								bAnimPosSw[0] = bAnimPosSw[1];
								bAnimPosSw[1] = -tmp;
								bOrigRotSw = glm::rotate(bOrigRotSw, -90.0f, glm::vec3(0,0,1));
								bAnimRotSw = glm::rotate(bAnimRotSw, -90.0f, glm::vec3(0,0,1));
							}
							
							if(!posePrepared)
							{
								glm::vec3 euler0 = glm::eulerAngles(bOrigRotSw);
								glm::vec3 euler = glm::eulerAngles(bAnimRotSw);
								printf("[%d] %f %f %f %f %f %f (%f %f %f %f)\n",b,bOrigPosSw[0],bOrigPosSw[1],bOrigPosSw[2],euler0[0],euler0[1],euler0[2],bOrigRotSw[0],bOrigRotSw[1],bOrigRotSw[2],bOrigRotSw[3]);
								printf("[%d] %f %f %f %f %f %f (%f %f %f %f)\n",b,bAnimPosSw[0],bAnimPosSw[1],bAnimPosSw[2],euler[0],euler[1],euler[2],bAnimRotSw[0],bAnimRotSw[1],bAnimRotSw[2],bAnimRotSw[3]);
							}
							
							
							int p = bone->parent;
							if(p>b) 
							{
								printf("parent bone has higher index than child!\n");
							}
							
							//glm::vec3 jointPos = bOrigPosSw;// glm::vec4(bonePos[b],1);
							
							/* if(p>=0)
							{
								int curPar = p;
								mdlBone* par = 0;
								do
								{
									par = mh->pBone(curPar);
									jointPos -= glm::vec4(bonePos[curPar],0);
									curPar = par->parent;
								} while(curPar >= 0);
								//jointPos = boneTransform[p] * jointPos;
							} */
							
							if(b==0)
							{
								//bAnimPosSw = bOrigPosSw;
								//boneRot[b] = bone->quat;
								//bAnimRotSw = bOrigRotSw;
							}
							
							//if(b==0)
							//{
							//	boneTransform[b] = glm::mat4(1);
							//}
							//else //if(b==1)
							//{
								/* glm::mat4 toSMD(
									0.0f,	0.0f,	-1.0f,	0.0f,
									1.0f,	0.0f,	0.0f,	0.0f,
									0.0f,	1.0f,	0.0f,	0.0f,
									0.0f,	0.0f,	0.0f,	1.0f
								);
								glm::mat4 fromSMD(
									0.0f,	1.0f,	0.0f,	0.0f,
									0.0f,	0.0f,	1.0f,	0.0f,
									-1.0f,	0.0f,	0.0f,	0.0f,
									0.0f,	0.0f,	0.0f,	1.0f
								); */
								glm::mat4 toSMD(1);/*
									0.0f,	-1.0f,	0.0f,	0.0f,
									0.0f,	0.0f,	1.0f,	0.0f,
									1.0f,	0.0f,	0.0f,	0.0f,
									0.0f,	0.0f,	0.0f,	1.0f
								);*/
								glm::mat4 fromSMD(1);/*
									0.0f,	0.0f,	1.0f,	0.0f,
									-1.0f,	0.0f,	0.0f,	0.0f,
									0.0f,	1.0f,	0.0f,	0.0f,
									0.0f,	0.0f,	0.0f,	1.0f
								);*/
								//toSMD = glm::transpose(toSMD);
								//fromSMD = glm::transpose(fromSMD);
								
								boneTransform[b] = fromSMD * glm::translate(glm::mat4(1), bAnimPosSw) * glm::mat4_cast(bAnimRotSw) * glm::transpose(glm::mat4_cast(bOrigRotSw)) * glm::translate(glm::mat4(1), -1.0f * bOrigPosSw) * toSMD;
							//} 
							//else
							//{
							//	boneTransform[b] = glm::translate(glm::mat4(1), bAnimPosSw) /* * glm::mat4_cast(boneRot[b]) * glm::transpose(glm::mat4_cast(bone->quat))*/ * glm::translate(glm::mat4(1), -1.0f * jointPos);
							//}
							
							if(p>=0)
							{
								boneTransform[b] = boneTransform[p] * boneTransform[b];
								// calculate new pos/rot with parent pos/rot
								//bonePos[b] = (boneRot[p] * bonePos[b]) + bonePos[p];
								//boneRot[b] = boneRot[p] * boneRot[b];
							}
						}
						posePrepared = true;
					}
					//}
				}
			}
		}
	}
	
}

void Model::Draw(ESContext *esContext)
{
	
	// transform calculation
	// not sure why it use left-to-right calculation though
	glm::mat4 m = glm::mat4(1);
	m = glm::translate(m, glm::vec3(position));
	m = glm::rotate(m,rotation[2],glm::vec3(0,0,1));
	m = glm::rotate(m,rotation[1],glm::vec3(0,1,0));
	m = glm::rotate(m,rotation[0],glm::vec3(1,0,0));
	
	modelTransform = m;
	
	if(state == FS_READY)
	{
		// vao
		if(Scene::enableVAO)
		{
			glBindVertexArray(this->vao);
		}
		else
		{
			this->SetVAO();
		}
		// shader check
		bool shaderActive = 
				(Scene::currentStep == RS_SCENE && shader != 0) || 
				(Scene::currentStep == RS_SHADOW && shaderShadow != 0);
		if(shaderActive)
		{
			// draw each strip
			for(int i=0;i<numStrip;i++) 
			{
				if(meshVBO[i]!=0)
				{
					
					if(Scene::currentStep==RS_SCENE) shader->Bind(this);
					else if(Scene::currentStep==RS_SHADOW) shaderShadow->Bind(this);
					
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshVBO[i]);
					
					if(Scene::currentStep==RS_SCENE) shader->Populate(this, i);
					else if(Scene::currentStep==RS_SHADOW) shaderShadow->Populate(this, i);
		
					// real drawing code, just 3 lines lol
					glDrawElements(GL_TRIANGLES, elementLength[i], GL_UNSIGNED_SHORT, 0);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
		
					if(Scene::currentStep==RS_SCENE) shader->Unbind(this);
					else if(Scene::currentStep==RS_SHADOW) shaderShadow->Unbind(this);
				}
			}
		}
		if(Scene::enableVAO)
		{
			glBindVertexArray(0);
		}
	}
}

void Model::SetVAO()
{
	if(this->vertexVBO[0]!=0)
	{ 
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexVBO[0]);
		// XYZ pos
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vvdVertexFormat), (void*) 16);
		glEnableVertexAttribArray(0);
		// Normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vvdVertexFormat), (void*) 28);
		glEnableVertexAttribArray(1);
		// UV
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vvdVertexFormat), (void*) 40);
		glEnableVertexAttribArray(2);
		// num bones
		glVertexAttribPointer(3, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vvdVertexFormat), (void*) 15);
		glEnableVertexAttribArray(3);
		// bone id
		glVertexAttribPointer(4, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vvdVertexFormat), (void*) 12);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(5, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vvdVertexFormat), (void*) 13);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(6, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vvdVertexFormat), (void*) 14);
		glEnableVertexAttribArray(6);
		// bone weight
		glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(vvdVertexFormat), (void*) 0);
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(vvdVertexFormat), (void*) 4);
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, sizeof(vvdVertexFormat), (void*) 8);
		glEnableVertexAttribArray(9);
	}
	
	if(this->vertexVBO[1]!=0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexVBO[1]);
		// tangent
		glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*) 0);
		glEnableVertexAttribArray(10);
	}
}

void Model::ExtractAnimValue( int frame, mdlAnimValue *panimvalue, float scale, float &v1)
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
}