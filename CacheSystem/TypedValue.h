#ifndef _TYPED_VALUE_H
#define _TYPED_VALUE_H
#include "Value.h"

namespace CacheSystem
{
	/**
	represents a concrete cached value
	Type is the type of the value
	*/
	template <class Type>
	class TypedValue : public Value
	{
	private:
		/**
		pointer to the cached value
		*/
		Type* value;

		/**
		pointer to a function which defines how to destroy the value (see TypedParameterInfo and TypedReturnInfo classes)
		*/
		void(*destroyFunction)(Type &, void*);

		/**
		pointer to the dependency object
		*/
		void* dependencyObject;

	public:
		/**
		initializes the value using the specified initFunction (see TypedParameterInfo and TypedReturnInfo classes)
		*/
		TypedValue(const Type & value, void(*initFunction)(const Type &, Type*, void*), void* dependencyObject, void(*destroyFunction)(Type &, void*));

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
	TypedValue<Type>::TypedValue(const Type & value, void(*initFunction)(const Type &, Type*, void*),
		void* dependencyObject, void(*destroyFunction)(Type &, void*))
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