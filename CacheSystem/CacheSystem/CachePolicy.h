#ifndef _CACHE_POLICY_H
#define _CACHE_POLICY_H

#include <stdint.h>
#include "CacheData.h"

namespace CacheSystem
{
	class CachePolicy
	{
	protected:
		/**
		this method is called right after the data is stored into cache
		*/
		virtual void dataCreationEvent(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime) = 0;

		/**
		this mathod is called every time the data is accessed, including the time the data is stored into the cache, in this case...
		...it is called right after the dataCreationEvent method
		*/
		virtual void cacheHitEvent(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime) = 0;

		/**
		this method is called when any data is supposed to be evicted from cache and priority calculation is needed
		it should return the data's current priority
		*/
		virtual double getPriority(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime) = 0;

		/**
		this method is called after the data has been evicted from cache and data with the given dataId no longer exists
		*/
		virtual void dataEvictionEvent(uint64_t dataId) = 0;

	public:
		void createData(CacheData* data);
		void hitData(CacheData* data);
		double getDataPriority(CacheData* data);
		void evictData(CacheData* data);
	};
}

#endif