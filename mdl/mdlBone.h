#pragma once

#include "../glm/glm.hpp"
#include "../glm/gtc/quaternion.hpp"

struct mdlBone
{
	
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int		 			parent;		// parent bone
	int					bonecontroller[6];	// bone controller index, -1 == none

	// default values
	glm::vec3				pos;
	glm::quat			quat;
	glm::vec3			rot;    // in radian euler (Rx,Ry,Rz) format
	// compression scale
	glm::vec3				posscale;
	glm::vec3				rotscale;

	glm::mat3x4			poseToBone;
	glm::quat			qAlignment;
	int					flags;
	int					proctype;
	int					procindex;		// procedural rule
	mutable int			physicsbone;	// index into physically simulated bone
	inline void *pProcedure( ) const { if (procindex == 0) return NULL; else return  (void *)(((byte *)this) + procindex); };
	int					surfacepropidx;	// index into string tablefor property name
	inline char * const pszSurfaceProp( void ) const { return ((char *)this) + surfacepropidx; }
	int					contents;		// See BSPFlags.h for the contents flags

	int					unused[8];		// remove as appropriate

};