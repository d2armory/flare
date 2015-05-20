#pragma once

#include "../glm/glm.hpp"

// code straight from https://developer.valvesoftware.com/wiki/MDL#File_format

struct mdlTexture
{
	// Number of bytes past the beginning of this structure
	// where the first character of the texture name can be found.
	int		name_offset; 	// Offset for null-terminated string
	inline char * const		pszName( void ) const { return ((char *)this) + name_offset; }
	int		flags;
	int		used; 		// ??
 
	int		unused; 	// ??
 
	int	material;		// Placeholder for IMaterial
	int	client_material;	// Placeholder for void*
 
	int		unused2[10];
};