#include "StandardOutputFunctions.h"

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <> void standardOutputFunction(int* const & storedValuePointer, int* & outputPointer, void**)
		{
			*outputPointer = *storedValuePointer;
		}

		template <> void standardOutputFunction(char* const & storedValuePointer, char* & outputPointer, void**)
		{
			*outputPointer = *storedValuePointer;
		}

		template <> void standardOutputFunction(std::string* const & storedValuePointer, std::string* & outputPointer, void**)
		{
			*outputPointer = *storedValuePointer;
		}
	}
}