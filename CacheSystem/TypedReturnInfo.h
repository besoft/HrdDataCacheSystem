#ifndef _TYPED_RETURN_INFO_H
#define _TYPED_RETURN_INFO_H

#include <stdint.h>
#include "ReturnInfo.h"
#include "ReturnType.h"
#include "DataManipulationFunctionsTypeTraits.h"
#include "StandardFunctions.h"

namespace CacheSystem
{
	/**
	contains all information about the return value needed by the cache system
	Type is the type of the return value
	*/
	template <class Type, class DependencyObj>
	struct TypedReturnInfoWithDepObj : public ReturnInfo, DataManipulationFunctionsTypeTraits<Type, DependencyObj>
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
		TypedReturnInfoWithDepObj();

		/**
		creates the object and sets the function pointers and return type
		N.B. data manipulation functions can be functions, functors, bound methods,
		lambda expressions, etc. - see CachedFunctionManager::createCachedFunction
		*/
		TypedReturnInfoWithDepObj(
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

	template <class Type, class DependencyObj>
	TypedReturnInfoWithDepObj<Type, DependencyObj>::TypedReturnInfoWithDepObj(
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

	template <class Type, class DependencyObj>
	TypedReturnInfoWithDepObj<Type, DependencyObj>::TypedReturnInfoWithDepObj() :
		ReturnInfo(ReturnType::UsedReturn),
		initFunction(StandardFunctionsWithDepObj<DependencyObj>::standardInitFunction<Type>),
		destroyFunction(StandardFunctionsWithDepObj<DependencyObj>::standardDestroyFunction<Type>),
		returnFunction(StandardFunctionsWithDepObj<DependencyObj>::DirectReturn<Type>),
		getSizeFunction(StandardFunctionsWithDepObj<DependencyObj>::standardGetSizeFunction<Type>)
	{
	}

	template <class Type, class DependencyObj>
	std::shared_ptr<ReturnInfo> TypedReturnInfoWithDepObj<Type, DependencyObj>::getCopy()
	{
		return std::shared_ptr<ReturnInfo>(
			new TypedReturnInfoWithDepObj<Type, DependencyObj>(returnType, initFunction, destroyFunction, returnFunction, getSizeFunction)
		);
	}	

	/** TypedParameterInfo with void* dependency object - used for the backward compatibility*/
	template<typename Type>
	using TypedReturnInfo = typename TypedReturnInfoWithDepObj<Type, void*>;
}

#endif
