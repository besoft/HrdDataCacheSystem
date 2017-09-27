#ifndef _ABSTRACT_CACHED_FUNCTION_H
#define _ABSTRACT_CACHED_FUNCTION_H

#include <windows.h>
#include <stdint.h>
#include "CachedFunctionParent.h"
#include "CacheConfiguration.h"
#include "CacheDataStructure.h"


namespace CacheSystem
{
	class CachedFunctionManager;
	/**
	manages the caching and contains all the cached data
	this is an abstract parent for both functions with a return value and functions without a return value (void)
	*/
	template <class DependencyObj, class ReturnType, class... ParamTypes>
	class AbstractCachedFunctionWithDepObj : public CachedFunctionParent
	{
	protected:
		/**
		contains configuration for the caching
		*/
		CacheConfigurationWithDepObj<DependencyObj> conf;

		/**
		number of parameters of the function
		*/
		const int numberOfParameters = sizeof...(ParamTypes);

		/**
		after the call method is called the value on the given address is set to true if the data on the output are stored in cache, otherwise it is set to false
		*/
		bool* dataInCacheIndicator;

		/**
		contains all cached data
		*/
		CacheDataStructureWithDepObj<DependencyObj> cacheData;

		/**
		ticks per millisecond
		*/
		int64_t cpuTicksPerMs;

		/**
		function which is called to create the data to cache
		the CachedFunction object simply simulates calling this function
		*/
		std::function<ReturnType(ParamTypes...)> function;

		/**
		recursively iterates through all parameters passed as otherParams and calculates the hash value of all input parameters
		*/
		template <class FirstType, class... OtherTypes> size_t calculateHash(int paramIndex, const FirstType & firstParam,
			const OtherTypes &... otherParams);

		/**
		stops the recursion of calculateHash
		*/
		template <class Type> size_t calculateHash(int paramIndex, const Type & param);

		/**
		recursively iterates through all parameters passed as otherParams and calculates the sum of their sizes
		*/
		template <class FirstType, class... OtherTypes> size_t calculateSize(int paramIndex, const FirstType & firstParam,
			const OtherTypes &... otherParams);

		/**
		stops the recursion of calculateSize
		*/
		template <class Type> size_t calculateSize(int paramIndex, const Type & param);

		/**
		creates the object, the conf object is copied
		*/
		AbstractCachedFunctionWithDepObj(const CacheConfigurationWithDepObj<DependencyObj> & conf,
			std::function<ReturnType(ParamTypes...)>& function, CachedFunctionManager* manager);

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

		/**
		resets the data structure's iterator to the beginning
		*/
		void resetDataIterator() { cacheData.resetDataIterator(); }

		/**
		returns the next data from the data structure using the data structure's iterator
		*/
		CacheData* getNextData() { return cacheData.getNextData(); }

		/**
		removes given data object from the data structure
		*/
		void removeData(CacheData* data) { cacheData.removeData(data); }
	};

	/**
	creates the object, the conf object is copied
	*/
	template <class DependencyObj, class ReturnType, class... ParamTypes>
	AbstractCachedFunctionWithDepObj<DependencyObj, ReturnType, ParamTypes...>::AbstractCachedFunctionWithDepObj(
		const CacheConfigurationWithDepObj<DependencyObj> & conf,
		std::function<ReturnType(ParamTypes...)>& function, CachedFunctionManager* manager)
		: CachedFunctionParent(manager), conf(conf), function(function), dataInCacheIndicator(nullptr)
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&cpuTicksPerMs);
		cpuTicksPerMs /= 1000;
	}

	/**
	recursively iterates through all parameters passed as otherParams and calculates the hash value of all input parameters
	*/
	template <class DependencyObj, class ReturnType, class... ParamTypes>
	template <class FirstType, class... OtherTypes> size_t AbstractCachedFunctionWithDepObj<DependencyObj, ReturnType, ParamTypes...>::calculateHash(int paramIndex,
		const FirstType & firstParam, const OtherTypes &... otherParams)
	{
		size_t seed = calculateHash(paramIndex, firstParam);
		hash_combine(seed, calculateHash(paramIndex + 1, otherParams...));
		return seed;
	}

	/**
	stops the recursion of calculateHash
	*/
	template <class DependencyObj, class ReturnType, class... ParamTypes>
	template <class Type> size_t AbstractCachedFunctionWithDepObj<DependencyObj, ReturnType, ParamTypes...>::calculateHash(int paramIndex, const Type & param)
	{		
		TypedParameterInfoWithDepObj<Type, DependencyObj>* paramInfo = (TypedParameterInfoWithDepObj<Type, DependencyObj>*)conf.getParamsInfo()[paramIndex].get();
		if (paramInfo->paramType == ParameterType::InputParam)
			return DMFuncInvoker<DependencyObj>(conf.getDependencyObject())(paramInfo->hashFunction, param);
		return 0;		
	}	

	/**
	recursively iterates through all parameters passed as otherParams and calculates the sum of their sizes
	*/
	template <class DependencyObj, class ReturnType, class... ParamTypes>
	template <class FirstType, class... OtherTypes> size_t AbstractCachedFunctionWithDepObj<DependencyObj, ReturnType, ParamTypes...>::calculateSize(int paramIndex,
		const FirstType & firstParam, const OtherTypes &... otherParams)
	{		
		return calculateSize(paramIndex, firstParam) + calculateSize(paramIndex + 1, otherParams...);
	}

	/**
	stops the recursion of calculateSize
	*/
	template <class DependencyObj, class ReturnType, class... ParamTypes>
	template <class Type> size_t AbstractCachedFunctionWithDepObj<DependencyObj, ReturnType, ParamTypes...>::calculateSize(int paramIndex, const Type & param)
	{		
		TypedParameterInfoWithDepObj<Type, DependencyObj>* paramInfo = (TypedParameterInfoWithDepObj<Type, DependencyObj>*)conf.getParamsInfo()[paramIndex].get();
		if (paramInfo->paramType != ParameterType::IgnoredParam)
			return DMFuncInvoker<DependencyObj>(conf.getDependencyObject())(paramInfo->getSizeFunction, param);
		return 0;
	}
}

#endif