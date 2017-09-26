#ifndef _TYPED_VALUE_H
#define _TYPED_VALUE_H
#include "Value.h"
#include "DataManipulationFunctionsTypeTraits.h"

namespace CacheSystem
{
	/**
	represents a concrete cached value
	Type is the type of the value
	*/
	template <class Type>
	class TypedValue : public Value, DataManipulationFunctionsTypeTraits<Type>
	{
	private:
		/**
		pointer to the cached value
		*/
		Type* value;

		/**
		pointer to a function which defines how to destroy the value (see TypedParameterInfo and TypedReturnInfo classes)
		*/
		DestroyFunction destroyFunction;

		/**
		pointer to the dependency object
		*/
		void* dependencyObject;

	public:
		/**
		initializes the value using the specified initFunction (see TypedParameterInfo and TypedReturnInfo classes)
		*/
		TypedValue(const Type & value, InitFunction initFunction, void* dependencyObject, DestroyFunction destroyFunction);

		/**
		correctly destroys the value using the destroyFunction
		*/
		~TypedValue();

		/**
		returns the value
		*/
		const Type & getValue() { return *value; }
	};

	template <class Type>
	TypedValue<Type>::TypedValue(const Type & value, InitFunction initFunction, void* dependencyObject, DestroyFunction destroyFunction)
		: value((Type*)new char[sizeof(Type)]), destroyFunction(destroyFunction), dependencyObject(dependencyObject)
	{
		initFunction(value, this->value, dependencyObject);
	}

	template <class Type>
	TypedValue<Type>::~TypedValue()
	{
		destroyFunction(*value, dependencyObject);
		delete[] ((char*)value);
	}
}

#endif