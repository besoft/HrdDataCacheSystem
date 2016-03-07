#ifndef _CACHE_DATA_STRUCTURE_H
#define _CACHE_DATA_STRUCTURE_H

#include <vector>
#include <memory>
#include <unordered_map>
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
		map of vectors of the data
		*/
		std::unordered_map<uint64_t, std::vector<std::shared_ptr<CacheData> > > cacheData;
		//std::unordered_map<uint64_t, int> collisions;

	public:
		//int maxCollisions = 0;
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
		if (cacheData.count(hash) == 0)
			return nullptr;
		std::vector<std::shared_ptr<CacheData> > & dataVector = cacheData[hash];
		for (unsigned int i = 0; i < dataVector.size(); i++)
			if (dataVector[i]->equals(paramsInfo, dependencyObject, params...))
				return dataVector[i];
		return nullptr;
	}
}

#endif