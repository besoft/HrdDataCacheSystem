#include "Cache.h"

CacheSystem::CachedFunctionManager* Cache::cacheManager = nullptr;

void Cache::initCache()
{
	CacheSystem::CacheManagerConfiguration managerConf;
	//managerConf.setCacheCapacity(10000000);
	/*CacheSystem::CachePolicies::DefaultCachePolicy policy;
	managerConf.setCachePolicy(policy);*/
	cacheManager = new CacheSystem::CachedFunctionManager(managerConf);
}

void Cache::destroyCache()
{
	delete cacheManager;
}

CacheSystem::CachedFunctionManager* Cache::getCacheManager()
{
	return cacheManager;
}