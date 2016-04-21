/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: vtkLHPImageStencilData.h,v $
Language:  C++
Date:      $Date: 2010-04-16 12:20:09 $
Version:   $Revision: 1.1.2.5 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2009
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lhpImageStencilData_H__
#define __lhpImageStencilData_H__

#include "vtkImageStencilData.h"


//------------------------------------------------------------------------------ 
// Forward declarations 
//------------------------------------------------------------------------------ 

class vtkSphereSource;
class vtkCubeSource;

/**
work-around for using Add and Subtract functions in VTK5 
vtkImageStencilData for VolumeBrush
Also added translation support to reuse brush stencils.
*/
class vtkLHPImageStencilData : public vtkImageStencilData
{
public:

	vtkTypeRevisionMacro(vtkLHPImageStencilData,vtkImageStencilData);

	static vtkLHPImageStencilData *New();

	//-----------------------------------
	// vtk 5 functions
	
	// Description:
	// Add merges the stencil supplied as argument into Self.
	// now is useless
	/** from VTK5 vtkImageStencilData */
	virtual void Add( vtkImageStencilData * ); 

	/** from VTK5 vtkImageStencilData */
	void InsertAndMergeExtent(int r1, int r2, int yIdx, int zIdx);

	// Description:
	// Subtract removes the portion of the stencil, supplied as argument, 
	// that lies within Self from Self. 
	// now is useless
	/** from VTK5 vtkImageStencilData */
	virtual void Subtract( vtkImageStencilData * ); 

	// Description:
	// Replaces the portion of the stencil, supplied as argument, 
	// that lies within Self from Self. 
	// now is useless
	/** from VTK5 vtkImageStencilData */
	virtual void Replace( vtkImageStencilData * ); 

	// Description:
	// Clip the stencil with the supplied extents. In other words, discard data
	// outside the specified extents. Return 1 if something changed.
	//virtual int Clip( int extent[6] );

	// Description:
	// Remove the extent from (r1,r2) at yIdx, zIdx
	// now is useless
	/** from VTK5 vtkImageStencilData */
	void RemoveExtent(int r1, int r2, int yIdx, int zIdx);




	//-----------------------------------
	// custom functions

	// Seperating adding and extent allocating to avoid performance
	// slowdown during brush painting
	/** reize the extent inadvance to avoid frequent DeepCopy in Add()*/
	void AddAndResizeExtent(int extent1[6]);

	/** Add with extent already allocated, it esstentially calls 
	InternalAdd()
	*/
	void AddWithExtentAllocated( vtkImageStencilData * ); 


	/** starting  recording translations */
	void StartRecordTranslation();
	/** rolling back recorded translation */
	void RollbackRecordedTranslation();
	/** stop recording translation */
	void StopRecordTranslation();

	/* translating the stencil data avoiding regeneration */
	void ApplyTranslation(int dx, int dy, int dz);


	/** Fast generation of a cube stencil */
	void GenBoxStencil(vtkCubeSource *);

	/** Fast generation of a sphere stencil */
	void GenSphereStencil(vtkSphereSource *);

	// TODO: Fast generation of a cylinder stencil 
	//void GenCylinderStencil(int r, int h, int direct, int center[3]);

	/** filling an extent with give lable */
	void Fill(unsigned char * ptr, int incr[3], unsigned char label, int boundExtent[6]);

	/* Clear the stencil */
	void Clear();

protected:
	vtkLHPImageStencilData();
	~vtkLHPImageStencilData();

	/** from VTK5 vtkImageStencilData */
	virtual void InternalAdd(vtkImageStencilData * );

	/** from VTK5 vtkImageStencilData */
	void CollapseAdditionalIntersections(int r2, int idx, int *clist, int &clistlen);

	// do we need it??
	//int m_Translation[3];

	/* Is recording translation */
	bool m_bRecordTranslation;
	
	/* for translation rollback */
	int m_RecordedTranslation[3];

private:
	// from vtk5 vtkMath
	int ExtentIsWithinOtherExtent(int extent1[6], int extent2[6]);

};

template<class T>
inline void lhpMemSet(T *out, const T val, int len)
{
	while (len--)
	{
		*out++ = val;
	} 
}

#endif
