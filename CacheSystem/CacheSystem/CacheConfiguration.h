#ifndef _CACHE_CONFIGURATION_H
#define _CACHE_CONFIGURATION_H
#include <vector>
#include "TypedParameterInfo.h"
#include "ReturnInfo.h"
#include "TypedReturnInfo.h"

namespace CacheSystem
{
	/**
	contains configuration for a cached function
	*/
	class CacheConfiguration
	{
	private:
		/**
		contains information about all parametes
		*/
		std::vector<std::shared_ptr<ParameterInfo> > paramsInfo;

		/**
		contains information about the return value
		*/
		std::shared_ptr<ReturnInfo> returnInfo;

	public:
		/**
		sets information about a single parameter with a given index
		*/
		template <class Type> void setParamInfo(unsigned int paramIndex, TypedParameterInfo<Type> paramInfo);

		/**
		sets information about the return value
		*/
		template <class Type> void setReturnInfo(TypedReturnInfo<Type> returnInfo)
		{
			this->returnInfo = std::shared_ptr<ReturnInfo>(new TypedReturnInfo<Type>(returnInfo));
		}

		/**
		creates a clear configuration with no information set
		*/
		CacheConfiguration() : returnInfo(nullptr) {}

		/**
		correctly creates a copy of a given configuration
		*/
		CacheConfiguration(const CacheConfiguration & conf);

		/**
		returns a vector containing information about all parameters, the info objects are in the same order as the parameters
		*/
		const std::vector<std::shared_ptr<ParameterInfo> > & getParamsInfo() { return paramsInfo; }

		/**
		returns information about the return value
		*/
		std::shared_ptr<ReturnInfo> getReturnInfo() { return returnInfo; }
	};

	template <class Type>
	void CacheConfiguration::setParamInfo(unsigned int paramIndex, TypedParameterInfo<Type> paramInfo)
	{
		while (paramsInfo.size() <= paramIndex)
			paramsInfo.push_back(nullptr);
		paramsInfo[paramIndex] = std::shared_ptr<ParameterInfo>(new TypedParameterInfo<Type>(paramInfo));
	}
}

#endif