#include "CacheDataStructure.h"

namespace CacheSystem
{
	CacheDataStructure::~CacheDataStructure()
	{
		for (unsigned int i = 0; i < cacheData.size(); i++)
			delete cacheData[i];
	}

	void CacheDataStructure::addCacheData(CacheData* data)
	{
		cacheData.push_back(data);
	}
}