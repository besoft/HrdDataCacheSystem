#ifndef _STANDARD_EQUAL_FUNCTION_H
#define _STANDARD_EQUAL_FUNCTION_H

#include <string>
#include <exception>

namespace CacheSystem
{
	namespace StandardFunctions
	{
		/**
		No generic standardEqualFunction available
		only throws exception
		*/
		template <class Type> bool standardEqualFunction(const Type & val1, const Type & val2, void*)
		{
			static_assert(false, "No standardEqualFunction available for this type");

			std::string message = "No standardEqualFunction available for type ";
			std::string type = std::string(typeid(Type).name());
			throw std::exception((message + type).c_str());
		}

#define STD_EQUALFUNC_SPECIALIZE(type) \
		template <> bool standardEqualFunction(const type& value1, const type& value2, void*);

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

#endif