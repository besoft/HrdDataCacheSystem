#ifndef _STANDARD_EQUAL_FUNCTION_H
#define _STANDARD_EQUAL_FUNCTION_H

#include <string>
#include <exception>

namespace CacheSystem
{
	namespace StandardFunctions
	{
		/**
		No generic standardEqualFunction available
		only throws exception
		*/
		template <class Type> bool standardEqualFunction(const Type & val1, const Type & val2, void*)
		{
			std::string message = "No standardEqualFunction available for type ";
			std::string type = std::string(typeid(Type).name());
			throw std::exception((message + type).c_str());
		}

		/**
		compares values using the == operator
		*/
		template <> bool standardEqualFunction(const int & val1, const int & val2, void*);

		/**
		compares values using the == operator
		*/
		template <> bool standardEqualFunction(const unsigned int & val1, const unsigned int & val2, void*);

		/**
		compares values using the == operator
		*/
		template <> bool standardEqualFunction(const char & val1, const char & val2, void*);

		/**
		compares values using the == operator
		*/
		template <> bool standardEqualFunction(const std::string & val1, const std::string & val2, void*);

		/**
		returns true if the absolute value of val1 - val2 is less than 0.001
		*/
		template <> bool standardEqualFunction(const double & val1, const double & val2, void*);

		/**
		returns true if the absolute value of val1 - val2 is less than 0.001
		*/
		template <> bool standardEqualFunction(const float & val1, const float & val2, void*);

		/**
		compares values using the == operator
		*/
		template <> bool standardEqualFunction(const bool & val1, const bool & val2, void*);
	}
}

#endif