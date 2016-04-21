/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleActorCoordsUtility.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpMultiscaleActorCoordsUtility_H__
#define __lhpMultiscaleActorCoordsUtility_H__

#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkActorCollection.h"



/*******************************************************************************
lhpMultiscaleActorCoordsUtility: 
Methods for reading and changing the size and position of vtkActor, in 
world, view and display coordinates.
*******************************************************************************/

class lhpMultiscaleActorCoordsUtility
{
public:
  /** Convert model coords to world coords.  Dimensions are coordsM[4] and coordsW[4] */
  void ConvertModelToWorld(vtkActor *actor, double *coordsM, double *coordsW) ;

  /** Convert world coords to view coords.  Dimensions are coordsW[4] and coordsV[3] */
  void ConvertWorldToView(vtkRenderer *ren, double *coordsW, double *coordsV) ;

  /** Convert world coords to display coords.  Dimensions are coordsW[4] and coordsD[3] */
  void ConvertWorldToDisplay(vtkRenderer *ren, double *coordsW, double *coordsD) ;

  /** Convert model coords to display coords.  Dimensions are coordsM[4] and coordsD[3] */
  void ConvertModelToDisplay(vtkActor *actor, vtkRenderer *ren, double *coordsM, double *coordsD) ;

  /** Get center of actor (world coords).  Dimensions are coordsW[4], homo coord = 1 */
  void GetCenterWorld(vtkActor *actor, double *centerW) ;

  /** Get center of actor in view coords.  Dimensions are centerV[3] */
  void GetCenterView(vtkActor *actor, vtkRenderer *ren, double *centerV) ;

  /** Get center of actor in display coords.  Dimensions are centerD[3] */
  void GetCenterDisplay(vtkActor *actor, vtkRenderer *ren, double *centerD) ;

  /** Translate actor1 so that its center is the same as actor2 */
  void MoveToCenter(vtkActor *actor1, vtkActor *actor2) ;

  /** Convert bounds from world to display coords.  Dimensions are boundsW[6], boundsD[6] */
  void ConvertBoundsWorldToDisplay(double* boundsW, double *boundsD, vtkRenderer *ren) ;

  /** Get bounds of actor in display coords.  Dimensions are boundsD[6] */
  void GetBoundsDisplay(vtkActor *actor, vtkRenderer *ren, double *boundsD) ;

  /** Get the size of an actor in world coords.  Dimensions are size[3] */
  void GetSizeWorld(vtkActor *actor, double *size) ;

  /** Get the size of an actor in pixels.  Dimensions are size[3] */
  void GetSizeDisplay(vtkActor *actor, vtkRenderer *ren, double *size) ;

  /** Get the min size of an actor in world coords. */
  double GetMinSizeWorld(vtkActor *actor) ;

  /** Get the max size of an actor in world coords. */
  double GetMaxSizeWorld(vtkActor *actor) ;

  /** Get the mean size of an actor in world coords. */
  double GetMeanSizeWorld(vtkActor *actor) ;

  /** Get the min size of an actor in pixels. 
  NB this depends on view direction: a long thin actor has a smaller screen
  size when viewed down the long axis */
  double GetMinSizeDisplay(vtkActor *actor, vtkRenderer *ren) ;

  /** Get the max size of an actor in pixels.
  NB this depends on view direction: a long thin actor has a smaller screen
  size when viewed down the long axis */
  double GetMaxSizeDisplay(vtkActor *actor, vtkRenderer *ren) ;

  /** Get the mean size of an actor in pixels.
  NB this depends on view direction: a long thin actor has a smaller screen
  size when viewed down the long axis */
  double GetMeanSizeDisplay(vtkActor *actor, vtkRenderer *ren) ;

  /** Get the max screen size which an actor can have from any view direction.
  This is approximately independent of the view direction.
  For a long thin actor, this returns the screen size of the long axis,
  regardless of the current view direction.
  cf GetMaxSizeDisplay() which returns the actual screen size. */
  double lhpMultiscaleActorCoordsUtility::GetMaxSizeDisplayAnyView(vtkActor *actor, vtkRenderer *ren) ;

  /** Rescale actor to desired display size in pixels */
  void SetActorDisplaySize(vtkActor *actor, vtkRenderer *ren, double newSize) ;

  /** Do two actors overlap in world coords */
  bool ActorsOverlapWorld(vtkActor *actor1, vtkActor *actor2) ;

  /** Get distance apart of two actors */
  double DistanceApartWorld(vtkActor *actor1, vtkActor *actor2) ;

  /** Get center of actor collection.  Dimensions are coordsW[4], homo coord = 1 */
  void GetCenterOfCollection(vtkActorCollection *AC, double *centerW) ;

  /** Get bounds of actor collection. */
  void GetBoundsOfCollection(vtkActorCollection *AC, double *bounds) ;

  /** Print center and bounds of collection */
  void PrintCenterAndBoundsOfCollection(vtkActorCollection *AC) ;
  
} ;



#endif