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
	static CacheSystem::CachedFunctionManager* cacheManager;
	static CacheSystem::CachedFunction<bool, vtkMEDPolyDataDeformationPKCaching*, vtkMEDPolyDataDeformationPKCaching*>* cachedFunction;

	static void log(std::string str);
	static void log(int num);
	static void log(std::string str, int num);

	static bool equalFunction(vtkMEDPolyDataDeformationPKCaching* const & obj1, vtkMEDPolyDataDeformationPKCaching* const & obj2, void*);
	static void initInput(vtkMEDPolyDataDeformationPKCaching* const & source, vtkMEDPolyDataDeformationPKCaching** destination, void*);
	static void destroyInput(vtkMEDPolyDataDeformationPKCaching* & data, void*);
	static uint32_t inputHash(vtkMEDPolyDataDeformationPKCaching* const & source, void*);
	static uint64_t inputGetSize(vtkMEDPolyDataDeformationPKCaching* const & source, void*);
	static void initOutput(vtkMEDPolyDataDeformationPKCaching* const & source, vtkMEDPolyDataDeformationPKCaching** destination, void*);
	static void outputOutput(vtkMEDPolyDataDeformationPKCaching* const & source, vtkMEDPolyDataDeformationPKCaching* & destination, void*);
	static void destroyOutput(vtkMEDPolyDataDeformationPKCaching* & data, void*);
	static uint64_t outputGetSize(vtkMEDPolyDataDeformationPKCaching* const & source, void*);

	static bool staticExectuteMultidata(vtkMEDPolyDataDeformationPKCaching* inputFilter, vtkMEDPolyDataDeformationPKCaching* outputFilter);

public:
	vtkMEDPolyDataDeformationPKCaching();
	bool ExecuteMultiData();
	static vtkMEDPolyDataDeformationPKCaching* New();
};


#endif