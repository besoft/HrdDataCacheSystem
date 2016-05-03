#ifndef _STANDARD_RETURN_FUNCTIONS_H
#define _STANDARD_RETURN_FUNCTIONS_H
#include <exception>

namespace CacheSystem
{
	namespace StandardFunctions
	{
		/**
		this function is never called and serves inly as a constant for comparing
		if this is set as the return function for the cache object the return value is returned directly from the cache
		*/
		template <class Type> Type DirectReturn(const Type & value, void*)
		{
			throw std::exception("Function DirectReturn cannot be called, it serves only as a constant for comparing.");
		}
	}
}

#endif