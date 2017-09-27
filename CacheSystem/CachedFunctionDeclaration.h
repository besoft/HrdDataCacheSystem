#ifndef _CACHED_FUNCTION_DECLARATION_H
#define _CACHED_FUNCTION_DECLARATION_H

#include "AbstractCachedFunction.h"

namespace CacheSystem
{
	/**
	manages the caching and contains all the cached data
	this class is for functions with a return value
	*/
	template <class DependencyObj, class ReturnType, class... ParamTypes>
	class CachedFunctionWithDepObj : public AbstractCachedFunctionWithDepObj<DependencyObj, ReturnType, ParamTypes...>
	{
		friend class CachedFunctionManager;
	private:
		/**
		initializing constructor
		first parameter is the cache configuration object
		second parameter is the function for data generating
		*/		
		CachedFunctionWithDepObj(const CacheConfigurationWithDepObj<DependencyObj> & conf, std::function<ReturnType(ParamTypes...)>& function, CachedFunctionManager* manager)
			: AbstractCachedFunctionWithDepObj(conf, function, manager) {}

	public:
		ReturnType call(ParamTypes... params);
	};

	/**
	manages the caching and contains all the cached data
	this specialization is for functions without a return value (void)
	*/
	template <class DependencyObj, class... ParamTypes>
	class CachedFunctionWithDepObj<DependencyObj, void, ParamTypes...> : public AbstractCachedFunctionWithDepObj<DependencyObj, void, ParamTypes...>
	{
		friend class CachedFunctionManager;
	private:
		/**
		initializing constructor
		first parameter is the cache configuration object
		second parameter is the function for data generating
		*/		
		CachedFunctionWithDepObj(const CacheConfigurationWithDepObj<DependencyObj> & conf, 
			std::function<ReturnType(ParamTypes...)>& function, CachedFunctionManager* manager)
			: AbstractCachedFunctionWithDepObj(conf, function, manager) {}

	public:
		void call(ParamTypes... params);
	};

	/**
	alias that manages the caching and contains all the cached data using void* dependency object	
	*/
	template <class ReturnType, class... ParamTypes>
	using CachedFunction = typename CachedFunctionWithDepObj<void*, ReturnType, ParamTypes...>;	

	/**
	alias that manages the caching and contains all the cached data using no dependency object
	*/
	template <class ReturnType, class... ParamTypes>
	using CachedFunctionNoDepObj = typename CachedFunctionWithDepObj<NoDepObj, ReturnType, ParamTypes...>;	
}

#endif