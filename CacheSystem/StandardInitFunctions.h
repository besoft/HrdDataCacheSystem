/**
DO NOT PLACE: #pragma once or #ifndef here
this file is worked out from StandardFunctions.h
*/

#ifndef _STANDARD_FUNCTIONS_INCLUDE
#include "StandardFunctions.h"
#else
/**
uses placement new operator to call the copy constructor on the uninitialized memory block
*/
template <typename Type>
static void standardInitFunction(const Type & source, Type* destination _DEPENDENCY_OBJECT)
{
	new(destination)Type(source);
}
		
#define STD_INITFUNC_SPECIALIZE(type) \
		template <> static void standardInitFunction(const type & source, type* destination _DEPENDENCY_OBJECT)\
		{ \
			*destination = source;\
		}

		STD_SPECIALIZE_STD_TYPES(STD_INITFUNC_SPECIALIZE)
#undef STD_INITFUNC_SPECIALIZE

#endif