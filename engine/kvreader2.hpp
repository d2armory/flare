// 
// Source 2 KeyValue Binary Reader
// by @bongikairu
//
// File structure studied from https://github.com/Dingf/Source-2-Decompiler
//
// A note on this:
// 
//    So, this new source format is like a self-describing data structure
//    (Think of xml or json). It put its structure in block called NTRO (Introspection)
//    and its data in DATA block. External referenced file is listed in RERL block
//    with can be easily accessed.
//
//    There's a lot of detail that I don't know, but basically, everything is lay out flat.
//    Nested-data is represented using 4:offset 4:count. Every offset is from the location
//    of that offset value (so, target is located at   ((char*) &offset) + offset   , easy?)
//

#pragma once

#include <stdlib.h>
#include <cstring>
#include <string>
#include <stdio.h>
#include <ctype.h>
#include <functional>

// File format struct for easy referencing
#include "../mdl/dmxHeader.h"
#include "../glm/glm.hpp"

// Direct from https://github.com/Dingf/Source-2-Decompiler/blob/master/ntro.h
enum NtroValueType
{
	NTRO_DATA_TYPE_STRUCT = 1,
	NTRO_DATA_TYPE_ENUM = 2,
	NTRO_DATA_TYPE_HANDLE = 3,
	NTRO_DATA_TYPE_STRING = 4,
	NTRO_DATA_TYPE_BYTE = 11,
	NTRO_DATA_TYPE_SHORT = 12,
	NTRO_DATA_TYPE_USHORT = 13,
	NTRO_DATA_TYPE_INTEGER = 14,
	NTRO_DATA_TYPE_UINTEGER = 15,
	NTRO_DATA_TYPE_INT64 = 16,
	NTRO_DATA_TYPE_UINT64 = 17,
	NTRO_DATA_TYPE_FLOAT = 18,
	NTRO_DATA_TYPE_VECTOR3 = 22,
	NTRO_DATA_TYPE_QUATERNION = 25,
	NTRO_DATA_TYPE_VECTOR4 = 27,
	NTRO_DATA_TYPE_COLOR = 28,
	NTRO_DATA_TYPE_BOOLEAN = 30,
	NTRO_DATA_TYPE_NAME = 31,
};

// KeyValue data structure
class KeyValue
{
	
public:
	
	// PCSTree main structure
	KeyValue* parent;
	KeyValue* child;
	KeyValue* sibling;
	unsigned int keyHash;	// hash for fast compare
	
	// pointer to key and value
	char* key;
	char* value;
	
	char* childCountAddress; // temp debug
	unsigned int realChildCount;
	
	unsigned char depth;
	unsigned char type;
	unsigned char childCount;
	unsigned char padding;	// Ed Keenan said I should pad
	
	//
	// Big-4
	//
	
	// Constructor
	KeyValue();
	// Destructor
	~KeyValue();
	// TODO: add other 2 big-4
	
	//
	// Tree manipulation
	//
	
	// Tell this node to calculate its hash, use on node with key != 0 only
	void CalcHash();
	
	// Attach node to parent
	void Attach(KeyValue* child);
	
	//
	// Utility Accessor
	//
	
	// Get n-th child (start at 0)
	KeyValue* Get(int index) const;
	
	// Find child node with given key
	KeyValue* Find(const char* name) const;

	// Return value part as string
	const char* AsName() const;
	// Return resolved external filename reference
	const char* AsHandle() const;
	// Return value part as unsigned char
	uint8_t AsByte() const;
	// Return value part as signed short
	int16_t AsShort() const;
	// Return value part as unsigned short
	uint16_t AsUshort() const;
	// Return value part as signed int
	int32_t AsInt() const;
	// Return value part as unsigned int
	uint32_t AsUint() const;
	// Return value part as signed long (64)
	int64_t AsLong() const;
	// Return value part as unsigned long (64)
	uint64_t AsUlong() const;
	// Return value part as float (32)
	float AsFloat() const;
	// Return value part as Vector3
	glm::vec3 AsVec3() const;
	// Return value part as Vector4
	glm::vec4 AsVec4() const;
};

class KVReader2
{
	
public:

	// Parse data into KV2 Format
	static KeyValue* Parse(char* data);
	// Clean up tree
	static void Clean(KeyValue* pData);
	
	// Debug function to dump tree structure
	static void Dump(KeyValue* cur, unsigned int startAddress);
	
private:

	// Applying structure recursively
	static void ApplyStruct(KeyValue* parent, ntroStruct* str, char* dataH, rerlHeader* rerlH, ntroHeader* ntroH);
	static void ApplyField(KeyValue* node, ntroField* f, char* dataF, rerlHeader* rerlH, ntroHeader* ntroH);
	
};