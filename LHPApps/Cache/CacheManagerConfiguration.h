#ifndef _CACHE_MANAGER_CONFIGURATION_H
#define _CACHE_MANAGER_CONFIGURATION_H

#include <stdint.h>
#include <memory>
#include "DefaultCachePolicy.h"

namespace CacheSystem
{
	class CacheManagerConfiguration
	{
	private:
		uint64_t cacheCapacity;
		std::shared_ptr<CachePolicy> policy;
		bool useCacheMissEvent;

	public:
		CacheManagerConfiguration() : cacheCapacity(1000000000), policy(std::shared_ptr<CachePolicy>(new CachePolicies::DefaultCachePolicy())),
			useCacheMissEvent(false) {}
		void setCacheCapacity(uint64_t bytes) { cacheCapacity = bytes; }
		uint64_t getCacheCapacity() { return cacheCapacity; }
		std::shared_ptr<CachePolicy> getCachePolicy() const { return policy; }
		void setUseCacheMissEvent(bool useCacheMissEvent) { this->useCacheMissEvent = useCacheMissEvent; }
		bool getUseCacheMissEvent() { return useCacheMissEvent; }
		template <class PolicyType> void setCachePolicy(const PolicyType & policy);
	};

	template <class PolicyType>
	void CacheManagerConfiguration::setCachePolicy(const PolicyType & policy)
	{
		this->policy = std::shared_ptr<PolicyType>(new PolicyType(policy));
	}
}

#endif