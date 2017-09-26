/** 
	DO NOT PLACE: #pragma once or #ifndef here
	this file is worked out from StandardFunctions.h 
*/

#ifndef _STANDARD_FUNCTIONS_INCLUDE
#include "StandardFunctions.h"
#else
/**
No generic standardEqualFunction available
*/
template <typename Type>
static bool standardEqualFunction(const Type & val1, const Type & val2, DependencyObj)
{
	static_assert(false, "No standardEqualFunction available for this type");

	//TODO: add specializations for types having operator ==
	//with a very special solution for pointers: 
	//pointers must equal or there must be operator == for (*val1 == *val2)
	//with the check for null pointers
}

#define STD_FUNC_SPECIALIZE(Type) \
		template <> static bool standardEqualFunction(const Type & value1, const Type & value2, DependencyObj) \
		{ \
			return value1 == value2;\
		}

STD_SPECIALIZE_STD_TYPES(STD_FUNC_SPECIALIZE)
#undef STD_FUNC_SPECIALIZE

#endif