#ifndef _STANDARD_RETURN_FUNCTIONS_H
#define _STANDARD_RETURN_FUNCTIONS_H
#include <exception>

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <class Type> Type DirectReturn(const Type & value, void*)
		{
			throw std::exception("Function DirectReturn cannot be called, it serves only as a constant for comparing.");
		}
	}
}

#endif