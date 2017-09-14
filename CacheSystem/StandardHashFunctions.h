#ifndef _STANDARD_HASH_FUNCTIONS_H
#define _STANDARD_HASH_FUNCTIONS_H

#include <string>
#include <exception>
#include <functional>
#include <stdint.h>

namespace CacheSystem
{
	namespace StandardFunctions
	{
		//to combine hashes CacheSystem::hash_combine_hsv function can be used

		/**
		no generic standardHashFunction available
		only throws exception
		*/
		template <class Type> size_t standardHashFunction(const Type & value, void* dependencyObject)
		{
			std::string message = "No standardHashFunction available for type ";
			std::string type = std::string(typeid(Type).name());
			throw std::exception((message + type).c_str());
		}


#define STD_HASHFUNC_SPECIALIZE(type) \
		template <> inline size_t standardHashFunction(const type& value, void*) \
		{ \
			return std::hash<type>{}(value);\
		}

		STD_HASHFUNC_SPECIALIZE(bool);
		STD_HASHFUNC_SPECIALIZE(char);
		STD_HASHFUNC_SPECIALIZE(signed char);
		STD_HASHFUNC_SPECIALIZE(unsigned char);
		STD_HASHFUNC_SPECIALIZE(wchar_t);

		STD_HASHFUNC_SPECIALIZE(short);
		STD_HASHFUNC_SPECIALIZE(unsigned short);
		STD_HASHFUNC_SPECIALIZE(int);
		STD_HASHFUNC_SPECIALIZE(unsigned int);
		STD_HASHFUNC_SPECIALIZE(long);
		STD_HASHFUNC_SPECIALIZE(unsigned long);
		STD_HASHFUNC_SPECIALIZE(unsigned long long);

		STD_HASHFUNC_SPECIALIZE(float);
		STD_HASHFUNC_SPECIALIZE(double);
		STD_HASHFUNC_SPECIALIZE(long double);

		STD_HASHFUNC_SPECIALIZE(std::string);

#undef STD_HASHFUNC_SPECIALIZE
	}
}

#endif