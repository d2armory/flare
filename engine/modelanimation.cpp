#include "modelanimation.hpp"

#include "model.hpp"

ModelAnimation::ModelAnimation(const char* fileName)
{
	
}
ModelAnimation::~ModelAnimation()
{
	
}
	
void ModelAnimation::Update(ESContext *esContext, float deltaTime)
{
	
}
void ModelAnimation::Draw(ESContext *esContext, Model* model)
{

	
}
// need model to compute inverse bind pose, and bone name mapping
	
void ModelAnimation::ExtractFrame(BoneData*& output, int frame)
{
	// find block contains that frame
	KeyValue* root = animRoot;
	KeyValue* animDesc = root->Find("m_animArray")->Get(0);
	KeyValue* animBlockDesc = animDesc->Find("m_pData")->Get(0);
	KeyValue* blockArray = animBlockDesc->Find("m_frameblockArray");
	int perBlock = animBlockDesc->Find("m_nFramesPerBlock")->AsInt();
	int totalFrame = animBlockDesc->Find("m_nFrames")->AsInt();
	int block = frame / perBlock;
	if(totalFrame>100 && frame==totalFrame-1) block = blockArray->childCount - 1; // last frame is stored separately
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
				eFrame = 1;
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
	
	output.rot = glm::quat(x,y,z,w);
}