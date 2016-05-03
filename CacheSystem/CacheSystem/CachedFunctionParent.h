#ifndef _CACHED_FUNCTION_PARENT_H
#define _CACHED_FUNCTION_PARENT_H

namespace CacheSystem
{
	class CachedFunctionManager;
	class CacheData;
	/**
	only defined to make a common perent for all CachedFunction classes
	*/
	class CachedFunctionParent
	{
	protected:
		/**
		pointer to the corresponding CachedFunctionManager
		*/
		CachedFunctionManager* manager;

	public:
		/**
		initializing constructor
		parameter is the manager by which the cache object was created
		*/
		CachedFunctionParent(CachedFunctionManager* manager) : manager(manager) {}
		
		/**
		only defined to make the destructor virtual
		*/
		virtual ~CachedFunctionParent() {}
		
		/**
		resets the data structure's iterator to the beginning
		*/
		virtual void resetDataIterator() = 0;
		
		/**
		returns the next data from the data structure using the data structure's iterator
		*/
		virtual CacheData* getNextData() = 0;

		/**
		removes given data object from the data structure
		*/
		virtual void removeData(CacheData* data) = 0;

		/**
		returns the manager by which the cache object was created
		*/
		CachedFunctionManager* getManager() { return manager; }
	};
}

#endif