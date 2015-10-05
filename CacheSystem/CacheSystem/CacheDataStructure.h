#ifndef _CACHE_DATA_STRUCTURE_H
#define _CACHE_DATA_STRUCTURE_H

#include <vector>
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
		std::vector<CacheData*> cacheData;

	public:
		/**
		finds and returns the CacheData object in which all the input parameters are equal to the corresponding input parameters passed as params
		*/
		template <class... Types> CacheData* getCacheData(const std::vector<ParameterInfo*> & paramsInfo,
			void** dependencies, const Types &... params);

		/**
		adds new cache data, the CacheData object must be created by the new operator and it will be deleted during deleting this object
		*/
		void addCacheData(CacheData* data);

		/**
		destroys all cache data added to this object
		*/
		~CacheDataStructure();
	};

	template <class... Types>
	CacheData* CacheDataStructure::getCacheData(const std::vector<ParameterInfo*> & paramsInfo, void** dependencies, const Types &... params)
	{
		for (unsigned int i = 0; i < cacheData.size(); i++)
			if (cacheData[i]->equals(paramsInfo, dependencies, params...))
				return cacheData[i];
		return nullptr;
	}
}

#endif