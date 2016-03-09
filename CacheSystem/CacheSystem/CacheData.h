#ifndef _CACHE_DATA_H
#define _CACHE_DATA_H

#include <vector>
#include <stdint.h>
#include "TypedValue.h"
#include "ParameterType.h"
#include "ParameterInfo.h"
#include "ReturnType.h"
#include "TypedReturnInfo.h"

namespace CacheSystem
{
	/**
	contains a single cached data object - all parameters used as a key, all parameters used as output parameters and the return value
	*/
	class CacheData
	{
	private:
		/**
		contains values of all parameters which are used as a key to identify this object (input parameters)
		*/
		std::vector<Value*> inputParameters;

		/**
		contains values of all output parameters
		*/
		std::vector<Value*> outputParameters;

		/**
		contains the return value
		*/
		Value* returnValue;

		/**
		time it took to create the data
		*/
		int64_t creationTime;

		/**
		size of the data in cache
		*/
		uint64_t size;


		/**
		recursively iterates through all parameters passed as otherParams, for each parameter it determines whether it is an input parameter
		using the paramsInfo vector
		
		if the parameter is an input parameter it is compared to a corresponding parameter in the
		inputParameters vector

		if results of all the comparsions are true, the method returs true, therwise it returs false
		*/
		template <class FirstType, class... OtherTypes> bool equalsIteration(int inputIndex, int paramIndex,
			const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, void* dependencyObject, const FirstType & firstParam, const OtherTypes &... otherParams);

		/**
		stops the recursion of equalIteration
		*/
		template <class Type> bool equalsIteration(int inputIndex, int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
			void* dependencyObject, const Type & param);

		/**
		recursively iterates through all parameters passed as otherParams and sotres all input parameters into the inputParameters vector
		and all outputParameters into the outputParameters vector using the paramsInfo vector
		*/
		template <class FirstType, class... OtherTypes> void setParamIteration(int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
			void* dependencyObject, const FirstType & firstParam, const OtherTypes &... otherParams);

		/**
		stops the recursion of setParamIteration
		*/
		template <class Type> void setParamIteration(int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
			void* dependencyObject, const Type & param);

		/**
		recursively iterates through all parameters passed as otherParams, for each parameter it determines whether it is an output parameter
		using the paramsInfo vector

		if the parameter is an output parameter it takes the corresponding value from the outputParameters vector and copies it into the parameter
		*/
		template <class FirstType, class... OtherTypes> void outputIteration(int outputIndex, int paramIndex,
			const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, void* dependencyObject, FirstType & firstParam, OtherTypes &... otherParams);

		/**
		stops the recursion of outputIteration
		*/
		template <class Type> void outputIteration(int outputIndex, int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
			void* dependencyObject, Type & param);

		/**
		adds a new value into the inputParameters vector
		*/
		template <class Type> void addInputParam(const Type & value, void(*initFunction)(const Type &, Type*, void*),
			void* dependencyObject, void(*destroyFunction)(Type &, void*))
		{
			inputParameters.push_back(new TypedValue<Type>(value, initFunction, dependencyObject, destroyFunction));
		}

		/**
		adds a new value into the outputParameters vecor
		*/
		template <class Type> void addOutputParam(const Type & value, void(*initFunction)(const Type &, Type*, void*),
			void* dependencyObject, void(*destroyFunction)(Type &, void*))
		{
			outputParameters.push_back(new TypedValue<Type>(value, initFunction, dependencyObject, destroyFunction));
		}

		/**
		sets the return value
		*/
		template <class Type> void setReturnValue(const Type & value, void(*initFunction)(const Type &, Type*, void*),
			void* dependencyObject, void(*destroyFunction)(Type &, void*))
		{
			returnValue = new TypedValue<Type>(value, initFunction, dependencyObject, destroyFunction);
		}

	public:
		CacheData() : returnValue(nullptr), size(0), creationTime(0) {}

		/**
		sets all parameters
		*/
		template <class... ParamTypes> void setParameters(const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, void* dependencyObject,
			const ParamTypes &... params)
		{
			setParamIteration(0, paramsInfo, dependencyObject, params...);
		}

		/**
		sets the return value
		*/
		template <class ReturnType> void setReturnValue(TypedReturnInfo<ReturnType>* returnInfo, void* dependencyObject,
			const ReturnType & returnValue)
		{
			if (returnInfo->returnType == CacheSystem::ReturnType::UsedReturn)
			{
				setReturnValue(returnValue, returnInfo->initFunction, dependencyObject, returnInfo->destroyFunction);
			}
		}

		/**
		returns true if all the input parameters passed as params are equal to their corresponding values stored in this object,
		otherwise returns false
		*/
		template <class... Types> bool equals(const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, void* dependencyObject, const Types &... params)
		{
			return equalsIteration(0, 0, paramsInfo, dependencyObject, params...);
		}

		/**
		copies all values that are stored in this object as output values into the corresponding parameters passed as params
		*/
		template <class... Types> void setOutput(const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, void* dependencyObject, const Types &... params)
		{
			outputIteration(0, 0, paramsInfo, dependencyObject, params...);
		}

		/**
		returns the return value
		*/
		Value* getReturnValue() { return returnValue; }

		/**
		returns the time it took to create the data
		*/
		int64_t getCreationTime() { return creationTime; }

		/**
		sets the time it took to create the data
		*/
		void setCreationTime(int64_t creationTime) { this->creationTime = creationTime; }

		/**
		returns size of the data in cache in bytes
		*/
		uint64_t getSize() { return size; }

		/**
		sets size of the data in cache in bytes
		*/
		void setSize(uint64_t bytes) { size = bytes; }

		/**
		correctly destroys the object
		*/
		~CacheData();
	};

	template <class FirstType, class... OtherTypes>
	bool CacheData::equalsIteration(int inputIndex, int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, void* dependencyObject,
		const FirstType & firstParam, const OtherTypes &... otherParams)
	{
		TypedParameterInfo<FirstType>* info = (TypedParameterInfo<FirstType>*)paramsInfo[paramIndex].get();
		if (info->paramType == ParameterType::InputParam)
		{
			TypedValue<FirstType>* value = (TypedValue<FirstType>*)inputParameters[inputIndex];
			if (!(info->equalFunction(value->getValue(), firstParam, dependencyObject)))
			{
				return false;
			}
			else
				return equalsIteration(inputIndex + 1, paramIndex + 1, paramsInfo, dependencyObject, otherParams...);
		}
		else
			return equalsIteration(inputIndex, paramIndex + 1, paramsInfo, dependencyObject, otherParams...);
	}

	template <class Type>
	bool CacheData::equalsIteration(int inputIndex, int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
		void* dependencyObject, const Type & param)
	{
		TypedParameterInfo<Type>* info = (TypedParameterInfo<Type>*)paramsInfo[paramIndex].get();
		if (info->paramType == ParameterType::InputParam)
		{
			TypedValue<Type>* value = (TypedValue<Type>*)inputParameters[inputIndex];
			return info->equalFunction(value->getValue(), param, dependencyObject);
		}
		else
			return true;
	}

	template <class FirstType, class... OtherTypes>
	void CacheData::setParamIteration(int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, void* dependencyObject,
		const FirstType & firstParam, const OtherTypes &... otherParams)
	{
		TypedParameterInfo<FirstType>* info = (TypedParameterInfo<FirstType>*)paramsInfo[paramIndex].get();
		switch (info->paramType)
		{
		case ParameterType::InputParam:
			addInputParam(firstParam, info->initFunction, dependencyObject, info->destroyFunction);
			break;
		case ParameterType::OutputParam:
			addOutputParam(firstParam, info->initFunction, dependencyObject, info->destroyFunction);
		}
		setParamIteration(paramIndex + 1, paramsInfo, dependencyObject, otherParams...);
	}

	template <class Type>
	void CacheData::setParamIteration(int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, void* dependencyObject, const Type & param)
	{
		TypedParameterInfo<Type>* info = (TypedParameterInfo<Type>*)paramsInfo[paramIndex].get();
		switch (info->paramType)
		{
		case ParameterType::InputParam:
			addInputParam(param, info->initFunction, dependencyObject, info->destroyFunction);
			break;
		case ParameterType::OutputParam:
			addOutputParam(param, info->initFunction, dependencyObject, info->destroyFunction);
		}
	}

	template <class FirstType, class... OtherTypes>
	void CacheData::outputIteration(int outputIndex, int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, void* dependencyObject,
		FirstType & firstParam, OtherTypes &... otherParams)
	{
		TypedParameterInfo<FirstType>* info = (TypedParameterInfo<FirstType>*)paramsInfo[paramIndex].get();
		if (info->paramType == ParameterType::OutputParam)
		{
			TypedValue<FirstType>* value = (TypedValue<FirstType>*)outputParameters[outputIndex];
			info->outputFunction(value->getValue(), firstParam, dependencyObject);
			outputIteration(outputIndex + 1, paramIndex + 1, paramsInfo, dependencyObject, otherParams...);
		}
		else
			outputIteration(outputIndex, paramIndex + 1, paramsInfo, dependencyObject, otherParams...);
	}

	template <class Type>
	void CacheData::outputIteration(int outputIndex, int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
		void* dependencyObject, Type & param)
	{
		TypedParameterInfo<Type>* info = (TypedParameterInfo<Type>*)paramsInfo[paramIndex].get();
		if (info->paramType == ParameterType::OutputParam)
		{
			TypedValue<Type>* value = (TypedValue<Type>*)outputParameters[outputIndex];
			info->outputFunction(value->getValue(), param, dependencyObject);
		}
	}
}

#endif