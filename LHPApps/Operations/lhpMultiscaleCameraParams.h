/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleCameraParams.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpMultiscaleCameraParams_H__
#define __lhpMultiscaleCameraParams_H__

#include "vtkRenderer.h"
#include <iostream>


/*******************************************************************************
Camera params class for saving camera parameters
*******************************************************************************/

class lhpMultiscaleCameraParams{
public:
  lhpMultiscaleCameraParams() ;                                                 ///< Default constructor
  lhpMultiscaleCameraParams(vtkRenderer *renderer) ;                            ///< constructor from renderer
  lhpMultiscaleCameraParams(const lhpMultiscaleCameraParams& cameraParams) ;    ///< Copy constructor
  void SaveCamera(vtkRenderer *renderer) ;                                      ///< Save camera parameters
  void RestoreCamera(vtkRenderer* renderer) ;                                   ///< Restore camera parameters
  void PrintSelf(std::ostream& os, vtkIndent indent) const ;                    ///< Print parameters
  bool operator==(const lhpMultiscaleCameraParams& cameraParams) ;              ///< == operator
private:
  double m_pos[3] ;
  double m_focalPoint[3] ;
  double m_viewUp[3] ;
  double m_viewAngle ;
  int m_parallelProjection ;
  double m_parallelScale ;
  double m_clippingPlanes[2] ;
} ;

#endif