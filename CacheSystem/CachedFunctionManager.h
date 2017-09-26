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
		size_t spaceTaken;

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
		size_t getSpaceTaken() { return spaceTaken; }

		/**
		checks if there is enaugh space to store a data object of a given size, if so, it returns true, otherwise it returns false
		*/
		bool checkSpace(size_t bytes) { return (spaceTaken + bytes) <= conf.getCacheCapacity(); }

		/**
		increases taken space by a given value
		*/
		void takeSpace(size_t bytes) { spaceTaken += bytes; }

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
		void makeSpace(size_t bytes);

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

	private:
		
		template<typename T> struct remove_class { };
		template<typename C, typename R, typename... A>
		struct remove_class<R(C::*)(A...)> { using type = R(A...); };
		template<typename C, typename R, typename... A>
		struct remove_class<R(C::*)(A...) const> { using type = R(A...); };
		template<typename C, typename R, typename... A>
		struct remove_class<R(C::*)(A...) volatile> { using type = R(A...); };
		template<typename C, typename R, typename... A>
		struct remove_class<R(C::*)(A...) const volatile> { using type = R(A...); };

		/** enables conversion of functor F (or lambda expression) into std::function */
		template<typename F>
		using make_function_type = std::function<
			typename remove_class<
			decltype(&std::remove_reference<F>::type::operator())
			>::type
		>;

	public:
		/**
		creates, registers and returns a new cache object
		the first parameter is the cache configuration object
		the second parameter is std::function object that generates the data
		it may be encapsulated function, bound method, functor, lambda expression, ...		
		N.B. methods are not supported directly by std::function but must be either
		bound first using std::bind or std::mem_f or enclosed in a lambda expression.
		Lambda expressions are preferable.
		Example: auto cf = createCachedFunction(conf, std::function<void(int, int, void*)>
				(std::bind(&A::virtualFce, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

				auto cf = createCachedFunction(conf, //lambda expression can be automatically converted to std::function
					[=](int p1, int p2, void* p3) { return this->virtualFce(p1, p2, p3); }
					);

		*/
		template <class ReturnType, class... ParamTypes>
		CachedFunction<ReturnType, ParamTypes...>* createCachedFunction(const CacheConfiguration& conf, std::function<ReturnType(ParamTypes...)>& function);

		/**
		creates, registers and returns a new cache object
		the first parameter is the cache configuration object
		the second parameter is the static function (e.g., static void Fce(int, void*)) that generates the data
		Example: auto cf = createCachedFunction(conf, Fce);
		N.B. for non-static methods, see the createCachedFunction taking std::function
		*/
		template <class ReturnType, class... ParamTypes>
		CachedFunction<ReturnType, ParamTypes...>* createCachedFunction(const CacheConfiguration& conf, ReturnType(*function)(ParamTypes...))
		{
			return createCachedFunction(conf, std::function<ReturnType(ParamTypes...)>(function));
		}

		/**
		creates, registers and returns a new cache object
		the first parameter is the cache configuration object
		the second parameter is the functor or lambda expression that generates the data
		N.B. lambda expressions are convenient way to support methods (including virtual one)
		Example: auto cf = createCachedFunction(conf, [](int par1, void*) -> void { this->VirtMethod(par1); });
		*/
		template <class FunctorType>
		auto createCachedFunction(const CacheConfiguration& conf, FunctorType& functor)
			-> decltype(createCachedFunction(conf, make_function_type<FunctorType>(functor)))
		{
			return createCachedFunction(conf, make_function_type<FunctorType>(functor));
		}
	};

	template <class ReturnType, class... ParamTypes>
	CachedFunction<ReturnType, ParamTypes...>* CachedFunctionManager::createCachedFunction(const CacheConfiguration& conf, std::function<ReturnType(ParamTypes...)>& function)
	{
		CachedFunction<ReturnType, ParamTypes...>* cachedFunction = new CachedFunction<ReturnType, ParamTypes...>(conf, function, this);
		cachedFunctions.push_back(cachedFunction);
		return cachedFunction;
	}
}

#endif