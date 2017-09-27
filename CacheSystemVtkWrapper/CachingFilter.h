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
	the caching object
	*/
	CacheSystem::CachedFunctionNoDepObj<int, FilterClass*, vtkInformation*, vtkInformationVector**, vtkInformationVector*>* cachedFunction;
	
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

		CacheSystem::CacheConfigurationNoDepObj conf;
		conf.setParamInfo(0, CacheSystem::TypedParameterInfoNoDepObj<FilterClass*>(CacheSystem::ParameterType::InputParam, 
			[=](FilterClass* const & filter1, FilterClass* const & filter2)
			{
				return this->filterEqualsFunction(filter1, filter2);
			},
			[=](FilterClass* const & source, FilterClass** destination){ this->filterInitFunction(source, *destination);	},
			nullptr, 
			[=](FilterClass* & filter) { return this->filterDestroyFunction(filter); },
			[=](FilterClass* const & filter) { return this->filterHashFunction(filter); },
			[=](FilterClass* const & filter) { return this->filterGetSizeFunction(filter); }
			)
		);
		conf.setParamInfo(1, CacheSystem::TypedParameterInfoNoDepObj<vtkInformation*>(CacheSystem::ParameterType::InputParam,
			[=](vtkInformation* const & request1, vtkInformation* const & request2)
			{
				return this->requestEqualsFunction(request1, request2);
			},
			[=](vtkInformation* const & source, vtkInformation** destination){ this->requestInitFunction(source, *destination); },
			nullptr,
			[=](vtkInformation* & request) { return this->requestDestroyFunction(request); },
			[=](vtkInformation* const & request)	{ return this->requestHashFunction(request); },
			[=](vtkInformation* const & request) { return this->requestGetSizeFunction(request); }
			)
		);
		conf.setParamInfo(2, CacheSystem::TypedParameterInfoNoDepObj<vtkInformationVector**>(CacheSystem::ParameterType::InputParam,
			[=](vtkInformationVector** const & input1, vtkInformationVector** const & input2)
			{
				return this->inputEqualsFunction(input1, input2, nullptr);
			},
			[=](vtkInformationVector** const & source, vtkInformationVector*** destination){	this->inputInitFunction(source, *destination); },
			nullptr, 
			[=](vtkInformationVector** & input) { return this->inputDestroyFunction(input); },
			[=](vtkInformationVector** const & input) { return this->inputHashFunction(input); },
			[=](vtkInformationVector** const & input) { return this->inputGetSizeFunction(input); }
			));
		conf.setParamInfo(3, CacheSystem::TypedParameterInfoNoDepObj<vtkInformationVector*>(CacheSystem::ParameterType::OutputParam,
			nullptr, 
			[=](vtkInformationVector* const & source, vtkInformationVector** destination) { return this->outputInitFunction(source, *destination); },
			[=](vtkInformationVector* const & storedValuePointer, vtkInformationVector* & outputPointer) { return this->outputCopyOutFunction(storedValuePointer, outputPointer); },
			[=](vtkInformationVector* & output) { return this->outputDestroyFunction(output); },
			nullptr, 
			[=](vtkInformationVector* const & output) { return this->outputGetSizeFunction(output); }
			)
		);

		conf.setReturnInfo(CacheSystem::TypedReturnInfoNoDepObj<int>());  //the default constructor uses the standard functions for int (see CacheSystem::StandardFunctions)
		cachedFunction = CacheManagerSource::cacheManager->createCachedFunction(conf, 
			[](FilterClass* o, vtkInformation* request,
				vtkInformationVector** inputVector, vtkInformationVector* outputVector)
			{
				++o->CacheMisses;	//data not found in the cache
				return o->SuperClass::RequestData(request, inputVector, outputVector); //using the original RequestData method to generate the output
			}		
			);

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
	virtual bool inputEqualsFunction(/*const vtkInformationVector**& input1, const vtkInformationVector**& input2, void**/
		vtkInformationVector** const & input1, vtkInformationVector** const & input2, void*
	)
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
	
protected:
	virtual ~CachingFilter();

protected:
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
	++RequestDataCalls;	//increase the number of counts

	return cachedFunction->call((FilterClass*)this, request, inputVector, outputVector);
}

#endif