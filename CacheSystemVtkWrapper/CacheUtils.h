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

/**
this class contais predefined data manipulation functions for the most commonly used data types in VTK
*/
class CacheUtils
{
public:
	/**
	this function compares two instances of vtkAbstractArray
	*/
	static bool CacheEquals(vtkAbstractArray* arr1, vtkAbstractArray* arr2);

	/**
	this function compares two instances of vtkDataSetAttributes
	*/
	static bool CacheEquals(vtkDataSetAttributes* data1, vtkDataSetAttributes* data2);

	/**
	this function compares two instances of vtkPolyData
	*/
	static bool CacheEquals(vtkPolyData* data1, vtkPolyData* data2);

	/**
	this function compares two instances of vtkUnstructuredGrid
	*/
	static bool CacheEquals(vtkUnstructuredGrid* data1, vtkUnstructuredGrid* data2);

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
	static size_t CacheHash(vtkDataSet* o);

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
	static size_t CacheHash(vtkStructuredGrid* data);

	/**
	this function calculates the hash of an instance of vtkImageData
	*/
	static size_t CacheHash(vtkImageData* data);
};

#endif