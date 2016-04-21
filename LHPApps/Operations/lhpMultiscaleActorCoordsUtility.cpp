/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleActorCoordsUtility.cpp,v $
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
#include "vtkRenderer.h"
#include "vtkMatrix4x4.h"
#include "vtkActorCollection.h"

#include "lhpMultiscaleActorCoordsUtility.h"
#include "lhpMultiscaleVectorMath.h"

#include <cmath>
#include <algorithm>


//------------------------------------------------------------------------------
// Convert model coords to world coords
// dimensions are coordsM[4] and coordsW[4]
void lhpMultiscaleActorCoordsUtility::ConvertModelToWorld(vtkActor *actor, double *coordsM, double *coordsW)
//------------------------------------------------------------------------------
{
  // convert model coords to world
  actor->GetMatrix()->MultiplyPoint(coordsM, coordsW) ;
}




//------------------------------------------------------------------------------
// Convert world coords to view coords
// dimensions are coordsW[4] and coordsV[3]
void lhpMultiscaleActorCoordsUtility::ConvertWorldToView(vtkRenderer *ren, double *coordsW, double *coordsV)
//------------------------------------------------------------------------------
{
  // convert world coords to view
  ren->GetActiveCamera() ;        // vtk problem: you need this or the camera won't be loaded in WorldToView()
  ren->SetWorldPoint(coordsW) ;
  ren->WorldToView() ;
  ren->GetViewPoint(coordsV) ;
}


//------------------------------------------------------------------------------
// Convert world coords to display coords
// dimensions are coordsW[4] and coordsD[3]
void lhpMultiscaleActorCoordsUtility::ConvertWorldToDisplay(vtkRenderer *ren, double *coordsW, double *coordsD)
//------------------------------------------------------------------------------
{
  // convert world coords to display
  ren->GetActiveCamera() ;        // vtk problem: you need this or the camera won't be loaded in WorldToDisplay()
  ren->SetWorldPoint(coordsW) ;
  ren->WorldToDisplay() ;
  ren->GetDisplayPoint(coordsD) ;
}


//------------------------------------------------------------------------------
// Convert model coords to display coords
// dimensions are coordsM[4] and coordsD[3]
void lhpMultiscaleActorCoordsUtility::ConvertModelToDisplay(vtkActor *actor, vtkRenderer *ren, double *coordsM, double *coordsD)
//------------------------------------------------------------------------------
{
  double coordsW[4] ;
  ConvertModelToWorld(actor, coordsM, coordsW) ;
  ConvertWorldToDisplay(ren, coordsW, coordsD) ;
}




//------------------------------------------------------------------------------
// Get center of actor (world coords)
// dimensions are coordsW[4], homo coord = 1
void lhpMultiscaleActorCoordsUtility::GetCenterWorld(vtkActor *actor, double *centerW)
//------------------------------------------------------------------------------
{
  // get bounds and center, in world coords
  double bounds[6] ;
  actor->GetBounds(bounds) ;
  centerW[0] = (bounds[1] + bounds[0]) / 2.0 ;
  centerW[1] = (bounds[3] + bounds[2]) / 2.0 ;
  centerW[2] = (bounds[5] + bounds[4]) / 2.0 ;
  centerW[3] = 1.0 ;
}



//------------------------------------------------------------------------------
// Get center of actor in view coords
// dimensions are centerV[3]
void lhpMultiscaleActorCoordsUtility::GetCenterView(vtkActor *actor, vtkRenderer *ren, double *centerV)
//------------------------------------------------------------------------------
{
  double centerW[4] ;
  GetCenterWorld(actor, centerW) ;
  ConvertWorldToView(ren, centerW, centerV) ;
}


//------------------------------------------------------------------------------
// Get center of actor in display coords
// dimensions are centerD[3]
void lhpMultiscaleActorCoordsUtility::GetCenterDisplay(vtkActor *actor, vtkRenderer *ren, double *centerD)
//------------------------------------------------------------------------------
{
  double centerW[4] ;
  GetCenterWorld(actor, centerW) ;
  ConvertWorldToDisplay(ren, centerW, centerD) ;
}



//------------------------------------------------------------------------------
// Translate actor1 so that its center is the same as actor2
void lhpMultiscaleActorCoordsUtility::MoveToCenter(vtkActor *actor1, vtkActor *actor2)
//------------------------------------------------------------------------------
{
  int i ;
  double center1[4], center2[4], pos1[3], posnew[3] ;

  // get centers (these are in homo coords)
  GetCenterWorld(actor1, center1) ;
  GetCenterWorld(actor2, center2) ;
  for (i = 0 ;  i < 3 ;  i++){
    center1[i] /= center1[3] ;
    center2[i] /= center2[3] ;
  }

  // set new position of actor 1
  actor1->GetPosition(pos1) ;
  for (i = 0 ;  i < 3 ;  i++)
    posnew[i] = pos1[i] + center2[i] - center1[i] ;
  actor1->SetPosition(posnew) ;

  GetCenterWorld(actor1, center1) ;
  GetCenterWorld(actor2, center2) ;
}



//------------------------------------------------------------------------------
// Convert bounds from world to display coords.  Dimensions are boundsW[6], boundsD[6] */
void lhpMultiscaleActorCoordsUtility::ConvertBoundsWorldToDisplay(double* boundsW, double *boundsD, vtkRenderer *ren)
//------------------------------------------------------------------------------
{
  int i ;

  // get corners of bounding box in homogeneous world coords
  double cornersW[8][4] ;
  cornersW[0][0] = boundsW[1] ; cornersW[0][1] = boundsW[3] ; cornersW[0][2] = boundsW[5] ; cornersW[0][3] = 1.0 ;
  cornersW[1][0] = boundsW[1] ; cornersW[1][1] = boundsW[3] ; cornersW[1][2] = boundsW[4] ; cornersW[1][3] = 1.0 ;
  cornersW[2][0] = boundsW[1] ; cornersW[2][1] = boundsW[2] ; cornersW[2][2] = boundsW[5] ; cornersW[2][3] = 1.0 ;
  cornersW[3][0] = boundsW[1] ; cornersW[3][1] = boundsW[2] ; cornersW[3][2] = boundsW[4] ; cornersW[3][3] = 1.0 ;
  cornersW[4][0] = boundsW[0] ; cornersW[4][1] = boundsW[3] ; cornersW[4][2] = boundsW[5] ; cornersW[4][3] = 1.0 ;
  cornersW[5][0] = boundsW[0] ; cornersW[5][1] = boundsW[3] ; cornersW[5][2] = boundsW[4] ; cornersW[5][3] = 1.0 ;
  cornersW[6][0] = boundsW[0] ; cornersW[6][1] = boundsW[2] ; cornersW[6][2] = boundsW[5] ; cornersW[6][3] = 1.0 ;
  cornersW[7][0] = boundsW[0] ; cornersW[7][1] = boundsW[2] ; cornersW[7][2] = boundsW[4] ; cornersW[7][3] = 1.0 ;

  // convert corners to display coords
  double cornersD[8][3] ;
  ren->GetActiveCamera() ;        // vtk problem: you need this or the camera won't be loaded in WorldToDisplay()
  for (i = 0 ;  i < 8 ;  i++){
    ren->SetWorldPoint(cornersW[i]) ;
    ren->WorldToDisplay() ;
    ren->GetDisplayPoint(cornersD[i]) ;
  }

  // get display bounds in pixels 
  for (i = 0 ;  i < 8 ;  i++){
    boundsD[0] = cornersD[0][0] ;
    boundsD[1] = cornersD[0][0] ;
    boundsD[2] = cornersD[0][1] ;
    boundsD[3] = cornersD[0][1] ;
    boundsD[4] = cornersD[0][2] ;
    boundsD[5] = cornersD[0][2] ;
  }
  for (i = 0 ;  i < 8 ;  i++){
    boundsD[0] = std::min(cornersD[i][0], boundsD[0]) ;
    boundsD[1] = std::max(cornersD[i][0], boundsD[1]) ;
    boundsD[2] = std::min(cornersD[i][1], boundsD[2]) ;
    boundsD[3] = std::max(cornersD[i][1], boundsD[3]) ;
    boundsD[4] = std::min(cornersD[i][2], boundsD[4]) ;
    boundsD[5] = std::max(cornersD[i][2], boundsD[5]) ;
  }
}


//------------------------------------------------------------------------------
// Get bounds of actor in display coords
// dimensions are boundsD[6]
void lhpMultiscaleActorCoordsUtility::GetBoundsDisplay(vtkActor *actor, vtkRenderer *ren, double *boundsD)
//------------------------------------------------------------------------------
{
  // get bounds (world coords)
  double boundsW[6] ;
  actor->GetBounds(boundsW) ;

  ConvertBoundsWorldToDisplay(boundsW, boundsD, ren) ;
}




//------------------------------------------------------------------------------
// Get the size of an actor in world coords
// dimensions are size[3]
void lhpMultiscaleActorCoordsUtility::GetSizeWorld(vtkActor *actor, double *size)
//------------------------------------------------------------------------------
{
  double bounds[6] ;
  actor->GetBounds(bounds) ;
  size[0] = bounds[1] - bounds[0] ;
  size[1] = bounds[3] - bounds[2] ;
  size[2] = bounds[5] - bounds[4] ;
}


//------------------------------------------------------------------------------
// Get the size of an actor in pixels
// dimensions are size[3]
void lhpMultiscaleActorCoordsUtility::GetSizeDisplay(vtkActor *actor, vtkRenderer *ren, double *size)
//------------------------------------------------------------------------------
{
  double boundsD[6] ;
  GetBoundsDisplay(actor, ren, boundsD) ;

  // return size in x, y and z
  size[0] = boundsD[1] - boundsD[0] ;
  size[1] = boundsD[3] - boundsD[2] ;
  size[2] = boundsD[5] - boundsD[4] ;
}


//------------------------------------------------------------------------------
// Get the min size of an actor in world coords.
double lhpMultiscaleActorCoordsUtility::GetMinSizeWorld(vtkActor *actor)
//------------------------------------------------------------------------------
{
  double siz[3], sizmin ;
  GetSizeWorld(actor, siz) ;
  sizmin = std::min(siz[0], siz[1]) ;
  sizmin = std::min(siz[2], sizmin) ;
  return sizmin ;
}

//------------------------------------------------------------------------------
// Get the max size of an actor in world coords.
double lhpMultiscaleActorCoordsUtility::GetMaxSizeWorld(vtkActor *actor)
//------------------------------------------------------------------------------
{
  double siz[3], sizmax ;
  GetSizeWorld(actor, siz) ;
  sizmax = std::max(siz[0], siz[1]) ;
  sizmax = std::max(siz[2], sizmax) ;
  return sizmax ;
}

//------------------------------------------------------------------------------
// Get the mean size of an actor in world coords.
double lhpMultiscaleActorCoordsUtility::GetMeanSizeWorld(vtkActor *actor)
//------------------------------------------------------------------------------
{
  double siz[3], sizmean ;
  GetSizeWorld(actor, siz) ;
  sizmean = (siz[0] + siz[1] + siz[2]) / 3.0 ;
  return sizmean ;
}

//------------------------------------------------------------------------------
// Get the min size of an actor in pixels.
double lhpMultiscaleActorCoordsUtility::GetMinSizeDisplay(vtkActor *actor, vtkRenderer *ren)
//------------------------------------------------------------------------------
{
  double siz[3], sizmin ;
  GetSizeDisplay(actor, ren, siz) ;
  sizmin = std::min(siz[0], siz[1]) ;
  return sizmin ;
}

//------------------------------------------------------------------------------
// Get the max size of an actor in pixels.
double lhpMultiscaleActorCoordsUtility::GetMaxSizeDisplay(vtkActor *actor, vtkRenderer *ren)
//------------------------------------------------------------------------------
{
  double siz[3], sizmax ;
  GetSizeDisplay(actor, ren, siz) ;
  sizmax = std::max(siz[0], siz[1]) ;
  return sizmax ;
}

//------------------------------------------------------------------------------
// Get the mean size of an actor in pixels.
double lhpMultiscaleActorCoordsUtility::GetMeanSizeDisplay(vtkActor *actor, vtkRenderer *ren)
//------------------------------------------------------------------------------
{
  double siz[3], sizmean ;
  GetSizeDisplay(actor, ren, siz) ;
  sizmean = (siz[0] + siz[1]) / 2.0 ;
  return sizmean ;
}


//------------------------------------------------------------------------------
// Get the max screen size which an actor can have from any view direction.
// This is approximately independent of the view direction.
// For a long thin actor, this returns the screen size of the long axis,
// regardless of the current view direction.
// cf GetMaxSizeDisplay() which returns the actual screen size.
double lhpMultiscaleActorCoordsUtility::GetMaxSizeDisplayAnyView(vtkActor *actor, vtkRenderer *ren)
//------------------------------------------------------------------------------
{
  double boundsW[6] ;
  actor->GetBounds(boundsW) ;

  // find the centre
  double xc = (boundsW[1] + boundsW[0]) / 2.0 ;
  double yc = (boundsW[3] + boundsW[2]) / 2.0 ;
  double zc = (boundsW[5] + boundsW[4]) / 2.0 ;

  // find the longest axis
  double dx = (boundsW[1] - boundsW[0]) / 2.0 ;
  double dy = (boundsW[3] - boundsW[2]) / 2.0 ;
  double dz = (boundsW[5] - boundsW[4]) / 2.0 ;

  // stretch the bounds to form a cube with dimensions equal to longest axis
  if ((dx >= dy) && (dx >= dz)){
    boundsW[2] = yc - dx ;
    boundsW[3] = yc + dx ;
    boundsW[4] = zc - dx ;
    boundsW[5] = zc + dx ;
  }
  else if ((dy > dx) && (dy >= dz)){
    boundsW[0] = xc - dy ;
    boundsW[1] = xc + dy ;
    boundsW[4] = zc - dy ;
    boundsW[5] = zc + dy ;
  }
  else {
    boundsW[0] = xc - dz ;
    boundsW[1] = xc + dz ;
    boundsW[2] = yc - dz ;
    boundsW[3] = yc + dz ;
  }

  // find the display size of the cube and return the max
  double boundsD[6], size[3] ;
  ConvertBoundsWorldToDisplay(boundsW, boundsD, ren) ;

  size[0] = boundsD[1] - boundsD[0] ;
  size[1] = boundsD[3] - boundsD[2] ;
  size[2] = boundsD[5] - boundsD[4] ;

  double sizmax = std::max(size[0], size[1]) ;
  return sizmax ;
}



//------------------------------------------------------------------------------
// Rescale actor to desired display size in pixels
void lhpMultiscaleActorCoordsUtility::SetActorDisplaySize(vtkActor *actor, vtkRenderer *ren, double newSize)
//------------------------------------------------------------------------------
{
  double sizD[3], s[3] ;
  double Scale, ScaleLast, ScaleNext, Size, SizeLast, Err, ErrLast, ErrStop ;

  const double ErrPCTarget = 1.0 ;    // target percentage error
  const int maxits = 10 ;            // set max no. of iterations

  // set up a scratch actor to test the values of scale
  // in case we accumulate errors by piling scale on scale.
  vtkActor *actorScratch = vtkActor::New() ;

  // Set stop value of error
  ErrStop = 0.01*ErrPCTarget*newSize ;

  // get initial scale of actor
  actor->GetScale(s) ;
  Scale = (s[0]+s[1]+s[2]) / 3.0 ;

  // Get initial display size
  GetSizeDisplay(actor, ren, sizD) ;
  Size = std::max(sizD[0], sizD[1]) ;

  // Get initial error in size
  Err = Size - newSize ;

  // Make initial guess at required scale, assuming it is proportional to the size ratio
  ScaleNext = Scale * newSize / Size ;

  for (int i = 0 ;  i < maxits && std::fabs(Err) > ErrStop ;  i++){
    // Shift variables
    ErrLast = Err ;
    ScaleLast = Scale ;
    Scale = ScaleNext ;
    SizeLast = Size ;

    // copy actor with latest guess at scale
    actorScratch->ShallowCopy(actor) ;
    actorScratch->SetScale(Scale) ;

    // Get scaled display size
    GetSizeDisplay(actorScratch, ren, sizD) ;
    Size = std::max(sizD[0], sizD[1]) ;

    // calculate error
    Err = (Size - newSize) ;

    // calculate next guess at scale by linear interp
    ScaleNext = Scale - Err * (Scale - ScaleLast) / (Err - ErrLast) ;
  }

  // transfer final scale to input actor
  actor->SetScale(Scale) ;

  // delete scratch actor
  actorScratch->Delete() ;

}



//------------------------------------------------------------------------------
// Get distance apart of two actors in world coords
double lhpMultiscaleActorCoordsUtility::DistanceApartWorld(vtkActor *actor1, vtkActor *actor2)
//------------------------------------------------------------------------------
{
  double center1[4], center2[4], dcenter[3] ;
  GetCenterWorld(actor1, center1) ;
  GetCenterWorld(actor2, center2) ;
  lhpMultiscaleVectorMath::SubtractVectors(center2, center1, dcenter) ;
  return lhpMultiscaleVectorMath::MagnitudeOfVector(dcenter) ;
}



//------------------------------------------------------------------------------
// Get center of actor collection.  Dimensions are coordsW[4], homo coord = 1
void lhpMultiscaleActorCoordsUtility::GetCenterOfCollection(vtkActorCollection *AC, double *centerW)
//------------------------------------------------------------------------------
{
  int i ;
  double vecsum[3] = {0.0, 0.0, 0.0}  ;
  double ctr[4] ;
  int n = AC->GetNumberOfItems() ;

  // sum the centers
  AC->InitTraversal() ;
  for (i = 0 ; i < n ;  i++){
    vtkActor *actor = AC->GetNextActor() ;
    GetCenterWorld(actor, ctr) ;
    lhpMultiscaleVectorMath::AddVectors(ctr, vecsum, vecsum) ;
  }

  // find the centre of the centres
  lhpMultiscaleVectorMath::DivideVectorByScalar((double)n, vecsum, centerW) ;

  // set homo coordinate to 1
  centerW[3] = 1.0 ;
}



//------------------------------------------------------------------------------
// Get bounds of actor collection.
void lhpMultiscaleActorCoordsUtility::GetBoundsOfCollection(vtkActorCollection *AC, double *bounds)
//------------------------------------------------------------------------------
{
  int i ;
  double bnds[6] ;
  int n = AC->GetNumberOfItems() ;

  // find the min and max of the bounds of all the actors
  AC->InitTraversal() ;
  for (i = 0 ; i < n ;  i++){
    vtkActor *actor = AC->GetNextActor() ;
    actor->GetBounds(bnds) ;
    if (i == 0){
      bounds[0] = bnds[0] ;
      bounds[1] = bnds[1] ;
      bounds[2] = bnds[2] ;
      bounds[3] = bnds[3] ;
      bounds[4] = bnds[4] ;
      bounds[5] = bnds[5] ;
    }
    else{
      bounds[0] = std::min(bnds[0], bounds[0]) ;
      bounds[1] = std::max(bnds[1], bounds[1]) ;
      bounds[2] = std::min(bnds[2], bounds[2]) ;
      bounds[3] = std::max(bnds[3], bounds[3]) ;
      bounds[4] = std::min(bnds[4], bounds[4]) ;
      bounds[5] = std::max(bnds[5], bounds[5]) ;
    }
  }
}


//------------------------------------------------------------------------------
// Print center and bounds of collection
void lhpMultiscaleActorCoordsUtility::PrintCenterAndBoundsOfCollection(vtkActorCollection *AC)
//------------------------------------------------------------------------------
{
  int i ;
  double bnds[6], centerW[4] ;
  int n = AC->GetNumberOfItems() ;

  // print the centers and bounds of all the actors
  AC->InitTraversal() ;
  for (i = 0 ; i < n ;  i++){
    vtkActor *actor = AC->GetNextActor() ;
    GetCenterWorld(actor, centerW) ;
    std::cout << i << "    center " << centerW[0] << " " << centerW[1] << " " << centerW[2] << std::endl ;
  }
  GetCenterOfCollection(AC, centerW) ;
  std::cout << "all: center " << centerW[0] << " " << centerW[1] << " " << centerW[2] << std::endl ;

  AC->InitTraversal() ;
  for (i = 0 ; i < n ;  i++){
    vtkActor *actor = AC->GetNextActor() ;
    actor->GetBounds(bnds) ;
    std::cout << i << "    bounds " << bnds[0] << " " << bnds[1] << " " << bnds[2] << " " << bnds[3] << " " << bnds[4] << " " << bnds[5] << " " << std::endl ;
  }
  GetBoundsOfCollection(AC, bnds) ;
  std::cout << "all: bounds " << bnds[0] << " " << bnds[1] << " " << bnds[2] << " " << bnds[3] << " " << bnds[4] << " " << bnds[5] << " " << std::endl ;
  std::cout << std::endl ;
}
