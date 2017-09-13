#ifndef _CACHE_UTILS_H
#define _CACHE_UTILS_H

#include <vtkAbstractArray.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRectilinearGrid.h>
#include <vtkDataSetAttributes.h>
#include <vtkStructuredGrid.h>
#include <vtkImageData.h>
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
	static uint64_t CacheGetSize(vtkDataObject* obj);

	/** 
	Gets the overall size of all data objects present in the given array of information vectors 
	Size of all other metadata is not calculated.
	\param n number of entries in the passed array of information vectors
	*/
	static uint64_t CacheGetSize(vtkInformationVector** a, int n);

	/** 
	Gets the overall size of all data objects present in the given array of information vectors .
	Size of all other metadata is not calculated.
	*/
	static uint64_t CacheGetSize(vtkInformationVector* a);

	/**
	this function calculates the hash of an instance of vtkAbstractArray
	*/
	static uint32_t CacheHash(vtkAbstractArray* arr);

	/**
	this function calculates the hash of an instance of vtkDataSetAttributes
	*/
	static uint32_t CacheHash(vtkDataSetAttributes* arr);

	/**
	this function calculates the hash of an instance of vtkPolyData
	*/
	static uint32_t CacheHash(vtkPolyData* data);

	/**
	this function calculates the hash of an instance of vtkUnstructuredGrid
	*/
	static uint32_t CacheHash(vtkUnstructuredGrid* data);

	/**
	this function calculates the hash of an instance of vtkRectilinearGrid
	*/
	static uint32_t CacheHash(vtkRectilinearGrid* data);

	/**
	this function calculates the hash of an instance of vtkStructuredGrid
	*/
	static uint32_t CacheHash(vtkStructuredGrid* data);

	/**
	this function calculates the hash of an instance of vtkImageData
	*/
	static uint32_t CacheHash(vtkImageData* data);
};

#endif