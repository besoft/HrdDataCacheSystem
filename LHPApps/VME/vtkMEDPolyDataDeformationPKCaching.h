#ifndef vtkMEDPolyDataDeformationPKCaching_h__
#define vtkMEDPolyDataDeformationPKCaching_h__

//#include <vld.h>

#include "medVMEMuscleWrapper.h"
#include "mafTransform.h"

#include "vtkMAFSmartPointer.h"

#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkTubeFilter.h"

#include "vtkMAFMuscleDecomposition.h"
#include "vtkMAFMuscleDecompositionMMSS.h"
#include "vtkMEDPolyDataDeformation.h"
#include "vtkMEDPolyDataDeformationPK.h"

#include <assert.h>
#include "mafDbg.h"
#include "mafMemDbg.h"
#include "mafDataChecksum.h"

#include "mafVMEGeneric.h"
#include "mafVMEItemVTK.h"
#include "mafDataVector.h"
#include "mafVMESurface.h"

#include "vtkMassSpringMuscle.h"

#include "Cache.h"

#define VTK_MED_POLY_DATA_DEFORMATION_PK_CACHING_CACHE_LOGGING
class vtkMEDPolyDataDeformationPKCaching : public vtkMEDPolyDataDeformationPK
{
private:
	/**
	pointer to the cache manager
	*/
	static CacheSystem::CachedFunctionManager* cacheManager;

	/**
	pointer to the cached function with return type bool, one input parameter of type vtkMEDPolyDataDeformationPKCaching and one output parameter of type vtkMEDPolyDataDeformationPKCaching
	*/
	static CacheSystem::CachedFunction<bool, vtkMEDPolyDataDeformationPKCaching*, vtkMEDPolyDataDeformationPKCaching*>* cachedFunction;

	static void log(std::string str);
	static void log(int num);
	static void log(std::string str, int num);

	/**
	equalFunction for the input parameter
	*/
	static bool equalFunction(vtkMEDPolyDataDeformationPKCaching* const & obj1, vtkMEDPolyDataDeformationPKCaching* const & obj2, void*);

	/**
	inputFunction for the input parameter
	*/
	static void initInput(vtkMEDPolyDataDeformationPKCaching* const & source, vtkMEDPolyDataDeformationPKCaching** destination, void*);

	/**
	destroyFunction for the input parameter
	*/
	static void destroyInput(vtkMEDPolyDataDeformationPKCaching* & data, void*);

	/**
	hashFunction for the input parameter
	*/
	static uint32_t inputHash(vtkMEDPolyDataDeformationPKCaching* const & source, void*);

	/**
	getSizeFunction for the input parameter
	*/
	static uint64_t inputGetSize(vtkMEDPolyDataDeformationPKCaching* const & source, void*);

	/**
	initFunction for the output parameter
	*/
	static void initOutput(vtkMEDPolyDataDeformationPKCaching* const & source, vtkMEDPolyDataDeformationPKCaching** destination, void*);

	/**
	outputFunction for the output parameter
	*/
	static void outputOutput(vtkMEDPolyDataDeformationPKCaching* const & source, vtkMEDPolyDataDeformationPKCaching* & destination, void*);

	/**
	destroyFunction for the output parameter
	*/
	static void destroyOutput(vtkMEDPolyDataDeformationPKCaching* & data, void*);

	/**
	getSizeFunction for the output parameter
	*/
	static uint64_t outputGetSize(vtkMEDPolyDataDeformationPKCaching* const & source, void*);

	/**
	the function for generating the data
	*/
	static bool staticExectuteMultidata(vtkMEDPolyDataDeformationPKCaching* inputFilter, vtkMEDPolyDataDeformationPKCaching* outputFilter);

public:
	vtkMEDPolyDataDeformationPKCaching();
	bool ExecuteMultiData();
	static vtkMEDPolyDataDeformationPKCaching* New();
};


#endif