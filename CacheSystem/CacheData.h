#ifndef _CACHE_DATA_H
#define _CACHE_DATA_H

#include <vector>
#include <stdint.h>
#include "TypedValue.h"
#include "ParameterType.h"
#include "ParameterInfo.h"
#include "ReturnType.h"
#include "TypedReturnInfo.h"
#include "CachedFunctionParent.h"

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
		size_t size;

		/**
		user data associated with the data object, it can be anything
		can be used by the cache policy
		*/
		void* userData;

		/**
		the data object's hash value
		*/
		size_t hash;

		/**
		the cache object in which this data object is stored
		*/
		CachedFunctionParent* cachedFunction;


		/**
		recursively iterates through all parameters passed as otherParams, for each parameter it determines whether it is an input parameter
		using the paramsInfo vector
		
		if the parameter is an input parameter it is compared to a corresponding parameter in the
		inputParameters vector

		if results of all the comparisons are true, the method returns true, otherwise it returns false
		*/
		template <class DepObj, class FirstType, class... OtherTypes> bool equalsIteration(int inputIndex, int paramIndex,
			const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, DepObj dependencyObject, const FirstType & firstParam, const OtherTypes &... otherParams);

		/**
		stops the recursion of equalIteration
		*/
		template <class DepObj, class Type> bool equalsIteration(int inputIndex, int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
			DepObj dependencyObject, const Type & param);

		/**
		recursively iterates through all parameters passed as otherParams and sotres all input parameters into the inputParameters vector
		and all outputParameters into the outputParameters vector using the paramsInfo vector
		*/
		template <class DepObj, class FirstType, class... OtherTypes> void setParamIteration(int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
			DepObj dependencyObject, const FirstType & firstParam, const OtherTypes &... otherParams);

		/**
		stops the recursion of setParamIteration
		*/
		template <class DepObj, class Type> void setParamIteration(int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
			DepObj dependencyObject, const Type & param);

		/**
		recursively iterates through all parameters passed as otherParams, for each parameter it determines whether it is an output parameter
		using the paramsInfo vector

		if the parameter is an output parameter it takes the corresponding value from the outputParameters vector and copies it into the parameter
		*/
		template <class DepObj, class FirstType, class... OtherTypes> void outputIteration(int outputIndex, int paramIndex,
			const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, DepObj dependencyObject, FirstType & firstParam, OtherTypes &... otherParams);

		/**
		stops the recursion of outputIteration
		*/
		template <class DepObj, class Type> void outputIteration(int outputIndex, int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
			DepObj dependencyObject, Type & param);

		/**
		adds a new value into the inputParameters vector
		*/
		template <class DepObj, class Type> void addInputParam(const Type & value, typename TypedReturnInfoWithDepObj<Type, DepObj>::InitFunction& initFunction,
			DepObj dependencyObject, typename TypedReturnInfoWithDepObj<Type, DepObj>::DestroyFunction& destroyFunction)
		{
			inputParameters.push_back(new TypedValueWithDepObj<Type, DepObj>(value, initFunction, dependencyObject, destroyFunction));
		}

		/**
		adds a new value into the outputParameters vecor
		*/
		template <class DepObj, class Type> void addOutputParam(const Type & value, typename TypedReturnInfoWithDepObj<Type, DepObj>::InitFunction& initFunction,
			DepObj dependencyObject, typename TypedReturnInfoWithDepObj<Type, DepObj>::DestroyFunction& destroyFunction)
		{
			outputParameters.push_back(new TypedValueWithDepObj<Type, DepObj>(value, initFunction, dependencyObject, destroyFunction));
		}

		/**
		sets the return value
		*/
		template <class DepObj, class Type> void setReturnValue(const Type & value, 
			typename TypedReturnInfoWithDepObj<Type, DepObj>::InitFunction& initFunction,
			DepObj dependencyObject, typename TypedReturnInfoWithDepObj<Type, DepObj>::DestroyFunction& destroyFunction)
		{
			returnValue = new TypedValueWithDepObj<Type, DepObj>(value, initFunction, dependencyObject, destroyFunction);
		}

	public:
		/**
		initializing constructor
		the parameter is the cache object in which the data will be stored
		*/
		CacheData(CachedFunctionParent* cachedFunction) : returnValue(nullptr), size(0), creationTime(0), cachedFunction(cachedFunction) {}

		/**
		sets all parameters
		*/
		template <class DepObj, class... ParamTypes> void setParameters(const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
			DepObj dependencyObject,	const ParamTypes &... params)
		{
			setParamIteration(0, paramsInfo, dependencyObject, params...);
		}

		/**
		sets the return value
		*/
		template <class DepObj, class ReturnType> void setReturnValue(ReturnInfo* returnInfo, DepObj dependencyObject, const ReturnType & returnValue);

		/**
		returns true if all the input parameters passed as params are equal to their corresponding values stored in this object,
		otherwise returns false
		*/
		template <class DepObj, class... Types> bool equals(const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, DepObj dependencyObject, const Types &... params)
		{
			return equalsIteration(0, 0, paramsInfo, dependencyObject, params...);
		}

		/**
		copies all values that are stored in this object as output values into the corresponding parameters passed as params
		*/
		template <class DepObj, class... Types> void setOutput(const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, DepObj dependencyObject, const Types &... params)
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
		size_t getSize() { return size; }

		/**
		sets size of the data in cache in bytes
		*/
		void setSize(size_t bytes) { size = bytes; }

		/**
		sets user data associated with the data object, it can be anything
		can be used by the cache policy
		*/
		void setUserData(void* userData) { this->userData = userData; }

		/**
		returns the user data associated with the data object
		*/
		void* getUserData() { return userData; }

		/**
		sets the hash value of this data object
		*/
		void setHash(size_t hash) { this->hash = hash; }

		/**
		returns this object's hash value
		*/
		size_t getHash() { return hash; }

		/**
		returns the cache object in which the data object is stored
		*/
		CachedFunctionParent* getCachedFunction() { return cachedFunction; }

		/**
		correctly destroys the object
		*/
		~CacheData();
	};

	template <class DepObj, class ReturnType> void CacheData::setReturnValue(ReturnInfo* returnInfo, DepObj dependencyObject, const ReturnType & returnValue)
	{
		if (returnInfo->returnType == CacheSystem::ReturnType::UsedReturn)
		{
			TypedReturnInfoWithDepObj<ReturnType, DepObj>* typedReturnInfo = (TypedReturnInfoWithDepObj<ReturnType, DepObj>*)returnInfo;
			setReturnValue(returnValue, typedReturnInfo->initFunction, dependencyObject, typedReturnInfo->destroyFunction);
		}
	}

	template <class DepObj, class FirstType, class... OtherTypes>
	bool CacheData::equalsIteration(int inputIndex, int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, DepObj dependencyObject,
		const FirstType & firstParam, const OtherTypes &... otherParams)
	{
		TypedParameterInfoWithDepObj<FirstType, DepObj>* info = (TypedParameterInfoWithDepObj<FirstType, DepObj>*)paramsInfo[paramIndex].get();
		if (info->paramType == ParameterType::InputParam)
		{
			TypedValueWithDepObj<FirstType, DepObj>* value = (TypedValueWithDepObj<FirstType, DepObj>*)inputParameters[inputIndex];
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

	template <class DepObj, class Type>
	bool CacheData::equalsIteration(int inputIndex, int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
		DepObj dependencyObject, const Type & param)
	{
		TypedParameterInfoWithDepObj<Type, DepObj>* info = (TypedParameterInfoWithDepObj<Type, DepObj>*)paramsInfo[paramIndex].get();
		if (info->paramType == ParameterType::InputParam)
		{
			TypedValueWithDepObj<Type, DepObj>* value = (TypedValueWithDepObj<Type, DepObj>*)inputParameters[inputIndex];
			return info->equalFunction(value->getValue(), param, dependencyObject);
		}
		else
			return true;
	}

	template <class DepObj, class FirstType, class... OtherTypes>
	void CacheData::setParamIteration(int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, DepObj dependencyObject,
		const FirstType & firstParam, const OtherTypes &... otherParams)
	{
		TypedParameterInfoWithDepObj<FirstType, DepObj>* info = (TypedParameterInfoWithDepObj<FirstType, DepObj>*)paramsInfo[paramIndex].get();
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

	template <class DepObj, class Type>
	void CacheData::setParamIteration(int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, DepObj dependencyObject, const Type & param)
	{
		TypedParameterInfoWithDepObj<Type, DepObj>* info = (TypedParameterInfoWithDepObj<Type, DepObj>*)paramsInfo[paramIndex].get();
		switch (info->paramType)
		{
		case ParameterType::InputParam:
			addInputParam(param, info->initFunction, dependencyObject, info->destroyFunction);
			break;
		case ParameterType::OutputParam:
			addOutputParam(param, info->initFunction, dependencyObject, info->destroyFunction);
		}
	}

	template <class DepObj, class FirstType, class... OtherTypes>
	void CacheData::outputIteration(int outputIndex, int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo, DepObj dependencyObject,
		FirstType & firstParam, OtherTypes &... otherParams)
	{
		TypedParameterInfoWithDepObj<FirstType, DepObj>* info = (TypedParameterInfoWithDepObj<FirstType, DepObj>*)paramsInfo[paramIndex].get();
		if (info->paramType == ParameterType::OutputParam)
		{
			TypedValueWithDepObj<FirstType, DepObj>* value = (TypedValueWithDepObj<FirstType, DepObj>*)outputParameters[outputIndex];
			info->outputFunction(value->getValue(), firstParam, dependencyObject);
			outputIteration(outputIndex + 1, paramIndex + 1, paramsInfo, dependencyObject, otherParams...);
		}
		else
			outputIteration(outputIndex, paramIndex + 1, paramsInfo, dependencyObject, otherParams...);
	}

	template <class DepObj, class Type>
	void CacheData::outputIteration(int outputIndex, int paramIndex, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
		DepObj dependencyObject, Type & param)
	{
		TypedParameterInfoWithDepObj<Type, DepObj>* info = (TypedParameterInfoWithDepObj<Type, DepObj>*)paramsInfo[paramIndex].get();
		if (info->paramType == ParameterType::OutputParam)
		{
			TypedValueWithDepObj<Type, DepObj>* value = (TypedValueWithDepObj<Type, DepObj>*)outputParameters[outputIndex];
			info->outputFunction(value->getValue(), param, dependencyObject);
		}
	}
}

#endif