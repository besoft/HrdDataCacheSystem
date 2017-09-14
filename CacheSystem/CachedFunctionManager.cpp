#include "CachedFunctionManager.h"

namespace CacheSystem
{
	CachedFunctionManager::~CachedFunctionManager()
	{
		for (size_t i = 0; i < cachedFunctions.size(); i++)
		{
			cachedFunctions[i]->resetDataIterator();
			CacheData* data;
			while ((data = cachedFunctions[i]->getNextData()) != nullptr)
				policy->evictData(data);
			delete cachedFunctions[i];
		}
	}

	CacheData* CachedFunctionManager::getEvictionCandidate()
	{
		double minPriority = DBL_MAX;
		CacheData* candidate = nullptr;
		for (unsigned int i = 0; i < cachedFunctions.size(); i++)
		{
			cachedFunctions[i]->resetDataIterator();
			CacheData* data;
			while ((data = cachedFunctions[i]->getNextData()) != nullptr)
			{
				double priority = policy->getDataPriority(data);
				if (priority < minPriority)
				{
					minPriority = priority;
					candidate = data;
				}
			}
		}
		return candidate;
	}

	void CachedFunctionManager::makeSpace(size_t bytes)
	{
		while (conf.getCacheCapacity() - spaceTaken < bytes)
		{
			CacheData* evictionCandidate = getEvictionCandidate();
			size_t candidateSize = evictionCandidate->getSize();
			policy->evictData(evictionCandidate);
			evictionCandidate->getCachedFunction()->removeData(evictionCandidate);
			spaceTaken -= candidateSize;
		}
	}

	void CachedFunctionManager::performCacheMissEvents()
	{
		if (conf.getUseCacheMissEvent())
		{
			for (size_t i = 0; i < cachedFunctions.size(); i++)
			{
				cachedFunctions[i]->resetDataIterator();
				CacheData* data;
				while ((data = cachedFunctions[i]->getNextData()) != nullptr)
					policy->missData(data);
			}
		}
	}
}