/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: vtkLHPSelectionVolumeGray.h,v $
Language:  C++
Date:      $Date: 2010-06-03 16:03:24 $
Version:   $Revision: 1.1.2.5 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2009
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpSelectionVolumeGray_H__
#define __lhpSelectionVolumeGray_H__

#include "vtkStructuredPoints.h"

//------------------------------------------------------------------------------ 
// Forward declarations 
//------------------------------------------------------------------------------ 
class vtkLHPImageStencilData;

/**
  an unsigned char selection volume used by VolumeBrush
  supporting select, unselect and clear operations
*/
class vtkLHPSelectionVolumeGray  : public vtkStructuredPoints
{
public:
	vtkTypeRevisionMacro(vtkLHPSelectionVolumeGray,vtkStructuredPoints);

	static vtkLHPSelectionVolumeGray * New();
	
	vtkLHPSelectionVolumeGray();
	~vtkLHPSelectionVolumeGray();

	/** create an empty selection volume with give dimension */
	void Create(int dim[3], double spacing[3], double origin[3]);
	
	/** select the stencil with given label */
	void Select(vtkLHPImageStencilData * stencil, unsigned char label);

	/** select by thresholding */
	void SelectByThreshold(vtkImageData * volume, double lower, double upper, unsigned char label);
	
	/** unselect the stencil */
	void UnSelect(vtkLHPImageStencilData * stencil);
	
	/** clear the selection volume with the background value*/
	void Clear();

protected:
	/** the background voxel value */
	unsigned char m_BackgroundValue;
	
	/** get the voxel data pointer */
	unsigned char * GetDataPointer();

private:

};



#endif

