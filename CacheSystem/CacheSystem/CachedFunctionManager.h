#ifndef _CACHED_FUNCTION_MANAGER_H
#define _CACHED_FUNCTION_MANAGER_H

#include <vector>
#include "CachedFunctionDeclaration.h"

namespace CacheSystem
{
	class CachedFunctionManager
	{
	private:
		std::vector<CachedFunctionParent*> cachedFunctions;

	public:
		~CachedFunctionManager();

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