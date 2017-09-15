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

#include "vtkInformationDataObjectKey.h"
#include "vtkDataObject.h"

#include <iostream>
using namespace std;

#include "CachingFilter.h"
#include "CacheUtils.h"

//#define EXPERIMENT_IVAPP

#ifdef EXPERIMENT_IVAPP
#define TEST_CASES			1000	//number of test cases, i.e., repetition of the call
#define CACHE_HIT_FACTOR	3  	//every CACHE_HIT_FACTOR-th test case will cause cache hit  
									//i.e., value 1 = 100% cache hit, value > TEST_CASES = 100% cache miss

//setting the cache capacity to hold just one object (100 KB)
#define DECIM_CACHE_CAPACITY 100000
#include <chrono>
#else
//setting the cache capacity to 200MB
#define DECIM_CACHE_CAPACITY 200000000
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
static int cacheHitCount = 0;
static int cacheMissCount = 0;
#endif

/**
this class represents a very simple caching version of a vtkDecimatePro filter class
N.B. this implementation assumes that the configuration of the filter does not change
*/
class CachingDecimator : public CachingFilter<CachingDecimator, vtkDecimatePro>
{
public:
	vtkTypeMacro(CachingDecimator, vtkDecimatePro);

protected:	
	
	//this method defines how to compute the hash of the filter		
	/*virtual*/ size_t filterHashFunction(CachingDecimator* filter) VTK_OVERRIDE
	{
		//TargetReduction is the most commonly changed parameter
		return std::hash<double>()(filter->GetTargetReduction());
	}

	//equalFunction of filter configuration parameters		
	/*virtual*/ bool filterEqualsFunction(CachingDecimator* filter1, CachingDecimator* filter2) VTK_OVERRIDE
	{
		//vtkDecimatePro has conditional configuration:
		//parameter					[preconditions]
		//--------------------------------------------
		//TargetReduction
		//PreserveTopology
		//FeatureAngle
		//Splitting					[PreserveTopology OFF]
		//SplitAngle				[Splitting ON]
		//PreSplitMesh				[Splitting ON]
		//MaximumError				[ErrorIsAbsolute OFF]
		//AccumulateError
		//ErrorIsAbsolute
		//AbsoluteError				[ErrorIsAbsolute ON]
		//BoundaryVertexDeletion
		//Degree
		//InflectionPointRatio
		//OutputPointsPrecision


		/******************************************************************/
		// here should come the comparison of vtkDecimatePro configuration 
		// parameters (see above) with correct handing of preconditions,
		// i.e., if a parameter is ignored in some state, it should
		// not be compared.

		//in this simple example, we assume that filter2 = filter1
		/*****************************************************************/

		return true;
	}
	

#ifdef EXPERIMENT_IVAPP
	//equalFunction of the input object
	/*virtual*/ bool inputEqualsFunction(vtkInformationVector** a, vtkInformationVector** b) VTK_OVERRIDE
	{				
		return CachingFilter<CachingDecimator, vtkDecimatePro>
			::inputEqualsFunction(a, b) && ((this->RequestDataCalls % CACHE_HIT_FACTOR) == 0);
	}
#endif

	//overridden RequestData method to get the data from the cache, if available
	/*virtual*/ int RequestData(vtkInformation* request,
		vtkInformationVector** inputVector, vtkInformationVector* outputVector) VTK_OVERRIDE
	{
#ifndef EXPERIMENT_IVAPP
		cout << "REQUESTING DATA: ";  //"REQUESTING DATA" is printed when the data is requested
#else
		callCount++;
#endif
		int cacheMisses = this->CacheMisses;
		int ret = CachingFilter<CachingDecimator, vtkDecimatePro>
			::RequestData(request, inputVector, outputVector);
		
		if (cacheMisses != this->CacheMisses)
#ifndef EXPERIMENT_IVAPP
			cout << "CACHE MISS" << endl;  //the original RequestData method from vtkDecimatePro is really called										
#else
			cacheMissCount++;
#endif
		else 
#ifndef EXPERIMENT_IVAPP
			cout << "CACHE HIT" << endl;  //the original RequestData method from vtkDecimatePro is really called										
#else
			cacheHitCount++;
#endif

		return ret;
	}

	/***	
	Although the default implementation of inputHashFunction provides
	a good hash in virtually every case, it is general and thus always 
	slightly slower than the implementation designed for the concrete class.
	If the performance is crucial, it is recommended to override the method
	similarly as shown below.

	//hashFunction of the input object	
	size_t inputHashFunction(vtkInformationVector** a)
	{
		//getting the input data
		vtkPolyData* inputPoly = vtkPolyData::GetData(a[0]);
		return CacheUtils::CacheHash(inputPoly); //using the predefined hash function from CacheUtils
	}
	*/


protected:	
	CachingDecimator()
	{

	}

public:
	static CachingDecimator* New();	
	
private:
		CachingDecimator(const CachingDecimator&) VTK_DELETE_FUNCTION;
		void operator=(const CachingDecimator&) VTK_DELETE_FUNCTION;
};

vtkStandardNewMacro(CachingDecimator);


/*
the main function
*/
int main(int argc, char* argv[])
{
	//set the custom cache capacity, if default value is not sufficient
	//N.B. this operation must be done before the cache is created, which
	//is when the first cached class is instantiated
	CacheManagerSource::CACHE_CAPACITY = DECIM_CACHE_CAPACITY;


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
	cout << "Cache hit count = " << cacheHitCount << "("
		<< (cacheHitCount * 100.0 / (cacheHitCount + cacheMissCount)) << "%)";
#endif


	renderWindowInteractor->CreateRepeatingTimer(1);
	RenderCommand* renderCallback = RenderCommand::New(trans);
	renderWindowInteractor->AddObserver(vtkCommand::TimerEvent, renderCallback);

	renderWindow->SetSize(600, 600);
	
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}