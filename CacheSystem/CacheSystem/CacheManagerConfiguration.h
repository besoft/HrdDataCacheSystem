#ifndef _CACHE_MANAGER_CONFIGURATION_H
#define _CACHE_MANAGER_CONFIGURATION_H

#include <stdint.h>

namespace CacheSystem
{
	class CacheManagerConfiguration
	{
	private:
		uint64_t cacheCapacity;

	public:
		CacheManagerConfiguration() : cacheCapacity(1000000000) {}
		void setCacheCapacity(uint64_t bytes) { cacheCapacity = bytes; }
		uint64_t getCacheCapacity() { return cacheCapacity; }
	};
}

#endif