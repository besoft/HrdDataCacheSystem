#pragma once

#include <functional>

namespace CacheSystem
{
	template<class Type>
	struct DataManipulationFunctionsTypeTraits
	{
		using EqualFunctionSig = typename bool(const Type &, const Type &, void*);
		using InitFunctionSig = typename void(const Type &, Type*, void*);
		using OutputFunctionSig = typename void(const Type &, Type &, void*);
		using DestroyFunctionSig = typename void(Type &, void*);
		using ReturnFunctionSig = typename Type(const Type &, void*);
		using GetSizeFunctionSig = typename size_t(const Type &, void*);
		using HashFunctionSig = typename size_t(const Type &, void*);
		
		using EqualFunction = typename std::function<EqualFunctionSig>;
		using InitFunction = typename std::function<InitFunctionSig>;
		using OutputFunction = typename std::function<OutputFunctionSig>;
		using DestroyFunction = typename std::function<DestroyFunctionSig>;
		using ReturnFunction = typename std::function<ReturnFunctionSig>;
		using GetSizeFunction = typename std::function<GetSizeFunctionSig>;
		using HashFunction = typename std::function<HashFunctionSig>;	
	};
}