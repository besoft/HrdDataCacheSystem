#ifndef _STANDARD_OUTPUT_FUNCTIONS_H
#define _STANDARD_OUTPUT_FUNCTIONS_H
#include <string>

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <class Type> void standardOutputFunction(const Type & storedValue, Type & outputValue, void**)
		{
			std::string message = "No standardOutputFunction available for type ";
			std::string type = std::string(typeid(Type).name());
			throw std::exception((message + type).c_str());
		}

		template <> void standardOutputFunction(int* const & storedValuePointer, int* & outputPointer, void**);

		template <> void standardOutputFunction(char* const & storedValuePointer, char* & outputPointer, void**);

		template <> void standardOutputFunction(std::string* const & storedValuePointer, std::string* & outputPointer, void**);
	}
}

#endif