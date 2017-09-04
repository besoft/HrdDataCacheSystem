#include "StandardOutputFunctions.h"

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <> void standardOutputFunction(int* const & storedValuePointer, int* & outputPointer, void*)
		{
			*outputPointer = *storedValuePointer;
		}

		template <> void standardOutputFunction(unsigned int* const & storedValuePointer, unsigned int* & outputPointer, void*)
		{
			*outputPointer = *storedValuePointer;
		}

		template <> void standardOutputFunction(char* const & storedValuePointer, char* & outputPointer, void*)
		{
			*outputPointer = *storedValuePointer;
		}

		template <> void standardOutputFunction(std::string* const & storedValuePointer, std::string* & outputPointer, void*)
		{
			*outputPointer = *storedValuePointer;
		}
		
		template <> void standardOutputFunction(double* const & storedValuePointer, double* & outputPointer, void*)
		{
			*outputPointer = *storedValuePointer;
		}

		template <> void standardOutputFunction(float* const & storedValuePointer, float* & outputPointer, void*)
		{
			*outputPointer = *storedValuePointer;
		}

		template <> void standardOutputFunction(bool* const & storedValuePointer, bool* & outputPointer, void*)
		{
			*outputPointer = *storedValuePointer;
		}
	}
}