#ifndef _STANDARD_DESTROY_FUNCTIONS_H
#define _STANDARD_DESTROY_FUNCTIONS_H

namespace CacheSystem
{
	namespace StandardFunctions
	{
		template <class Type> void standardDestroyFunction(Type & instance, void* dependencyObject)
		{
			instance.~Type();
		}
	}
}

#endif