
#include "model.hpp"

Model::Model(const char* fileName)
{
	//printf("Copying filename %s\n",fileName);
	strncpy(this->fileName,fileName,MODEL_NAME_LENGTH);
	//printf("Copied %s\n",fileName);
	
	state = FS_UNINIT;
	mdlState = FS_UNINIT;
	meshState = FS_UNINIT;
	vagrpState = FS_UNINIT;
	
	vmdlData = 0;
	vmeshData = 0;
	vanimData = 0;
	
	subModel = 0;
	subModelCount = 0;
	
	nextModel = 0;
	
	shader = 0;
	shaderShadow = 0;
	
	numBone = 0;
	//bonePos = 0;	// init after we know exact number
	//boneRot = 0;
	
	//posePrepared = false;
	useAnimation = false;
	//curFrame = -1;
	//frameTime = 99999.0f;
}

Model::~Model()
{
	if(meshRoot)
	{
		KVReader2::Clean(meshRoot);
	}
	if(subModel) 
	{
		for(int i=0;i<subModelCount;i++)
		{
			delete subModel[i];
		}
		delete subModel;
	}
	//if(bonePos) delete bonePos;
	//if(boneRot) delete boneRot;
}

void Model::Update(ESContext *esContext, float deltaTime)
{
	//printf("Updating %s\n",fileName);
	if(state==FS_UNINIT)
	{
		state = FS_LOADING;
		mdlState = FS_LOADING;
		FileLoader::Load(fileName);
	}
	if(state==FS_LOADING)
	{
		if(mdlState == FS_LOADING && FileLoader::FileExist(fileName))
		{
			mdlState = FS_READY;
			
			vmdlData = FileLoader::ReadFile(fileName);
			KeyValue* root = KVReader2::Parse(vmdlData);
			mdlRoot = root;
			
			KeyValue* modelRefs = root->Find("m_refMeshes");
			KeyValue* lod0 = modelRefs->Get(0);
			
			// Read vmdl and load related mesh and animation files
			std::string mFilename = std::string(lod0->AsHandle()) + "_c";
			strncpy(this->meshFileName,mFilename.c_str(),MODEL_NAME_LENGTH);
			FileLoader::Load(meshFileName);
			meshState = FS_LOADING;
			
			// Read vagrps
			
			KeyValue* animGrp = root->Find("m_refAnimGroups");
			if(animGrp)
			{
				std::string vaFilename = std::string(animGrp->Get(0)->AsName()) + "_c";
				strncpy(this->vagrpFileName,vaFilename.c_str(),MODEL_NAME_LENGTH);
				FileLoader::Load(vagrpFileName);
				vagrpState = FS_LOADING;
				
				printf("VAGRP: %s\n",vaFilename.c_str());
			}
			else
			{
				vagrpState = FS_READY;
				useAnimation = 0;
				anim = 0;
				animParent = 0;
			}
			
			// Prepare bone data
			
			//printf("- Num bones: %d\n",mh->numbones);
			KeyValue* skeleton = root->Find("m_modelSkeleton");
			if(skeleton)
			{
				numBone = skeleton->Find("m_boneName")->childCount;
				printf("- Num bones: %d\n",numBone);
			
				KeyValue* boneNameList = skeleton->Find("m_boneName");
			
				for(int i=0;i<numBone;i++)
				{
					//boneTransform[i] = glm::mat4(1);
					printf("--- %i: %s\n",i,boneNameList->Get(i)->AsName());
				}
				
				if(numBone==0) numBone = 1;
				
				boneMap = new int[numBone];
				boneTransform = new glm::mat4[numBone];
				invBoneTransform = new glm::mat4[numBone];
				KeyValue* boneParent = skeleton->Find("m_nParent");
				KeyValue* bonePos = skeleton->Find("m_bonePosParent");
				KeyValue* boneRot = skeleton->Find("m_boneRotParent");
				for(int i=0;i<numBone;i++)
				{
					boneMap[i] = -1;
					// note: store temp bone transform to calc inv transform
					
					//boneTransform[i] = glm::mat4(1);
					//invBoneTransform[i] = glm::mat4(1);
					boneTransform[i] = glm::mat4(1);
					short bparent = boneParent->Get(i)->AsShort();
					
					boneTransform[i] = glm::mat4_cast(boneRot->Get(i)->AsQuat()) * boneTransform[i];
					boneTransform[i] = glm::translate(boneTransform[i],bonePos->Get(i)->AsVec3());
					if(bparent >= 0)
					{
						boneTransform[i] = boneTransform[bparent] * boneTransform[i];
					}
					
					invBoneTransform[i] = glm::inverse(boneTransform[i]);
				}
				
				glGenTextures(1, &boneTransformTexture);
				glActiveTexture(GL_TEXTURE0 + 5);
				glBindTexture(GL_TEXTURE_2D, boneTransformTexture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			else
			{
				// no skeleton data
				numBone = 0;
			}
			
			KVReader2::Clean(root);

		}
		if(meshState == FS_LOADING && FileLoader::FileExist(meshFileName))
		{
			meshState = FS_READY;
		}
		if(vagrpState == FS_LOADING && FileLoader::FileExist(vagrpFileName))
		{
			// TODO: complete this
			bool loadoutFound = 0;
			if(loadoutFound)
			{
				anim = 0;
				useAnimation = 1;
				animParent = 0;
			}
			else
			{
				useAnimation = 0;
				anim = 0;
				animParent = 0;
			}
		}
		if(mdlState == FS_READY && meshState == FS_READY)
		{
			state = FS_READY;
			
			vmeshData = FileLoader::ReadFile(meshFileName);
			
			//KeyValue* root = KVReader2::Parse(vmeshData);
			meshRoot = 0;
			
			//KeyValue* scene = root->Find("m_sceneObjects");
			//KeyValue* drawCalls = scene->Get(0)->Find("m_drawCalls");
			
			dmxHeader* dmxH = (dmxHeader*) vmeshData;
			// Getting VBIB block
			dmxBlockHeader* blockH = dmxH->block(3);
			char* dataM = (char*) blockH->data();
			
			int vhOffset = *((emscripten_align1_int*) (dataM));
			int vhCount = *((emscripten_align1_int*) (dataM + 4));
			int ihOffset = *((emscripten_align1_int*) (dataM + 8));
			int ihCount = *((emscripten_align1_int*) (dataM + 12));
			
			printf("Mesh %s: dB:%c, adr: 0x%X, v[%d @%d], i[%d @%d]\n",meshFileName,blockH->name[0],(unsigned int) dataM,vhCount,vhOffset,ihCount,ihOffset);
			
			//subModelCount = drawCalls->childCount;
			subModelCount = vhCount;
			subModel = new ModelDrawCall*[subModelCount];
			
			dmxBlockHeader* rBlockH = dmxH->block(0);
			rerlHeader* rerlH = rBlockH->rerlData();
			
			//printf("%c%c%c%c\n",rBlockH->name[0],rBlockH->name[1],rBlockH->name[2],rBlockH->name[3]);
			
			rerlRecord* recEntries = (rerlRecord*) (((char*) &rerlH->recordOffset) + rerlH->recordOffset);
			
			/* for(int x=0;x<rerlH->recordCount - 1;x++)
			{
				rerlRecord* rec = recEntries + 1 + x;
				char* recName = ((char*) &rec->nameOffset) + rec->nameOffset;
				printf("rerl %s\n",recName);
			} */
			
			for(int d=0;d<subModelCount;d++)
			{
				
				//KeyValue* dc = drawCalls->Get(d);
				
				// Prepare material
				//KeyValue* materialNode = dc->Find("m_pMaterial");
				
				rerlRecord* rec = recEntries + (rerlH->recordCount - subModelCount + d);
				char* recName = ((char*) &rec->nameOffset) + rec->nameOffset;
				printf("mat %s\n",recName);
				std::string txtFilename = std::string(recName);
				txtFilename = txtFilename + "_c";
				
				printf("-- mat name: %s\n",txtFilename.c_str());
				
				Material* mat = new Material(txtFilename.c_str());
				Manager::add(mat);
				
				// Prepar variable
				subModel[d] = new ModelDrawCall();
				ModelDrawCall* sm = subModel[d];
				
				glGenBuffers(1, (GLuint*) &sm->vertexVBO);
				glGenBuffers(1, (GLuint*) &sm->vertexVBOex);
				glGenBuffers(1, (GLuint*) &sm->meshVBO);
				if(Scene::enableVAO)
				{
					glGenVertexArrays(1, &sm->VAO);
				}
				
				sm->vertexCount = *((emscripten_align1_int*) (dataM + vhOffset + 24*d + 0));
				sm->indexCount = *((emscripten_align1_int*) (dataM + 8 + ihOffset + 24*d + 0));
				sm->material = mat;
				
				printf("-- vertex count: %d, indexCount: %d\n",sm->vertexCount,sm->indexCount);
				
				// Load vertex
				//KeyValue* vbNode = root->Find("m_vertexBuffers")->Get(d);
				char* vertexData = dataM + vhOffset + 24*d + 16 + *((emscripten_align1_int*) (dataM + vhOffset + 24*d + 16));
				int vertexSize = *((emscripten_align1_int*) (dataM + vhOffset + 24*d + 4));
				
				glBindBuffer(GL_ARRAY_BUFFER, sm->vertexVBO);
				unsigned int vertexBufferSize = vertexSize * sm->vertexCount;
				glBufferData(GL_ARRAY_BUFFER, vertexBufferSize , vertexData, GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			
				// Load strip data
				//KeyValue* ibNode = root->Find("m_indexBuffers")->Get(d);
				char* indexData = dataM + 8 + ihOffset + 24*d + 16 + *((emscripten_align1_int*) (dataM + 8 + ihOffset + 24*d + 16));
				int indexSize = *((emscripten_align1_int*) (dataM + 8 + ihOffset + 24*d + 4));
				unsigned int indexBufferSize = indexSize * sm->indexCount;
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sm->meshVBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, indexData, GL_STATIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				
				sm->vertexSize = vertexSize;
				sm->indexSize = indexSize;
				
				sm->vtOffset_Pos = 0;
				sm->vtOffset_Norm = 0;
				sm->vtOffset_UV = 0;
				sm->vtOffset_bIndex = 0;
				sm->vtOffset_bWeight = 0;
				
				// attribute pointer preparation
				int vaCount = *((emscripten_align1_int*) (dataM + vhOffset + 24*d + 12));
				char* vaLabelF = dataM + vhOffset + 24*d + 8 + *((emscripten_align1_int*) (dataM + vhOffset + 24*d + 8));
				
				for(int va=0;va<vaCount;va++)
				{
					char* vaLabel = vaLabelF + (va * 56);	// label size seem to always be 56 char long
					if(strcmp("POSITION",vaLabel)==0)
					{
						sm->vtOffset_Pos = *((emscripten_align1_int*) (vaLabel + 40)); // the offset is at 40 from start of text
					}
					else if(strcmp("NORMAL",vaLabel)==0)
					{
						sm->vtOffset_Norm = *((emscripten_align1_int*) (vaLabel + 40));
					}
					else if(strcmp("TEXCOORD",vaLabel)==0)
					{
						sm->vtOffset_UV = *((emscripten_align1_int*) (vaLabel + 40));
					}
					else if(strcmp("BLENDINDICE",vaLabel)==0)
					{
						sm->vtOffset_bIndex = *((emscripten_align1_int*) (vaLabel + 40));
					}
					else if(strcmp("BLENDWEIGHT",vaLabel)==0)
					{
						sm->vtOffset_bWeight = *((emscripten_align1_int*) (vaLabel + 40));
					}
				}
				
				printf("- data size: vertex: %d, index: %d\n",vertexSize,indexSize);
				printf("- data loc: vertex: 0x%X, index: 0x%X\n",(unsigned int) vertexData,(unsigned int) indexData);
				
				printf("- attrib layout: pos: %d, norm: %d, uv: %d, bInx: %d, bWght: %d\n",sm->vtOffset_Pos,sm->vtOffset_Norm,sm->vtOffset_UV,sm->vtOffset_bIndex,sm->vtOffset_bWeight);
				
				printf("-- Preparing extension data\n");
				// Prepare vertex data extension
				VertEx* vertexExData = new VertEx[sm->vertexCount];
				//for(int v=0;v<sm->vertexCount;v++)	// might consider merging into below loop
				//{
				//	vertexExData[v] = new VertEx();
				//}
				
				// really don't know how to unpack these normal (it's in RGBA8 format that also use alpha for correction)
				// so, fuck it, just calculate normal from vertex
				int trisCount = sm->indexCount / 3;
				glm::vec3* surfaceNormal = new glm::vec3[trisCount];
				emscripten_align1_short* indexes = (emscripten_align1_short*) indexData;
				unsigned short* vertexMeshCount = new unsigned short[sm->vertexCount];
				unsigned short** vertexMesh = new unsigned short*[sm->vertexCount];
				for(int v=0;v<sm->vertexCount;v++)
				{
					vertexMesh[v] = new unsigned short[8];	// 8 mesh per vertex, might not be enough
					vertexMeshCount[v] = 0;
				}
				
				for(int t=0;t<trisCount;t++)
				{
					unsigned short idx1 = *(indexes + (t*3 + 0));
					unsigned short idx2 = *(indexes + (t*3 + 1));
					unsigned short idx3 = *(indexes + (t*3 + 2));
					
					//glm::vec3 v1 = *((glm::vec3*) (vertexData + (28 * idx1)));
					//glm::vec3 v2 = *((glm::vec3*) (vertexData + (28 * idx2)));
					//glm::vec3 v3 = *((glm::vec3*) (vertexData + (28 * idx3)));
					
					//printf("%d %d %d ",idx1,idx2,idx3);
					
					emscripten_align1_float* vData = (emscripten_align1_float*) (vertexData + vertexSize * idx1 + sm->vtOffset_Pos);	// just in case position is not first..
					//vData = vertexData + 28 * idx1;
					float t1 = *(vData+0);
					float t2 = *(vData+1);
					float t3 = *(vData+2);
					glm::vec3 v1(t1,t2,t3);
					vData = (emscripten_align1_float*) (vertexData + vertexSize * idx2);
					t1 = *(vData+0);
					t2 = *(vData+1);
					t3 = *(vData+2);
					glm::vec3 v2(t1,t2,t3);
					vData = (emscripten_align1_float*) (vertexData + vertexSize * idx3);
					t1 = *(vData+0);
					t2 = *(vData+1);
					t3 = *(vData+2);
					glm::vec3 v3(t1,t2,t3);
					
					surfaceNormal[t] = glm::normalize(glm::cross(v2-v1,v3-v1));
					vertexMesh[idx1][vertexMeshCount[idx1]] = t;
					vertexMesh[idx2][vertexMeshCount[idx2]] = t;
					vertexMesh[idx3][vertexMeshCount[idx3]] = t;
					vertexMeshCount[idx1]++;
					vertexMeshCount[idx2]++;
					vertexMeshCount[idx3]++;
				}
				//printf("\n");
				//int alert = 0;
				
				for(int v=0;v<sm->vertexCount;v++)
				{
					//char* nc = (vertexData + (v * vertexSize) + 12);
					//int normal = ((*(nc + 0))<<24) + ((*(nc + 8))<<16) + ((*(nc + 16))<<8) + ((*(nc + 24))<<0);
					//int normal = *((int*) (vertexData + (v * vertexSize) + 12));
					//int nz = (((int) normal & 0x000003FF) << 22) >> 22;	// so we get signed padding
					//int ny = ((normal & 0x000FFC00) << 12) >> 22;
					//int nx = ((normal & 0x3FF00000) << 2) >> 22;
					/*
					int nx = (((int) normal & 0xFFC00000) << 0) >> 20;	// so we get signed padding
					int ny = ((normal & 0x003FF000) << 10) >> 20;
					int nz = ((normal & 0x00000FFC) << 20) >> 20;
					*/
					//PackedNorm* normal = (PackedNorm*) (vertexData + (v * vertexSize) + 12);
					//vertexExData[v].normal[0] = normal->x * maxN;
					//vertexExData[v].normal[1] = normal->y * maxN;
					//vertexExData[v].normal[2] = normal->z * maxN;
					//vertexExData[v].normal = glm::normalize(vertexExData[v].normal);
					for(int t=0;t<vertexMeshCount[v];t++)
					{
						int mid = vertexMesh[v][t];
						vertexExData[v].normal += surfaceNormal[mid];
					}
					vertexExData[v].normal = glm::normalize(vertexExData[v].normal * (1.0f / vertexMeshCount[v]));
					
					// unpack uv from half float to single float
					// (gles2 lacks so many function ...)
					unsigned short tmp1 = *((emscripten_align1_short*) (vertexData + (v * vertexSize) + sm->vtOffset_UV));
					unsigned short tmp2 = *((emscripten_align1_short*) (vertexData + (v * vertexSize) + sm->vtOffset_UV + 2));
					half_float::half* txtU = (half_float::half*) (&tmp1);
					half_float::half* txtV = (half_float::half*) (&tmp2);
					float tU = *txtU;
					float tV = *txtV;
					vertexExData[v].uv = glm::vec2(tU,tV);
					
					if(tU<0 || tU>1 || tV<0 || tV>1) printf("- vertex %d: UV out of bound - %f %f\n",v,tU,tV);
				}
				//if(alert==1) printf("UV out of bound [0-1]\n");
				// tangent calculation moved to fragment shader
				
				// transfer data
				glBindBuffer(GL_ARRAY_BUFFER, sm->vertexVBOex);
				unsigned int vertexExBufferSize = sizeof(VertEx) * sm->vertexCount;
				glBufferData(GL_ARRAY_BUFFER, vertexExBufferSize , vertexExData, GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				delete vertexExData;
				
				for(int v=0;v<sm->vertexCount;v++)
				{
					// cause crash for unknown reason
					//if(vertexMesh[v]) delete vertexMesh[v];
				}
				delete vertexMesh;
				delete surfaceNormal;
				delete vertexMeshCount;
				
				// Bind VAO
				if(Scene::enableVAO)
				{
					glBindVertexArray(sm->VAO);
					this->SetVAO(d);
					glBindVertexArray(0);
				}
				
				printf("-- Preparing bone data\n");
				
				/* printf("draw call %d: %d verts %d indexes\n",d,sm->vertexCount,sm->indexCount);
				for(int v=0;v<sm->vertexCount;v++)
				{
					char* vt = vertexData + (28*v);
					printf("v %d: [%f\t%f\t%f]\tN: [%f\t%f\t%f] l=%f m=%d\n",v,*((float*)(vt)),*((float*)(vt+4)),*((float*)(vt+8)),vertexExData[v].normal[0],vertexExData[v].normal[1],vertexExData[v].normal[2],glm::length(vertexExData[v].normal),vertexMeshCount[v]);
				}
				for(int m=0;m<sm->indexCount;m++)
				{
					char* vi = indexData + (2*m);
					printf("%hu ",(*((short*) vi)));
				}
				printf("\n"); */
				
				
				// calculate bone data
				// TODO: calculate them
				//sm->boneCount = 1;
				//sm->meshBoneList = new unsigned int[sm->boneCount];
				//sm->meshBoneIndex = new unsigned int[128];	// = MAX_BONE
				
				printf("-- Finished\n");
				
			}

			// dont't clean up yet
			//KVReader2::Clean(root);
			
		}
	}
	else if(state==FS_READY)
	{
		// handle animation here
		/* if(useAnimation)
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
							
							//if(p>=0)
							//{
							//	int curPar = p;
							//	mdlBone* par = 0;
							//	do
							//	{
							//		par = mh->pBone(curPar);
							//		jointPos -= glm::vec4(bonePos[curPar],0);
							//		curPar = par->parent;
							//	} while(curPar >= 0);
							//	//jointPos = boneTransform[p] * jointPos;
							//}
							
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
								// glm::mat4 toSMD(
								//	0.0f,	0.0f,	-1.0f,	0.0f,
								//	1.0f,	0.0f,	0.0f,	0.0f,
								//	0.0f,	1.0f,	0.0f,	0.0f,
								//	0.0f,	0.0f,	0.0f,	1.0f
								//);
								//glm::mat4 fromSMD(
								//	0.0f,	1.0f,	0.0f,	0.0f,
								//	0.0f,	0.0f,	1.0f,	0.0f,
								//	-1.0f,	0.0f,	0.0f,	0.0f,
								//	0.0f,	0.0f,	0.0f,	1.0f
								//);
								//glm::mat4 toSMD(1);
								//	0.0f,	-1.0f,	0.0f,	0.0f,
								//	0.0f,	0.0f,	1.0f,	0.0f,
								//	1.0f,	0.0f,	0.0f,	0.0f,
								//	0.0f,	0.0f,	0.0f,	1.0f
								//);
								//glm::mat4 fromSMD(1);
								//	0.0f,	0.0f,	1.0f,	0.0f,
								//	-1.0f,	0.0f,	0.0f,	0.0f,
								//	0.0f,	1.0f,	0.0f,	0.0f,
								//	0.0f,	0.0f,	0.0f,	1.0f
								//);
								//toSMD = glm::transpose(toSMD);
								//fromSMD = glm::transpose(fromSMD);
								
								boneTransform[b] = fromSMD * glm::translate(glm::mat4(1), bAnimPosSw) * glm::mat4_cast(bAnimRotSw) * glm::transpose(glm::mat4_cast(bOrigRotSw)) * glm::translate(glm::mat4(1), -1.0f * bOrigPosSw) * toSMD;
							//} 
							//else
							//{
							//	boneTransform[b] = glm::translate(glm::mat4(1), bAnimPosSw) * glm::translate(glm::mat4(1), -1.0f * jointPos);
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
		}*/
	}
	
}

void Model::Draw(ESContext *esContext)
{
	//return;
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
		// shader check
		bool shaderActive = 
			(Scene::currentStep == RS_SCENE && shader != 0) || 
			(Scene::currentStep == RS_SHADOW && shaderShadow != 0);
			
		if(shaderActive)
		{
			// draw each submodel
			for(int i=0;i<subModelCount;i++) 
			{
				if(subModel[i]->vertexVBO!=0)
				{
					//printf("Drawing subModel %d\n",i);
					// vao
					if(Scene::enableVAO)
					{
						glBindVertexArray(subModel[i]->VAO);
					}
					else
					{
						this->SetVAO(i);
					}
					
					if(Scene::currentStep==RS_SCENE) shader->Bind(this, i);
					else if(Scene::currentStep==RS_SHADOW) shaderShadow->Bind(this, i);
					
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subModel[i]->meshVBO);
					
					if(Scene::currentStep==RS_SCENE) shader->Populate(this, i);
					else if(Scene::currentStep==RS_SHADOW) shaderShadow->Populate(this, i);
		
					// real drawing code, just 3 lines lol
					glDrawElements(GL_TRIANGLES, subModel[i]->indexCount, GL_UNSIGNED_SHORT, 0);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
		
					if(Scene::currentStep==RS_SCENE) shader->Unbind(this, i);
					else if(Scene::currentStep==RS_SHADOW) shaderShadow->Unbind(this, i);
					
					if(Scene::enableVAO)
					{
						glBindVertexArray(0);
					}
					
					//for(int j=0;j<10;j++) glDisableVertexAttribArray(j);
					
				}
			}
		}
	}
}

void Model::SetVAO(int i)
{
	if(subModel[i]->vertexVBO!=0)
	{ 
		unsigned int vts = subModel[i]->vertexSize;
		glBindBuffer(GL_ARRAY_BUFFER, subModel[i]->vertexVBO);
		// XYZ pos
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vts, (void*) subModel[i]->vtOffset_Pos);
		glEnableVertexAttribArray(0);
		// Normal
		//glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, 28, (void*) 12);
		//glEnableVertexAttribArray(1);
		// UV
		//glVertexAttribPointer(2, 2, GL_HALF_FLOAT, GL_FALSE, 28, (void*) 16);
		//glEnableVertexAttribArray(2);
		// num bones
		glVertexAttribPointer(3, 1, GL_UNSIGNED_BYTE, GL_FALSE, vts, (void*) (subModel[i]->vtOffset_bIndex + 3));	// not used anymore?
		glEnableVertexAttribArray(3);
		// bone id
		glVertexAttribPointer(4, 1, GL_UNSIGNED_BYTE, GL_FALSE, vts, (void*) (subModel[i]->vtOffset_bIndex));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(5, 1, GL_UNSIGNED_BYTE, GL_FALSE, vts, (void*) (subModel[i]->vtOffset_bIndex + 1));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(6, 1, GL_UNSIGNED_BYTE, GL_FALSE, vts, (void*) (subModel[i]->vtOffset_bIndex + 2));
		glEnableVertexAttribArray(6);
		// bone weight
		glVertexAttribPointer(7, 1, GL_UNSIGNED_BYTE, GL_TRUE, vts, (void*) (subModel[i]->vtOffset_bWeight));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(8, 1, GL_UNSIGNED_BYTE, GL_TRUE, vts, (void*) (subModel[i]->vtOffset_bWeight + 1));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(9, 1, GL_UNSIGNED_BYTE, GL_TRUE, vts, (void*) (subModel[i]->vtOffset_bWeight + 2));
		glEnableVertexAttribArray(9);
	}
	
	if(subModel[i]->vertexVBOex!=0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, subModel[i]->vertexVBOex);
		// Normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertEx), (void*) 0);
		glEnableVertexAttribArray(1);
		
		// UV
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertEx), (void*) 12);
		glEnableVertexAttribArray(2);
		
		// tangent - moved to frag shader
		//glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*) 0);
		//glEnableVertexAttribArray(10);
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