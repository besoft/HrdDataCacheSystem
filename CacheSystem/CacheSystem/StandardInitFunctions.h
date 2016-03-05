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
	}
}

#endif