#include "StandardGetSizeFunctions.h"

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <> uint64_t standardGetSizeFunction(const std::string & value, void* dependencyObject)
		{
			return sizeof(value) + (value.size() + 1) * sizeof(char);
		}
	}
}