#include <iostream>
#include <vector>
#include "CachedFunction.h"
using namespace CacheSystem;

/**
funkce generujici data, jejiz chovani bude cache objekt simulovat
na vstupu prebira pole intu (prvni parametr) a jeho velikost (druhy parametr) a vraci vektor, kde vsechny hodnoty vstupniho pole 
jsou prenasobene hodnotou 10, zaroven do vystupniho parametru (treti parametr) sum ulozi soucet prvku vstupniho pole
*/
std::vector<int> function(int* const & inputArray, int size, int* sum)
{
	std::cout << "CALLING" << std::endl; //aby bylo poznat, kdy se funkce volala a kdy ne
	std::vector<int> ret;
	*sum = 0;
	for (int i = 0; i < size; i++)
	{
		ret.push_back(10 * inputArray[i]);
		*sum += inputArray[i];
	}
	return ret;
}

struct DependencyObject
{
	int* array1;
	int* array2;
	int** destinationPtr;
};

/**
equalFunction pro prvni parametr
pouze ulozi do dependecy objektu obe pole a vrati true, protoze nezname velikost pole
*/
bool param0EqualFunction(int* const & val1, int* const & val2, void* dependencyObj)
{
	DependencyObject* obj = (DependencyObject*)dependencyObj;
	obj->array1 = val1;
	obj->array2 = val2;
	return true;
}

/**
equalFunction pro druhy parametr
provede skutecne porovnani poli
zaroven ulozi velikost pole do dependency objektu - bude vyuzito v returnFunction
*/
bool param1EqualFunction(int const & size1, int const & size2, void* dependencyObj)
{
	if (size1 != size2)
		return false;
	DependencyObject* obj = (DependencyObject*)dependencyObj;
	for (int i = 0; i < size1; i++)
		if (obj->array1[i] != obj->array2[i])
			return false;
	return true;
}

/**
initFunction pro prvni parametr
pouze ulozi zdrojove pole a adresu ciloveho pole do dependency objektu
*/
void param0InitFunction(int* const & source, int** destination, void* dependencyObj)
{
	DependencyObject* obj = (DependencyObject*)dependencyObj;
	obj->array1 = source;
	obj->destinationPtr = destination;
}

/**
initFunction pro druhy parametr
provede prekpirovani do cache
*/
void param1InitFunction(int const & source, int* destination, void* dependencyObj)
{
	DependencyObject* obj = (DependencyObject*)dependencyObj;
	*destination = source;
	*obj->destinationPtr = new int[source];
	for (int i = 0; i < source; i++)
		(*obj->destinationPtr)[i] = obj->array1[i];
}

/**
initFunction pro treti, vystupni parametr
*/
void param2InitFunction(int* const & source, int** destination, void*)
{
	*destination = new int(*source);
}

/**
initFunction pro navratovou hodnotu
inicializuje vector v cahce pomocí placement new a prekopiruje polozky
*/
void returnInitFunction(const std::vector<int> & source, std::vector<int>* destination, void*)
{
	new(destination)std::vector<int>();
	for (unsigned int i = 0; i < source.size(); i++)
		destination->push_back(source[i]);
}

/**
outpuFunction pro treti, vystupni parametr
pouze prekopiruje hodnotu z cache do ukazatele na vystupu
*/
void param2OutputFunction(int* const & source, int* & destination, void*)
{
	*destination = *source;
}

/**
destroyFunction pro prvni parametr
*/
void param0DestroyFunction(int* & value, void* dependencyObj)
{
	delete[] value;
}

/**
destroyFunction pro druhy parametr
*/
void param1DestroyFunction(int & value, void*) { /*zde neni treba nic delat*/ }

/**
destroyFunction pro treti, vystupni parametr
*/
void param2DestroyFunction(int* & value, void*)
{
	delete value;
}

/**
destroyFunction pro navratovou hodnotu
pouze zavola destruktor nad vektorem
*/
void returnDestroyFunction(std::vector<int> & value, void*)
{
	value.~vector<int>();
}

/**
hashFunction pro prvni parametr
pouze ulozi pole do dependency objektu
*/
uint32_t param0HashFunction(int* const & value, void* dependencyObj)
{
	DependencyObject* obj = (DependencyObject*)dependencyObj;
	obj->array1 = value;
	return 0;
}

/**
hashFunction pro druhy parametr
vraci soucet prvniho a posledniho prvku pole a velikosti pole
*/
uint32_t param1HashFunction(int const & value, void* dependencyObj)
{
	DependencyObject* obj = (DependencyObject*)dependencyObj;
	return obj->array1[0] + obj->array1[value - 1] + value;
}

/**
getSizeFunction pro prvni parametr
*/
uint64_t param0GetSizeFunction(int* const & value, void* dependencyObj) { /*zde neni treba nic delat*/ return 0; }

/**
getSizeFunction pro druhy parametr
vraci velikost parametru size + soucet velikosti prvku pole
*/
uint64_t param1GetSizeFunction(int const & value, void* dependencyObj)
{
	return sizeof(value) + value * sizeof(int);
}

/**
getSizeFunction pro treti, vystupni parametr
vraci velikost ukazatele + velikost hodnoty, na kterou ukazuje
*/
uint64_t param2GetSizeFunction(int* const & value, void*)
{
	return sizeof(value) + sizeof(int);
}

/**
getSizeFunction pro navratovou hodnotu
vraci velikost vektoru + soucet velikosti jeho prvku
*/
uint64_t returnGetSizeFunction(const std::vector<int> & value, void*)
{
	return sizeof(value) + value.size() * sizeof(int);
}

/**
return function pro navratovou hodnotu
provede hlubokou kopii vektoru a zkopirovany vektor vrati
*/
std::vector<int> returnFunction(const std::vector<int> & value, void* dependencyObj)
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
	CacheConfiguration conf;
	DependencyObject dependencyObj;
	conf.setDependencyObject(&dependencyObj);
	conf.setParamInfo(0, TypedParameterInfo<int*>(
		ParameterType::InputParam,
		param0EqualFunction,
		param0InitFunction,
		nullptr,
		param0DestroyFunction,
		param0HashFunction,
		param0GetSizeFunction
		));
	conf.setParamInfo(1, TypedParameterInfo<int>(
		ParameterType::InputParam,
		param1EqualFunction,
		param1InitFunction,
		nullptr,
		param1DestroyFunction,
		param1HashFunction,
		param1GetSizeFunction
		));
	conf.setParamInfo(2, TypedParameterInfo<int*>(
		ParameterType::OutputParam,
		nullptr,
		param2InitFunction,
		param2OutputFunction,
		param2DestroyFunction,
		nullptr,
		param2GetSizeFunction
		));
	conf.setReturnInfo(TypedReturnInfo<std::vector<int> >(
		ReturnType::UsedReturn,
		returnInitFunction,
		returnDestroyFunction,
		returnFunction,
		returnGetSizeFunction
		));
	CachedFunction<std::vector<int>, int* const &, int, int*>* cachedFunction = manager.createCachedFunction(conf, function);

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



/*#include <iostream>
#include "CachedFunction.h"
#include <time.h>
using namespace CacheSystem;
using namespace std;

const int vectorSize = 5;
int* const function(int* const & vector1, int* const & vector2, int* & output)
{
	cout << "CALLING!" << endl;
	int* const ret = new int[vectorSize];
	for (int i = 0; i < vectorSize; i++)
	{
		ret[i] = vector1[i] + vector2[i];
	}
	int sum = 0;
	for (int i = 0; i < vectorSize; i++)
	{
		sum += ret[i];
	}
	*output = sum;
	return ret;
}

void voidFunction(int* const & vector1, int* const & vector2, int* & output)
{
	cout << "CALLING!" << endl;
	int* ret = new int[vectorSize];
	for (int i = 0; i < vectorSize; i++)
	{
		ret[i] = vector1[i] + vector2[i];
	}
	int sum = 0;
	for (int i = 0; i < vectorSize; i++)
	{
		sum += ret[i];
	}
	delete[] ret;
	*output = sum;
}

void init(int* const & source, int** destination, void*)
{
	new(destination) int*;
	*destination = new int[vectorSize];
	for (int i = 0; i < vectorSize; i++)
	{
		(*destination)[i] = source[i];
	}
}

void returnInit(int* const & source, int** destination, void*)
{
	new(destination) int*;
	*destination = new int[vectorSize];
	for (int i = 0; i < vectorSize; i++)
	{
		(*destination)[i] = source[i];
	}
	delete[] source;
}

void outputInit(int* const & source, int** destination, void*)
{
	new(destination) int*;
	*destination = new int;
	**destination = *source;
}

void destroy(int* & value, void*)
{
	delete[] value;
}

void outputDestroy(int* & value, void*)
{
	delete value;
}

bool equalF(int* const & val1, int* const & val2, void*)
{
	for (int i = 0; i < vectorSize; i++)
	{
		if (val1[i] != val2[i])
			return false;
	}
	return true;
}

uint32_t hashFunction(int* const & value, void*)
{
	uint32_t hash = 0;
	for (int i = 0; i < vectorSize; i++)
	{
		hash += value[i] * (i + 1);
	}
	return hash;
}

uint64_t getSize(int* const & value, void*)
{
	return sizeof(value) + sizeof(int) * vectorSize;
}

uint64_t outputGetSize(int* const & value, void*)
{
	return sizeof(value) + sizeof(int);
}

int* returnF(int* const & retVal, void*)
{
	int* ret = new int[vectorSize];
	for (int i = 0; i < vectorSize; i++)
	{
		ret[i] = retVal[i];
	}
	return ret;
}

void print(int* v)
{
	for (int i = 0; i < vectorSize; i++)
		cout << v[i] << ", ";
	cout << endl;
}


int main()
{	
	const int count = 30;
	const int numOfVectors = 5;
	int** vectors = new int*[numOfVectors];
	for (int i = 0; i < numOfVectors; i++)
	{
		vectors[i] = new int[vectorSize];
		for (int j = 0; j < vectorSize; j++)
		{
			vectors[i][j] = rand() % 1000;
		}
	}

	CacheManagerConfiguration managerConf;  //vytvoøení konfiguraèního objektu
	managerConf.setCachePolicy(CachePolicies::DefaultCachePolicy());  //nastavení politiky
	managerConf.setUseCacheMissEvent(true);  //nastavení použití cache miss eventu (hodnota R se pøepoèítává pøi každém pøístupu)
	managerConf.setCacheCapacity(500);
	CachedFunctionManager manager(managerConf);  //vytvoøení manažera
	CacheConfiguration conf;
	//conf.setMinimumDataCreationTime(2);
	conf.setParamInfo(0, TypedParameterInfo<int*>(ParameterType::InputParam, equalF, init, StandardFunctions::standardOutputFunction<int*>, destroy, hashFunction, getSize));
	conf.setParamInfo(1, TypedParameterInfo<int*>(ParameterType::InputParam, equalF, init, StandardFunctions::standardOutputFunction<int*>, destroy, hashFunction, getSize));
	conf.setParamInfo(2, TypedParameterInfo<int*>(ParameterType::OutputParam, nullptr, outputInit, StandardFunctions::standardOutputFunction<int*>, outputDestroy, nullptr, outputGetSize));
	conf.setReturnInfo(TypedReturnInfo<int*>(ReturnType::UsedReturn, returnInit, destroy, returnF, getSize));

	CachedFunction<int* const, int* const &, int* const &, int* &>* func = manager.createCachedFunction(conf, function);
	//CachedFunction<void, int* const &, int* const &, int* &>* func = manager.createCachedFunction(conf, voidFunction);
	int t = clock();
	for (int i = 0; i < count; i++)
	{
		int index1 = rand() % numOfVectors;
		int index2 = rand() % numOfVectors;
		int output;
		int* outputPtr = &output;
		bool stored;
		func->setDataInCacheIndicator(&stored);
		int* result = func->call(vectors[index1], vectors[index2], outputPtr);
		//func->call(vectors[index1], vectors[index2], outputPtr);
		cout << "Stored: " << stored << endl;
		//int* result = function(vectors[index1], vectors[index2], outputPtr);
		print(vectors[index1]);
		print(vectors[index2]);
		print(result);
		cout << output << endl;
		cout << "-------------------------------------" << endl;
		delete[] result;
	}
	t = clock() - t;
	cout << "Time: " << t;

	cin.get();
	for (int i = 0; i < numOfVectors; i++)
	{
		delete[] vectors[i];
	}
	delete[] vectors;
	return 0;
}*/

