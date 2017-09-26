/**
DO NOT PLACE: #pragma once or #ifndef here
this file is worked out from StandardFunctions.h
*/

#ifndef _STANDARD_FUNCTIONS_INCLUDE
#include "StandardFunctions.h"
#else
/**
this function is never called and serves inly as a constant for comparing
if this is set as the return function for the cache object the return value is returned directly from the cache
*/
template <class Type> static Type DirectReturn(const Type & value, DependencyObj)
{
	throw std::exception("Function DirectReturn cannot be called, it serves only as a constant for comparing.");
}
#endif