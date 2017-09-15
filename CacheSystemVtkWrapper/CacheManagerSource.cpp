#include "CacheManagerSource.h"

size_t CacheManagerSource::CACHE_CAPACITY = 100000000;	//100 MB

int CacheManagerSource::cacheInstanceCounter = 0;

CacheSystem::CachedFunctionManager* CacheManagerSource::cacheManager = nullptr;