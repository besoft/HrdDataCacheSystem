#ifndef _CACHED_FUNCTION_DECLARATION_H
#define _CACHED_FUNCTION_DECLARATION_H

#include "AbstractCachedFunction.h"

namespace CacheSystem
{
	/**
	manages the caching and contains all the cached data
	this class is for functions with a return value
	*/
	template <class ReturnType, class... ParamTypes>
	class CachedFunction : public AbstractCachedFunction<ReturnType, ParamTypes...>
	{
		friend class CachedFunctionManager;
	private:
		/**
		initializing constructor
		first parameter is the cache configuration object
		second parameter is the function for data generating
		*/
		CachedFunction(const CacheConfiguration & conf, ReturnType(*function)(ParamTypes...), CachedFunctionManager* manager)
			: AbstractCachedFunction(conf, function, manager) {}

	public:
		ReturnType call(ParamTypes... params);
	};

	/**
	manages the caching and contains all the cached data
	this specialization is for functions without a return value (void)
	*/
	template <class... ParamTypes>
	class CachedFunction<void, ParamTypes...> : public AbstractCachedFunction<void, ParamTypes...>
	{
		friend class CachedFunctionManager;
	private:
		/**
		initializing constructor
		first parameter is the cache configuration object
		second parameter is the function for data generating
		*/
		CachedFunction(const CacheConfiguration & conf, void(*function)(ParamTypes...), CachedFunctionManager* manager)
			: AbstractCachedFunction(conf, function, manager) {}

	public:
		void call(ParamTypes... params);
	};
}

#endif