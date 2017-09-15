#ifndef _CACHING_FILTER_H
#define _CACHING_FILTER_H

#include "CachedFunction.h"
#include "CacheManagerSource.h"
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <mutex>
#include <vector>

/**
this class represents the caching extension for vtk filters
CACHE_CAPACITY defines the cache's capacity in bytes
*/
template <class FilterClass, class SuperClass>
class CachingFilter : public SuperClass
{
protected:
	/**
	a mutex for thread safety
	*/
	static std::recursive_mutex mutex;

	/**
	stack of filters for possible recursive calls
	*/
	static std::vector<CachingFilter<FilterClass, SuperClass>*> filterStack;

	/**
	the current caching filter to call the data manipulation functions on
	*/
	static CachingFilter<FilterClass, SuperClass>* currentCachingFilter;

	/**
	the caching object
	*/
	CacheSystem::CachedFunction<int, FilterClass*, vtkInformation*, vtkInformationVector**, vtkInformationVector*>* cachedFunction;

	/**
	sets this as the currentCachingFilter
	*/
	void setThisTheCurrentFilter();

	/**
	unsets this the currentCachingFilter
	*/
	void unsetThisTheCurrentFilter();
	
	/**
	constructor for initialization
	*/
	CachingFilter()
	{
		mutex.lock();
		CacheManagerSource::cacheInstanceCounter++;
		if (CacheManagerSource::cacheManager == nullptr)  //the cache manager is created only of it is uninitialized
		{
			CacheSystem::CacheManagerConfiguration managerConf;
			managerConf.setCacheCapacity(CacheManagerSource::CACHE_CAPACITY);
			CacheManagerSource::cacheManager = new CacheSystem::CachedFunctionManager(managerConf);
		}
		mutex.unlock();
		CacheSystem::CacheConfiguration conf;
		conf.setParamInfo(0, CacheSystem::TypedParameterInfo<FilterClass*>(CacheSystem::ParameterType::InputParam, filterStaticEqualsFunction, filterStaticInitFunction, nullptr, filterStaticDestroyFunction, filterStaticHashFunction, filterStaticGetSizeFunction));
		conf.setParamInfo(1, CacheSystem::TypedParameterInfo<vtkInformation*>(CacheSystem::ParameterType::InputParam, requestStaticEqualsFunction, requestStaticInitFunction, nullptr, requestStaticDestroyFunction, requestStaticHashFunction, requestStaticGetSizeFunction));
		conf.setParamInfo(2, CacheSystem::TypedParameterInfo<vtkInformationVector**>(CacheSystem::ParameterType::InputParam, inputStaticEqualsFunction, inputStaticInitFunction, nullptr, inputStaticDestroyFunction, inputStaticHashFunction, inputStaticGetSizeFunction));
		conf.setParamInfo(3, CacheSystem::TypedParameterInfo<vtkInformationVector*>(CacheSystem::ParameterType::OutputParam, nullptr, outputStaticInitFunction, outputStaticCopyOutFunction, outputStaticDestroyFunction, nullptr, outputStaticGetSizeFunction));
		conf.setReturnInfo(CacheSystem::TypedReturnInfo<int>());  //the default constructor uses the standard functions for int (see CacheSystem::StandardFunctions)
		cachedFunction = CacheManagerSource::cacheManager->createCachedFunction(conf, &staticRequestData);

		this->RequestDataCalls = 0;
		this->CacheMisses = 0;
	}

	/**
	this method defines how two filters should be compared
	this method should be always overridden by the user
	*/
	virtual bool filterEqualsFunction(FilterClass* filter1, FilterClass* filter2) = 0;	

	/**
	this method defines how two requests should be compared
	this method should be overridden by the user, if the request is considered as an input argument,
	i.e., the original RequestData method uses information from the request argument in 
	the preparation of the output data
	*/
	virtual bool requestEqualsFunction(vtkInformation* request1, vtkInformation* request2)
	{
		return true;
	}

	/**
	this method defines how the two input arguments should be compared
	usually there is no need to override this method	
	*/
	virtual bool inputEqualsFunction(vtkInformationVector** input1, vtkInformationVector** input2)
	{
		//each filter may have multiple input ports and 
		//each port may have multiple input objects

		int ports = this->GetNumberOfInputPorts();
		return CacheUtils::CacheEquals(input1, input2, ports);
	}

	/**
	this method defines how to compute the hash of the filter
	this method should be always overridden by the user
	it is recommended to implement it to return a hash value
	of the most commonly changed configuration parameters of the filter
	*/
	virtual size_t filterHashFunction(FilterClass* filter) = 0;	

	/**
	this method defines how to compute the hash of the request
	this method should be overridden by the user, if the request is considered as an input argument
	*/
	virtual size_t requestHashFunction(vtkInformation* request)
	{
		return 0;
	}

	/**
	this method defines how to compute the hash of the input argument
	the default implementation is general and should be sufficient in most cases
	it is recommended to override this method, if the performance is an issue
	*/
	virtual size_t inputHashFunction(vtkInformationVector** input)
	{
		//each filter may have multiple input ports and 
		//each port may have multiple input objects

		int ports = this->GetNumberOfInputPorts();
		return CacheUtils::CacheHash(input, ports);
	}

	/**
	this method defines how to compute the filter's size in bytes
	this method should be overridden by the user
	*/
	virtual size_t filterGetSizeFunction(FilterClass* filter)
	{
		return sizeof(FilterClass);
	}

	/**
	this method defines how to compute the request's size in bytes	
	this method should be overridden only if the request is considered as an input argument	
	*/
	virtual size_t requestGetSizeFunction(vtkInformation* request)
	{
		return 0;
	}

	/**
	this method defines how to compute the input argument's size in bytes
	usually there is no need to override this method
	*/
	virtual size_t inputGetSizeFunction(vtkInformationVector** input)
	{
		//each filter may have multiple input ports and 
		//each port may have multiple input objects

		int ports = this->GetNumberOfInputPorts();
		return CacheUtils::CacheGetSize(input, ports);
	}

	/**
	this method defines how to compute the output's argument size in bytes (after filling it with the output data)
	usually there is no need to override this method
	*/
	virtual size_t outputGetSizeFunction(vtkInformationVector* output)
	{
		return CacheUtils::CacheGetSize(output);
	}

	/**
	this method defines how the filter should be copied into the cache
	this method must be overridden by the user, if the filter contains
	any configuration parameter (may be inherited) that is not of integral type,
	i.e., some of its configuration parameters is a pointer (or an object),
	typically declared by vtkSetObjectMacro
	*/
	virtual void filterInitFunction(FilterClass* source, FilterClass* & destination)
	{		
		destination = source->NewInstance();
		memcpy(destination, source, sizeof(*source));
	}
	
	
	/**
	this method defines how the request should be copied into the cache
	this method should be overridden by the user if the request is considered an input argument
	*/
	virtual void requestInitFunction(vtkInformation* source, vtkInformation* & destination)
	{
		destination = nullptr;
	}

	/**
	this method defines how the input argument should be copied into the cache
	usually there is no need for the user to override this method
	*/
	virtual void inputInitFunction(vtkInformationVector** source, vtkInformationVector** & destination)
	{
		int ports = this->GetNumberOfInputPorts();
		CacheUtils::CacheInit(source, destination, ports);
	}

	/**
	this method defines how the output argument should be copied into the cache (after filling it with the output data)
	usually there is no need for the user to override this method
	*/
	virtual void outputInitFunction(vtkInformationVector* source, vtkInformationVector* & destination)
	{
		CacheUtils::CacheInit(source, destination);
	}

	/**
	this method defines how the output value should be copied from the cache into the output argument
	usually there is no need for the user to override this method
	*/
	virtual void outputCopyOutFunction(vtkInformationVector* storedValuePointer, vtkInformationVector* & outputPointer)
	{
		outputPointer->Copy(storedValuePointer);
	}

	/**
	this method defines how the filter should be destroyed when removed from the cache
	usually there is no need for the user to override this method
	*/
	virtual void filterDestroyFunction(FilterClass* filter)
	{
		if (filter != nullptr)
			filter->Delete();
	}

	/**
	this method defines how the request should be destroyed when removed from the cache
	usually there is no need for the user to override this method
	*/
	virtual void requestDestroyFunction(vtkInformation* request)
	{
		if (request != nullptr)
			request->Delete();
	}

	/**
	this method defines how the input argument should be destroyed when removed from the cache
	usually there is no need for the user to override this method
	*/
	virtual void inputDestroyFunction(vtkInformationVector** input)
	{
		int ports = this->GetNumberOfInputPorts();
		CacheUtils::CacheDestroy(input, ports);
	}

	/**
	this method defines how the output argument should be destroyed when removed from the cache
	usually there is no need for the user to override this method
	*/
	virtual void outputDestroyFunction(vtkInformationVector* output)
	{
		CacheUtils::CacheDestroy(output);
	}



	/*static wrappers for the data manipulation functions*/
	/******************************************************/

	static bool filterStaticEqualsFunction(FilterClass* const & filter1, FilterClass* const & filter2, void*)
	{
		return currentCachingFilter->filterEqualsFunction(filter1, filter2);
	}

	static bool requestStaticEqualsFunction(vtkInformation* const & request1, vtkInformation* const & request2, void*)
	{
		return currentCachingFilter->requestEqualsFunction(request1, request2);
	}

	static bool inputStaticEqualsFunction(vtkInformationVector** const & input1, vtkInformationVector** const & input2, void*)
	{
		return currentCachingFilter->inputEqualsFunction(input1, input2);
	}

	static size_t filterStaticHashFunction(FilterClass* const & filter, void*)
	{
		return currentCachingFilter->filterHashFunction(filter);
	}

	static size_t requestStaticHashFunction(vtkInformation* const & request, void*)
	{
		return currentCachingFilter->requestHashFunction(request);
	}

	static size_t inputStaticHashFunction(vtkInformationVector** const & input, void*)
	{
		return currentCachingFilter->inputHashFunction(input);
	}

	static size_t filterStaticGetSizeFunction(FilterClass* const & filter, void*)
	{
		return currentCachingFilter->filterGetSizeFunction(filter);
	}

	static size_t requestStaticGetSizeFunction(vtkInformation* const & request, void*)
	{
		return currentCachingFilter->requestGetSizeFunction(request);
	}

	static size_t inputStaticGetSizeFunction(vtkInformationVector** const & input, void*)
	{
		return currentCachingFilter->inputGetSizeFunction(input);
	}

	static size_t outputStaticGetSizeFunction(vtkInformationVector* const & output, void*)
	{
		return currentCachingFilter->outputGetSizeFunction(output);
	}

	static void filterStaticInitFunction(FilterClass* const & source, FilterClass** destination, void*)
	{
		currentCachingFilter->filterInitFunction(source, *destination);
	}

	static void requestStaticInitFunction(vtkInformation* const & source, vtkInformation** destination, void*)
	{
		currentCachingFilter->requestInitFunction(source, *destination);
	}

	static void inputStaticInitFunction(vtkInformationVector** const & source, vtkInformationVector*** destination, void*)
	{
		currentCachingFilter->inputInitFunction(source, *destination);
	}

	static void outputStaticInitFunction(vtkInformationVector* const & source, vtkInformationVector** destination, void*)
	{
		currentCachingFilter->outputInitFunction(source, *destination);
	}

	static void outputStaticCopyOutFunction(vtkInformationVector* const & storedValuePointer, vtkInformationVector* & outputPointer, void*)
	{
		currentCachingFilter->outputCopyOutFunction(storedValuePointer, outputPointer);
	}

	static void filterStaticDestroyFunction(FilterClass* & filter, void*)
	{
		currentCachingFilter->filterDestroyFunction(filter);
	}

	static void requestStaticDestroyFunction(vtkInformation* & request, void*)
	{
		currentCachingFilter->requestDestroyFunction(request);
	}

	static void inputStaticDestroyFunction(vtkInformationVector** & input, void*)
	{
		currentCachingFilter->inputDestroyFunction(input);
	}
	
	static void outputStaticDestroyFunction(vtkInformationVector* & output, void*)
	{
		currentCachingFilter->outputDestroyFunction(output);
	}

	/**************************************************************/
	/*end of the static wrappers*/

protected:
	virtual ~CachingFilter();

protected:

	//static wrapper for the RequestData method
	static int staticRequestData(FilterClass* o, vtkInformation* request,
		vtkInformationVector** inputVector, vtkInformationVector* outputVector)
	{
		++o->CacheMisses;	//data not found in the cache
		return o->SuperClass::RequestData(request, inputVector, outputVector); //using the original RequestData method to generate the output
	}	

	//overridden RequestData method to get the data from the cache, if available
	/*virtual*/ int RequestData(vtkInformation* request,
		vtkInformationVector** inputVector, vtkInformationVector* outputVector);

protected:
	int RequestDataCalls;	//<! number of total calls of RequestData method
	int CacheMisses;		//<! number of the calls when the data has not been found in cache

};

template <class FilterClass, class SuperClass>
std::recursive_mutex CachingFilter<FilterClass, SuperClass>::mutex;// = std::recursive_mutex();

template <class FilterClass, class SuperClass>
std::vector<CachingFilter<FilterClass, SuperClass>*> CachingFilter<FilterClass, SuperClass>::filterStack = std::vector<CachingFilter<FilterClass, SuperClass>*>();

template <class FilterClass, class SuperClass>
CachingFilter<FilterClass, SuperClass>* CachingFilter<FilterClass, SuperClass>::currentCachingFilter = nullptr;

template <class FilterClass, class SuperClass>
void CachingFilter<FilterClass, SuperClass>::setThisTheCurrentFilter()
{
	mutex.lock();
	filterStack.push_back(this);
	currentCachingFilter = this;
}

template <class FilterClass, class SuperClass>
void CachingFilter<FilterClass, SuperClass>::unsetThisTheCurrentFilter()
{
	filterStack.pop_back();
	currentCachingFilter = filterStack.empty() ? nullptr : filterStack.back();
	mutex.unlock();
}

template <class FilterClass, class SuperClass>
CachingFilter<FilterClass, SuperClass>::~CachingFilter()
{
	mutex.lock();
	CacheManagerSource::cacheInstanceCounter--;
	mutex.unlock();
	if (CacheManagerSource::cacheInstanceCounter == 0)
	{
		delete CacheManagerSource::cacheManager;
		CacheManagerSource::cacheManager = nullptr;
	}
}

template <class FilterClass, class SuperClass>
int CachingFilter<FilterClass, SuperClass>::RequestData(vtkInformation* request,
	vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{	
	setThisTheCurrentFilter();
	int ret = cachedFunction->call((FilterClass*)this, request, inputVector, outputVector);
	unsetThisTheCurrentFilter();
	
	++RequestDataCalls;	//increase the number of counts
	return ret;
}

#endif