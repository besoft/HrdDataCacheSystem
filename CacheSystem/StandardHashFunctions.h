/**
DO NOT PLACE: #pragma once or #ifndef here
this file is worked out from StandardFunctions.h
*/

#ifndef _STANDARD_FUNCTIONS_INCLUDE
#include "StandardFunctions.h"
#else
//to combine hashes CacheSystem::hash_combine_hsv function can be used

/**
no generic standardHashFunction available
*/
template <class Type>
static size_t standardHashFunction(const Type & value _DEPENDENCY_OBJECT)
{
	static_assert(false, "No standardHashFunction available for this type");
}

#define STD_HASHFUNC_SPECIALIZE(type) \
		template <> static size_t standardHashFunction(const type& value _DEPENDENCY_OBJECT)\
		{ \
			return std::hash<type>{}(value); \
		}

STD_SPECIALIZE_STD_TYPES(STD_HASHFUNC_SPECIALIZE)
#undef STD_HASHFUNC_SPECIALIZE

#endif