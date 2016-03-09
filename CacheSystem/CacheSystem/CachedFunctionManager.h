#ifndef _CACHED_FUNCTION_MANAGER_H
#define _CACHED_FUNCTION_MANAGER_H

#include <vector>
#include "CachedFunctionDeclaration.h"
#include "CacheManagerConfiguration.h"

namespace CacheSystem
{
	class CachedFunctionManager
	{
	private:
		std::vector<CachedFunctionParent*> cachedFunctions;
		CacheManagerConfiguration conf;
		uint64_t spaceTaken;

	public:
		CachedFunctionManager(const CacheManagerConfiguration & conf) : conf(conf), spaceTaken(0) {}
		~CachedFunctionManager();
		uint64_t getSpaceTaken() { return spaceTaken; }
		bool checkSpace(uint64_t bytes) { return (spaceTaken + bytes) <= conf.getCacheCapacity(); }
		void takeSpace(uint64_t bytes) { spaceTaken += bytes; }

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