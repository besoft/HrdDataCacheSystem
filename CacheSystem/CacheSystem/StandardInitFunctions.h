#ifndef _STANDARD_INIT_FUNCTIONS_H
#define _STANDARD_INIT_FUNCTIONS_H

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <class Type> void standardInitFunction(const Type & source, Type* destination, void*)
		{
			new(destination)Type(source);
		}

		template <> void standardInitFunction(const int &, int*, void*);

		template <> void standardInitFunction(const unsigned int &, unsigned int*, void*);

		template <> void standardInitFunction(const bool &, bool*, void*);

		template <> void standardInitFunction(const double &, double*, void*);

		template <> void standardInitFunction(const float &, float*, void*);

		template <> void standardInitFunction(const char &, char*, void*);
	}
}

#endif