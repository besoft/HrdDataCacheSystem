#ifndef _TYPED_PARAMETER_INFO_H
#define _TYPED_PARAMETER_INFO_H
#include "ParameterInfo.h"
#include "ParameterType.h"
#include "StandardInitFunctions.h"
#include "StandardEqualFunctions.h"
#include "StandardDestroyFunctions.h"
#include "StandardOutputFunctions.h"

namespace CacheSystem
{
	/**
	contains all iformation about a parameter needed by the cache system
	Type is the type of the parameter
	*/
	template <class Type>
	struct TypedParameterInfo : public ParameterInfo
	{
		/**
		pointer to a function that defines how the parameter will be compared to any cached value of this parameter

		the first two parameters are values to compare, the return value must be true if the values are equal and false otherwise
		*/
		bool(*equalFunction)(const Type &, const Type &, void*);

		/**
		pointer to a function that defines how to use the parameter to initialize the cached value of the parameter

		first parameter is the value to be used for initialization
		second parameter is a pointer to an uninitialized but allocated block of memory of size sizeof(Type) which the function must initialize,
		the placement new operator can be used to initialize it

		for objects the body should look like this ('ptr' is the pointer to the memory to initialize, 'value' is the value to use for the initialization):
		new(ptr)Type(value); //uses copy constructor but can use any constructor and then copy the attributes
		*/
		void(*initFunction)(const Type &, Type*, void*);

		/**
		pointer to a function that defines how to copy the cached value of an output parameter into the parameter

		first parameter is the cached value
		second parameter is the actual parameter into which the function must copy the cached value
		*/
		void(*outputFunction)(const Type &, Type &, void*);

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
		delete &value; //the memory is delocated automatically
		*/
		void(*destroyFunction)(Type &);

		/**
		creates the object and sets function pointers to standard values and parameter type to Input
		*/
		TypedParameterInfo();

		/**
		creates the object and sets the function pointers and parameter type
		*/
		TypedParameterInfo(
			ParameterType paramType,
			bool(*equalFunction)(const Type &, const Type &, void*),
			void(*initFunction)(const Type &, Type*, void*),
			void(*outputFunction)(const Type &, Type &, void*),
			void(*destroyFunction)(Type &)
			);

		/**
		creates a copy of this object a returns pointer to the new object,
		it must be deleted by the delete operator in the end
		*/
		std::shared_ptr<ParameterInfo> getCopy();
	};

	template <class Type>
	TypedParameterInfo<Type>::TypedParameterInfo() :
		ParameterInfo(ParameterType::InputParam),
		equalFunction(StandardFunctions::standardEqualFunction<Type>),
		initFunction(StandardFunctions::standardInitFunction<Type>),
		outputFunction(StandardFunctions::standardOutputFunction<Type>),
		destroyFunction(StandardFunctions::standardDestroyFunction<Type>)
	{
	}

	template <class Type>
	TypedParameterInfo<Type>::TypedParameterInfo(
		ParameterType paramType,
		bool(*equalFunction)(const Type &, const Type &, void*),
		void(*initFunction)(const Type &, Type*, void*),
		void(*outputFunction)(const Type &, Type &, void*),
		void(*destroyFunction)(Type &)
		) :
		ParameterInfo(paramType),
		equalFunction(equalFunction),
		initFunction(initFunction),
		outputFunction(outputFunction),
		destroyFunction(destroyFunction)
	{
	}

	template <class Type>
	std::shared_ptr<ParameterInfo> TypedParameterInfo<Type>::getCopy()
	{
		return std::shared_ptr<ParameterInfo> (
			new TypedParameterInfo<Type>(paramType, equalFunction, initFunction, outputFunction, destroyFunction)
		);
	}
}

#endif