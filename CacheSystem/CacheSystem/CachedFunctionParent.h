#ifndef _CACHED_FUNCTION_PARENT_H
#define _CACHED_FUNCTION_PARENT_H

namespace CacheSystem
{
	/**
	only defined to make a common perent for all CachedFunction classes
	*/
	class CachedFunctionParent
	{
	public:
		/**
		only defined to make the destructor virtual
		*/
		virtual ~CachedFunctionParent() {}
	};
}

#endif