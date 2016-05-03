#ifndef _CACHE_MANAGER_CONFIGURATION_H
#define _CACHE_MANAGER_CONFIGURATION_H

#include <stdint.h>
#include <memory>
#include "DefaultCachePolicy.h"

namespace CacheSystem
{
	/**
	contains configuration for a cached function manager
	*/
	class CacheManagerConfiguration
	{
	private:
		/**
		capacity of the cache in bytes
		*/
		uint64_t cacheCapacity;

		/**
		pointer to the cache policy object
		*/
		std::shared_ptr<CachePolicy> policy;

		/**
		determines whether or not the manager should use the policy's cache miss event for all data objects after every cache hit 
		*/
		bool useCacheMissEvent;

	public:
		/**
		initializing constructor
		*/
		CacheManagerConfiguration() : cacheCapacity(1000000000), policy(std::shared_ptr<CachePolicy>(new CachePolicies::DefaultCachePolicy())),
			useCacheMissEvent(false) {}

		/**
		sets the cache manager's total capacity in bytes
		default is 1000000000
		*/
		void setCacheCapacity(uint64_t bytes) { cacheCapacity = bytes; }

		/**
		returns the cache manager's total capacity in bytes
		*/
		uint64_t getCacheCapacity() { return cacheCapacity; }

		/**
		returns the cache policy object
		*/
		std::shared_ptr<CachePolicy> getCachePolicy() const { return policy; }

		/**
		sets the flag that determines whether or not the manager should use the policy's cache miss event for all data objects after every cache hit
		true means it shloud, false means it should not
		default is false
		*/
		void setUseCacheMissEvent(bool useCacheMissEvent) { this->useCacheMissEvent = useCacheMissEvent; }

		/**
		returns the flag that determines whether or not the manager should use the policy's cache miss event for all data objects after every cache hit
		true means it shloud, false means it should not
		*/
		bool getUseCacheMissEvent() { return useCacheMissEvent; }

		/**
		sets the cache policy object
		the class representing the cache policy object must have the copy constructor correctly defined
		*/
		template <class PolicyType> void setCachePolicy(const PolicyType & policy);
	};

	template <class PolicyType>
	void CacheManagerConfiguration::setCachePolicy(const PolicyType & policy)
	{
		this->policy = std::shared_ptr<PolicyType>(new PolicyType(policy));
	}
}

#endif