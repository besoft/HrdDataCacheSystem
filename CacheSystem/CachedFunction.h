#ifndef _CACHED_FUNCTION_H
#define _CACHED_FUNCTION_H

#include "CachedFunctionManager.h"
#include "CachedFunctionDeclaration.h"


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
	/*
	this method's code is long and ugly but it cannot be simply split into more smaller methods because it would create unnecessary copying of parameters and return value into and from lower levels during calling the methods
	*/
	template <class ReturnType, class... ParamTypes>
	ReturnType CachedFunction<ReturnType, ParamTypes...>::call(ParamTypes... params)
	{
		CachedFunctionManager::CachedFunctionCallLocker locker(getManager());
		if (numberOfParameters == -1)
			setNumberOfParameters(0, params...);
		size_t hash = calculateHash(0, params...);
		std::shared_ptr<CacheData> data = cacheData.getCacheData(hash, conf.getParamsInfo(), conf.getDependencyObject(), params...);  //find data for given input
		TypedReturnInfo<ReturnType>* returnInfo = (TypedReturnInfo<ReturnType>*)conf.getReturnInfo().get();
		if (data == nullptr) //if there are no data for given input
		{
			data = std::shared_ptr<CacheData>(new CacheData(this));
			int64_t t1, t2;
			QueryPerformanceCounter((LARGE_INTEGER*)&t1);
			ReturnType returnValue = function(params...);
			QueryPerformanceCounter((LARGE_INTEGER*)&t2);
			int64_t creationTime = (t2 - t1) / cpuTicksPerMs;
			size_t dataSize = calculateSize(0, params...);
			if (returnInfo->returnType == CacheSystem::ReturnType::UsedReturn)
				dataSize += returnInfo->getSizeFunction(returnValue, conf.getDependencyObject());
			if (creationTime < conf.getMinimumDataCreationTime() ||   //if the data were created too quickly
				dataSize > conf.getMaximumDataSize() || dataSize > manager->getCacheCapacity())  //or the data is too big
			{
				if (dataInCacheIndicator != nullptr)
					*dataInCacheIndicator = false;  //we do not store the data
				return returnValue;  //the value is returned directly, even if there is IgnoredReturn set
			}
			data->setReturnValue(conf.getReturnInfo().get(), conf.getDependencyObject(), returnValue);  //copies the return value into the data object
			data->setParameters(conf.getParamsInfo(), conf.getDependencyObject(), params...);  //copies the parameters into the data object
			data->setCreationTime(creationTime);
			data->setSize(dataSize);
			data->setHash(hash);
			if (!manager->checkSpace(dataSize))
				manager->makeSpace(dataSize);
			manager->takeSpace(dataSize);
			manager->getCachePolicy()->createData(data.get());
			cacheData.addCacheData(hash, data);
		}
		if (dataInCacheIndicator != nullptr)
			*dataInCacheIndicator = true;  //the data is stored
		manager->getCachePolicy()->hitData(data.get());
		manager->performCacheMissEvents();
		data->setOutput(conf.getParamsInfo(), conf.getDependencyObject(), params...);  //sets output praramters of this method
		//cout << "Collisions: " << cacheData.maxCollisions << endl;
		if (returnInfo->returnType == CacheSystem::ReturnType::UsedReturn)  //if return value is not ignored
		{
			ReturnType(*returnFunction)(const ReturnType &, void*) = returnInfo->returnFunction;
			if (returnFunction == StandardFunctions::DirectReturn<ReturnType>)
				return ((TypedValue<ReturnType>*)data->getReturnValue())->getValue();  //returns the value directly
			return returnFunction(((TypedValue<ReturnType>*)data->getReturnValue())->getValue(), conf.getDependencyObject());  //returns the value using the return function
		}
	}

	/*
	this method's code is long and ugly but it cannot be simply split into more smaller methods because it would create unnecessary copying of parameters into lower levels during calling the methods
	*/
	template <class... ParamTypes>
	void CachedFunction<void, ParamTypes...>::call(ParamTypes... params)
	{
		CachedFunctionManager::CachedFunctionCallLocker locker(getManager());
		if (numberOfParameters == -1)
			setNumberOfParameters(0, params...);
		size_t hash = calculateHash(0, params...);
		std::shared_ptr<CacheData> data = cacheData.getCacheData(hash, conf.getParamsInfo(), conf.getDependencyObject(), params...);
		if (data == nullptr) //if there are no data for given input
		{
			data = std::shared_ptr<CacheData>(new CacheData(this));
			int64_t t1, t2;
			QueryPerformanceCounter((LARGE_INTEGER*)&t1);
			function(params...);
			QueryPerformanceCounter((LARGE_INTEGER*)&t2);
			int64_t creationTime = (t2 - t1) / cpuTicksPerMs;
			size_t dataSize = calculateSize(0, params...);
			if (creationTime < conf.getMinimumDataCreationTime() ||  //if the data were created too quickly
				dataSize > conf.getMaximumDataSize() || dataSize > manager->getCacheCapacity()) //or the data is too big
			{
				if (dataInCacheIndicator != nullptr)
					*dataInCacheIndicator = false;  //we do not store the data
				return;
			}
			data->setParameters(conf.getParamsInfo(), conf.getDependencyObject(), params...);  //copies the parameters into the data object
			data->setCreationTime(creationTime);
			data->setSize(dataSize);
			data->setHash(hash);
			if (!manager->checkSpace(dataSize))
				manager->makeSpace(dataSize);
			manager->takeSpace(dataSize);
			manager->getCachePolicy()->createData(data.get());
			cacheData.addCacheData(hash, data);
		}
		if (dataInCacheIndicator != nullptr)
			*dataInCacheIndicator = true;  //the data is stored
		manager->getCachePolicy()->hitData(data.get());
		manager->performCacheMissEvents();
		data->setOutput(conf.getParamsInfo(), conf.getDependencyObject(), params...);  //sets output parameters of this method
	}
}

#endif