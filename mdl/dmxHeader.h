#pragma once

#include "../glm/glm.hpp"
#include <emscripten.h>

// Source 2 data structure
// mostly from https://github.com/Dingf/Source-2-Decompiler/blob/master/vtex.h

struct ntroField
{
	emscripten_align1_int nameOffset;
	emscripten_align1_short diskSize;
	emscripten_align1_short diskOffset;
	emscripten_align1_int indirectOffset;
	emscripten_align1_int indirectLevel;
	emscripten_align1_int typeData;
	emscripten_align1_short type;
	emscripten_align1_short padding1;
};

struct ntroStruct
{
	emscripten_align1_int version;
	emscripten_align1_int id;
	emscripten_align1_int nameOffset;
	emscripten_align1_int crc;
	emscripten_align1_int userVersion;
	emscripten_align1_short size;
	emscripten_align1_short alignment;
	emscripten_align1_int baseStructId;
	emscripten_align1_int fieldOffset;
	emscripten_align1_int fieldCount;
	emscripten_align1_int flags;
	
	inline const char* name()
	{
		return ((char*) &this->nameOffset) + nameOffset;	
	};
};

struct ntroEnumRes
{
	emscripten_align1_int nameOffset;
	emscripten_align1_int data;
};

struct ntroEnum
{
	emscripten_align1_int version;
	emscripten_align1_int id;
	emscripten_align1_int nameOffset;
	emscripten_align1_int u1;
	emscripten_align1_int u2;
	emscripten_align1_int resourceOffset;
	emscripten_align1_int resourceCount;
	emscripten_align1_int u3;
	
};

struct ntroHeader
{
	emscripten_align1_int version;
	emscripten_align1_int structOffset;
	emscripten_align1_int structCount;
	emscripten_align1_int enumOffset;
	emscripten_align1_int enumCount;
};

struct vtexFrame
{
	emscripten_align1_float displayTime;
	emscripten_align1_int dataOffset;
	emscripten_align1_int dataCount;
	glm::vec2 croppedMin;
	glm::vec2 croppedMax;
	glm::vec2 uncroppedMin;
	glm::vec2 uncroppedMax;
};

struct vtexSeq
{
	emscripten_align1_int id;
	emscripten_align1_int flags;
	emscripten_align1_int frameOffset;
	emscripten_align1_int frameCount;
	emscripten_align1_float totalTime;
	emscripten_align1_int nameOffset;
	emscripten_align1_int unknown1Offset;	// float params what?
	emscripten_align1_int unknown1Count;
};

struct vtexSheet
{
	emscripten_align1_int seqOffset;
	emscripten_align1_int seqCount;
};

// 52 bytes total
struct vtexHeader
{
	unsigned int unused[5];	// 20 byte of sheet offset, reflectivity, and other things
	emscripten_align1_short width;
	emscripten_align1_short height;
	emscripten_align1_short depth;
	unsigned char format;	// 1=dxt1, 2=dxt5
	unsigned char mipLevel;
	unsigned char unknown1[4];
	unsigned int unused2[4]; // another unknown of 20 byte
	emscripten_align1_int dataOffset;
};

/* struct vtexHeader
{
	emscripten_align1_short width;
	emscripten_align1_short height;
	emscripten_align1_short depth;
	unsigned char format;	// same as vtf, 13=dxt1, 15=dxt5
	unsigned char mipLevel;
	unsigned char unknown1[4];
	emscripten_align1_short msType;
	emscripten_align1_short flags;
	glm::vec4 reflectivity;
	emscripten_align1_int sheetOffset;
	emscripten_align1_int sheetCount;
	// follows by preview data? (if it's the same as mdl, it should be small dxt1 preview data, 1380 char long)
	// and then texture data (which I assume is in the same format as vtf, 
	// judging from how https://github.com/tranek/vtex_c2tga/blob/master/vtex_c2tga.cpp did it)
}; */

struct rerlRecord
{
	emscripten_align1_int id[2];
	emscripten_align1_int nameOffset;
	emscripten_align1_int pad;
};

struct rerlHeader
{
	emscripten_align1_int recordOffset;
	emscripten_align1_int recordCount;
};

struct dmxBlockHeader
{

	char name[4];	// warning: it's not null-terminated
	emscripten_align1_int dataOffset;
	emscripten_align1_int dataSize;
	
	inline void* data()
	{
		return (void*) (((char*) &this->dataOffset) + this->dataOffset);
	};
	
	inline vtexHeader* vtexData()
	{
		return (vtexHeader*) data();
	};
	
	inline ntroHeader* ntroData()
	{
		return (ntroHeader*) data();	
	};
	
	inline rerlHeader* rerlData()
	{
		return (rerlHeader*) data();	
	};
	
};

struct dmxHeader
{

	emscripten_align1_int fileSize;
	emscripten_align1_int sig;	// it always be 0x0c
	emscripten_align1_int blockOffset;
	emscripten_align1_int blockCount;
	
	inline dmxBlockHeader* block(int b)
	{
		// add assert?
		return ((dmxBlockHeader*) (((char*) &this->blockOffset) + this->blockOffset)) + b;	
	};
	
};