#ifndef _STANDARD_OUTPUT_FUNCTIONS_H
#define _STANDARD_OUTPUT_FUNCTIONS_H
#include <string>

namespace CacheSystem
{
	namespace StandardFunctions
	{
		/**
		no generic standardOutputFunction available
		only throws exception
		*/
		template <class Type> void standardOutputFunction(const Type & storedValue, Type & outputValue, void*)
		{
			std::string message = "No standardOutputFunction available for type ";
			std::string type = std::string(typeid(Type).name());
			throw std::exception((message + type).c_str());
		}

		/**
		uses the = operator to copy the value
		*/
		template <> void standardOutputFunction(int* const & storedValuePointer, int* & outputPointer, void*);

		/**
		uses the = operator to copy the value
		*/
		template <> void standardOutputFunction(unsigned int* const & storedValuePointer, unsigned int* & outputPointer, void*);

		/**
		uses the = operator to copy the value
		*/
		template <> void standardOutputFunction(char* const & storedValuePointer, char* & outputPointer, void*);

		/**
		uses the = operator to copy the value
		*/
		template <> void standardOutputFunction(std::string* const & storedValuePointer, std::string* & outputPointer, void*);

		/**
		uses the = operator to copy the value
		*/
		template <> void standardOutputFunction(double* const & storedValuePointer, double* & outputPointer, void*);

		/**
		uses the = operator to copy the value
		*/
		template <> void standardOutputFunction(float* const & storedValuePointer, float* & outputPointer, void*);

		/**
		uses the = operator to copy the value
		*/
		template <> void standardOutputFunction(bool* const & storedValuePointer, bool* & outputPointer, void*);
	}
}

#endif