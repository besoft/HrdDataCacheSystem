#include "vtkConfigure.h"

//this is required to initialize rendering
#include <vtkAutoInit.h>
#if VTK_MAJOR_VERSION <= 6
VTK_MODULE_INIT(vtkRenderingOpenGL);
#endif

#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkElevationFilter.h>
#include <vtkDataSetMapper.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkDecimatePro.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkCommand.h>
#include "CachedFunction.h"

#include <iostream>
using namespace std;

//setting the cache capacity to 200MB
#define DECIM_CACHE_CAPACITY 200000000
#include "CachingFilter.h"
#include "CacheUtils.h"

#define EXPERIMENT_IVAPP
#ifdef EXPERIMENT_IVAPP
#define TEST_CASES			1000	//number of test cases, i.e., repetition of the call
#define CACHE_HIT_FACTOR	2000	//every CACHE_HIT_FACTOR-th test case will cause cache hit  
									//i.e., value 1 = 100% cache hit, value > TEST_CASES = 100% cache miss
#include <chrono>
#endif

class RenderCommand : public vtkCommand
{
private:
	vtkTransform* trans;

	RenderCommand(vtkTransform* trans)
		: trans(trans) {}

public:
	static RenderCommand * New(vtkTransform* trans)
	{
		return new RenderCommand(trans);
	}

	void Execute(vtkObject * vtkNotUsed(caller),
		unsigned long vtkNotUsed(eventId),
		void * vtkNotUsed(callData))
	{
		trans->Modified(); //makes sure that the decimation is ran everytime user interacts with the window to show the cache works
	}
};

#ifdef EXPERIMENT_IVAPP
static int callCount = 0;
#endif

/**
this class represents a caching version of a vtkDecimatePro filter class
*/
class CachingDecimator : public vtkDecimatePro, public CachingFilter<CachingDecimator, DECIM_CACHE_CAPACITY>
{
protected:
	//equalFunction of the input object
	bool inputEqualsFunction(vtkInformationVector** a, vtkInformationVector** b)
	{
		//getting the input data
		vtkPolyData* inputPoly1 = vtkPolyData::GetData(a[0]);
		vtkPolyData* inputPoly2 = vtkPolyData::GetData(b[0]);
#ifndef EXPERIMENT_IVAPP
		return CacheUtils::CacheEquals(inputPoly1, inputPoly2); //using the predefined comparator from CacheUtils
#else
		bool ret = CacheUtils::CacheEquals(inputPoly1, inputPoly2); //using the predefined comparator from CacheUtils
		if (((++callCount) % CACHE_HIT_FACTOR) == 0)
			return ret;
		else
			return false;
#endif
	}

	//hashFunction of the input object
	uint32_t inputHashFunction(vtkInformationVector** a)
	{
		//getting the input data
		vtkPolyData* inputPoly = vtkPolyData::GetData(a[0]);
		return CacheUtils::CacheHash(inputPoly); //using the predefined hash function from CacheUtils
	}

	//getSizeFunction of the input object
	uint64_t inputGetSizeFunction(vtkInformationVector** a)
	{
		vtkPolyData* inputPoly = vtkPolyData::GetData(a[0]);  //getting the input data
		return CacheUtils::CacheGetSize(inputPoly);  //returns the size in bytes using the predefined function from CacheUtils
	}

	//getSizeFunction of the output parameter
	uint64_t outputGetSizeFunction(vtkInformationVector* a)
	{
		vtkPolyData* outputPoly = vtkPolyData::GetData(a);  //getting the input data
		return CacheUtils::CacheGetSize(outputPoly);  //returns the size in bytes using the predefined function from CacheUtils
	}

	//initFunction of the input parameter (defines how to copy the input data into the cache)
	void inputInitFunction(vtkInformationVector** source, vtkInformationVector** & destination)
	{
		destination = new vtkInformationVector*;
		*destination = vtkInformationVector::New();  //creates the new object
		(*destination)->Copy(*source, 1);  //copies the input object into the object in cache
		vtkPolyData* sourceData = vtkPolyData::GetData(*source);  //getting the input data
		vtkPolyData* destData;
		CacheUtils::CacheInit(sourceData, destData); //copying the input data using the predefined function from CacheUtils
		(*destination)->GetInformationObject(0)->Set(vtkDataObject::DATA_OBJECT(), destData);  //puts the input data into the object in cache
		destData->Delete();
	}

	//initFunction of the output parameter (defines hot the newly calculated data is copied into the cache) 
	void outputInitFunction(vtkInformationVector* source, vtkInformationVector* & destination)
	{
		destination = vtkInformationVector::New();  //creates the new object
		destination->Copy(source, 1);  //copies the input object into the object in cache
		vtkPolyData* sourceData = vtkPolyData::GetData(source);  //getting the input data
		vtkPolyData* destData;
		CacheUtils::CacheInit(sourceData, destData); //copying the data using the predefined function from CacheUtils
		destination->GetInformationObject(0)->Set(vtkDataObject::DATA_OBJECT(), destData);  //puts the data into the object in cache
		destData->Delete();
	}

	//outputFunction for the output parameter (defines how the data is copied from the cache into the output parameter)
	void outputCopyOutFunction(vtkInformationVector* storedValuePointer, vtkInformationVector* & outputPointer)
	{
		outputPointer->Copy(storedValuePointer, 1);
		vtkPolyData* sourceData = vtkPolyData::GetData(storedValuePointer);  //getting the data
		vtkPolyData* destData;
		CacheUtils::CacheInit(sourceData, destData);  //copying the data using the predefined function from CacheUtils
		outputPointer->GetInformationObject(0)->Set(vtkDataObject::DATA_OBJECT(), destData);  //puts the data into the output parameter
	}

	//destroyFunction of the input parameter
	void inputDestroyFunction(vtkInformationVector** data)
	{
		(*data)->Delete();
		delete data;
	}

	//destroyFunction of the output parameter (defines how the output data is destroyed when it is removed from the cache)
	void outputDestroyFunction(vtkInformationVector* data)
	{
		data->Delete();
	}

	/**************************************************/
	/*  In some cases there will be more of the data  */
	/*   manipulation functions but in this example   */
	/*     we ignore the request argument of the      */
	/*   RequestData method and we also ignore the    */
	/*  filter's configuration so the functions for   */
	/* these two arguments do not need to be defined  */
	/**************************************************/



	//static wrapper for the RequestData method
	static int staticRequestData(CachingDecimator* decimator, vtkInformation* request,
		vtkInformationVector** inputVector, vtkInformationVector* outputVector)
	{
#ifndef EXPERIMENT_IVAPP
		cout << "CALLING" << endl;  //"CALLING" is printed when the original RequestData method from vtkDecimatePro is realy called
									//if this line is not printet thogether with "REQUESTING DATA" it means the output was cached		
#endif
		return decimator->vtkDecimatePro::RequestData(request, inputVector, outputVector); //using the original RequestData method to generate the ouput
	}

	//overriden RequestData method
	int RequestData(vtkInformation* request,
		vtkInformationVector** inputVector, vtkInformationVector* outputVector)
	{
#ifndef EXPERIMENT_IVAPP
		cout <<"REQUESTING DATA" << endl;  //"REQUESTING DATA" is printed when the data is requested
#endif
		return RequestDataCaching(request, inputVector, outputVector); //using the caching version of RequestData from CachingFilter class
	}

public:
	//the constructor needs to pass the staticRequestData method to the CachingFilter constructor
	CachingDecimator() : CachingFilter(staticRequestData)
	{

	}

	static CachingDecimator* New()
	{
		CachingDecimator* dec = new CachingDecimator;
		dec->InitializeObjectBase();
		return dec;
	}
};


/*
the main function
*/
int main(int argc, char* argv[])
{
	vtkSphereSource* sphere = vtkSphereSource::New(); //creates a sphere
	sphere->SetPhiResolution(30);
	sphere->SetThetaResolution(30);

	vtkTransform* trans = vtkTransform::New();
	trans->Scale(1, 1.5, 1);
	vtkTransformFilter* transFilter = vtkTransformFilter::New(); //scales the spher
	transFilter->SetInputConnection(sphere->GetOutputPort());
	transFilter->SetTransform(trans);

	CachingDecimator* decimator = CachingDecimator::New(); //this is the caching decimating filter
	//vtkDecimatePro* decimator = vtkDecimatePro::New(); //this is the ordinary decimating filter
	decimator->SetInputConnection(transFilter->GetOutputPort());
	decimator->SetTargetReduction(0.9);
	

	vtkElevationFilter* filter = vtkElevationFilter::New(); //used to color the scaled sphere
	filter->SetInputConnection(decimator->GetOutputPort());
	filter->SetLowPoint(0, 0, -0.3);
	filter->SetHighPoint(0, 0, 0.5);

	vtkDataSetMapper* mapper = vtkDataSetMapper::New();
	mapper->SetInputConnection(filter->GetOutputPort());

	vtkActor* actor = vtkActor::New();
	actor->SetMapper(mapper);
	actor->SetPosition(0, 0, 0);

	vtkRenderer* renderer = vtkRenderer::New();
	renderer->AddActor(actor);
	vtkRenderWindow* renderWindow = vtkRenderWindow::New();
	renderWindow->AddRenderer(renderer);
	vtkRenderWindowInteractor* renderWindowInteractor = vtkRenderWindowInteractor::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	renderWindowInteractor->Initialize();

#ifdef EXPERIMENT_IVAPP
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < TEST_CASES; i++)
	{
		trans->Modified();
		filter->Update();
	}

	end = std::chrono::high_resolution_clock::now();
	auto mcs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	cout << "Run finished in " << mcs.count() << " ms\n";
#endif


	renderWindowInteractor->CreateRepeatingTimer(1);
	RenderCommand* renderCallback = RenderCommand::New(trans);
	renderWindowInteractor->AddObserver(vtkCommand::TimerEvent, renderCallback);

	renderWindow->SetSize(600, 600);
	
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}