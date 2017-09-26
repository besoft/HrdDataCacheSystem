#ifndef _TYPED_PARAMETER_INFO_H
#define _TYPED_PARAMETER_INFO_H

#include <stdint.h>
#include "ParameterInfo.h"
#include "ParameterType.h"
#include "StandardInitFunctions.h"
#include "StandardEqualFunctions.h"
#include "StandardDestroyFunctions.h"
#include "StandardOutputFunctions.h"
#include "StandardHashFunctions.h"
#include "StandardGetSizeFunctions.h"
#include "Hash.h"
#include "DataManipulationFunctionsTypeTraits.h"

namespace CacheSystem
{
	/**
	contains all information about a parameter needed by the cache system
	Type is the type of the parameter
	Use DependencyObj = void to have no dependency object
	*/
	template <class Type>
	struct TypedParameterInfo : public ParameterInfo, DataManipulationFunctionsTypeTraits<Type>
	{
		/**
		pointer to a function that defines how the parameter will be compared to any cached value of this parameter

		the first two parameters are values to compare, the return value must be true if the values are equal and false otherwise
		*/		
		EqualFunction equalFunction;

		/**
		pointer to a function that defines how to use the parameter to initialize the cached value of the parameter

		first parameter is the value to be used for initialization
		second parameter is a pointer to an uninitialized but allocated block of memory of size sizeof(Type) which the function must initialize,
		the placement new operator can be used to initialize it

		for objects the body should look like this ('ptr' is the pointer to the memory to initialize, 'value' is the value to use for the initialization):
		new(ptr)Type(value); //uses copy constructor but can use any constructor and then copy the attributes
		*/
		InitFunction initFunction;

		/**
		pointer to a function that defines how to copy the cached value of an output parameter into the parameter

		first parameter is the cached value
		second parameter is the actual parameter into which the function must copy the cached value
		*/
		OutputFunction outputFunction;

		/**
		pointer to a function that defines how the cahced value of the parameter will be destroyed
		note that the destructor is not called automatically but the memory is deallocated

		the body should look like this ('value' is the value to delete):
		for object:
		value.~Type();

		for pointer if the pointer points to a dynamically allocated value:
		delete value;
		or
		delete[] value;

		for primitive types it can be empty

		but do not do this:
		delete &value; //the memory is deallocated automatically
		*/
		DestroyFunction destroyFunction;

		/**
		pointer to a function that defines how a hash value of the parameter is calculated
		if the parameter is composed of several items, one may calculate the hash
		value for each of them and combine the hashes together using the helper function
		hash_combine_hashcodes
		*/
		HashFunction hashFunction;

		/**
		pointer to a function that defines how a size of the parameter is calculated
		*/
		GetSizeFunction getSizeFunction;

		/**
		creates the object and sets function pointers to standard values and parameter type to Input
		*/		
		TypedParameterInfo(ParameterType paramType = ParameterType::InputParam);

		/**
		creates the object and sets the data manipulation functions and parameter type
		N.B. data manipulation functions can be functions, functors, bound methods,
		lambda expressions, etc. - see CachedFunctionManager::createCachedFunction
		*/
		TypedParameterInfo(
			ParameterType paramType,
			EqualFunction equalFunction,
			InitFunction initFunction,
			OutputFunction outputFunction,
			DestroyFunction destroyFunction,
			HashFunction hashFunction,
			GetSizeFunction getSizeFunction
			);

		/**
		creates a copy of this object a returns pointer to the new object,
		*/
		std::shared_ptr<ParameterInfo> getCopy();
	};

	template <class Type>
	TypedParameterInfo<Type>::TypedParameterInfo(ParameterType paramType) :
		ParameterInfo(paramType),
		equalFunction(StandardFunctions::standardEqualFunction<Type>),
		initFunction(StandardFunctions::standardInitFunction<Type>),
		outputFunction(StandardFunctions::standardOutputFunction<Type>),
		destroyFunction(StandardFunctions::standardDestroyFunction<Type>),
		hashFunction(StandardFunctions::standardHashFunction<Type>),
		getSizeFunction(StandardFunctions::standardGetSizeFunction<Type>)
	{
	}

	template <class Type>
	TypedParameterInfo<Type>::TypedParameterInfo(
		ParameterType paramType,
		EqualFunction equalFunction,
		InitFunction initFunction,
		OutputFunction outputFunction,
		DestroyFunction destroyFunction,
		HashFunction hashFunction,
		GetSizeFunction getSizeFunction
		) :
		ParameterInfo(paramType),
		equalFunction(equalFunction),
		initFunction(initFunction),
		outputFunction(outputFunction),
		destroyFunction(destroyFunction),
		hashFunction(hashFunction),
		getSizeFunction(getSizeFunction)
	{
	}

	template <class Type>
	std::shared_ptr<ParameterInfo> TypedParameterInfo<Type>::getCopy()
	{
		return std::shared_ptr<ParameterInfo> (
			new TypedParameterInfo<Type>(paramType, equalFunction, initFunction, outputFunction, destroyFunction, hashFunction, getSizeFunction)
		);
	}
}

#endif