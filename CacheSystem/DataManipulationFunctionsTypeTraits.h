#pragma once

#include <functional>

namespace CacheSystem
{
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
}