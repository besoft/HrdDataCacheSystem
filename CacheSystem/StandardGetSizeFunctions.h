/**
DO NOT PLACE: #pragma once or #ifndef here
this file is worked out from StandardFunctions.h
*/

#ifndef _STANDARD_FUNCTIONS_INCLUDE
#include "StandardFunctions.h"
#else

/**
no generic standardGetSizeFunction available
*/
template <class Type>
static size_t standardGetSizeFunction(const Type & value _DEPENDENCY_OBJECT)
{
	static_assert(false, "No standardGetSizeFunction available for this type");
}

/**
returns sum of sizeof(value) and number of characters in the string
*/
template <> static size_t standardGetSizeFunction(const std::string & value _DEPENDENCY_OBJECT)
{	
	return sizeof(value) + (value.size() + 1) * sizeof(char);
}

#define STD_FUNC_SPECIALIZE(type) \
		template <> static size_t standardGetSizeFunction(const type& value _DEPENDENCY_OBJECT)\
		{ \
			return sizeof(value); \
		}

STD_SPECIALIZE_STD_TYPES_NOSTRING(STD_FUNC_SPECIALIZE)
#undef STD_FUNC_SPECIALIZE
#endif