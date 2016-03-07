#include "CacheDataStructure.h"

namespace CacheSystem
{
	void CacheDataStructure::addCacheData(uint64_t hash, std::shared_ptr<CacheData> data)
	{
		if (cacheData.count(hash) == 0)
			cacheData[hash] = std::vector<std::shared_ptr<CacheData> >();
		cacheData[hash].push_back(data);

		/*if (collisions.count(hash) == 0)
			collisions[hash] = 0;
		collisions[hash]++;
		if (collisions[hash] > maxCollisions)
			maxCollisions = collisions[hash];*/
	}
}