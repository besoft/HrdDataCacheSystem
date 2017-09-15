#ifndef _CACHE_UTILS_H
#define _CACHE_UTILS_H

#include <vtkAbstractArray.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRectilinearGrid.h>
#include <vtkDataSetAttributes.h>
#include <vtkStructuredGrid.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "Hash.h"

/**
this class contains predefined data manipulation functions for the most commonly used data types in VTK
*/
class CacheUtils
{
public:
	/**
	this function compares two arrays of information vectors
	N.B. this is a general function with performance impact and, therefore,
	its use should be avoided, if possible
	\param n number of entries in the passed array of information vectors
	*/
	static bool CacheEquals(vtkInformationVector** infoVecs1, vtkInformationVector** infoVecs2, int n);

	/**
	this function compares two instances of vtkInformationVector
	N.B. this is a general function with performance impact and, therefore,
	its use should be avoided, if possible
	*/
	static bool CacheEquals(vtkInformationVector* infoVec1, vtkInformationVector* infoVec2);

	/**
	this function compares two instances of vtkDataObject
	N.B. this is a general function with performance impact and, therefore,
	its use should be avoided, if possible
	*/
	static bool CacheEquals(vtkDataObject* o1, vtkDataObject* o2);

	/**
	this function compares two instances of vtkAbstractArray
	*/
	static bool CacheEquals(vtkAbstractArray* arr1, vtkAbstractArray* arr2);

	/**
	this function compares two instances of vtkDataSetAttributes
	*/
	static bool CacheEquals(vtkFieldData* data1, vtkFieldData* data2);

	/**
	this function compares two instances of vtkDataSet
	*/
	inline static bool CacheEquals(vtkDataSet* data1, vtkDataSet* data2)
	{
		return CacheEquals(data1->GetPointData(), data2->GetPointData()) &&
			CacheEquals(data1->GetCellData(), data2->GetCellData());
	}

	/**
	this function compares two instances of vtkPointSet
	*/
	inline static bool CacheEquals(vtkPointSet* data1, vtkPointSet* data2)
	{
		return CacheEquals(data1->GetPoints()->GetData(), data2->GetPoints()->GetData()) &&
			CacheEquals((vtkDataSet*)data1, (vtkDataSet*)data2);
	}

	/**
	this function compares two instances of vtkPolyData
	*/
	static bool CacheEquals(vtkPolyData* data1, vtkPolyData* data2);

	/**
	this function compares two instances of vtkUnstructuredGrid
	*/
	inline static bool CacheEquals(vtkUnstructuredGrid* data1, vtkUnstructuredGrid* data2)
	{
		return CacheEquals((vtkPointSet*)data1, (vtkPointSet*)data2) &&
			CacheEquals(data1->GetCells()->GetData(), data2->GetCells()->GetData());
	}

	/**
	this function compares two instances of vtkRectilinearGrid
	*/
	static bool CacheEquals(vtkRectilinearGrid* data1, vtkRectilinearGrid* data2);

	/**
	this function compares two instances of vtkStructuredGrid
	*/
	static bool CacheEquals(vtkStructuredGrid* data1, vtkStructuredGrid* data2);

	/**
	this function compares two instances of vtkImageData
	*/
	static bool CacheEquals(vtkImageData* data1, vtkImageData* data2);

	/**
	this function copies one instance of vtkDataObject
	*/
	static void CacheInit(vtkDataObject* source, vtkDataObject* & dataToInit);	

	/**
	initialize dataToInit array and deep copies the data from the source to it
	\param n number of entries in the source array of information vectors
	*/
	static void CacheInit(vtkInformationVector** source, vtkInformationVector**& dataToInit, int n)
	{
		dataToInit = new vtkInformationVector*;
		for (int i = 0; i < n; i++) {
			CacheInit(source[i], dataToInit[i]);
		}
	}

	/**
	initialize dataToInit array and deep copies the data from the source to it	
	*/
	static void CacheInit(vtkInformationVector* source, vtkInformationVector*& dataToInit);	

	/**
	this function destroys an instance of vtkDataObject
	*/
	static void CacheDestroy(vtkDataObject* obj)
	{
		if (obj != nullptr)
			obj->Delete();
	}

	/**
	this function destroys an array of information vectors
	*/
	static void CacheDestroy(vtkInformationVector** infoVecs, int n)
	{
		for (int i = 0; i < n; i++) {
			CacheDestroy(infoVecs[i]);
		}

		delete[] infoVecs;
	}

	/**
	this function destroys an information vector
	*/
	static void CacheDestroy(vtkInformationVector* infoVec)
	{
		infoVec->Delete();
	}

	/**
	this function calculates size in bytes of an instance of vtkDataObject
	*/
	static size_t CacheGetSize(vtkDataObject* obj);

	/** 
	Gets the overall size of all data objects present in the given array of information vectors 
	Size of all other metadata is not calculated.
	\param n number of entries in the passed array of information vectors
	*/
	static size_t CacheGetSize(vtkInformationVector** a, int n);

	/** 
	Gets the overall size of all data objects present in the given array of information vectors .
	Size of all other metadata is not calculated.
	*/
	static size_t CacheGetSize(vtkInformationVector* a);


	/**
	this function calculates the hash of the given array of information vectors 
	N.B. this is a general function with performance impact and, therefore,
	its use should be avoided, if possible
	\param n number of entries in the passed array of information vectors
	*/
	static size_t CacheHash(vtkInformationVector** infoVecs, int n);

	/**
	this function calculates the hash of the given information vector
	N.B. this is a general function with performance impact and, therefore,
	its use should be avoided, if possible	
	*/
	static size_t CacheHash(vtkInformationVector* infoVec);	

	/**
	this function calculates the hash of the vtkDataObject
	N.B. this is a general function with performance impact and, therefore,
	its use should be avoided, if possible
	*/
	static size_t CacheHash(vtkDataObject* o);

	/**
	this function calculates the hash of an instance of vtkDataSet
	*/
	inline static size_t CacheHash(vtkDataSet* data)
	{
		return
			CacheSystem::hash_combine_hvs(
				CacheHash(data->GetPointData()), CacheHash(data->GetCellData())
			);
	}

	/**
	this function calculates the hash of an instance of vtkAbstractArray
	*/
	static size_t CacheHash(vtkAbstractArray* arr);

	/**
	this function calculates the hash of an instance of vtkFieldData (and vtkDataSetAttributes)
	*/
	static size_t CacheHash(vtkFieldData* arr);

	/**
	this function calculates the hash of an instance of vtkPointSet
	*/
	static size_t CacheHash(vtkPointSet* data);

	/**
	this function calculates the hash of an instance of vtkPolyData
	*/
	static size_t CacheHash(vtkPolyData* data);

	/**
	this function calculates the hash of an instance of vtkUnstructuredGrid
	*/
	static size_t CacheHash(vtkUnstructuredGrid* data);

	/**
	this function calculates the hash of an instance of vtkRectilinearGrid
	*/
	static size_t CacheHash(vtkRectilinearGrid* data);

	/**
	this function calculates the hash of an instance of vtkStructuredGrid
	*/
	inline static size_t CacheHash(vtkStructuredGrid* data)
	{
		return CacheHash((vtkPointSet*)data);
	}

	/**
	this function calculates the hash of an instance of vtkImageData
	*/
	inline static size_t CacheHash(vtkImageData* data)
	{
		return CacheHash((vtkDataSet*)data);
	}
};

#endif