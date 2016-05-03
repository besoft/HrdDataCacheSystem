#ifndef _STANDARD_GET_SIZE_FUNCTIONS_H
#define _STANDARD_GET_SIZE_FUNCTIONS_H

#include <string>
#include <exception>
#include <stdint.h>

namespace CacheSystem
{
	namespace StandardFunctions
	{
		/**
		returns sizeof(value)
		*/
		template <class Type> uint64_t standardGetSizeFunction(const Type & value, void* dependencyObject)
		{
			return sizeof(value);
		}

		/**
		returns sum of sizeof(value) and number of characters in the string 
		*/
		template <> uint64_t standardGetSizeFunction(const std::string & value, void* dependencyObject);
	}
}

#endif