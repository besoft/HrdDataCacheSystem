#ifndef _CACHE_MANAGER_SOURCE_H
#define _CACHE_MANAGER_SOURCE_H

#include "CachedFunction.h"

/**
this class contains the static cache manager which is common for all caching filters
*/
class CacheManagerSource
{
public:
	static CacheSystem::CachedFunctionManager* cacheManager;
	static int cacheInstanceCounter;
};

#endif