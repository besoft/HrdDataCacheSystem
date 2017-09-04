#include "StandardInitFunctions.h"

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <> void standardInitFunction(const int & source, int* destination, void*)
		{
			*destination = source;
		}
		
		template <> void standardInitFunction(const unsigned int & source, unsigned int* destination, void*)
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

		template <> void standardInitFunction(const float & source, float* destination, void*)
		{
			*destination = source;
		}
		
		template <> void standardInitFunction(const char & source, char* destination, void*)
		{
			*destination = source;
		}
	}
}