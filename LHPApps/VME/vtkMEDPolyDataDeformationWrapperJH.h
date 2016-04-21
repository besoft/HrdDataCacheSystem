/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medVMEMuscleWrapper.h,v $
Language:  C++
Date:      $Date: 2012-04-30 14:52:43 $
Version:   $Revision: 1.1.1.1.2.20 $
Authors:   Jana Hajkova
=========================================================================*/
#ifndef __vtkMEDPolyDataDeformationWrapperJH_h
#define __vtkMEDPolyDataDeformationWrapperJH_h
#endif

#include "vtkMEDPolyDataDeformation.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"

class WrapperInterpoler;


class WrapperInterpoler : public vtkMEDPolyDataDeformation {
public:
	WrapperInterpoler() : vtkMEDPolyDataDeformation() {};
	static void MatchCurves(vtkPolyData* startWrapper, vtkPolyData* endWrapper);
	void MyMatchCurves(vtkPolyData* startWrapper, vtkPolyData* endWrapper);
};

#define DIST_LIMIT 10
