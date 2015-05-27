#pragma once

#include "../glm/glm.hpp"

#define MAX_NUM_BONES_PER_VERT 3
#define MAX_NUM_LODS 8

#pragma pack(push, 1)
struct vtxHeader
{
	// file version as defined by OPTIMIZED_MODEL_FILE_VERSION (currently 7)
	int version;
 
	// hardware params that affect how the model is to be optimized.
	int vertCacheSize;
	unsigned short maxBonesPerStrip;
	unsigned short maxBonesPerFace;
	int maxBonesPerVert;
 
	// must match checkSum in the .mdl
	int checkSum;
 
	int numLODs; 
 
	int materialReplacementListOffset;
 
	int numBodyParts;
	int bodyPartOffset; // offset to an array of BodyPartHeader_t's
};

struct vtxBodyPart
{
	int numModels;
	int modelOffset;
};

// match 1:1 to models in mdl
struct vtxModelHeader
{
	int numLODs;	// should match other place
	int lodOffset;
};

struct vtxModelLOD
{
	int numMeshes;
	int meshOffset;
	float switchPoint;
};

struct vtxMesh
{
	int numStripGroups;
	int stripGroupHeaderOffset;

	unsigned char flags;
};

// a locking group
// a single vertex buffer
// a single index buffer
struct vtxStripGroup
{
	// These are the arrays of all verts and indices for this mesh.  strips index into this.
	int numVerts;
	int vertOffset;
	
	int numIndices;
	int indexOffset;

	// used for software skinning?
	int numStrips;
	int stripOffset;

	unsigned char flags;
	
	int unknown1;
	int unknown2;

};

enum vtxStripType {
	STRIP_IS_TRILIST	= 0x01,
	STRIP_IS_TRISTRIP	= 0x02
};

// a strip is a piece of a stripgroup that is divided by bones 
// (and potentially tristrips if we remove some degenerates.)
struct vtxStrip
{
	// indexOffset offsets into the mesh's index array.
	int numIndices;
	int indexOffset;

	// vertexOffset offsets into the mesh's vert array.
	int numVerts;
	int vertOffset;

	// use this to enable/disable skinning.  
	// May decide (in optimize.cpp) to put all with 1 bone in a different strip 
	// than those that need skinning.
	short numBones;  
	
	unsigned char flags;
	
	int numBoneStateChanges;
	int boneStateChangeOffset;
};

struct vtxVertex
{
	// these index into the mesh's vert[origMeshVertID]'s bones
	unsigned char boneWeightIndex[MAX_NUM_BONES_PER_VERT];
	unsigned char numBones;

	unsigned short origMeshVertID;

	// for sw skinned verts, these are indices into the global list of bones
	// for hw skinned verts, these are hardware bone indices
	char boneID[MAX_NUM_BONES_PER_VERT];
};
#pragma pack(pop)