#ifndef _STANDARD_FUNCTIONS_H
#define _STANDARD_FUNCTIONS_H

#include "DataManipulationFunctionsTypeTraits.h"
#include <string>

namespace CacheSystem
{		
	/**
		This macro allows declaration/definition of the specialization
		functions for standard types.

		Use:
		#include "StandardFunctions"

		#define MYSPECIALIZATION(type) \
			... implementation using type

		STD_SPECIALIZE_STD_TYPES(MYSPECIALIZATION)
	*/
#define STD_SPECIALIZE_STD_TYPES_NOSTRING(STD_SPECIALIZE_FUN) \
		STD_SPECIALIZE_FUN(bool);\
		STD_SPECIALIZE_FUN(char);\
		STD_SPECIALIZE_FUN(signed char);\
		STD_SPECIALIZE_FUN(unsigned char);\
		STD_SPECIALIZE_FUN(wchar_t);\
		\
		STD_SPECIALIZE_FUN(short);\
		STD_SPECIALIZE_FUN(unsigned short);\
		STD_SPECIALIZE_FUN(int);\
		STD_SPECIALIZE_FUN(unsigned int);\
		STD_SPECIALIZE_FUN(long);\
		STD_SPECIALIZE_FUN(unsigned long);\
		STD_SPECIALIZE_FUN(unsigned long long);\
		\
		STD_SPECIALIZE_FUN(float);\
		STD_SPECIALIZE_FUN(double);\
		STD_SPECIALIZE_FUN(long double);

#define STD_SPECIALIZE_STD_TYPES(STD_SPECIALIZE_FUN) \
		STD_SPECIALIZE_STD_TYPES_NOSTRING(STD_SPECIALIZE_FUN)\
		STD_SPECIALIZE_FUN(std::string);

//this is used as a suffix in the functions
#define _DEPENDENCY_OBJECT	, DependencyObj

//this is used in #include to detect the include recursion
#define _STANDARD_FUNCTIONS_INCLUDE

	template<typename DependencyObj>
	class StandardFunctionsWithDepObj
	{
	public:
#include "StandardEqualFunctions.h"
#include "StandardInitFunctions.h"
#include "StandardOutputFunctions.h"
#include "StandardDestroyFunctions.h"
#include "StandardHashFunctions.h"
#include "StandardGetSizeFunctions.h"
#include "StandardReturnFunctions.h"
	};

#undef _DEPENDENCY_OBJECT

#define _DEPENDENCY_OBJECT
	template<>
	class StandardFunctionsWithDepObj<NoDepObj>
	{
	public:
#include "StandardEqualFunctions.h"
#include "StandardInitFunctions.h"
#include "StandardOutputFunctions.h"
#include "StandardDestroyFunctions.h"
#include "StandardHashFunctions.h"
#include "StandardGetSizeFunctions.h"
#include "StandardReturnFunctions.h"
	};

#undef _DEPENDENCY_OBJECT
#undef _STANDARD_FUNCTIONS_INCLUDE

	//alias for backward compatibility with void* dependency objects
	using StandardFunctions = typename StandardFunctionsWithDepObj<void*>;

	//alias for standard functions with no dependency object
	using StandardFunctionsNoDepObj = typename StandardFunctionsWithDepObj<NoDepObj>;
}
#endif