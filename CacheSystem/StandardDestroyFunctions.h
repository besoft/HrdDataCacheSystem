/**
DO NOT PLACE: #pragma once or #ifndef here
this file is worked out from StandardFunctions.h
*/

#ifndef _STANDARD_FUNCTIONS_INCLUDE
#include "StandardFunctions.h"
#else
		/**
		calls the instance's destructor (if it's not a primitive type or pointer)
		*/
		template <class Type> 
		static void standardDestroyFunction(Type & instance _DEPENDENCY_OBJECT)
		{
			instance.~Type();
		}		
#endif