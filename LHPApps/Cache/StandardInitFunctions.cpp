#include "StandardInitFunctions.h"

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <> void standardInitFunction(const int & source, int* destination, void*)
		{
			*destination = source;
		}

		template <> void standardInitFunction(const bool & source, bool* destination, void*)
		{
			*destination = source;
		}

		template <> void standardInitFunction(const double & source, double* destination, void*)
		{
			*destination = source;
		}
	}
}