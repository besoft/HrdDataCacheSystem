/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleCameraParams.cpp,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "lhpMultiscaleCameraParams.h"
#include "lhpMultiscaleVectorMath.h"
#include <iostream>




//------------------------------------------------------------------------------
// Default constructor
lhpMultiscaleCameraParams::lhpMultiscaleCameraParams()
//------------------------------------------------------------------------------
{
  double zerovec[3] = {0.0, 0.0, 0.0} ;
  lhpMultiscaleVectorMath::CopyVector(zerovec, this->m_pos) ;
  lhpMultiscaleVectorMath::CopyVector(zerovec, this->m_focalPoint) ;
  lhpMultiscaleVectorMath::CopyVector(zerovec, this->m_viewUp) ;
  this->m_viewAngle = 0.0 ;
  this->m_parallelProjection = 0.0 ;
  this->m_parallelScale = 0.0 ;
  this->m_clippingPlanes[0] = 0.0 ;
  this->m_clippingPlanes[1] = 0.0 ;
}


//------------------------------------------------------------------------------
// Constructor from renderer
lhpMultiscaleCameraParams::lhpMultiscaleCameraParams(vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  SaveCamera(renderer) ;
}


//------------------------------------------------------------------------------
// Copy constructor
lhpMultiscaleCameraParams::lhpMultiscaleCameraParams(const lhpMultiscaleCameraParams& cameraParams)
//------------------------------------------------------------------------------
{
  lhpMultiscaleVectorMath::CopyVector(cameraParams.m_pos, this->m_pos) ;
  lhpMultiscaleVectorMath::CopyVector(cameraParams.m_focalPoint, this->m_focalPoint) ;
  lhpMultiscaleVectorMath::CopyVector(cameraParams.m_viewUp, this->m_viewUp) ;
  this->m_viewAngle = cameraParams.m_viewAngle ;
  this->m_parallelProjection = cameraParams.m_parallelProjection ;
  this->m_parallelScale = cameraParams.m_parallelScale ;
  this->m_clippingPlanes[0] = cameraParams.m_clippingPlanes[0] ;
  this->m_clippingPlanes[1] = cameraParams.m_clippingPlanes[1] ;
}


//------------------------------------------------------------------------------
// Save camera parameters
void lhpMultiscaleCameraParams::SaveCamera(vtkRenderer* renderer)
//------------------------------------------------------------------------------
{
  vtkCamera *camera = renderer->GetActiveCamera() ;
  double a[3], r[2] ;

  camera->GetPosition(a) ;
  lhpMultiscaleVectorMath::CopyVector(a, m_pos) ;

  camera->GetFocalPoint(a) ;
  lhpMultiscaleVectorMath::CopyVector(a, m_focalPoint) ;

  camera->GetViewUp(a) ;
  lhpMultiscaleVectorMath::CopyVector(a, m_viewUp) ;

  m_viewAngle = camera->GetViewAngle() ;

  m_parallelProjection = camera->GetParallelProjection() ;

  m_parallelScale = camera->GetParallelScale() ;

  camera->GetClippingRange(r) ;
  m_clippingPlanes[0] = r[0] ;
  m_clippingPlanes[1] = r[1] ;
}


//------------------------------------------------------------------------------
// Restore camera parameters
void lhpMultiscaleCameraParams::RestoreCamera(vtkRenderer* renderer)
//------------------------------------------------------------------------------
{
  vtkCamera *camera = renderer->GetActiveCamera() ;

  camera->SetPosition(m_pos) ;
  camera->SetFocalPoint(m_focalPoint) ;
  camera->SetViewUp(m_viewUp) ;
  camera->SetViewAngle(m_viewAngle) ;
  camera->SetParallelProjection(m_parallelProjection) ;
  camera->SetParallelScale(m_parallelScale) ;
  camera->SetClippingRange(m_clippingPlanes) ;
  camera->Modified() ;
}


//------------------------------------------------------------------------------
// Restore camera parameters
void lhpMultiscaleCameraParams::PrintSelf(std::ostream& os, vtkIndent indent) const
//------------------------------------------------------------------------------
{
  os << indent << "Position    " ;   lhpMultiscaleVectorMath::PrintVector(os, this->m_pos) ;
  os << indent << "Focal Point " ;   lhpMultiscaleVectorMath::PrintVector(os, this->m_focalPoint) ;
  os << indent << "View Up     " ;   lhpMultiscaleVectorMath::PrintVector(os, this->m_viewUp) ;
  os << indent << "View angle          " << this->m_viewAngle << std::endl ;
  os << indent << "Parallel Projection " << this->m_parallelProjection << std::endl ;
  os << indent << "Parallel Scale      " << this->m_parallelScale << std::endl ;
  os << indent << "Clipping Range      " << this->m_clippingPlanes[0] << " " << this->m_clippingPlanes[1] << std::endl ;
  os << std::endl ;
}



//------------------------------------------------------------------------------
// == operator
// Note that we have to allow a tolerance in this because the camera 
// can change a bit after being saved - don't know why.
bool lhpMultiscaleCameraParams::operator==(const lhpMultiscaleCameraParams& cameraParams)
//------------------------------------------------------------------------------
{
  double eq0, eq1, eq2, eq3, tol ;

  tol = 0.05 * lhpMultiscaleVectorMath::MagnitudeOfVector(this->m_pos) ;
  eq0 = lhpMultiscaleVectorMath::Equals(cameraParams.m_pos, this->m_pos, tol) ;

  tol = 0.05 * lhpMultiscaleVectorMath::MagnitudeOfVector(this->m_focalPoint) ;
  eq1 = lhpMultiscaleVectorMath::Equals(cameraParams.m_focalPoint, this->m_focalPoint, tol) ;

  tol = 0.05 ;
  eq2 = lhpMultiscaleVectorMath::Equals(cameraParams.m_viewUp, this->m_viewUp, tol) ;

  tol = 0.05 ;
  eq3 = ((this->m_viewAngle > cameraParams.m_viewAngle - tol) && (this->m_viewAngle < cameraParams.m_viewAngle + tol)) ;

  return (eq0 && eq1 && eq2 && eq3) ;
}
