#ifndef _CACHE_MANAGER_SOURCE_H
#define _CACHE_MANAGER_SOURCE_H

#include "CachedFunction.h"

/**
this class contains the static cache manager which is common for all caching filters
*/
class CacheManagerSource
{
public:
	/** required capacity of the cache in bytes (default is 100 MB)
	N.B. this value can be changed only before the first cached VTK object
	is instanced (e.g., in main), any later change is ignored. */
	static size_t CACHE_CAPACITY;

protected:
	static CacheSystem::CachedFunctionManager* cacheManager;
	static int cacheInstanceCounter;

	template <class FilterClass, class SuperClass>
	friend class CachingFilter;
};

#endif