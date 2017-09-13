#include "CacheUtils.h"
#include "vtkInformation.h"

#include <math.h>
#include <memory>

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

bool CacheUtils::CacheEquals(vtkDataSetAttributes* data1, vtkDataSetAttributes* data2)
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
	return CacheEquals(data1->GetPoints()->GetData(), data2->GetPoints()->GetData()) &&
		CacheEquals(data1->GetVerts()->GetData(), data2->GetVerts()->GetData()) &&
		CacheEquals(data1->GetPolys()->GetData(), data2->GetPolys()->GetData()) &&
		CacheEquals(data1->GetLines()->GetData(), data2->GetLines()->GetData()) &&
		CacheEquals(data1->GetStrips()->GetData(), data2->GetStrips()->GetData()) &&
		CacheEquals((vtkDataSetAttributes*)data1->GetPointData(), (vtkDataSetAttributes*)data2->GetPointData()) &&
		CacheEquals((vtkDataSetAttributes*)data1->GetCellData(), (vtkDataSetAttributes*)data2->GetCellData());
}

bool CacheUtils::CacheEquals(vtkUnstructuredGrid* data1, vtkUnstructuredGrid* data2)
{
	return CacheEquals(data1->GetPoints()->GetData(), data2->GetPoints()->GetData()) &&
		CacheEquals(data1->GetCells()->GetData(), data2->GetCells()->GetData()) &&
		CacheEquals((vtkDataSetAttributes*)data1->GetPointData(), (vtkDataSetAttributes*)data2->GetPointData()) &&
		CacheEquals((vtkDataSetAttributes*)data1->GetCellData(), (vtkDataSetAttributes*)data2->GetCellData());
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
		CacheEquals((vtkDataSetAttributes*)data1->GetPointData(), (vtkDataSetAttributes*)data2->GetPointData()) &&
		CacheEquals((vtkDataSetAttributes*)data1->GetCellData(), (vtkDataSetAttributes*)data2->GetCellData());
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
	return dimensionsFit && CacheEquals(data1->GetPoints()->GetData(), data2->GetPoints()->GetData()) &&
		CacheEquals((vtkDataSetAttributes*)data1->GetPointData(), (vtkDataSetAttributes*)data2->GetPointData()) &&
		CacheEquals((vtkDataSetAttributes*)data1->GetCellData(), (vtkDataSetAttributes*)data2->GetCellData());
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
		CacheEquals((vtkDataSetAttributes*)data1->GetPointData(), (vtkDataSetAttributes*)data2->GetPointData()) &&
		CacheEquals((vtkDataSetAttributes*)data1->GetCellData(), (vtkDataSetAttributes*)data2->GetCellData());
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

uint64_t CacheUtils::CacheGetSize(vtkDataObject* obj)
{
	return obj->GetActualMemorySize() * 1024;
}

uint64_t CacheUtils::CacheGetSize(vtkInformationVector** a, int n)
{
	uint64_t sizeRet = 0;
	for (int i = 0; i < n; i++) {
		sizeRet += CacheGetSize(a[i]);
	}

	return sizeRet;
}

uint64_t CacheUtils::CacheGetSize(vtkInformationVector* a)
{
	uint64_t sizeRet = 0;

	int n = a->GetNumberOfInformationObjects();
	for (int j = 0; j < n; j++)
	{
		vtkInformation* info = a->GetInformationObject(j);
		vtkDataObject* o = info->Get(vtkDataObject::DATA_OBJECT());
		sizeRet += CacheUtils::CacheGetSize(o);  //gets the size in bytes using the predefined function from CacheUtils
	}

	return sizeRet;
}

uint32_t CacheUtils::CacheHash(vtkAbstractArray* arr)
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
	return (uint32_t)hash;
}

uint32_t CacheUtils::CacheHash(vtkDataSetAttributes* data)
{
	uint32_t hash = 0;
	for (int i = 0; i < data->GetNumberOfArrays(); i++)
	{
		hash += CacheHash(data->GetAbstractArray(i));
	}
	return hash;
}

uint32_t CacheUtils::CacheHash(vtkPolyData* data)
{
	return CacheHash(data->GetPoints()->GetData()) + CacheHash(data->GetPolys()->GetData()) + CacheHash(data->GetVerts()->GetData()) +
		CacheHash(data->GetLines()->GetData()) + CacheHash(data->GetStrips()->GetData()) +
		CacheHash((vtkDataSetAttributes*)data->GetPointData()) + CacheHash((vtkDataSetAttributes*)data->GetCellData());
}

uint32_t CacheUtils::CacheHash(vtkUnstructuredGrid* data)
{
	return CacheHash(data->GetPoints()->GetData()) + CacheHash(data->GetCells()->GetData()) +
		CacheHash((vtkDataSetAttributes*)data->GetPointData()) + CacheHash((vtkDataSetAttributes*)data->GetCellData());
}

uint32_t CacheUtils::CacheHash(vtkRectilinearGrid* data)
{
	return CacheHash(data->GetXCoordinates()) + CacheHash(data->GetYCoordinates()) + CacheHash(data->GetZCoordinates()) +
		CacheHash((vtkDataSetAttributes*)data->GetPointData()) + CacheHash((vtkDataSetAttributes*)data->GetCellData());
}

uint32_t CacheUtils::CacheHash(vtkStructuredGrid* data)
{
	return CacheHash(data->GetPoints()->GetData()) +
		CacheHash((vtkDataSetAttributes*)data->GetPointData()) + CacheHash((vtkDataSetAttributes*)data->GetCellData());
}

uint32_t CacheUtils::CacheHash(vtkImageData* data)
{
	return CacheHash((vtkDataSetAttributes*)data->GetPointData()) + CacheHash((vtkDataSetAttributes*)data->GetCellData());
}