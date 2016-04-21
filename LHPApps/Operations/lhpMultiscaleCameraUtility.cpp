/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleCameraUtility.cpp,v $
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

#include "vtkActor.h"
#include "vtkActorCollection.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkRenderWindowInteractor.h"

#include "lhpMultiscaleCameraUtility.h"
#include "lhpMultiscaleActorCoordsUtility.h"
#include "lhpMultiscaleVectorMath.h"

#include <cmath>
#include <algorithm>



//------------------------------------------------------------------------------
// Set projection direction of camera to the vector v
// v does not have to be normalized
void lhpMultiscaleCameraUtility::SetDirectionOfProjection(vtkCamera *camera, double *v)
//------------------------------------------------------------------------------
{
  double cameraPos[3], F[3] ;
  camera->GetPosition(cameraPos) ;   // get camera position
  camera->GetFocalPoint(F) ;         // get current focal point

  // get current focal length and direction
  double dirToF[3] ;
  lhpMultiscaleVectorMath::SubtractVectors(F, cameraPos, dirToF) ;
  double focalLength = lhpMultiscaleVectorMath::MagnitudeOfVector(dirToF) ;
  lhpMultiscaleVectorMath::NormalizeVector(dirToF, dirToF) ;

  // calculate new position of focal point
  double dirToFnew[3], Fnew[3] ;
  lhpMultiscaleVectorMath::NormalizeVector(v, dirToFnew) ;
  lhpMultiscaleVectorMath::MultiplyVectorByScalar(focalLength, dirToFnew, Fnew) ;   // create vector with original focal length in new direction
  lhpMultiscaleVectorMath::AddVectors(Fnew, cameraPos, Fnew) ;                      // add the camera position back again

  // set the focal point
  camera->SetFocalPoint(Fnew) ;

  // make sure view up is ok
  camera->OrthogonalizeViewUp() ;
}




//------------------------------------------------------------------------------
// Rotate camera about an arbitrary axis w
// w does not have to be normalized, phi is in degrees
/* 
We construct a polar coord system about the rotation axis w:
Let x(phi) = rsin(t)cos(phi)*u + rsin(t)sin(phi)*v + rcos(t)*w
and x(0) = rsin(t)*u + rcos(t)*w
where u, v and w are vectors: w is the rotation axis, u orthog. to w in the x direction, and v is the third orthog. vector.
x(0) is the current direction, and x(phi) is the desired roated direction.
(1) rcos(t) = x(0).w
(2) rsin(t)*u = x(0) - rcos(t)*w
(3) x(phi) = cos(phi)*x(0) + rcos(t)(1-cos(phi))*w + rsin(t)sin(phi)*v
(4) v = w ^ u  =>  rsin(t)*v = w ^ (x(0) - rcos(t)*w)
= w ^ x(0)
(5) x(phi) = cos(phi)*x(0) + rcos(t)(1-cos(phi))*w + sin(phi)*(w^x(0))
*/
void lhpMultiscaleCameraUtility::RotateDirectionOfProjection(vtkCamera *camera, double *rotaxis, double phi)
//------------------------------------------------------------------------------
{
  // convert phi to radians
  double phirad = phi * 3.14159 / 180.0 ;
  double cosphi = cos(phirad) ;
  double sinphi = sin(phirad) ;

  // get camera position and current focal point
  double cameraPos[3], F[3] ;
  camera->GetPosition(cameraPos) ;
  camera->GetFocalPoint(F) ;

  // get current focal length and direction
  double dirToF[3] ;
  lhpMultiscaleVectorMath::SubtractVectors(F, cameraPos, dirToF) ;
  double focalLength = lhpMultiscaleVectorMath::MagnitudeOfVector(dirToF) ;
  lhpMultiscaleVectorMath::NormalizeVector(dirToF, dirToF) ;

  // get normalized axis of rotation
  double w[3] ;
  lhpMultiscaleVectorMath::NormalizeVector(rotaxis, w) ;

  // calculate the 1st term in eqn (5) above, where 'x' = dirToF
  double term1[3] ;
  lhpMultiscaleVectorMath::MultiplyVectorByScalar(cosphi, dirToF, term1) ;

  // calculate the 2nd term (nb r = 1 because we normalised dirToF)
  double term2[3] ;
  double cost = lhpMultiscaleVectorMath::DotProduct(dirToF, w) ;
  lhpMultiscaleVectorMath::MultiplyVectorByScalar(cost*(1-cosphi), w, term2) ;

  // calculate the 3rd term
  double term3[3], vprod[3] ;
  lhpMultiscaleVectorMath::VectorProduct(w, dirToF, vprod) ;
  lhpMultiscaleVectorMath::MultiplyVectorByScalar(sinphi, vprod, term3) ;

  // add the terms to get the new direction
  double dirToFnew[3] ;
  lhpMultiscaleVectorMath::AddVectors(term1, term2, dirToFnew) ;
  lhpMultiscaleVectorMath::AddVectors(dirToFnew, term3, dirToFnew) ;

  double newmag = lhpMultiscaleVectorMath::MagnitudeOfVector(dirToFnew) ;

  // calculate new position of focal point
  double Fnew[3] ;
  lhpMultiscaleVectorMath::MultiplyVectorByScalar(focalLength, dirToFnew, Fnew) ;   // create vector with original focal length in new direction
  lhpMultiscaleVectorMath::AddVectors(Fnew, cameraPos, Fnew) ;                      // add the camera position back again

  // set the focal point
  camera->SetFocalPoint(Fnew) ;

  // make sure view up is ok
  camera->OrthogonalizeViewUp() ;

}




//------------------------------------------------------------------------------
// Set camera to a new direction vector v with slow rotation
// w does not have to be normalized, phi is in degrees
// timestep is in milliseconds
void lhpMultiscaleCameraUtility::SetDirectionOfProjectionSlow(vtkRenderer *renderer, double *v, int timestep, int nsteps)
//------------------------------------------------------------------------------
{
  vtkCamera *camera = renderer->GetActiveCamera() ;

  double cameraPos[3], F[3] ;
  camera->GetPosition(cameraPos) ;   // get camera position
  camera->GetFocalPoint(F) ;         // get current focal point

  // get current focal length and direction
  double dirToF[3] ;
  lhpMultiscaleVectorMath::SubtractVectors(F, cameraPos, dirToF) ;
  double focalLength = lhpMultiscaleVectorMath::MagnitudeOfVector(dirToF) ;
  lhpMultiscaleVectorMath::NormalizeVector(dirToF, dirToF) ;

  // get rotation axis, orthog to current direction and desired direction
  double rotaxis[3] ;
  lhpMultiscaleVectorMath::VectorProduct(dirToF, v, rotaxis) ;

  // check in case we are already pointing in the desired direction
  double mag = lhpMultiscaleVectorMath::MagnitudeOfVector(rotaxis) ;
  if (mag > 0.000001){
    lhpMultiscaleVectorMath::NormalizeVector(rotaxis, rotaxis) ;

    // get size of rotation per step in degrees
    double costheta = lhpMultiscaleVectorMath::DotProduct(dirToF, v) / lhpMultiscaleVectorMath::MagnitudeOfVector(v) ;
    double theta = (180.0 / 3.14159) * std::acos(costheta) ;
    double dtheta = theta / (double)nsteps ;

    // rotate to new position in steps
    for (int i = 0 ;  i < nsteps ;  i++){
      RotateDirectionOfProjection(camera, rotaxis, dtheta) ;
      renderer->GetRenderWindow()->Render() ;               // nb this line causes a small once-only memory leak - why ?
      Sleep(timestep) ;
    }
  }
}




//------------------------------------------------------------------------------
// Center camera on point.  Dimensions are x[3] or x[4] (homo world coords with h = 1)
void lhpMultiscaleCameraUtility::CenterCameraOnPoint(double *x, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  const int nsteps = 10 ;
  const int timestep = 100 ;

  // get camera position
  vtkCamera *camera = renderer->GetActiveCamera() ;
  double cameraPos[3] ;
  camera->GetPosition(cameraPos) ;

  // get direction from camera to actor
  double dirToX[3] ;
  lhpMultiscaleVectorMath::SubtractVectors(x, cameraPos, dirToX) ;
  SetDirectionOfProjectionSlow(renderer, dirToX, timestep, nsteps) ;
}


//------------------------------------------------------------------------------
// Center camera on actor
void lhpMultiscaleCameraUtility::CenterCameraOnActor(vtkActor *actor, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  lhpMultiscaleActorCoordsUtility ACU ;
  double actorCenter[4] ;

  ACU.GetCenterWorld(actor, actorCenter) ;      // get center of actor
  CenterCameraOnPoint(actorCenter, renderer) ;  // rotate camera to the actor
}



//------------------------------------------------------------------------------
// Zoom camera on bounds by changing the viewing angle
void lhpMultiscaleCameraUtility::ZoomCameraOnBounds(double *bounds, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  const int nsteps = 10 ;
  const int timestep = 100 ;
  double centerW[4] ;
  vtkCamera *camera = renderer->GetActiveCamera() ;

  // calculate center of bounds
  centerW[0] = (bounds[0] + bounds[1]) / 2.0 ;
  centerW[1] = (bounds[2] + bounds[3]) / 2.0 ;
  centerW[2] = (bounds[4] + bounds[5]) / 2.0 ;
  centerW[3] = 1.0 ;

  // rotate camera to center
  CenterCameraOnPoint(centerW, renderer) ;

  // place the focal point in the centre
  camera->SetFocalPoint(centerW) ;

  // get the effective radius of the bounds
  double boundsSize, effRadius ;
  double sizx = (bounds[1] - bounds[0]) ;
  double sizy = (bounds[3] - bounds[2]) ;
  double sizz = (bounds[5] - bounds[4]) ;
  boundsSize = std::max(sizx, sizy) ;
  boundsSize = std::max(boundsSize, sizz) ;
  effRadius = 1.2 * boundsSize / 2.0 ;

  // calculate the viewing angle required
  double FL = camera->GetDistance() ;
  double viewAngleTarget = 2.0 * (180.0/3.14159) * atan(effRadius / FL) ;

  // change slowly to the new viewing angle
  double viewAngleCurrent = camera->GetViewAngle() ;
  double angleInc = (viewAngleTarget - viewAngleCurrent) / (double)nsteps ;
  for (int i = 1 ;  i <= nsteps ;  i++){
    double angle = viewAngleCurrent + (double)i*angleInc ;
    camera->SetViewAngle(angle) ;
    renderer->GetRenderWindow()->Render() ;
    Sleep(timestep) ;
  }

  // make sure view up is ok
  camera->OrthogonalizeViewUp() ;

  // Here we would like to manually set the clipping range, to exclude actors we don't want to see.
  // However, style->SetAutoAdjustCameraClippingRange() overrides any changes we make here.
  // The auto adjust must be switched on for the interactor to work properly.
  // The clipping range will be based on all the visible actors, so we will have to switch off
  // any actors which we don't want to see.
  renderer->GetRenderWindow()->Render() ;
}



//------------------------------------------------------------------------------
// Reposition camera on bounds
void lhpMultiscaleCameraUtility::RepositionCameraOnBounds(double *bounds, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  double centerW[4] ;
  vtkCamera *camera = renderer->GetActiveCamera() ;

  // calculate center of bounds
  centerW[0] = (bounds[0] + bounds[1]) / 2.0 ;
  centerW[1] = (bounds[2] + bounds[3]) / 2.0 ;
  centerW[2] = (bounds[4] + bounds[5]) / 2.0 ;
  centerW[3] = 1.0 ;

  // rotate camera to center
  CenterCameraOnPoint(centerW, renderer) ;

  // place the focal point in the centre
  camera->SetFocalPoint(centerW) ;

  // get the effective radius of the bounds
  double boundsSize, effRadius ;
  double sizx = (bounds[1] - bounds[0]) ;
  double sizy = (bounds[3] - bounds[2]) ;
  double sizz = (bounds[5] - bounds[4]) ;
  boundsSize = std::max(sizx, sizy) ;
  boundsSize = std::max(boundsSize, sizz) ;
  effRadius = 1.2 * boundsSize / 2.0 ;

  // set the view angle target to 17.0 degrees and calculate
  // the focal length required to view the whole bounds
  double viewAngleTarget = 17.0 ;
  double viewAngleTargetRad = (3.14159/180.0)*viewAngleTarget ;
  double FLTarget = effRadius / tan(viewAngleTargetRad/2.0) ;

  // calculate the required camera position
  double FLCurrent = camera->GetDistance() ;
  double pos[3], fp[3], projVec[3], posTarget[3] ;
  camera->GetPosition(pos) ;
  camera->GetFocalPoint(fp) ;
  lhpMultiscaleVectorMath::SubtractVectors(pos, fp, projVec) ;
  lhpMultiscaleVectorMath::MultiplyVectorByScalar(FLTarget/FLCurrent, projVec, projVec) ;
  lhpMultiscaleVectorMath::AddVectors(fp, projVec, posTarget) ;

  // set new viewing angle and position
  camera->SetViewAngle(viewAngleTarget) ;
  camera->SetPosition(posTarget) ;

  // make sure view up is ok
  camera->OrthogonalizeViewUp() ;

  renderer->GetRenderWindow()->Render() ;
}



//------------------------------------------------------------------------------
// Zoom camera on actor
void lhpMultiscaleCameraUtility::ZoomCameraOnActor(vtkActor *actor, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  double bounds[6] ;
  actor->GetBounds(bounds) ;
  ZoomCameraOnBounds(bounds, renderer) ;
}


//------------------------------------------------------------------------------
// Zoom camera on actor collection
void lhpMultiscaleCameraUtility::ZoomCameraOnActorCollection(vtkActorCollection *AC, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  lhpMultiscaleActorCoordsUtility ACU ;
  double bounds[6] ;
  ACU.GetBoundsOfCollection(AC, bounds) ;
  ZoomCameraOnBounds(bounds, renderer) ;
}


//------------------------------------------------------------------------------
// Reposition camera on actor collection
void lhpMultiscaleCameraUtility::RepositionCameraOnActorCollection(vtkActorCollection *AC, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  lhpMultiscaleActorCoordsUtility ACU ;
  double bounds[6] ;
  ACU.GetBoundsOfCollection(AC, bounds) ;
  RepositionCameraOnBounds(bounds, renderer) ;
}



//------------------------------------------------------------------------------
// Get scale.  This is the display width in world coords at the focal distance.
double lhpMultiscaleCameraUtility::CalculateScale(vtkCamera *camera)
//------------------------------------------------------------------------------
{
  double FL = camera->GetDistance() ;
  double viewAngleRad = (3.14159/180.0) * camera->GetViewAngle() ;
  double scale = 2.0 * FL * std::tan(viewAngleRad/2.0) ;
  return scale ;
}



//------------------------------------------------------------------------------
// Calculate conversion factor from world coords to pixels
double lhpMultiscaleCameraUtility::CalculateWorldToPixelsFactor(vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  // get the display width in world coords
  vtkCamera *camera = renderer->GetActiveCamera() ;
  double widthWorld = CalculateScale(camera) ;

  // get the display width in display coords
  int *siz = renderer->GetRenderWindow()->GetSize() ;
  double widthDisplay = siz[0] ;

  return widthDisplay / widthWorld ;
}





