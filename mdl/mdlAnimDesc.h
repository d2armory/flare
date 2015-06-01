#pragma once

#include "../glm/glm.hpp"
#include "../glm/gtc/quaternion.hpp"
#include "../glm/gtx/quaternion.hpp"
#include <inttypes.h>
#include <emscripten.h>
#include <stdio.h>

#define STUDIO_ANIM_RAWPOS	0x01 // Vector48
#define STUDIO_ANIM_RAWROT	0x02 // Quaternion48
#define STUDIO_ANIM_ANIMPOS	0x04 // mstudioanim_valueptr_t
#define STUDIO_ANIM_ANIMROT	0x08 // mstudioanim_valueptr_t
#define STUDIO_ANIM_DELTA	0x10
#define STUDIO_ANIM_RAWROT2	0x20 // Quaternion64

union mdlAnimValue
{
	struct 
	{
		byte	valid;
		byte	total;
	} num;
	short		value;
};

struct mdlAnimValuePtr
{
	short	offset[3];
	inline mdlAnimValue *pAnimvalue( int i ) const { if (offset[i] > 0) return  (mdlAnimValue *)(((byte *)this) + offset[i]); else return 0; };
};

union Float32
{
	float rawFloat;
	struct 
	{
		unsigned int mantissa : 23;
		unsigned int biased_exponent : 8;
		unsigned int sign : 1;
	} bits;
};

union Float16
{
	unsigned short data;	
	struct
	{
		unsigned short mantissa : 10;
		unsigned short biased_exponent : 5;
		unsigned short sign : 1;
	} bits;
	
	inline float unpack(void) const
	{
		Float32 output;
		
		const int float32bias = 127;
		const int float16bias = 15;

		if( this->bits.biased_exponent == 0 && this->bits.mantissa != 0 )
		{
			// denorm
			const float half_denorm = (1.0f/16384.0f); // 2^-14
			float mantissa = ((float)(this->bits.mantissa)) / 1024.0f;
			float sgn = (this->bits.sign)? -1.0f :1.0f;
			output.rawFloat = sgn*mantissa*half_denorm;
		}
		else
		{
			// regular number
			unsigned mantissa = this->bits.mantissa;
			unsigned biased_exponent = this->bits.biased_exponent;
			unsigned sign = ((unsigned) this->bits.sign) << 31;
			biased_exponent = ( (biased_exponent - float16bias + float32bias) * (biased_exponent != 0) ) << 23;
			mantissa <<= (23-10);

			*((unsigned *)&output) = ( mantissa | biased_exponent | sign );
		}
		
		return output.rawFloat;
	};
};
struct Quaternion48
{
	unsigned short x:16;
	unsigned short y:16;
	unsigned short z:15;
	unsigned short wneg:1;
	
	inline glm::quat unpack(void) const
	{
		glm::quat tmp;
		tmp[0] = ((int)x - 32768) * (1 / 32768.0);
		tmp[1] = ((int)y - 32768) * (1 / 32768.0);
		tmp[2] = ((int)z - 16384) * (1 / 16384.0);
		tmp[3] = sqrt( 1 - tmp[0] * tmp[0] - tmp[1] * tmp[1] - tmp[2] * tmp[2] );
		if (wneg)
			tmp[3] = -tmp[3];
		return tmp; 	
	};
};
struct Quaternion64
{
	/* emscripten_align1_int x:21;
	emscripten_align1_int y:21;
	emscripten_align1_int z:21;
	emscripten_align1_int wneg:1; */
	unsigned char data[64];
	inline glm::quat unpack(void) const
	{
		glm::quat tmp;
		
		unsigned char* p = (unsigned char*) this;
		unsigned int nx = ((p[0]&0xFF)<<0) + ((p[1]&0xFF)<<8) + ((p[2]&0x1F)<<16);
		unsigned int ny = ((p[2]&0xE0)>>5) + ((p[3]&0xFF)<<3) + ((p[4]&0xFF)<<11) + ((p[5]&0x03)<<19);
		unsigned int nz = ((p[5]&0xFC)>>2) + ((p[6]&0xFF)<<6) + ((p[7]&0x7F)<<14);
		unsigned int nw = ((p[7]&0x80)>>7);
		
		//int nw = ((int) *(((char*) this) + 63));
		// shift to -1048576, + 1048575, then round down slightly to -1.0 < x < 1.0
		tmp[0] = ((int)nx - 1048576) * (1 / 1048576.5f);
		tmp[1] = ((int)ny - 1048576) * (1 / 1048576.5f);
		tmp[2] = ((int)nz - 1048576) * (1 / 1048576.5f);
		tmp[3] = sqrt( 1 - tmp[0] * tmp[0] - tmp[1] * tmp[1] - tmp[2] * tmp[2] );
		//if(tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2] >= 1.0f)
		//{
		//	printf("quaternion error, w > 1 (%f, %f, %f)\n",tmp[0],tmp[1],tmp[2]);
		//}
		if (nw)
			tmp[3] = -tmp[3];
		return tmp; 
	}
};
struct Vector48
{
	union Float16 data[3];
	
	inline glm::vec3 unpack(void) const
	{
		glm::vec3 ret;
		ret[0] = data[0].unpack();
		ret[1] = data[1].unpack();
		ret[2] = data[2].unpack();
		return ret;
	};
};

struct mdlAnim
{

	byte				bone;
	byte				flags;		// weighing options

	// valid for animating data only
	inline byte				*pData( void ) const { return (((byte *)this) + sizeof( struct mdlAnim )); };
	inline mdlAnimValuePtr	*pRotV( void ) const { return (mdlAnimValuePtr *)(pData()); };
	inline mdlAnimValuePtr	*pPosV( void ) const { return (mdlAnimValuePtr *)(pData()) + ((flags & STUDIO_ANIM_ANIMROT) != 0); };

	// valid if animation unvaring over timeline
	inline Quaternion48		*pQuat48( void ) const { return (Quaternion48 *)(pData()); };
	inline Quaternion64		*pQuat64( void ) const { return (Quaternion64 *)(pData()); };
	inline Vector48			*pPos( void ) const { return (Vector48 *)(pData() + ((flags & STUDIO_ANIM_RAWROT) != 0) * sizeof( *pQuat48() ) + ((flags & STUDIO_ANIM_RAWROT2) != 0) * sizeof( *pQuat64() ) ); };

	emscripten_align1_short				nextoffset;
	inline mdlAnim	*pNext( void ) const { if (nextoffset != 0) return  (mdlAnim *)(((byte *)this) + nextoffset); else return NULL; };
};

struct mdlAnimSection
{
	emscripten_align1_int					animblock;
	emscripten_align1_int					animindex;
};

struct mdlAnimDesc
{

	emscripten_align1_int					baseptr;
	//inline studiohdr_t	*pStudiohdr( void ) const { return (studiohdr_t *)(((byte *)this) + baseptr); }

	emscripten_align1_int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }

	emscripten_align1_float				fps;		// frames per second	
	emscripten_align1_int					flags;		// looping/non-looping flags

	emscripten_align1_int					numframes;

	// piecewise movement
	emscripten_align1_int					nummovements;
	emscripten_align1_int					movementindex;
	//inline mstudiomovement_t * const pMovement( int i ) const { return (mstudiomovement_t *)(((byte *)this) + movementindex) + i; };

	emscripten_align1_int					unused1[6];			// remove as appropriate (and zero if loading older versions)	

	emscripten_align1_int					animblock;
	emscripten_align1_int					animindex;	 // non-zero when anim data isn't in sections
	//mstudioanim_t *pAnimBlock( int block, int index ) const; // returns pointer to a specific anim block (local or external)
	//mstudioanim_t *pAnim( int *piFrame, float &flStall ) const; // returns pointer to data and new frame index
	//mstudioanim_t *pAnim( int *piFrame ) const; // returns pointer to data and new frame index

	emscripten_align1_int					numikrules;
	emscripten_align1_int					ikruleindex;	// non-zero when IK data is stored in the mdl
	emscripten_align1_int					animblockikruleindex; // non-zero when IK data is stored in animblock file
	//mstudioikrule_t *pIKRule( int i ) const;

	emscripten_align1_int					numlocalhierarchy;
	emscripten_align1_int					localhierarchyindex;
	//mstudiolocalhierarchy_t *pHierarchy( int i ) const;

	emscripten_align1_int					sectionindex;
	emscripten_align1_int					sectionframes; // number of frames used in each fast lookup section, zero if not used
	inline mdlAnimSection * const pSection( int i ) const { return (mdlAnimSection *)(((byte *)this) + sectionindex) + i; }

	emscripten_align1_short				zeroframespan;	// frames per span
	emscripten_align1_short				zeroframecount; // number of spans
	emscripten_align1_int					zeroframeindex;
	byte				*pZeroFrameData( ) const { if (zeroframeindex) return (((byte *)this) + zeroframeindex); else return NULL; };
	mutable emscripten_align1_float		zeroframestalltime;		// saved during read stalls


};