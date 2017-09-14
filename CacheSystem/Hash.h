#pragma once

#include <functional>

namespace CacheSystem
{
	//hashing based on http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3876.pdf
	
	/**
	Modified the seed hash value by combining it with the passed hash value.
	It is assumed that both hash values already contain decent hash.
	*/
	template<typename ... Types>
	inline void hash_combine_hvs(std::size_t& seed, std::size_t firsthv)
	{
		seed ^= firsthv + 0x9e3779b9
			+ (seed << 6) + (seed >> 2);
	}


	/**
	Modified the seed hash value by combining them with the passed hash values.
	It is assumed that all the hash values contain decent hash.
	*/
	template<typename ... Types>
	void hash_combine_hvs(std::size_t& seed, std::size_t firsthv, Types ... hvs)
	{
		hash_combine_hvs(seed, firsthv);
		hash_combine_hvs(seed, hvs ...);
	}
	
	/**
	Combines all the passed hash values into a single hash value.
	Providing that all the hash values contain decent hash, 
	the result is also a decent hash.
	*/
	template <typename... Types>
	std::size_t hash_combine_hvs(Types... args)
	{
		std::size_t seed = 0;
		hash_combine_hvs(seed, args...);
		return seed;
	}
	
	/**
	Modified the seed hash value by combining it with a hash value of val argument.
	The hash value is computed using a standard std::hash<T> hasher 
	(or its custom specialization).

	Example:

	size_t seed = 0;
	hash_combine(seed, name);
	hash_combine(seed, age);
	hash_combine(seed, sex, education);	//version with a variable number of arguments
	*/
	template <typename T>
	inline void hash_combine(std::size_t& seed, const T& val)
	{
		hash_combine_hvs(seed, std::hash<T>()(val));
	}

	// generic functions to create a hash value using a seed
	template <typename T, typename... Types>
	void hash_combine(std::size_t& seed,
		const T& val, const Types&... args)
	{
		hash_combine(seed, val);
		hash_combine(seed, args...);
	}

	// optional auxiliary generic functions to support hash_value() without arguments
	inline void hash_combine(std::size_t& seed)
	{
	}
		
	/**
	Computes a hash value out of a heterogeneous list of arguments.
	The function computes a hash value for every of the passed argument
	using standard std::hash<T> hasher and then combines these values
	together to get the final hash value.
	
	The function is typically used when a hash value of a structure/class
	is needed and hash values for its member attributes can be obtained
	by using a standard hasher (or a custom hasher compatible with it).

	Example:

	class Person {
		std::string name;
		int age;
		bool sex;

		size_t GetHashCode() {
			return hash_value(name, age, sex);
		}
	};
	*/
	template <typename... Types>
	std::size_t hash_value(const Types&... args)
	{
		std::size_t seed = 0;
		hash_combine(seed, args...);
		return seed;
	}

}