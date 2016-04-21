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
		CachedFunctionParent(CachedFunctionManager* manager) : manager(manager) {}
		/**
		only defined to make the destructor virtual
		*/
		virtual ~CachedFunctionParent() {}

		virtual void resetDataIterator() = 0;

		virtual CacheData* getNextData() = 0;

		virtual void removeData(CacheData* data) = 0;

		CachedFunctionManager* getManager() { return manager; }
	};
}

#endif