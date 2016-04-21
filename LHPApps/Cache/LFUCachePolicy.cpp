#include "LFUCachePolicy.h"

namespace CacheSystem
{
	namespace CachePolicies
	{
		void LFUCachePolicy::dataCreationEvent(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime)
		{
			CacheData* data = (CacheData*)dataId;
			data->setUserData(new double(0));
		}

		void LFUCachePolicy::cacheHitEvent(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime)
		{
			CacheData* data = (CacheData*)dataId;
			(*(double*)data->getUserData())++;
		}

		double LFUCachePolicy::getPriority(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime)
		{
			CacheData* data = (CacheData*)dataId;
			return *(double*)data->getUserData();
		}

		void LFUCachePolicy::dataEvictionEvent(uint64_t dataId)
		{
			CacheData* data = (CacheData*)dataId;
			delete (double*)data->getUserData();
		}
	}
}