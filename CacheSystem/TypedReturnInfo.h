#ifndef _TYPED_RETURN_INFO_H
#define _TYPED_RETURN_INFO_H

#include <stdint.h>
#include "ReturnInfo.h"
#include "StandardReturnFunctions.h"
#include "StandardInitFunctions.h"
#include "StandardDestroyFunctions.h"
#include "StandardGetSizeFunctions.h"
#include "ReturnType.h"
#include "DataManipulationFunctionsTypeTraits.h"

namespace CacheSystem
{
	/**
	contains all information about the return value needed by the cache system
	Type is the type of the return value
	*/
	template <class Type>
	struct TypedReturnInfo : public ReturnInfo, DataManipulationFunctionsTypeTraits<Type>
	{
		/**
		pointer to a function that defines how to use the returned value to initialize the cached value of the return value

		first parameter is the value to be used for initialization
		second parameter is a pointer to an uninitialized but allocated block of memory of size sizeof(Type) which the function must initialize,
		the placement new operator can be used to initialize it

		for objects the body should look like this ('ptr' is the pointer to the memory to initialize, 'value' is the value to use for the initialization):
		new(ptr)Type(value); //uses copy constructor but can use any constructor and then copy the attributes
		*/
		InitFunction initFunction;

		/**
		pointer to a function that defines how the cahced value of the return value will be destroyed
		note that the destructor is not called automatically but the memory is deallocated

		the body should look like this ('value' is the value to delete):
		for object:
		value.~Type();

		for pointer if the pointer points to a dynamically allocated value:
		delete value;
		or
		delete[] value;

		if the pointer does not point to a dynamically allocated value then it can be emtpy
		for primitive types it can be empty

		but do not do this:
		delete &value; //the memory is delocated automatically
		*/
		DestroyFunction destroyFunction;

		/**
		pointer to a function that defines how the cached return value will be returned by the cache system

		first parameter is the cached value and the function should return in some way (perhaps create a copy and return it)
		
		if set to a CacheSystem::StandardFunctions::DirectReturn<Type> then the value will be returned directly
		*/
		ReturnFunction returnFunction;

		/**
		pointer to a function that defines how a size of the parameter is calculated
		*/
		GetSizeFunction getSizeFunction;

		/**
		creates the object and sets function pointers to standard values and return type to Used
		*/
		TypedReturnInfo();

		/**
		creates the object and sets the function pointers and return type
		N.B. data manipulation functions can be functions, functors, bound methods,
		lambda expressions, etc. - see CachedFunctionManager::createCachedFunction
		*/
		TypedReturnInfo(
			ReturnType returnType,
			InitFunction initFunction,
			DestroyFunction destroyFunction,
			ReturnFunction returnFunction,
			GetSizeFunction getSizeFunction
			);

		/**
		creates a copy of this object a returns pointer to the new object,
		*/
		std::shared_ptr<ReturnInfo> getCopy();
	};

	template <class Type>
	TypedReturnInfo<Type>::TypedReturnInfo(
		ReturnType returnType,
		InitFunction initFunction,
		DestroyFunction destroyFunction,
		ReturnFunction returnFunction,
		GetSizeFunction getSizeFunction
		) :
		ReturnInfo(returnType),
		initFunction(initFunction),
		destroyFunction(destroyFunction),
		returnFunction(returnFunction),
		getSizeFunction(getSizeFunction)
	{
	}

	template <class Type>
	TypedReturnInfo<Type>::TypedReturnInfo() :
		ReturnInfo(ReturnType::UsedReturn),
		initFunction(StandardFunctions::standardInitFunction<Type>),
		destroyFunction(StandardFunctions::standardDestroyFunction<Type>),
		returnFunction(StandardFunctions::DirectReturn<Type>),
		getSizeFunction(StandardFunctions::standardGetSizeFunction<Type>)
	{
	}

	template <class Type>
	std::shared_ptr<ReturnInfo> TypedReturnInfo<Type>::getCopy()
	{
		return std::shared_ptr<ReturnInfo>(
			new TypedReturnInfo<Type>(returnType, initFunction, destroyFunction, returnFunction, getSizeFunction)
		);
	}
}

#endif
