#include "StandardEqualFunctions.h"

namespace CacheSystem
{
	namespace StandardFunctions
	{
#define STD_EQUALFUNC_SPECIALIZE(type) \
		template <> bool standardEqualFunction(const type& value1, const type& value2, void*) \
		{ \
			return value1 == value2;\
		}

		STD_EQUALFUNC_SPECIALIZE(bool);
		STD_EQUALFUNC_SPECIALIZE(char);
		STD_EQUALFUNC_SPECIALIZE(signed char);
		STD_EQUALFUNC_SPECIALIZE(unsigned char);
		STD_EQUALFUNC_SPECIALIZE(wchar_t);

		STD_EQUALFUNC_SPECIALIZE(short);
		STD_EQUALFUNC_SPECIALIZE(unsigned short);
		STD_EQUALFUNC_SPECIALIZE(int);
		STD_EQUALFUNC_SPECIALIZE(unsigned int);
		STD_EQUALFUNC_SPECIALIZE(long);
		STD_EQUALFUNC_SPECIALIZE(unsigned long);
		STD_EQUALFUNC_SPECIALIZE(unsigned long long);

		STD_EQUALFUNC_SPECIALIZE(float);
		STD_EQUALFUNC_SPECIALIZE(double);
		STD_EQUALFUNC_SPECIALIZE(long double);

		STD_EQUALFUNC_SPECIALIZE(std::string);

#undef STD_EQUALFUNC_SPECIALIZE		

	}
}