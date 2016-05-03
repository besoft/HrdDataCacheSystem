#ifndef _STANDARD_HASH_FUNCTIONS_H
#define _STANDARD_HASH_FUNCTIONS_H

#include <string>
#include <exception>
#include <stdint.h>

namespace CacheSystem
{
	namespace StandardFunctions
	{
		/**
		no generic standardHashFunction available
		only throws exception
		*/
		template <class Type> uint32_t standardHashFunction(const Type & value, void* dependencyObject)
		{
			std::string message = "No standardHashFunction available for type ";
			std::string type = std::string(typeid(Type).name());
			throw std::exception((message + type).c_str());
		}

		/**
		returns the value casted to uint32_t
		*/
		template <> uint32_t standardHashFunction(const int & value, void*);

		/**
		returns the value casted to uint32_t
		*/
		template <> uint32_t standardHashFunction(const unsigned int & value, void*);

		/**
		returns the value casted to uint32_t
		*/
		template <> uint32_t standardHashFunction(const char & value, void*);

		/**
		returns the sum of the string's characters casted to uint32_t
		*/
		template <> uint32_t standardHashFunction(const std::string & value, void*);

		/**
		returns the value multiplied by 1000 casted to uint32_t
		*/
		template <> uint32_t standardHashFunction(const double & value, void*);

		/**
		returns the value multiplied by 1000 casted to uint32_t
		*/
		template <> uint32_t standardHashFunction(const float & value, void*);

		/**
		returns the value casted to uint32_t
		*/
		template <> uint32_t standardHashFunction(const bool & value, void*);
	}
}

#endif