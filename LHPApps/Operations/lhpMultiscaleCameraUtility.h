/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleCameraUtility.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpMultiscaleCameraUtility_H__
#define __lhpMultiscaleCameraUtility_H__

#include "vtkRenderer.h"
#include "vtkActor.h"
#include "vtkActorCollection.h"
#include "vtkCamera.h"


/*******************************************************************************
lhpMultiscaleCameraUtility:
Methods for moving camera
*******************************************************************************/

class lhpMultiscaleCameraUtility
{
public:
  /** Set projection direction of camera
  v does not have to be normalized */
  void SetDirectionOfProjection(vtkCamera *camera, double *newdir) ;

  /** Rotate camera about an arbitrary axis w
  w does not have to be normalized, phi is in degrees */
  void RotateDirectionOfProjection(vtkCamera *camera, double *rotaxis, double phi) ;

  /** Set camera to a new direction vector v with slow rotation
  w does not have to be normalized, phi is in degrees
  timestep is in milliseconds */
  void SetDirectionOfProjectionSlow(vtkRenderer *renderer, double *v, int timestep, int nsteps) ;

  /** Center camera on point.  Dimensions are x[3] or x[4] (homo world coords with h = 1) */
  void CenterCameraOnPoint(double *x, vtkRenderer *renderer) ;

  /** Center camera on actor */
  void CenterCameraOnActor(vtkActor *actor, vtkRenderer *renderer) ;

  /** Zoom camera on bounds by changing the viewing angle */
  void ZoomCameraOnBounds(double *bounds, vtkRenderer *renderer) ; 

  /** Reposition camera on bounds */
  void RepositionCameraOnBounds(double *bounds, vtkRenderer *renderer) ;

  /** Zoom camera on actor */
  void ZoomCameraOnActor(vtkActor *actor, vtkRenderer *renderer) ;

  /** Zoom camera on actor collection
  Camera stays in same position, viewing angle changes */
  void ZoomCameraOnActorCollection(vtkActorCollection *AC, vtkRenderer *renderer) ;

  /** Reposition camera on actor collection
  Camera changes position to accommodate bounds in view */
  void RepositionCameraOnActorCollection(vtkActorCollection *AC, vtkRenderer *renderer) ;

  /** Calculate scale - the size of the view at the focal point in world coords */
  double CalculateScale(vtkCamera *camera) ;

  /** Calculate conversion factor from world coords to pixels */
  double CalculateWorldToPixelsFactor(vtkRenderer *renderer) ;
} ;

#endif
