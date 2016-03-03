#ifndef _CACHE_DATA_STRUCTURE_H
#define _CACHE_DATA_STRUCTURE_H

#include <vector>
#include <memory>
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
		template <class... Types> std::shared_ptr<CacheData> getCacheData(const std::vector<ParameterInfo*> & paramsInfo,
			void** dependencies, const Types &... params);

		/**
		adds new cache data, the CacheData object must be created by the new operator and it will be deleted during deleting this object
		*/
		void addCacheData(std::shared_ptr<CacheData> data);
	};

	template <class... Types>
	std::shared_ptr<CacheData> CacheDataStructure::getCacheData(const std::vector<ParameterInfo*> & paramsInfo, void** dependencies, const Types &... params)
	{
		for (unsigned int i = 0; i < cacheData.size(); i++)
			if (cacheData[i]->equals(paramsInfo, dependencies, params...))
				return cacheData[i];
		return nullptr;
	}
}

#endif