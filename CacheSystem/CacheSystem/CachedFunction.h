#ifndef _CACHED_FUNCTION_H
#define _CACHED_FUNCTION_H

#include "AbstractCachedFunction.h"

/*
disables warning "not all control paths return a value" for the call method
in cases when the user sets IgnoredReturn for the return value the call method does not return anything which means it returns an undefined value of the given return type
the warning is ignored because this can only happen when the user wants it to happen
for primitive types and pointers, if the user does not use the returned value, this doesn't realy matter
for objects the IgnoredReturn option should be used with high caution, because it may cause runtime errors due to calling the copy constructor even when the user does not use the returned value
*/
#pragma warning(disable : 4715)

namespace CacheSystem
{
	/**
	manages the caching and contains all the cached data
	this class is for functions with a return value
	*/
	template <class ReturnType, class... ParamTypes>
	class CachedFunction : public AbstractCachedFunction<ReturnType, ParamTypes...>
	{
	public:
		CachedFunction(const CacheConfiguration & conf, ReturnType(*function)(ParamTypes...)) : AbstractCachedFunction(conf, function) {}
		ReturnType call(ParamTypes... params);
	};

	/**
	manages the caching and contains all the cached data
	this specialization is for functions without a return value (void)
	*/
	template <class... ParamTypes>
	class CachedFunction<void, ParamTypes...> : public AbstractCachedFunction<void, ParamTypes...>
	{
	public:
		CachedFunction(const CacheConfiguration & conf, void(*function)(ParamTypes...)) : AbstractCachedFunction(conf, function) {}
		void call(ParamTypes... params);
	};

	template <class ReturnType, class... ParamTypes>
	ReturnType CachedFunction<ReturnType, ParamTypes...>::call(ParamTypes... params)
	{
		if (numberOfParameters == -1)
			setNumberOfParameters(0, params...);
		std::shared_ptr<CacheData> data = cacheData.getCacheData(conf.getParamsInfo(), conf.getDependencyObject(), params...);
		if (data == nullptr)
		{
			data = std::shared_ptr<CacheData>(new CacheData);
			ReturnType returnValue = function(params...);
			data->setReturnValue((TypedReturnInfo<ReturnType>*)conf.getReturnInfo().get(), conf.getDependencyObject(), returnValue);
			data->setParameters(conf.getParamsInfo(), conf.getDependencyObject(), params...);
			cacheData.addCacheData(data);
		}
		data->setOutput(conf.getParamsInfo(), conf.getDependencyObject(), params...);
		TypedReturnInfo<ReturnType>* returnInfo = (TypedReturnInfo<ReturnType>*)conf.getReturnInfo().get();
		if (returnInfo->returnType == CacheSystem::ReturnType::UsedReturn)
		{
 			ReturnType(*returnFunction)(const ReturnType &, void*) = returnInfo->returnFunction;
			if (returnFunction == StandardFunctions::DirectReturn<ReturnType>)
				return ((TypedValue<ReturnType>*)data->getReturnValue())->getValue();
			return returnFunction(((TypedValue<ReturnType>*)data->getReturnValue())->getValue(), conf.getDependencyObject());
		}
	}

	template <class... ParamTypes>
	void CachedFunction<void, ParamTypes...>::call(ParamTypes... params)
	{
		if (numberOfParameters == -1)
			setNumberOfParameters(0, params...);
		std::shared_ptr<CacheData> data = cacheData.getCacheData(conf.getParamsInfo(), conf.getDependencyObject(), params...);
		if (data == nullptr)
		{
			data = std::shared_ptr<CacheData>(new CacheData);
			function(params...);
			data->setParameters(conf.getParamsInfo(), conf.getDependencyObject(), params...);
			cacheData.addCacheData(data);
		}
		data->setOutput(conf.getParamsInfo(), conf.getDependencyObject(), params...);
	}
}

#endif