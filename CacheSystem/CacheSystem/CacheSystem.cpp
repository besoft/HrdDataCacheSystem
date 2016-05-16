#include <iostream>
#include "CachedFunction.h"
using namespace CacheSystem;
using namespace std;

int function(int input) { return input * 10; }  //funkce generujici data
bool paramEqualFunction(const int & val1, const int & val2, void*) { return val1 == val2; }
void paramInitFunction(const int & source, int* destination, void*) { *destination = source; }
void paramDestroyFunction(int & data, void*) { /*zde neni potreba nic provadet*/ }
uint32_t paramHashFunction(const int & data, void*) { return (uint32_t)data; }
uint64_t paramGetSizeFunction(const int & data, void*) { return sizeof(int); }
void returnInitFunction(const int & source, int* destination, void*) { *destination = source; }
void returnDestroyFunction(int & data, void*) { /*zde neni potreba nic provadet*/ }
uint64_t returnGetSizeFunction(const int & data, void*) { return sizeof(int); }

int main()
{
	CachedFunctionManager manager;
	CacheConfiguration conf;
	conf.setParamInfo(0, TypedParameterInfo<int>(
		ParameterType::InputParam,
		paramEqualFunction,  //funkce pro porovnani
		paramInitFunction,  //funkce pro inicializaci
		nullptr,  //funkci pro kopirovani dat do parametru u vstupniho parametru nevyuzijeme
		paramDestroyFunction,  //funkce pro zniceni dat
		paramHashFunction,  //hash funkce
		paramGetSizeFunction  //funkce pro zjisteni velikosti
		));
	conf.setReturnInfo(TypedReturnInfo<int>(
		ReturnType::UsedReturn,
		returnInitFunction,  //funkce pro inicializaci
		returnDestroyFunction,  //funkce pro zniceni dat
		StandardFunctions::DirectReturn<int>,  //chceme hodnotu vracet primo z cache
		returnGetSizeFunction  //funkce pro zjisteni velikosti
		));
	CachedFunction<int, int>* cachedFunction = manager.createCachedFunction(conf, function);

	int a = cachedFunction->call(10);
	int b = function(10);

	cout << cachedFunction->call(7) << endl;
	cout << cachedFunction->call(5) << endl;
	cout << cachedFunction->call(2) << endl;
	cout << cachedFunction->call(7) << endl;
	cout << cachedFunction->call(5) << endl;
	cout << cachedFunction->call(2) << endl;

	bool dataInCache = false;
	cachedFunction->setDataInCacheIndicator(&dataInCache);
	int result = cachedFunction->call(5);
	if (dataInCache)
	{
		//tento blok se vykona pouze tehdy, pokud vystupni hodnota pochazi z cache
	}

	cin.get();
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

