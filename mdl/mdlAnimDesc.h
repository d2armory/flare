#pragma once

struct mdlAnim
{

	byte				bone;
	byte				flags;		// weighing options

	// valid for animating data only
	//inline byte				*pData( void ) const { return (((byte *)this) + sizeof( struct mstudioanim_t )); };
	//inline mstudioanim_valueptr_t	*pRotV( void ) const { return (mstudioanim_valueptr_t *)(pData()); };
	//inline mstudioanim_valueptr_t	*pPosV( void ) const { return (mstudioanim_valueptr_t *)(pData()) + ((flags & STUDIO_ANIM_ANIMROT) != 0); };

	// valid if animation unvaring over timeline
	//inline Quaternion48		*pQuat48( void ) const { return (Quaternion48 *)(pData()); };
	//inline Quaternion64		*pQuat64( void ) const { return (Quaternion64 *)(pData()); };
	//inline Vector48			*pPos( void ) const { return (Vector48 *)(pData() + ((flags & STUDIO_ANIM_RAWROT) != 0) * sizeof( *pQuat48() ) + ((flags & STUDIO_ANIM_RAWROT2) != 0) * sizeof( *pQuat64() ) ); };

	short				nextoffset;
	inline mdlAnim	*pNext( void ) const { if (nextoffset != 0) return  (mdlAnim *)(((byte *)this) + nextoffset); else return NULL; };
};

struct mdlAnimSection
{
	int					animblock;
	int					animindex;
};

struct mdlAnimDesc
{

	int					baseptr;
	//inline studiohdr_t	*pStudiohdr( void ) const { return (studiohdr_t *)(((byte *)this) + baseptr); }

	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }

	float				fps;		// frames per second	
	int					flags;		// looping/non-looping flags

	int					numframes;

	// piecewise movement
	int					nummovements;
	int					movementindex;
	//inline mstudiomovement_t * const pMovement( int i ) const { return (mstudiomovement_t *)(((byte *)this) + movementindex) + i; };

	int					unused1[6];			// remove as appropriate (and zero if loading older versions)	

	int					animblock;
	int					animindex;	 // non-zero when anim data isn't in sections
	//mstudioanim_t *pAnimBlock( int block, int index ) const; // returns pointer to a specific anim block (local or external)
	//mstudioanim_t *pAnim( int *piFrame, float &flStall ) const; // returns pointer to data and new frame index
	//mstudioanim_t *pAnim( int *piFrame ) const; // returns pointer to data and new frame index

	int					numikrules;
	int					ikruleindex;	// non-zero when IK data is stored in the mdl
	int					animblockikruleindex; // non-zero when IK data is stored in animblock file
	//mstudioikrule_t *pIKRule( int i ) const;

	int					numlocalhierarchy;
	int					localhierarchyindex;
	//mstudiolocalhierarchy_t *pHierarchy( int i ) const;

	int					sectionindex;
	int					sectionframes; // number of frames used in each fast lookup section, zero if not used
	inline mdlAnimSection * const pSection( int i ) const { return (mdlAnimSection *)(((byte *)this) + sectionindex) + i; }

	short				zeroframespan;	// frames per span
	short				zeroframecount; // number of spans
	int					zeroframeindex;
	byte				*pZeroFrameData( ) const { if (zeroframeindex) return (((byte *)this) + zeroframeindex); else return NULL; };
	mutable float		zeroframestalltime;		// saved during read stalls


};