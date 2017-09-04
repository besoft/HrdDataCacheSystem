#ifndef _STANDARD_DESTROY_FUNCTIONS_H
#define _STANDARD_DESTROY_FUNCTIONS_H

namespace CacheSystem
{
	namespace StandardFunctions
	{
		/**
		calls the instance's destructor (if it's not a primitive type or pointer)
		*/
		template <class Type> void standardDestroyFunction(Type & instance, void* dependencyObject)
		{
			instance.~Type();
		}
	}
}

#endif