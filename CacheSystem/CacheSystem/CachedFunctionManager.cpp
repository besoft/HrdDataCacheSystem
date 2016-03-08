#include "CachedFunctionManager.h"

namespace CacheSystem
{
	CachedFunctionManager::~CachedFunctionManager()
	{
		for (unsigned int i = 0; i < cachedFunctions.size(); i++)
			delete cachedFunctions[i];
	}
}