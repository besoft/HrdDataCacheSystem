#include "StandardEqualFunctions.h"

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <> bool standardEqualFunction(const int & val1, const int & val2, void*)
		{
			return val1 == val2;
		}

		template <> bool standardEqualFunction(const char & val1, const char & val2, void*)
		{
			return val1 == val2;
		}

		template <> bool standardEqualFunction(const std::string & val1, const std::string & val2, void*)
		{
			return val1 == val2;
		}
	}
}