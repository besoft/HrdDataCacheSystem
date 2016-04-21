#ifndef _CACHED_FUNCTION_MANAGER_H
#define _CACHED_FUNCTION_MANAGER_H

#include <vector>
#include "CachedFunctionDeclaration.h"
#include "CacheManagerConfiguration.h"
#include "CachePolicy.h"
#include "DefaultCachePolicy.h"

namespace CacheSystem
{
	class CachedFunctionManager
	{
	private:
		std::vector<CachedFunctionParent*> cachedFunctions;
		CacheManagerConfiguration conf;
		uint64_t spaceTaken;
		std::shared_ptr<CachePolicy> policy;

		CacheData* getEvictionCandidate();

	public:
		CachedFunctionManager(const CacheManagerConfiguration & conf) : conf(conf), spaceTaken(0), policy(conf.getCachePolicy()) {}
		~CachedFunctionManager();
		uint64_t getSpaceTaken() { return spaceTaken; }
		bool checkSpace(uint64_t bytes) { return (spaceTaken + bytes) <= conf.getCacheCapacity(); }
		void takeSpace(uint64_t bytes) { spaceTaken += bytes; }
		uint64_t getCacheCapacity() { return conf.getCacheCapacity(); }
		CachePolicy* getCachePolicy() { return policy.get(); }
		void makeSpace(uint64_t bytes);
		void performCacheMissEvents();

		template <class ReturnType, class... ParamTypes>
		CachedFunction<ReturnType, ParamTypes...>* createCachedFunction(CacheConfiguration conf, ReturnType(*function)(ParamTypes...));
	};

	template <class ReturnType, class... ParamTypes>
	CachedFunction<ReturnType, ParamTypes...>* CachedFunctionManager::createCachedFunction(CacheConfiguration conf, ReturnType(*function)(ParamTypes...))
	{
		CachedFunction<ReturnType, ParamTypes...>* cachedFunction = new CachedFunction<ReturnType, ParamTypes...>(conf, function, this);
		cachedFunctions.push_back(cachedFunction);
		return cachedFunction;
	}
}

#endif