#ifndef _STANDARD_EQUAL_FUNCTION_H
#define _STANDARD_EQUAL_FUNCTION_H

#include <string>
#include <exception>

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <class Type> bool standardEqualFunction(const Type & val1, const Type & val2, void*)
		{
			std::string message = "No standardEqualFunction available for type ";
			std::string type = std::string(typeid(Type).name());
			throw std::exception((message + type).c_str());
		}

		template <> bool standardEqualFunction(const int & val1, const int & val2, void*);

		template <> bool standardEqualFunction(const char & val1, const char & val2, void*);

		template <> bool standardEqualFunction(const std::string & val1, const std::string & val2, void*);
	}
}

#endif