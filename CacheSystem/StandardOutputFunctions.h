/**
DO NOT PLACE: #pragma once or #ifndef here
this file is worked out from StandardFunctions.h
*/

#ifndef _STANDARD_FUNCTIONS_INCLUDE
#include "StandardFunctions.h"
#else
/**
no generic standardOutputFunction available
*/
template <class Type>
static void standardOutputFunction(const Type & storedValue, Type & outputValue _DEPENDENCY_OBJECT)
{
	static_assert(false, "No standardOutputFunction available for this type");
}

#define STD_OUTPUTFUNC_SPECIALIZE(Type) \
		template <> static void standardOutputFunction(const Type & storedValue, Type & outputValue _DEPENDENCY_OBJECT)\
		{ \
			outputValue = storedValue;\
		}

STD_SPECIALIZE_STD_TYPES(STD_OUTPUTFUNC_SPECIALIZE)
#undef STD_OUTPUTFUNC_SPECIALIZE
#endif