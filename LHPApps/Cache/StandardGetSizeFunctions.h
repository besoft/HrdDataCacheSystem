#ifndef _STANDARD_GET_SIZE_FUNCTIONS_H
#define _STANDARD_GET_SIZE_FUNCTIONS_H

#include <string>
#include <exception>
#include <stdint.h>

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <class Type> uint64_t standardGetSizeFunction(const Type & value, void* dependencyObject)
		{
			return sizeof(value);
		}

		template <> uint64_t standardGetSizeFunction(const std::string & value, void* dependencyObject);
	}
}

#endif