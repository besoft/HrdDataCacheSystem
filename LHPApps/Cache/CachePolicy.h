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
		when a cache hit occurs this method is called for all the data objects in cache except the that was hit
		to use this method the useCacheMissEvent on CacheManagerConfiguration must be enabled
		*/
		virtual void cacheMissEvent(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime) = 0;

		/**
		this method is called when any data is supposed to be evicted from cache and priority calculation is needed
		it should return the data's current priority
		*/
		virtual double getPriority(uint64_t dataId, uint64_t dataSize, int64_t dataCreationTime) = 0;

		/**
		this method is called right before the data has been evicted from cache
		*/
		virtual void dataEvictionEvent(uint64_t dataId) = 0;

	public:
		void createData(CacheData* data);
		void hitData(CacheData* data);
		void missData(CacheData* data);
		double getDataPriority(CacheData* data);
		void evictData(CacheData* data);

		/**
		defined just to make the destructor virtual
		*/
		virtual ~CachePolicy() {}
	};
}

#endif