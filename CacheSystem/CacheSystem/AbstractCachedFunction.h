#ifndef _ABSTRACT_CACHED_FUNCTION_H
#define _ABSTRACT_CACHED_FUNCTION_H

#include <memory>
#include <windows.h>
#include <stdint.h>
#include "CachedFunctionParent.h"
#include "CacheConfiguration.h"
#include "CacheDataStructure.h"

namespace CacheSystem
{
	/**
	pre-declaration of CacheFunctionManager class
	*/
	class CachedFunctionManager;
	
	/**
	manages the caching and contains all the cached data
	this is an abstract parent for both functions with a return value and functions without a return value (void)
	*/
	template <class ReturnType, class... ParamTypes>
	class AbstractCachedFunction : public CachedFunctionParent
	{
	protected:
		/**
		pointer to the corresponding CachedFunctionManager
		*/
		CachedFunctionManager* manager;

		/**
		contains configuration for the caching
		*/
		CacheConfiguration conf;

		/**
		number of parameters of the function
		*/
		int numberOfParameters;

		/**
		after the call method is called the value on the given adress is set to true if the data on the output are stored in cache, otherwise it is set to false
		*/
		bool* dataInCacheIndicator;

		/**
		contains all cached data
		*/
		CacheDataStructure cacheData;

		/**
		ticks per millisecond
		*/
		int64_t cpuTicksPerMs;

		/**
		function which is called to create the data to cache
		the CachedFunction object simply simulates calling this function
		*/
		ReturnType(*function)(ParamTypes...);

		/**
		recursively iterates through all parameters passed as otherParams and counts them, the result is stored into the numberOfParameters
		*/
		template <class FirstType, class... OtherTypes> void setNumberOfParameters(int numberOfParameters, const FirstType & firstParam,
			const OtherTypes &... otherParams)
		{
			setNumberOfParameters(numberOfParameters + 1, otherParams...);
		}

		/**
		stops the recursion of setNumberOfParameters
		*/
		template <class Type> void setNumberOfParameters(int numberOfParameters, const Type & param)
		{
			this->numberOfParameters = numberOfParameters + 1;
		}

		/**
		recursively iterates through all parameters passed as otherParams and calculates the hash value of all input parameters
		*/
		template <class FirstType, class... OtherTypes> uint64_t calculateHash(int paramIndex, int inputParamIndex, const FirstType & firstParam,
			const OtherTypes &... otherParams)
		{
			uint64_t hash = 0;
			TypedParameterInfo<FirstType>* paramInfo = (TypedParameterInfo<FirstType>*)conf.getParamsInfo()[paramIndex].get();
			if (paramInfo->paramType == ParameterType::InputParam)
			{
				inputParamIndex++;
				hash = inputParamIndex * paramInfo->hashFunction(firstParam, conf.getDependencyObject());
			}
			return hash + calculateHash(paramIndex + 1, inputParamIndex, otherParams...);
		}

		/**
		stops the recursion of calculateHash
		*/
		template <class Type> uint64_t calculateHash(int paramIndex, int inputParamIndex, const Type & param)
		{
			uint64_t hash = 0;
			TypedParameterInfo<Type>* paramInfo = (TypedParameterInfo<Type>*)conf.getParamsInfo()[paramIndex].get();
			if (paramInfo->paramType == ParameterType::InputParam)
			{
				inputParamIndex++;
				hash = inputParamIndex * paramInfo->hashFunction(param, conf.getDependencyObject());
			}
			return hash;
		}

		/**
		recursively iterates through all parameters passed as otherParams and calculates the sum of their sizes
		*/
		template <class FirstType, class... OtherTypes> uint64_t calculateSize(int paramIndex, const FirstType & firstParam, const OtherTypes &... otherParams)
		{
			uint64_t size = 0;
			TypedParameterInfo<FirstType>* paramInfo = (TypedParameterInfo<FirstType>*)conf.getParamsInfo()[paramIndex].get();
			if (paramInfo->paramType != ParameterType::IgnoredParam)
				size += paramInfo->getSizeFunction(firstParam, conf.getDependencyObject());
			return size + calculateSize(paramIndex + 1, otherParams...);
		}

		/**
		stops the recursion of calculateSize
		*/
		template <class Type> uint64_t calculateSize(int paramIndex, const Type & param)
		{
			uint64_t size = 0;
			TypedParameterInfo<Type>* paramInfo = (TypedParameterInfo<Type>*)conf.getParamsInfo()[paramIndex].get();
			if (paramInfo->paramType != ParameterType::IgnoredParam)
				size += paramInfo->getSizeFunction(param, conf.getDependencyObject());
			return size;
		}

		/**
		creates the object, the conf object is copied
		*/
		AbstractCachedFunction(const CacheConfiguration & conf, ReturnType(*function)(ParamTypes...), CachedFunctionManager* manager)
			: conf(conf), function(function), numberOfParameters(-1), dataInCacheIndicator(nullptr), manager(manager)
		{
			QueryPerformanceFrequency((LARGE_INTEGER*)&cpuTicksPerMs);
			cpuTicksPerMs /= 1000;
		}

	public:
		/**
		looks into the cache data structure, finds the return value and output parameters which correspond to the given input parameters,
		the return value is then returned and the output parameters are copied into the actual parameters of this method

		if no data is found, the function is called to create it
		*/
		virtual ReturnType call(ParamTypes... params) = 0;

		/**
		sets the "data in cache indicator"
		after the call method is called the value on the given adress is set to true if the data on the output are stored in cache, otherwise it is set to false
		the pointer can be reset before each call
		*/
		void setDataInCacheIndicator(bool* ptr) { dataInCacheIndicator = ptr; }
	};
}

#endif