#include "CacheDataStructure.h"

namespace CacheSystem
{
	void CacheDataStructure::addCacheData(uint64_t hash, std::shared_ptr<CacheData> data)
	{
		if (cacheData.count(hash) == 0)
			cacheData[hash] = std::vector<std::shared_ptr<CacheData> >();
		cacheData[hash].push_back(data);

		/*if (collisions.count(hash) == 0)
			collisions[hash] = 0;
		collisions[hash]++;
		if (collisions[hash] > maxCollisions)
			maxCollisions = collisions[hash];*/
	}

	void CacheDataStructure::resetDataIterator()
	{
		mapIterator = cacheData.begin();
		listIndex = 0;
	}

	CacheData* CacheDataStructure::getNextData()
	{
		if (mapIterator == cacheData.end())
			return nullptr;
		std::vector<std::shared_ptr<CacheData> >* list = &mapIterator->second;
		CacheData* data = (*list)[listIndex].get();
		listIndex++;
		if (listIndex >= list->size())
		{
			mapIterator++;
			listIndex = 0;
		}
		return data;
	}

	void CacheDataStructure::removeData(CacheData* data)
	{
		uint64_t hash = data->getHash();
		if (cacheData.count(hash) > 0)
		{
			std::vector<std::shared_ptr<CacheData> > & vector = cacheData[hash];
			for (unsigned int i = 0; i < vector.size(); i++)
				if (vector[i].get() == data)
				{
					vector.erase(vector.begin() + i);
					break;
				}
			if (vector.size() == 0)
				cacheData.erase(hash);
		}
	}
}