#include "CacheUtils.h"
#include "vtkInformation.h"

#include <math.h>
#include <memory>

bool CacheUtils::CacheEquals(vtkInformationVector** infoVecs1, vtkInformationVector** infoVecs2, int n)
{	
	for (int i = 0; i < n; i++) {
		if (!CacheEquals(infoVecs1[i], infoVecs2[i]))
			return false;
	}

	return true;
}

bool CacheUtils::CacheEquals(vtkInformationVector* infoVec1, vtkInformationVector* infoVec2)
{	
	int n = infoVec1->GetNumberOfInformationObjects();
	if (n != infoVec2->GetNumberOfInformationObjects())
		return false;

	for (int i = 0; i < n; i++)
	{
		vtkDataObject* o1 = infoVec1->GetInformationObject(i)
			->Get(vtkDataObject::DATA_OBJECT());
		vtkDataObject* o2 = infoVec1->GetInformationObject(i)
			->Get(vtkDataObject::DATA_OBJECT());

		if (!CacheEquals(o1, o2))
			return false;
	}

	return true;
}

bool CacheUtils::CacheEquals(vtkDataObject* o1, vtkDataObject* o2)
{
	if (o1->GetDataObjectType() != o2->GetDataObjectType())
		return false;	//o1 and o2 are instances of different classes

	//detect the concrete class of o and use the
	//predefined implementation for it, if available
	//the order of tests is by their frequencies of use

	switch (o1->GetDataObjectType())
	{
	case VTK_POLY_DATA:
		return CacheEquals(vtkPolyData::SafeDownCast(o1), vtkPolyData::SafeDownCast(o2));

	case VTK_STRUCTURED_POINTS:	//vtkStructuredPoints is in fact vtkImageData	
		return CacheEquals(vtkImageData::SafeDownCast(o1), vtkImageData::SafeDownCast(o2));

	case VTK_STRUCTURED_GRID:
		return CacheEquals(vtkStructuredGrid::SafeDownCast(o1), vtkStructuredGrid::SafeDownCast(o2));

	case VTK_RECTILINEAR_GRID:
		return CacheEquals(vtkRectilinearGrid::SafeDownCast(o1), vtkRectilinearGrid::SafeDownCast(o2));

	case VTK_UNSTRUCTURED_GRID:
		return CacheEquals(vtkUnstructuredGrid::SafeDownCast(o1), vtkUnstructuredGrid::SafeDownCast(o2));

	case VTK_IMAGE_DATA:
		return CacheEquals(vtkImageData::SafeDownCast(o1), vtkImageData::SafeDownCast(o2));

	case VTK_DATA_OBJECT:
		break;	//processed later in this method

	case VTK_DATA_SET:
		return CacheEquals(vtkDataSet::SafeDownCast(o1), vtkDataSet::SafeDownCast(o2));

	case VTK_POINT_SET:
		return CacheEquals(vtkPointSet::SafeDownCast(o1), vtkPointSet::SafeDownCast(o2));

	case VTK_UNIFORM_GRID:		//vtkUniformGrid is in fact vtkImageData
		return CacheEquals(vtkImageData::SafeDownCast(o1), vtkImageData::SafeDownCast(o2));

	case VTK_UNSTRUCTURED_GRID_BASE:	//there is no difference in comparison with vtkPointSet
		return CacheEquals(vtkPointSet::SafeDownCast(o1), vtkPointSet::SafeDownCast(o2));

	case VTK_HYPER_OCTREE:
	case VTK_HYPER_TREE_GRID:	//not supported but nearest is vtkDataSet
		CacheEquals(vtkDataSet::SafeDownCast(o1), vtkDataSet::SafeDownCast(o2));

	case VTK_PATH:					//VTK_PATH is in fact VTK_POINT_SET
		return CacheEquals(vtkPointSet::SafeDownCast(o1), vtkPointSet::SafeDownCast(o2));

	default:
		//not supported data types are to be processed as vtkDataObject
		//case VTK_TEMPORAL_DATA_SET:		//no longer used
		//case VTK_GRAPH:					//derived from VTK_DATA_OBJECT
		//case VTK_DIRECTED_GRAPH:			//derived from VTK_GRAPH
		//case VTK_UNDIRECTED_GRAPH:		//derived from VTK_GRAPH
		//case VTK_TREE:					//derived from VTK_DIRECTED_ACYCLIC_GRAPH
		//case VTK_SELECTION:				//derived from VTK_DATA_OBJECT
		//case VTK_MULTIPIECE_DATA_SET:		//derived in fact from VTK_COMPOSITE_DATA_SET
		//case VTK_DIRECTED_ACYCLIC_GRAPH:	//derived from VTK_DIRECTED_GRAPH 
		//case VTK_ARRAY_DATA:				//derived from VTK_DATA_OBJECT
		//case VTK_REEB_GRAPH:				//derived in fact from VTK_DIRECTED_GRAPH
		//case VTK_UNIFORM_GRID_AMR:		//derived from VTK_COMPOSITE_DATA_SET
		//case VTK_NON_OVERLAPPING_AMR:		//derived from VTK_UNIFORM_GRID_AMR
		//case VTK_OVERLAPPING_AMR:			//derived from VTK_UNIFORM_GRID_AMR
		//case VTK_MOLECULE:				//derived from VTK_UNDIRECTED_GRAPH
		//case VTK_PISTON_DATA_OBJECT:		//no longer used
		//case VTK_COMPOSITE_DATA_SET:		//there is no difference in comparison with vtkDataObject
		//case VTK_PIECEWISE_FUNCTION:		//derived from VTK_DATA_OBJECT
		//case VTK_MULTIGROUP_DATA_SET:		//no longer exist
		//case VTK_MULTIBLOCK_DATA_SET:		//derived in fact from VTK_COMPOSITE_DATA_SET
		//case VTK_HIERARCHICAL_DATA_SET:	//no longer exist	
		//case VTK_HIERARCHICAL_BOX_DATA_SET:		//derived from VTK_OVERLAPPING_AMR
		//case VTK_TABLE:					//derived from VTK_DATA_OBJECT
		//case VTK_GENERIC_DATA_SET:		//derived from VTK_DATA_OBJECT
		//case VTK_DATA_OBJECT:
		break;
	}

	return CacheEquals(o1->GetFieldData(), o2->GetFieldData());
}

bool CacheUtils::CacheEquals(vtkAbstractArray* arr1, vtkAbstractArray* arr2)
{
	if (arr1->GetDataSize() != arr2->GetDataSize())
		return false;
	if (arr1->GetDataTypeSize() != arr2->GetDataTypeSize())
		return false;
	int size = arr1->GetDataSize() * arr1->GetDataTypeSize();
#ifndef __VTK_WRAP__
	return memcmp(arr1->GetVoidPointer(0), arr2->GetVoidPointer(0), size) == 0;
#else
	std::shared_ptr<void> array1(new int8_t[size], std::default_delete<int8_t[]>());
	std::shared_ptr<void> array2(new int8_t[size], std::default_delete<int8_t[]>());
	arr1->ExportToVoidPointer(array1.get());
	arr2->ExportToVoidPointer(array2.get());
	return memcmp(array1.get(), array2.get(), size) == 0;
#endif
}

bool CacheUtils::CacheEquals(vtkFieldData* data1, vtkFieldData* data2)
{
	if (data1->GetNumberOfArrays() != data2->GetNumberOfArrays())
		return false;
	for (int i = 0; i < data1->GetNumberOfArrays(); i++)
	{
		if (!CacheEquals(data1->GetAbstractArray(i), data2->GetAbstractArray(i)))
			return false;
	}
	return true;
}

bool CacheUtils::CacheEquals(vtkPolyData* data1, vtkPolyData* data2)
{
	return CacheEquals((vtkPointSet*)data1, (vtkPointSet*)data2) &&
		CacheEquals(data1->GetVerts()->GetData(), data2->GetVerts()->GetData()) &&
		CacheEquals(data1->GetPolys()->GetData(), data2->GetPolys()->GetData()) &&
		CacheEquals(data1->GetLines()->GetData(), data2->GetLines()->GetData()) &&
		CacheEquals(data1->GetStrips()->GetData(), data2->GetStrips()->GetData());		
}


bool CacheUtils::CacheEquals(vtkRectilinearGrid* data1, vtkRectilinearGrid* data2)
{
	if (data1->GetDataDimension() != data2->GetDataDimension())
		return false;
	int dimensions1[3];
	int dimensions2[3];
	data1->GetDimensions(dimensions1);
	data2->GetDimensions(dimensions2);
	bool dimensionsFit = data1->GetDataDimension() == 2 ?
		(dimensions1[0] == dimensions2[0] && dimensions1[1] == dimensions2[1]) :
		(dimensions1[0] == dimensions2[0] && dimensions1[1] == dimensions2[1] && dimensions1[2] == dimensions2[2]);
	return dimensionsFit &&
		CacheEquals(data1->GetXCoordinates(), data2->GetXCoordinates()) &&
		CacheEquals(data1->GetYCoordinates(), data2->GetYCoordinates()) &&
		CacheEquals(data1->GetZCoordinates(), data2->GetZCoordinates()) &&
		CacheEquals((vtkDataSet*)data1, (vtkDataSet*)data2);		
}

bool CacheUtils::CacheEquals(vtkStructuredGrid* data1, vtkStructuredGrid* data2)
{
	if (data1->GetDataDimension() != data2->GetDataDimension())
		return false;
	int dimensions1[3];
	int dimensions2[3];
	data1->GetDimensions(dimensions1);
	data2->GetDimensions(dimensions2);
	bool dimensionsFit = data1->GetDataDimension() == 2 ?
		(dimensions1[0] == dimensions2[0] && dimensions1[1] == dimensions2[1]) :
		(dimensions1[0] == dimensions2[0] && dimensions1[1] == dimensions2[1] && dimensions1[2] == dimensions2[2]);
	return dimensionsFit &&
		CacheEquals((vtkPointSet*)data1, (vtkPointSet*)data2);
}

bool CacheUtils::CacheEquals(vtkImageData* data1, vtkImageData* data2)
{
	if (data1->GetDataDimension() != data2->GetDataDimension())
		return false;
	int dimensions1[3];
	int dimensions2[3];
	data1->GetDimensions(dimensions1);
	data2->GetDimensions(dimensions2);
	bool dimensionsFit = data1->GetDataDimension() == 2 ?
		(dimensions1[0] == dimensions2[0] && dimensions1[1] == dimensions2[1]) :
		(dimensions1[0] == dimensions2[0] && dimensions1[1] == dimensions2[1] && dimensions1[2] == dimensions2[2]);
	double spacing1[3];
	double spacing2[3];
	data1->GetSpacing(spacing1);
	data2->GetSpacing(spacing2);
	bool spacingFits = data1->GetDataDimension() == 2 ?
		(spacing1[0] == spacing2[0] && spacing1[1] == spacing2[1]) :
		(spacing1[0] == spacing2[0] && spacing1[1] == spacing2[1] && spacing1[2] == spacing2[2]);
	double origin1[3];
	double origin2[3];
	data1->GetOrigin(origin1);
	data2->GetOrigin(origin2);
	bool originFits = data1->GetDataDimension() == 2 ?
		(origin1[0] == origin2[0] && origin1[1] == origin2[1]) :
		(origin1[0] == origin2[0] && origin1[1] == origin2[1] && origin1[2] == origin2[2]);
	return dimensionsFit && spacingFits && originFits &&
		CacheEquals((vtkDataSet*)data1, (vtkDataSet*)data2);
}

void CacheUtils::CacheInit(vtkDataObject* source, vtkDataObject* & dataToInit)
{
	if (source == nullptr)
	{
		dataToInit = nullptr;
		return;
	}

	//BES: 13.9.2017 - NewInstance method calls the protected virtual NewInstanceInternal
	//method which ensures that we will get an instance of the correct derived type 
	//and since DeepCopy is a virtual method we well get the correct copy
	dataToInit = source->NewInstance();
	dataToInit->DeepCopy(source);
}

void CacheUtils::CacheInit(vtkInformationVector* source, vtkInformationVector*& dataToInit)
{
	dataToInit = source->NewInstance();
	dataToInit->Copy(source, 1);			//copies everything

	//BES: 13.9.2017 - current implementation of Copy method does not perform
	//deep copying of the data, which is what we need => we need to do it ourselves
	int n = source->GetNumberOfInformationObjects();
	for (int j = 0; j < n; j++)
	{
		vtkInformation* info = source->GetInformationObject(j);
		vtkDataObject* o = info->Get(vtkDataObject::DATA_OBJECT());
		
		vtkDataObject* oDest;
		CacheInit(o, oDest);

		info = dataToInit->GetInformationObject(j);
		info->Set(vtkDataObject::DATA_OBJECT(), oDest);

		oDest->UnRegister(nullptr);	//no longer needed
	}
}

size_t CacheUtils::CacheGetSize(vtkDataObject* obj)
{
	return obj->GetActualMemorySize() * 1024;
}

size_t CacheUtils::CacheGetSize(vtkInformationVector** a, int n)
{
	size_t sizeRet = 0;
	for (int i = 0; i < n; i++) {
		sizeRet += CacheGetSize(a[i]);
	}

	return sizeRet;
}

size_t CacheUtils::CacheGetSize(vtkInformationVector* a)
{
	size_t sizeRet = 0;

	int n = a->GetNumberOfInformationObjects();
	for (int j = 0; j < n; j++)
	{
		vtkInformation* info = a->GetInformationObject(j);
		vtkDataObject* o = info->Get(vtkDataObject::DATA_OBJECT());
		sizeRet += CacheUtils::CacheGetSize(o);  //gets the size in bytes using the predefined function from CacheUtils
	}

	return sizeRet;
}

size_t CacheUtils::CacheHash(vtkInformationVector** infoVecs, int n)
{
	size_t hash = 0;
	for (int i = 0; i < n; i++) {
		CacheSystem::hash_combine_hvs(hash, CacheHash(infoVecs[i]));
	}

	return hash;
}

size_t CacheUtils::CacheHash(vtkInformationVector* infoVec)
{
	size_t hash = 0;
	int n = infoVec->GetNumberOfInformationObjects();
	for (int i = 0; i < n; i++)
	{
		CacheSystem::hash_combine_hvs(hash,
			CacheHash(infoVec->GetInformationObject(i)
				->Get(vtkDataObject::DATA_OBJECT())));
	}
	return hash;
}

/**
this function calculates the hash of the vtkDataObject
N.B. this is a general function with performance impact and, therefore,
its use should be avoided, if possible
*/
size_t CacheUtils::CacheHash(vtkDataObject* o)
{
	//detect the concrete class of o and use the
	//predefined implementation for it, if available
	//the order of tests is by their frequencies of use

	switch (o->GetDataObjectType())
	{
	case VTK_POLY_DATA:
		return CacheHash(vtkPolyData::SafeDownCast(o));
			
	case VTK_STRUCTURED_POINTS:	//vtkStructuredPoints is in fact vtkImageData	
		return CacheHash(vtkImageData::SafeDownCast(o));
	
	case VTK_STRUCTURED_GRID:
		return CacheHash(vtkStructuredGrid::SafeDownCast(o));	
		
	case VTK_RECTILINEAR_GRID:
		return CacheHash(vtkRectilinearGrid::SafeDownCast(o));

	case VTK_UNSTRUCTURED_GRID:
		return CacheHash(vtkUnstructuredGrid::SafeDownCast(o));	
	
	case VTK_IMAGE_DATA:
		return CacheHash(vtkImageData::SafeDownCast(o));			

	case VTK_DATA_OBJECT:
		break;	//processed later in this method

	case VTK_DATA_SET:
		return CacheHash(vtkDataSet::SafeDownCast(o));

	case VTK_POINT_SET:
		return CacheHash(vtkPointSet::SafeDownCast(o));

	case VTK_UNIFORM_GRID:		//vtkUniformGrid is in fact vtkImageData
		return CacheHash(vtkImageData::SafeDownCast(o));
			
	case VTK_UNSTRUCTURED_GRID_BASE:	//there is no difference in comparison with vtkPointSet
		return CacheHash(vtkPointSet::SafeDownCast(o));
			
	case VTK_HYPER_OCTREE:		
	case VTK_HYPER_TREE_GRID:	//not supported but nearest is vtkDataSet
		CacheHash(vtkDataSet::SafeDownCast(o));

	case VTK_PATH:					//VTK_PATH is in fact VTK_POINT_SET
		return CacheHash(vtkPointSet::SafeDownCast(o));		

	default:
		//not supported data types are to be processed as vtkDataObject
		//case VTK_TEMPORAL_DATA_SET:		//no longer used
		//case VTK_GRAPH:					//derived from VTK_DATA_OBJECT
		//case VTK_DIRECTED_GRAPH:			//derived from VTK_GRAPH
		//case VTK_UNDIRECTED_GRAPH:		//derived from VTK_GRAPH
		//case VTK_TREE:					//derived from VTK_DIRECTED_ACYCLIC_GRAPH
		//case VTK_SELECTION:				//derived from VTK_DATA_OBJECT
		//case VTK_MULTIPIECE_DATA_SET:		//derived in fact from VTK_COMPOSITE_DATA_SET
		//case VTK_DIRECTED_ACYCLIC_GRAPH:	//derived from VTK_DIRECTED_GRAPH 
		//case VTK_ARRAY_DATA:				//derived from VTK_DATA_OBJECT
		//case VTK_REEB_GRAPH:				//derived in fact from VTK_DIRECTED_GRAPH
		//case VTK_UNIFORM_GRID_AMR:		//derived from VTK_COMPOSITE_DATA_SET
		//case VTK_NON_OVERLAPPING_AMR:		//derived from VTK_UNIFORM_GRID_AMR
		//case VTK_OVERLAPPING_AMR:			//derived from VTK_UNIFORM_GRID_AMR
		//case VTK_MOLECULE:				//derived from VTK_UNDIRECTED_GRAPH
		//case VTK_PISTON_DATA_OBJECT:		//no longer used
		//case VTK_COMPOSITE_DATA_SET:		//there is no difference in comparison with vtkDataObject
		//case VTK_PIECEWISE_FUNCTION:		//derived from VTK_DATA_OBJECT
		//case VTK_MULTIGROUP_DATA_SET:		//no longer exist
		//case VTK_MULTIBLOCK_DATA_SET:		//derived in fact from VTK_COMPOSITE_DATA_SET
		//case VTK_HIERARCHICAL_DATA_SET:	//no longer exist	
		//case VTK_HIERARCHICAL_BOX_DATA_SET:		//derived from VTK_OVERLAPPING_AMR
		//case VTK_TABLE:					//derived from VTK_DATA_OBJECT
		//case VTK_GENERIC_DATA_SET:		//derived from VTK_DATA_OBJECT
		//case VTK_DATA_OBJECT:
		break;
	}

	return CacheHash(o->GetFieldData());
}

size_t CacheUtils::CacheHash(vtkAbstractArray* arr)
{
	typedef float DataType;
	int size = (int)(arr->GetDataSize() - sizeof(DataType));
	if (size < 4)
		return 0;
	DataType* values[15] = {
		(DataType*)arr->GetVoidPointer(0), (DataType*)arr->GetVoidPointer(1),
		(DataType*)arr->GetVoidPointer(2), (DataType*)arr->GetVoidPointer(size / 2 - 1),
		(DataType*)arr->GetVoidPointer(size / 2), (DataType*)arr->GetVoidPointer(size / 2 + 1),
		(DataType*)arr->GetVoidPointer(size - 1), (DataType*)arr->GetVoidPointer(size - 2),
		(DataType*)arr->GetVoidPointer(size - 3), (DataType*)arr->GetVoidPointer(size / 4 - 1),
		(DataType*)arr->GetVoidPointer(size / 4), (DataType*)arr->GetVoidPointer(size / 4 + 1),
		(DataType*)arr->GetVoidPointer(size - size / 4 - 2), (DataType*)arr->GetVoidPointer(size - size / 4 - 1),
		(DataType*)arr->GetVoidPointer(size - size / 4)
	};
	int dummy;
	DataType hash = 0;
	for (int i = 0; i < 15; i++)
		hash += std::frexp(*values[i], &dummy);
	hash *= 10e8;
	return (size_t)hash;
}

size_t CacheUtils::CacheHash(vtkFieldData* data)
{
	size_t hash = 0;
	for (int i = 0; i < data->GetNumberOfArrays(); i++)
	{
		CacheSystem::hash_combine_hvs(hash, CacheHash(data->GetAbstractArray(i)));
	}
	return hash;
}

size_t CacheUtils::CacheHash(vtkPointSet* data)
{
	return
		CacheSystem::hash_combine_hvs(
			CacheHash(data->GetPoints()->GetData()),
			CacheHash((vtkDataSet*)data)
		);
}

size_t CacheUtils::CacheHash(vtkPolyData* data)
{
	return
		CacheSystem::hash_combine_hvs(
			CacheHash((vtkPointSet*)data),
			CacheHash(data->GetPolys()->GetData()), CacheHash(data->GetVerts()->GetData()),
			CacheHash(data->GetLines()->GetData()), CacheHash(data->GetStrips()->GetData())			
			);
}

size_t CacheUtils::CacheHash(vtkUnstructuredGrid* data)
{
	return CacheSystem::hash_combine_hvs(
		CacheHash((vtkPointSet*)data),
		CacheHash(data->GetCells()->GetData())		
			);
}

size_t CacheUtils::CacheHash(vtkRectilinearGrid* data)
{
	return CacheSystem::hash_combine_hvs(
		CacheHash(data->GetXCoordinates()), CacheHash(data->GetYCoordinates()), CacheHash(data->GetZCoordinates()),
		CacheHash((vtkDataSet*)data)
		);
}