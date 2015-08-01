#include "modelanimation.hpp"

#include "model.hpp"

ModelAnimation::ModelAnimation(const char* fileName)
{
	// load file
	strncpy(this->fileName,fileName,MODEL_ANIMATION_NAME_LENGTH);
	state = FS_UNINIT;
	animRoot = 0;
	parent = 0;
	
	boneCount = 1;
	
	frameCount = 0;
	fps = 0;
	curTime = 0.0f;
	speed = 1.0f;
	
	frameC = 0;
	frameN = 0;
	frameB = 0;
	
}
ModelAnimation::~ModelAnimation()
{
	// unload data
	if(animRoot)
	{
		KVReader2::Clean(animRoot);
	}
	if(frameC)
	{
		delete frameC;
		delete frameN;
		delete frameB;
	}
}
	
void ModelAnimation::Update(ESContext *esContext, float deltaTime)
{
	if(state==FS_UNINIT)
	{
		state = FS_LOADING;
		FileLoader::Load(fileName);
	}
	else if(state==FS_LOADING)
	{
		if(FileLoader::FileExist(fileName))
		{
			
			printf("File %s loaded\n",fileName);
			
			// load files
			state = FS_READY;
			fileData = FileLoader::ReadFile(fileName);
			animRoot = KVReader2::Parse(fileData);
			
			printf("File %s parsed\n",fileName);
			
			// prepare variable
			KeyValue* animDesc = animRoot->Find("m_animArray")->Get(0);
			fps = animDesc->Find("fps")->AsFloat();
			KeyValue* animBlockDesc = animDesc->Find("m_pData")->Get(0);
			frameCount = animBlockDesc->Find("m_nFrames")->AsInt();
			
			// allocate bone data space
			frameC = new BoneData[boneCount];	// bone count set from parent
			frameN = new BoneData[boneCount];
			frameB = new BoneData[boneCount];
			boneTransform = new glm::mat4[boneCount];
			
			printf("File %s prepared\n",fileName);
			
			ExtractFrame(frameC,fNum);
			
			printf("Frame 0 data\n");
			for(int i=0;i<boneCount;i++)
			{
				printf("b %d - pos: %f %f %f - rot: %f %f %f %f\n",i,frameC[i].pos[0],frameC[i].pos[1],frameC[i].pos[2],frameC[i].rot[0],frameC[i].rot[1],frameC[i].rot[2],frameC[i].rot[3]);
			}
		}
	}
	else if(state==FS_READY)
	{
		// calculate frame number
		curTime += deltaTime * speed;
		
		float maxTime = (frameCount-1) / fps;	// not for seamless lopp
		if(curTime > maxTime) curTime -= maxTime;
		
		float fVal = curTime * fps;
		int fNum = floor(fVal);
		float bNum = fVal - fNum;
		
		int nfNum = fNum + 1;
		if(nfNum == frameCount) nfNum = 0;
		
		// prepare current frame
		//printf("Extracting c frame %d\n",fNum);
		ExtractFrame(frameC,fNum);
		
		// prepare next frame
		//printf("Extracting n frame %d\n",nfNum);
		ExtractFrame(frameN,nfNum);
		
		// blend frame
		KeyValue* skeleton = parent->mdlRoot->Find("m_modelSkeleton");
		KeyValue* boneParent = skeleton->Find("m_nParent");
			
		//printf("Blending frame %d and %d with a = %f @ t=%f [delta: %f , speed: %f , fps: %f]\n",fNum,nfNum,bNum,curTime,deltaTime,speed,fps);
			
		for(int i=0;i<boneCount;i++)
		{
			frameB[i].pos = glm::lerp(frameC[i].pos,frameN[i].pos,bNum);
			frameB[i].rot = glm::slerp(frameC[i].rot,frameN[i].rot,bNum);
			
			short bparent = -1;
			if(boneParent->Get(i)) bparent = boneParent->Get(i)->AsShort();
			else printf("Unknown bone %d\n",i);
			
			boneTransform[i] = glm::mat4(1);
			boneTransform[i] = glm::mat4_cast(frameB[i].rot) * boneTransform[i];
			boneTransform[i] = glm::translate(boneTransform[i],frameB[i].pos);
			if(bparent >= 0)
			{
				boneTransform[i] = boneTransform[bparent] * boneTransform[i];
			}
			
			/* if(fNum==0)
			{
				printf("b %d: ",i);
				for(int j=0;j<16;j++)
				{
					printf("%f ",boneTransform[i][j/4][j%4]);
				}
				printf("\n");
			} */
			
		}
		
	}
	
}
void ModelAnimation::Draw(ESContext *esContext, Model* model)
{
	if(state==FS_READY)
	{
		// copy data by bone mapping
		for(int i=0;i<model->numBone;i++)
		{
			if(model->boneMap[i] >= 0)
			{
				model->boneTransform[i] = glm::transpose(boneTransform[model->boneMap[i]] * model->invBoneTransform[i]);
			}
			else
			{
				model->boneTransform[i] = glm::mat4(1);
			}
			
		}
		
		if(model->boneTransformTexture != 0 && Scene::enableFloatTexture)
		{
			// put bone transform on GPU
			/* glActiveTexture(GL_TEXTURE0 + 5);
			glBindTexture(GL_TEXTURE_2D, model->boneTransformTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, model->numBone, 0, GL_RGBA, GL_FLOAT, &model->boneTransform[0]); */
		}
	}
	else
	{
		for(int i=0;i<model->numBone;i++)
		{
			model->boneTransform[i] = glm::mat4(1);
		}
	}
}
// need model to compute inverse bind pose, and bone name mapping
	
void ModelAnimation::ExtractFrame(BoneData*& output, int frame)
{
	// find block contains that frame
	// might move these to constructor
	KeyValue* root = animRoot;
	KeyValue* animDesc = root->Find("m_animArray")->Get(0);
	KeyValue* animBlockDesc = animDesc->Find("m_pData")->Get(0);
	KeyValue* blockArray = animBlockDesc->Find("m_frameblockArray");
	int perBlock = animBlockDesc->Find("m_nFramesPerBlock")->AsInt();
	int totalFrame = animBlockDesc->Find("m_nFrames")->AsInt();
	int block = frame / perBlock;
	if(totalFrame>100 && frame==totalFrame-1) block = blockArray->childCount - 1; // last frame is stored separately
	// TODO: change to linear search?
	
	// loop over each segment
	KeyValue* blockObj = blockArray->Get(block);
	KeyValue* segIdxArray = blockObj->Find("m_segmentIndexArray");
	int framePerBlock = blockObj->Find("m_nEndFrame")->AsInt() - blockObj->Find("m_nStartFrame")->AsInt();
	int frameInBlock = frame - blockObj->Find("m_nStartFrame")->AsInt();
	if(framePerBlock==0) framePerBlock = 1;	// 1 frame case
	// data access
	KeyValue* segmentArray = root->Find("m_segmentArray");
	KeyValue* decoder = root->Find("m_decoderArray");
	int boneCount = 59;
	for(int i=0;i<segIdxArray->childCount;i++)
	{
		int segId = segIdxArray->Get(i)->AsInt();
		KeyValue* segment = segmentArray->Get(segId);
		char* container = segment->Find("m_container")->Get(0)->value;
		short type1 = *((short*)(container+0)); // data type as in decode array?
		short type2 = *((short*)(container+2));	// size of some kind? may be number of elements?
		short count = *((short*)(container+4));
		short containerSize = *((short*)(container+6));
		const char* keyType = decoder->Get(type1)->Find("m_szName")->AsName();
		char* containerData = container + 8 + count*2;
		int containerDataSize = containerSize - 8 - count*2;
		int perKey = containerDataSize/count;
		int perKeyPerFrame = containerDataSize/count/framePerBlock;
		
		void (ModelAnimation::*Extract)(BoneData&,const char*);
		
		if(strcmp(keyType,"CCompressedStaticFullVector3")==0)
		{
			Extract = &ModelAnimation::ExtractDataFullVector;
		}
		else if(strcmp(keyType,"CCompressedStaticVector3")==0)
		{
			Extract = &ModelAnimation::ExtractDataHalfVector;
		}
		else if(strcmp(keyType,"CCompressedFullVector3")==0)
		{
			Extract = &ModelAnimation::ExtractDataFullVector;
		}
		else if(strcmp(keyType,"CCompressedAnimVector3")==0)
		{
			Extract = &ModelAnimation::ExtractDataHalfVector;
		}
		else if(strcmp(keyType,"CCompressedStaticQuaternion")==0)
		{
			Extract = &ModelAnimation::ExtractDataQuaternion;
		}
		else if(strcmp(keyType,"CCompressedAnimQuaternion")==0)
		{
			Extract = &ModelAnimation::ExtractDataQuaternion;
		}
		
		for(int j=0;j<count;j++)
		{
			int boneIdx = *((short*)(container+8+(j*2)));
			boneIdx = boneIdx % boneCount;
			int eFrame = frameInBlock;
			if(perKeyPerFrame==0)
			{
				eFrame = 0;
			}
			char* fkData = containerData + perKeyPerFrame * count * eFrame + perKeyPerFrame * j;
			((*this).*Extract)(output[boneIdx],fkData);
		}
	}
	
}
void ModelAnimation::ExtractDataFullVector(BoneData& output, const char* fkData)
{
	float x = *((emscripten_align1_float*) (fkData));
	float y = *((emscripten_align1_float*) (fkData + 4));
	float z = *((emscripten_align1_float*) (fkData + 8));
	output.pos = glm::vec3(x,y,z);
}
void ModelAnimation::ExtractDataHalfVector(BoneData& output, const char* fkData)
{
	unsigned short ix = *((emscripten_align1_short*) (fkData));
	unsigned short iy = *((emscripten_align1_short*) (fkData + 2));
	unsigned short iz = *((emscripten_align1_short*) (fkData + 4));
	half_float::half* px = (half_float::half*) (&ix);
	half_float::half* py = (half_float::half*) (&iy);
	half_float::half* pz = (half_float::half*) (&iz);
	float x = *px;
	float y = *py;
	float z = *pz;
	output.pos = glm::vec3(x,y,z);
}
void ModelAnimation::ExtractDataQuaternion(BoneData& output, const char* fkData)
{
	unsigned short ix = *((emscripten_align1_short*) (fkData));
	unsigned short iy = *((emscripten_align1_short*) (fkData + 2));
	unsigned short iz = *((emscripten_align1_short*) (fkData + 4));
	
	const unsigned short mask = 0x7FFF;
	const unsigned short midpoint = 0x4000;
	const float scale = 4.3162985E-5;	// 1/16384/1.414
	
	float x = ((float) ((ix & mask) - midpoint)) * scale;
	float y = ((float) ((iy & mask) - midpoint)) * scale;
	float z = ((float) ((iz & mask) - midpoint)) * scale;
	float w = 1 - (x*x) - (y*y) - (z*z);
	if(w>FLT_EPSILON) w = sqrtf(w);
	if((((iz & 0x8000) >> 15) & 1)) w = -w;
	int pos = ((((ix & 0x8000) >> 15) & 1) << 1) | ((((iy & 0x8000) >> 15) & 1) << 0);
	// TODO: swizzle on pos
	
	switch(pos)
	{
		case 3:
			output.rot = glm::quat(w,x,y,z);
			break;
		case 2:
			output.rot = glm::quat(x,w,y,z);
			break;
		case 1:
			output.rot = glm::quat(x,y,w,z);
			break;
		case 0:
			output.rot = glm::quat(x,y,z,w);
			break;
			
	}
}