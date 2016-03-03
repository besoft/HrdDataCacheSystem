#ifndef _CACHED_FUNCTION_H
#define _CACHED_FUNCTION_H

#include <memory>
#include "CacheConfiguration.h"
#include "CacheDataStructure.h"

namespace CacheSystem
{
	/**
	manages the caching and contains all the cached data
	*/
	template <class ReturnType, class... ParamTypes>
	class CachedFunction
	{
	private:
		/**
		contains configuration for the caching
		*/
		CacheConfiguration conf;
		int numberOfParameters;

		/**
		contains all cached data
		*/
		CacheDataStructure cacheData;

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

	public:
		/**
		creates the object, the conf object is copied
		*/
		CachedFunction(const CacheConfiguration & conf, ReturnType(*function)(ParamTypes...))
			: conf(conf), function(function), numberOfParameters(-1) {}

		/**
		looks into the cache data structure, finds the return value and output parameters which correspond to the given input parameters,
		the return value is then returned and the output parameters are copied into the actual parameters of this method

		if no data is found, the function is called to create it
		*/
		ReturnType call(ParamTypes... params);
		template <class... FuncParamTypes> void call(FuncParamTypes... params);
	};

	template <class ReturnType, class... ParamTypes>
	ReturnType CachedFunction<ReturnType, ParamTypes...>::call(ParamTypes... params)
	{
		if (numberOfParameters == -1)
			setNumberOfParameters(0, params...);
		std::shared_ptr<CacheData> data = cacheData.getCacheData(conf.getParamsInfo(), nullptr, params...);
		if (data == nullptr)
		{
			data = std::shared_ptr<CacheData>(new CacheData);
			data->setReturnValue((TypedReturnInfo<ReturnType>*)conf.getReturnInfo(), nullptr, function(params...));
			data->setParameters(conf.getParamsInfo(), nullptr, params...);
			cacheData.addCacheData(data);
		}
		data->setOutput(conf.getParamsInfo(), nullptr, params...);
		TypedReturnInfo<ReturnType>* returnInfo = (TypedReturnInfo<ReturnType>*)conf.getReturnInfo();
		if (returnInfo->returnType == CacheSystem::ReturnType::UsedReturn)
		{
 			ReturnType(*returnFunction)(const ReturnType &, void**) = returnInfo->returnFunction;
			if (returnFunction == StandardFunctions::DirectReturn<ReturnType>)
				return ((TypedValue<ReturnType>*)data->getReturnValue())->getValue();
			return returnFunction(((TypedValue<ReturnType>*)data->getReturnValue())->getValue(), nullptr);
		}
	}

	template <class ReturnType, class... ParamTypes>
	template <class... FuncParamTypes>
	void CachedFunction<ReturnType, ParamTypes...>::call(FuncParamTypes... params)
	{
		std::cout << "VOID!!!!!!" << std::endl;
	}
}

#endif