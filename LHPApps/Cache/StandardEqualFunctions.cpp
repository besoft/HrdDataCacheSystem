#include "StandardEqualFunctions.h"

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <> bool standardEqualFunction(const int & val1, const int & val2, void*)
		{
			return val1 == val2;
		}
		
		template <> bool standardEqualFunction(const unsigned int & val1, const unsigned int & val2, void*)
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

		template <> bool standardEqualFunction(const double & val1, const double & val2, void*)
		{
			double diff = val1 - val2;
			diff = diff < 0 ? -diff : diff;
			return diff < 0.001;
		}

		template <> bool standardEqualFunction(const float & val1, const float & val2, void*)
		{
			double diff = val1 - val2;
			diff = diff < 0 ? -diff : diff;
			return diff < 0.001;
		}
		
		template <> bool standardEqualFunction(const bool & val1, const bool & val2, void*)
		{
			return val1 == val2;
		}

	}
}