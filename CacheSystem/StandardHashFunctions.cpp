#include "StandardHashFunctions.h"

namespace CacheSystem
{
	namespace StandardFunctions
	{
		//to combine hashes CacheSystem::hash_combine_hsv function can be used

#define STD_HASHFUNC_SPECIALIZE(type) \
		template <> size_t standardHashFunction(const type& value, void*) \
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