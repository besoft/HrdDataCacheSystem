#ifndef _CACHE_DATA_STRUCTURE_H
#define _CACHE_DATA_STRUCTURE_H

#include <vector>
#include <memory>
#include <stdint.h>
#include "CacheData.h"

namespace CacheSystem
{
	/**
	contains all cached data
	*/
	class CacheDataStructure
	{
	private:
		/**
		vector of cached data
		*/
		std::vector<std::shared_ptr<CacheData> > cacheData;

	public:
		/**
		finds and returns the CacheData object in which all the input parameters are equal to the corresponding input parameters passed as params
		*/
		template <class... Types> std::shared_ptr<CacheData> getCacheData(uint64_t hash, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
			void* dependencyObject, const Types &... params);

		/**
		adds new cache data, the CacheData object must be created by the new operator and it will be deleted during deleting this object
		*/
		void addCacheData(uint64_t hash, std::shared_ptr<CacheData> data);
	};

	template <class... Types>
	std::shared_ptr<CacheData> CacheDataStructure::getCacheData(uint64_t hash, const std::vector<std::shared_ptr<ParameterInfo> > & paramsInfo,
		void* dependencyObject, const Types &... params)
	{
		for (unsigned int i = 0; i < cacheData.size(); i++)
			if (cacheData[i]->equals(paramsInfo, dependencyObject, params...))
				return cacheData[i];
		return nullptr;
	}
}

#endif