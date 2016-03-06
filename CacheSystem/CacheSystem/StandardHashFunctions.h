#ifndef _STANDARD_HASH_FUNCTIONS_H
#define _STANDARD_HASH_FUNCTIONS_H

#include <string>
#include <exception>
#include <stdint.h>

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <class Type> uint32_t standardHashFunction(const Type & value, void* dependencyObject)
		{
			std::string message = "No standardHashFunction available for type ";
			std::string type = std::string(typeid(Type).name());
			throw std::exception((message + type).c_str());
		}

		template <> uint32_t standardHashFunction(const int & value, void*);

		template <> uint32_t standardHashFunction(const char & value, void*);

		template <> uint32_t standardHashFunction(const std::string & value, void*);
	}
}

#endif