#include "CacheDataStructure.h"

namespace CacheSystem
{
	void CacheDataStructure::addCacheData(std::shared_ptr<CacheData> data)
	{
		cacheData.push_back(data);
	}
}