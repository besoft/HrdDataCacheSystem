#include <iostream>
#include "CacheData.h"
#include "CachedFunction.h"
#include "StandardInitFunctions.h"
#include "StandardDestroyFunctions.h"
#include "StandardEqualFunctions.h"
#include "StandardOutputFunctions.h"
#include <time.h>
using namespace CacheSystem;
using namespace std;

const int vectorSize = 5;
int* function(int* vector1, int* vector2, int* output)
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
	*output = sum;
	return ret;
}

void voidFunction(int* vector1, int* vector2, int* output)
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
		hash += value[i];
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
	/*chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
	for (int i = 0; i < 3; i++)
		cout << "Something" << endl;
	chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();

	long long time = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
	cout << time << endl;
	cin.get();*/
	
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

	CacheConfiguration conf;
	conf.setMinimumDataCreationTime(2);
	conf.setParamInfo(0, TypedParameterInfo<int*>(ParameterType::InputParam, equalF, init, StandardFunctions::standardOutputFunction<int*>, destroy, hashFunction, getSize));
	conf.setParamInfo(1, TypedParameterInfo<int*>(ParameterType::InputParam, equalF, init, StandardFunctions::standardOutputFunction<int*>, destroy, hashFunction, getSize));
	conf.setParamInfo(2, TypedParameterInfo<int*>(ParameterType::OutputParam, nullptr, outputInit, StandardFunctions::standardOutputFunction<int*>, outputDestroy, nullptr, outputGetSize));
	conf.setReturnInfo(TypedReturnInfo<int*>(ReturnType::UsedReturn, returnInit, destroy, returnF, getSize));

	CachedFunction<int*, int*, int*, int*> func(conf, function);
	//CachedFunction<void, int*, int*, int*> func(conf, voidFunction);
	int t = clock();
	for (int i = 0; i < count; i++)
	{
		int index1 = rand() % numOfVectors;
		int index2 = rand() % numOfVectors;
		int output;
		bool stored;
		func.setDataInCacheIndicator(&stored);
		int* result = func.call(vectors[index1], vectors[index2], &output);
		//func.call(vectors[index1], vectors[index2], &output);
		cout << "Stored: " << stored << endl;
		//int* result = function(vectors[index1], vectors[index2], &output);
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
}

