/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpVolumeInterpolator.h,v $
  Language:  C++
  Date:      $Date: 2011-12-12 12:33:41 $
  Version:   $Revision: 1.1.1.1.2.2 $
  Authors:   Gianluigi Crimi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/


#include "lhpOpBonematCommon.h"


class vtkDataSet;
class vtkDataArray; 

class lhpVolumeInterpolator: public mafObject
{
public:

	mafTypeMacro(lhpVolumeInterpolator, mafObject);

	void SetVolume(vtkDataSet *volume);

	double Interpolate(double pos[3], double (*valueFunction)(double) = NULL);

protected:

	double InternalInterpolate(double (*valueFunction)(double));

	double InterpolateSP(double pos[3], double (*valueFunction)(double));
	double InterpolateRG(double pos[3], double (*valueFunction)(double));
	bool m_IsRectiliearGrid;

	vtkDataSet *m_Volume;
	vtkDataArray *m_Scalars;

	int m_Dims[3];

	double m_Weights[3];
	unsigned int m_Ids[6];
		
};