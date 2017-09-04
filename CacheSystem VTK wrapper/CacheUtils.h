#ifndef _CACHE_UTILS_H
#define _CACHE_UTILS_H

#include <vtkAbstractArray.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRectilinearGrid.h>
#include <vtkDataSetAttributes.h>
#include <vtkStructuredGrid.h>
#include <vtkImageData.h>

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
	this function copies one instance of vtkPolyData to another
	*/
	static void CacheInit(vtkPolyData* source, vtkPolyData* & dataToInit);

	/**
	this function copies one instance of vtkUnstructuredGrid to another
	*/
	static void CacheInit(vtkUnstructuredGrid* source, vtkUnstructuredGrid* & dataToInit);

	/**
	this function copies one instance of vtkRectilinearGrid to another
	*/
	static void CacheInit(vtkRectilinearGrid* source, vtkRectilinearGrid* & dataToInit);

	/**
	this function copies one instance of vtkStructuredGrid to another
	*/
	static void CacheInit(vtkStructuredGrid* source, vtkStructuredGrid* & dataToInit);

	/**
	this function copies one instance of vtkImageData to another
	*/
	static void CacheInit(vtkImageData* source, vtkImageData* & dataToInit);

	/**
	this function destroys an instance of vtkDataObject
	*/
	static void CacheDestroy(vtkDataObject* obj);

	/**
	this function calculates size in bytes of an instance of vtkDataObject
	*/
	static uint64_t CacheGetSize(vtkDataObject* obj);

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