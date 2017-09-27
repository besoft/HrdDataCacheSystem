#ifndef _CACHE_CONFIGURATION_H
#define _CACHE_CONFIGURATION_H
#include <vector>
#include <stdint.h>
#include "TypedParameterInfo.h"
#include "ReturnInfo.h"
#include "TypedReturnInfo.h"

namespace CacheSystem
{
	/**
	contains configuration for a cached function
	*/
	template<typename DependencyObj>
	class CacheConfigurationWithDepObj
	{
	private:
		/**
		contains information about all parameters
		*/
		std::vector<std::shared_ptr<ParameterInfo> > paramsInfo;

		/**
		contains information about the return value
		*/
		std::shared_ptr<ReturnInfo> returnInfo;

		/**
		the dependency object that is passed to every function manipulating the cached data
		*/
		DependencyObj dependencyObject{};

		/**
		any data that are created in shorter time will not be cached
		*/
		int64_t minimumDataCreationTime;

		/**
		any data bigger than this will not be cached
		*/
		uint64_t maximumDataSize;

	public:
		/**
		sets information about a single parameter with a given index
		*/
		template <class Type> void setParamInfo(unsigned int paramIndex, const TypedParameterInfoWithDepObj<Type, DependencyObj>& paramInfo);

		/**
		sets information about the return value
		*/
		template <class Type> void setReturnInfo(const TypedReturnInfoWithDepObj<Type, DependencyObj>& returnInfo)
		{
			this->returnInfo = std::shared_ptr<ReturnInfo>(new TypedReturnInfoWithDepObj<Type, DependencyObj>(returnInfo));
		}

		/**
		creates a clear configuration with no information set
		*/
		CacheConfigurationWithDepObj() : returnInfo(nullptr), minimumDataCreationTime(0), maximumDataSize(UINT64_MAX) {}

		/**
		correctly creates a copy of a given configuration
		*/
		CacheConfigurationWithDepObj(const CacheConfigurationWithDepObj & conf);

		/**
		returns a vector containing information about all parameters, the info objects are in the same order as the parameters
		*/
		const std::vector<std::shared_ptr<ParameterInfo> > & getParamsInfo() { return paramsInfo; }

		/**
		returns information about the return value
		*/
		std::shared_ptr<ReturnInfo> getReturnInfo() { return returnInfo; }

		/**
		dependency object setter
		*/
		void setDependencyObject(DependencyObj dependencyObject) { this->dependencyObject = dependencyObject; }

		/**
		dependency object getter
		*/
		DependencyObj getDependencyObject() { return dependencyObject; }

		/**
		sets the minimum data creation time
		any data that are created in shorter time will not be cached
		*/
		void setMinimumDataCreationTime(int64_t time) { minimumDataCreationTime = time; }

		/**
		gets the minimum data creation time
		*/
		int64_t getMinimumDataCreationTime() { return minimumDataCreationTime; }

		/**
		sets the maximum data size
		any data bigger than this will not be cached
		*/
		void setMaximumDataSize(uint64_t bytes) { maximumDataSize = bytes; }

		/**
		gets the maximum data size
		*/
		uint64_t getMaximumDataSize() { return maximumDataSize; }
	};

	template <typename DependencyObj>
	CacheConfigurationWithDepObj<DependencyObj>::CacheConfigurationWithDepObj(const CacheConfigurationWithDepObj<DependencyObj> & conf)
	{
		for (unsigned int i = 0; i < conf.paramsInfo.size(); i++)
			paramsInfo.push_back(conf.paramsInfo[i]->getCopy());
		returnInfo = conf.returnInfo == nullptr ? nullptr : conf.returnInfo->getCopy();
		minimumDataCreationTime = conf.minimumDataCreationTime;		
		dependencyObject = conf.dependencyObject;
		maximumDataSize = conf.maximumDataSize;
	}
	
	template <typename DependencyObj>
	template <class Type>
	void CacheConfigurationWithDepObj<DependencyObj>::setParamInfo(unsigned int paramIndex, const TypedParameterInfoWithDepObj<Type, DependencyObj>& paramInfo)
	{
		while (paramsInfo.size() <= paramIndex)
			paramsInfo.push_back(nullptr);
		paramsInfo[paramIndex] = std::shared_ptr<ParameterInfo>(new TypedParameterInfoWithDepObj<Type, DependencyObj>(paramInfo));
	}

	/**
	contains configuration for a cached function with void* dependency object
	*/
	using CacheConfiguration = typename CacheConfigurationWithDepObj<void*>;

	/**
	contains configuration for a cached function using no dependency object
	*/
	using CacheConfigurationNoDepObj = typename CacheConfigurationWithDepObj<NoDepObj>;
}

#endif