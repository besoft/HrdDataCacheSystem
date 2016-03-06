#include "CacheDataStructure.h"

namespace CacheSystem
{
	void CacheDataStructure::addCacheData(uint64_t hash, std::shared_ptr<CacheData> data)
	{
		cacheData.push_back(data);
	}
}