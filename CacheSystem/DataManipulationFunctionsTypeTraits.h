#pragma once

#include <functional>

namespace CacheSystem
{
	/** Defines no dependency object */
	struct NoDepObj {};

	template<class Type, class DependencyObj>
	struct DataManipulationFunctionsTypeTraits
	{
		using EqualFunctionSig = typename bool(const Type &, const Type &, DependencyObj);
		using InitFunctionSig = typename void(const Type &, Type*, DependencyObj);
		using OutputFunctionSig = typename void(const Type &, Type &, DependencyObj);
		using DestroyFunctionSig = typename void(Type &, DependencyObj);
		using ReturnFunctionSig = typename Type(const Type &, DependencyObj);
		using GetSizeFunctionSig = typename size_t(const Type &, DependencyObj);
		using HashFunctionSig = typename size_t(const Type &, DependencyObj);
		
		using EqualFunction = typename std::function<EqualFunctionSig>;
		using InitFunction = typename std::function<InitFunctionSig>;
		using OutputFunction = typename std::function<OutputFunctionSig>;
		using DestroyFunction = typename std::function<DestroyFunctionSig>;
		using ReturnFunction = typename std::function<ReturnFunctionSig>;
		using GetSizeFunction = typename std::function<GetSizeFunctionSig>;
		using HashFunction = typename std::function<HashFunctionSig>;	
	};

	template<class Type>
	struct DataManipulationFunctionsTypeTraits<Type, NoDepObj>
	{
		using EqualFunctionSig = typename bool(const Type &, const Type &);
		using InitFunctionSig = typename void(const Type &, Type*);
		using OutputFunctionSig = typename void(const Type &, Type &);
		using DestroyFunctionSig = typename void(Type &);
		using ReturnFunctionSig = typename Type(const Type &);
		using GetSizeFunctionSig = typename size_t(const Type &);
		using HashFunctionSig = typename size_t(const Type &);

		using EqualFunction = typename std::function<EqualFunctionSig>;
		using InitFunction = typename std::function<InitFunctionSig>;
		using OutputFunction = typename std::function<OutputFunctionSig>;
		using DestroyFunction = typename std::function<DestroyFunctionSig>;
		using ReturnFunction = typename std::function<ReturnFunctionSig>;
		using GetSizeFunction = typename std::function<GetSizeFunctionSig>;
		using HashFunction = typename std::function<HashFunctionSig>;
	};
	
	/**
	This helper allows calling data manipulation functions
	with or without dependency object depending on the type of DepObj.
	F is the function to be called, RetType(Args...) is the signature of
	the same function stripped of the dependency object
	*/
	template <typename DepObj>
	class DMFuncInvoker
	{
	protected:
		DepObj _dependecyObj;
	public:
		DMFuncInvoker(DepObj dependecyObj) : _dependecyObj(dependecyObj)
		{

		}

		template <typename F, typename... Args>
		auto operator()(F func, Args...args) ->
			decltype(func(args..., _dependecyObj))
		{
			return func(args..., _dependecyObj);
		}
	};

	/** specialization for no dependency objects */
	template <>
	class DMFuncInvoker<NoDepObj>
	{
	protected:
		NoDepObj _dependecyObj;
	public:
		DMFuncInvoker(NoDepObj dependecyObj) : _dependecyObj(dependecyObj)
		{

		}
	public:
		template <typename F, typename... Args>
		auto operator()(F func, Args...args) ->
			decltype(func(args...))
		{
			return func(args...);
		}
	};
}