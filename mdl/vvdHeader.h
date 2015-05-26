#pragma once

#include "../glm/glm.hpp"

#define MAX_NUM_BONES_PER_VERT 3
#define MAX_NUM_LODS 8

struct vvdHeader
{
	int	id;				// MODEL_VERTEX_FILE_ID
	int	version;			// MODEL_VERTEX_FILE_VERSION
	long	checksum;			// same as studiohdr_t, ensures sync
	int	numLODs;			// num of valid lods
	int	numLODVertexes[MAX_NUM_LODS];	// num verts for desired root lod
	int	numFixups;			// num of vertexFileFixup_t
	int	fixupTableStart;		// offset from base to fixup table
	int	vertexDataStart;		// offset from base to vertex block
	int	tangentDataStart;		// offset from base to tangent block
};

struct vvdFixupTable
{
	int	lod;			// used to skip culled root lod
	int	sourceVertexID;		// absolute index from start of vertex/tangent blocks
	int	numVertexes;
};

// 48 bytes
struct vvdVertexFormat
{
	float	boneweight[MAX_NUM_BONES_PER_VERT];	// 0, 4, 8
	char	boneid[MAX_NUM_BONES_PER_VERT]; 	// 12, 13, 14
	byte	numbones;							// 15
	glm::vec3			m_vecPosition;			// 16
	glm::vec3			m_vecNormal;			// 28
	glm::vec2		m_vecTexCoord;				// 40
};