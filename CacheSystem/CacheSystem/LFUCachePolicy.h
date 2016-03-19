#ifndef _LFU_CACHE_POLICY_H
#define _LFU_CACHE_POLICY_H

#include "CachePolicy.h"

namespace CacheSystem
{
	namespace CachePolicies
	{
		class LFUCachePolicy : public CachePolicy
		{
		protected:
			/**
			this method is called right after the data is stored into cache
			*/
			void dataCreationEvent(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime);

			/**
			this mathod is called every time the data is accessed, including the time the data is stored into the cache, in this case...
			...it is called right after the dataCreationEvent method
			*/
			void cacheHitEvent(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime);

			/**
			when a cache hit occurs this method is called for all the data objects in cache except the that was hit
			to use this method the useCacheMissEvent on CacheManagerConfiguration must be enabled
			*/
			void cacheMissEvent(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime) {}

			/**
			this method is called when any data is supposed to be evicted from cache and priority calculation is needed
			it should return the data's current priority
			*/
			double getPriority(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime);

			/**
			this method is called right before the data has been evicted from cache
			*/
			void dataEvictionEvent(uint64_t dataId);
		};
	}
}

#endif