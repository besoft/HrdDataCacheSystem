#ifndef _CACHE_H_
#define _CACHE_H_

#include "CachedFunction.h"

class Cache
{
private:
	static CacheSystem::CachedFunctionManager* cacheManager;

public:
	static void initCache();
	static void destroyCache();
	static CacheSystem::CachedFunctionManager* getCacheManager();
};

#endif