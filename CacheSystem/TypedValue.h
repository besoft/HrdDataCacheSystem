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
	template <class Type, class DepObj>
	class TypedValueWithDepObj : public Value, DataManipulationFunctionsTypeTraits<Type, DepObj>
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
		DepObj dependencyObject;

	public:
		/**
		initializes the value using the specified initFunction (see TypedParameterInfo and TypedReturnInfo classes)
		*/
		TypedValueWithDepObj(const Type & value, InitFunction initFunction, DepObj dependencyObject, DestroyFunction destroyFunction);

		/**
		correctly destroys the value using the destroyFunction
		*/
		~TypedValueWithDepObj();

		/**
		returns the value
		*/
		const Type & getValue() { return *value; }
	};

	template <class Type, class DepObj>
	TypedValueWithDepObj<Type,DepObj>::TypedValueWithDepObj(const Type & value, InitFunction initFunction, DepObj dependencyObject, DestroyFunction destroyFunction)
		: value((Type*)new char[sizeof(Type)]), destroyFunction(destroyFunction), dependencyObject(dependencyObject)
	{
		initFunction(value, this->value, dependencyObject);
	}

	template <class Type, class DepObj>
	TypedValueWithDepObj<Type, DepObj>::~TypedValueWithDepObj()
	{
		destroyFunction(*value, dependencyObject);
		delete[] ((char*)value);
	}
}

#endif