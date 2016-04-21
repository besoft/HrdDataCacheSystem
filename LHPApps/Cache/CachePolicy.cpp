#include "CachePolicy.h"

namespace CacheSystem
{
	void CachePolicy::createData(CacheData* data)
	{
		dataCreationEvent((uint64_t)data, data->getSize(), data->getCreationTime());
	}

	void CachePolicy::hitData(CacheData* data)
	{
		cacheHitEvent((uint64_t)data, data->getSize(), data->getCreationTime());
	}

	void CachePolicy::missData(CacheData* data)
	{
		cacheMissEvent((uint64_t)data, data->getSize(), data->getCreationTime());
	}

	double CachePolicy::getDataPriority(CacheData* data)
	{
		return getPriority((uint64_t)data, data->getSize(), data->getCreationTime());
	}

	void CachePolicy::evictData(CacheData* data)
	{
		dataEvictionEvent((uint64_t)data);
	}
}