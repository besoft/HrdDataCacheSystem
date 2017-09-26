#include <iostream>
#include <vector>
#include "CachedFunction.h"
using namespace CacheSystem;

/**
this is a "slow" function that should be cached
input:	an array of integers (1st parameter)
		its size (in number of elements) (2nd parameter)
output: an array of integers encapsulated in std::vector with values equal to 10 times of the values in the input array (return)
		sum of the output array (3rd parameter)
*/
std::vector<int> function(/*IN*/ int* const & inputArray, /*IN*/ int size, /*OUT*/ int* sum)

{
	std::cout << "CALLING" << std::endl; //display CALLING, whenever this function is called
	std::vector<int> ret;
	*sum = 0;
	for (int i = 0; i < size; i++)
	{
		ret.push_back(10 * inputArray[i]);
		*sum += inputArray[i];
	}
	return ret;
}

/**
Helper structure that allows sharing the values of parameters
*/
struct DependencyObject
{
	int* array1;
	int* array2;
	int** destinationPtr;
};

/**
equalFunction for the 1st parameter (an array of integers)
this function saves the pointers to the arrays and returns true
because the sizes of both arrays are unknown to this function
*/
bool param0EqualFunction(int* const & val1, int* const & val2, DependencyObject* obj)
{	
	obj->array1 = val1;
	obj->array2 = val2;
	return true;
}

/**
equalFunction for the 2nd parameter (size of the array)
when this function is called, we know the pointers to both arrays (we
stored them into a dependency object when param0EqualFunction was called)
and we also know their sizes, so we can finally compare the values of arrays
*/
bool param1EqualFunction(int const & size1, int const & size2, DependencyObject* obj)
{
	if (size1 != size2)
		return false;

	for (int i = 0; i < size1; i++)
		if (obj->array1[i] != obj->array2[i])
			return false;
	return true;
}

/**
initFunction for the 1st parameter (an array of integers)
similarly to param0EqualFunction, the size of the array is unknown
until param1InitFunction is called, so this function just 
stores the pointers to the source and destination array to the dependency object
*/
void param0InitFunction(int* const & source, int** destination, DependencyObject* obj)
{	
	obj->array1 = source;
	obj->destinationPtr = destination;
}

/**
initFunction for the 2nd parameter (size of the array)
performs the duplication of the source array whose pointer was stored
in the param0InitFunction call into the dependency object
*/
void param1InitFunction(int const & source, int* destination, DependencyObject* obj)
{	
	*destination = source;	//copy size

	*obj->destinationPtr = new int[source];
	for (int i = 0; i < source; i++)
		(*obj->destinationPtr)[i] = obj->array1[i];
}

/**
initFunction for the 3rd parameter (out: sum of the output array)
*/
void param2InitFunction(int* const & source, int** destination, DependencyObject*)

{
	*destination = new int(*source);
}

/**
initFunction for the return value
initializes std::vector in the cache using placement new operator and copies the values
*/
void returnInitFunction(const std::vector<int> & source, std::vector<int>* destination, DependencyObject*)
{
	new(destination)std::vector<int>();
	for (unsigned int i = 0; i < source.size(); i++)
		destination->push_back(source[i]);
}

/**
outputFunction for the 3rd parameter (out: sum of the output array)
copies the value from the cache to destination
*/
void param2OutputFunction(int* const & source, int* & destination, DependencyObject*)
{
	*destination = *source;
}

/**
destroyFunction for the 1st parameter (an array of integers)
*/
void param0DestroyFunction(int* & value, DependencyObject*)
{
	delete[] value;
}

/**
destroyFunction for the 2nd parameter (size of the array)
*/
void param1DestroyFunction(int &, DependencyObject*)
{ 
	/* nothing to do */ 
}

/**
destroyFunction for the 3rd parameter (out: sum of the output array)
*/
void param2DestroyFunction(int* & value, DependencyObject*)
{
	delete value;
}

/**
destroyFunction for the return value
calls destructor upon the vector
*/
void returnDestroyFunction(std::vector<int> & value, DependencyObject*)
{
	value.~vector<int>();
}

/**
hashFunction for the 1st parameter (an array of integers)
stores the pointer to the array into the dependency object and returns 0
*/
size_t param0HashFunction(int* const & value, DependencyObject* obj)
{	
	obj->array1 = value;
	return 0;
}

/**
hashFunction for the 2nd parameter (size of the array)
value contains the size of the array that is in the dependency object
*/
size_t param1HashFunction(int const & value, DependencyObject* obj)
{		
	//this is a very weak hash and not recommended for general use
	return obj->array1[0] + obj->array1[value - 1] + value;
}

/**
getSizeFunction for the 1st parameter (an array of integers)
*/
size_t param0GetSizeFunction(int* const & , DependencyObject*)
{ 	
	return 0;	//always return 0
}

/**
getSizeFunction for the 2nd parameter (size of the array)
returns the number of bytes required to cache the input array 
(passed in the dependency object) with the given size
see also: param1InitFunction
*/
size_t param1GetSizeFunction(int const & value, DependencyObject*)
{
	return sizeof(value) + value * sizeof(int);
}

/**
getSizeFunction for the 3rd parameter (out: sum of the output array)
returns the size of the pointer + size of the value pointed by this pointer
see also: param2InitFunction
*/
size_t param2GetSizeFunction(int* const & value, DependencyObject*)
{
	return sizeof(value) + sizeof(int);
}

/**
getSizeFunction for the return value
see also: param1GetSizeFunction
*/
size_t returnGetSizeFunction(const std::vector<int> & value, DependencyObject*)
{
	return sizeof(value) + value.size() * sizeof(int);
}

/**
return function for the return value
performs a deep copy of the vector (in the cache) and returns the copy
*/
std::vector<int> returnFunction(const std::vector<int> & value, DependencyObject*)
{
	std::vector<int> ret;
	for (unsigned int i = 0; i < value.size(); i++)
		ret.push_back(value[i]);
	return ret;
}

int main()
{
	CacheManagerConfiguration managerConf;
	CachedFunctionManager manager(managerConf);

	CacheConfigurationWithDepObj<DependencyObject*> conf;
	DependencyObject dependencyObj;
	conf.setDependencyObject(&dependencyObj);
	conf.setParamInfo(0, TypedParameterInfoWithDepObj<int*, DependencyObject*>(
		ParameterType::InputParam,
		param0EqualFunction,
		param0InitFunction,
		nullptr,
		param0DestroyFunction,
		param0HashFunction,
		param0GetSizeFunction
		));
	conf.setParamInfo(1, TypedParameterInfoWithDepObj<int, DependencyObject*>(
		ParameterType::InputParam,
		param1EqualFunction,
		param1InitFunction,
		nullptr,
		param1DestroyFunction,
		param1HashFunction,
		param1GetSizeFunction
		));
	conf.setParamInfo(2, TypedParameterInfoWithDepObj<int*, DependencyObject*>(
		ParameterType::OutputParam,
		nullptr,
		param2InitFunction,
		param2OutputFunction,
		param2DestroyFunction,
		nullptr,
		param2GetSizeFunction
		));
	conf.setReturnInfo(TypedReturnInfoWithDepObj<std::vector<int>, DependencyObject*>(
		ReturnType::UsedReturn,
		returnInitFunction,
		returnDestroyFunction,
		returnFunction,
		returnGetSizeFunction
		));

	auto cachedFunction = manager.createCachedFunction(conf, function);

	unsigned int arrayCount = 10;
	unsigned int numberOfCalculations = 100;
	unsigned int* arraySizes = new unsigned int[arrayCount];
	int** arrays = new int*[arrayCount];
	for (unsigned int i = 0; i < arrayCount; i++)
	{
		int size = rand() % 5 + 2;
		arraySizes[i] = size;
		arrays[i] = new int[size];
		for (int j = 0; j < size; j++)
			arrays[i][j] = rand() % 11;
	}

	for (unsigned int i = 0; i < numberOfCalculations; i++)
	{
		int rnd = rand() % arrayCount;
		std::cout << "Input array: ";
		for (unsigned int j = 0; j < arraySizes[rnd]; j++)
		{
			std::cout << arrays[rnd][j] << ", ";
		}
		std::cout << std::endl;
		
		int outputSum;
		//std::vector<int> outputVector = function(arrays[rnd], arraySizes[rnd], &outputSum);
		std::vector<int> outputVector = cachedFunction->call(arrays[rnd], arraySizes[rnd], &outputSum);

		std::cout << "Output array: ";
		for (unsigned int j = 0; j < arraySizes[rnd]; j++)
		{
			std::cout << outputVector[j] << ", ";
		}
		std::cout << std::endl;

		std::cout << "Output sum: " << outputSum << std::endl;
		std::cout << "---------------------------------------" << std::endl;

	}

	std::cin.get();

	for (unsigned int i = 0; i < arrayCount; i++)
		delete[] arrays[i];
	delete[] arrays;
	delete[] arraySizes;
}


