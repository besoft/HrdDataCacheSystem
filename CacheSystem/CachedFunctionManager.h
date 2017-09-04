#ifndef _CACHED_FUNCTION_MANAGER_H
#define _CACHED_FUNCTION_MANAGER_H

#include <vector>
#include <mutex>
#include "CachedFunctionDeclaration.h"
#include "CacheManagerConfiguration.h"
#include "CachePolicy.h"
#include "DefaultCachePolicy.h"

namespace CacheSystem
{
	/**
	factory class for the cache objects
	*/
	class CachedFunctionManager
	{
	private:
		/**
		vector of cached objects created by this manager
		*/
		std::vector<CachedFunctionParent*> cachedFunctions;

		/**
		manager configuration object
		*/
		CacheManagerConfiguration conf;

		/**
		space taken by all data in all cache objects created by this manager in bytes
		*/
		uint64_t spaceTaken;

		/**
		pointer to the cache policy object
		*/
		std::shared_ptr<CachePolicy> policy;

		/**
		finds and returns the best candidate for eviction using the cache policy
		*/
		CacheData* getEvictionCandidate();

		/**
		mutex for thread safe CachedFunction calls
		*/
		std::recursive_mutex mutex;

	public:
		/**
		instances of this class can be used to lock a given CachedFunctionManager
		*/
		class CachedFunctionCallLocker
		{
		private:
			CachedFunctionManager* manager;

		public:
			CachedFunctionCallLocker(CachedFunctionManager* manager) : manager(manager) { manager->lockCachedFunctionCalls(); }
			~CachedFunctionCallLocker() { manager->unlockCachedFunctionCalls(); }
		};

		/**
		initializing constructor
		*/
		CachedFunctionManager() : conf(CacheManagerConfiguration()), spaceTaken(0), policy(conf.getCachePolicy()) {}

		/**
		initializing constructor
		parameter is the cache manager configuration object
		*/
		CachedFunctionManager(const CacheManagerConfiguration & conf) : conf(conf), spaceTaken(0), policy(conf.getCachePolicy()) {}

		/**
		correctly destroys the manager and all cache object created by this manager
		*/
		~CachedFunctionManager();

		/**
		returns the space taken by all data in all cache objects created by this manager in bytes
		*/
		uint64_t getSpaceTaken() { return spaceTaken; }

		/**
		checks if there is enaugh space to store a data object of a given size, if so, it returns true, otherwise it returns false
		*/
		bool checkSpace(uint64_t bytes) { return (spaceTaken + bytes) <= conf.getCacheCapacity(); }

		/**
		increases taken space by a given value
		*/
		void takeSpace(uint64_t bytes) { spaceTaken += bytes; }

		/**
		returns this manager's total capacity in bytes
		*/
		uint64_t getCacheCapacity() { return conf.getCacheCapacity(); }

		/**
		returns the cache policy object
		*/
		CachePolicy* getCachePolicy() { return policy.get(); }

		/**
		evicts as many data objects as needed using the cache policy to make space of a given value in bytes
		*/
		void makeSpace(uint64_t bytes);

		/**
		iterates through all data objects in all cache objects and calls cacheMissEvent on the cache policy for all of them
		*/
		void performCacheMissEvents();

		/**
		after this method is called no CachedFuncion call can be made from a different thread
		*/
		void lockCachedFunctionCalls() { mutex.lock(); }

		/**
		this method must be called after calling the lockCachedFunctionCalls method
		after this method is called CachedFunction calls can be made from different threads again
		*/
		void unlockCachedFunctionCalls() { mutex.unlock(); }

		/**
		creates, registers and returns a new cache object
		first parameter is the cache configuration object
		second parameter is the function for generating data
		*/
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