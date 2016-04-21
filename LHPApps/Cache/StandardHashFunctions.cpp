#include "StandardHashFunctions.h"

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <> uint32_t standardHashFunction(const int & value, void*)
		{
			return (uint32_t)value;
		}

		template <> uint32_t standardHashFunction(const char & value, void*)
		{
			return (uint32_t)value;
		}

		template <> uint32_t standardHashFunction(const std::string & value, void*)
		{
			uint32_t ret = 0;
			for (unsigned int i = 0; i < value.size(); i++)
				ret += (uint32_t)value[i];
			return ret;
		}
	}
}