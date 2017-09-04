#ifndef _STANDARD_INIT_FUNCTIONS_H
#define _STANDARD_INIT_FUNCTIONS_H

namespace CacheSystem
{
	namespace StandardFunctions
	{
		/**
		uses placement new operator to call the copy constructor on the uninitialized memory block
		*/
		template <class Type> void standardInitFunction(const Type & source, Type* destination, void*)
		{
			new(destination)Type(source);
		}

		/**
		uses the = operator to copy the value
		*/
		template <> void standardInitFunction(const int &, int*, void*);

		/**
		uses the = operator to copy the value
		*/
		template <> void standardInitFunction(const unsigned int &, unsigned int*, void*);

		/**
		uses the = operator to copy the value
		*/
		template <> void standardInitFunction(const bool &, bool*, void*);

		/**
		uses the = operator to copy the value
		*/
		template <> void standardInitFunction(const double &, double*, void*);

		/**
		uses the = operator to copy the value
		*/
		template <> void standardInitFunction(const float &, float*, void*);

		/**
		uses the = operator to copy the value
		*/
		template <> void standardInitFunction(const char &, char*, void*);
	}
}

#endif