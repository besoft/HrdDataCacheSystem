/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleActor.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpMultiscaleActor_H__
#define __lhpMultiscaleActor_H__

#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "lhpMultiscaleVisualPipes.h"
#include <iostream>


namespace lhpMultiscale{
  /** types of multiscale actor */
  enum MultiscaleActorType {
    MSCALE_DATA_ACTOR = 0,   ///< visual data actor
    MSCALE_TOKEN             ///< token representing single data actor
  } ;

  /** screen size mode for tokens */
  enum ScreenSizeMode {
    VARIABLE_SIZE = 0,             ///< size fully variable
    FIXED_SIZE,                    ///< size constant
    LIMITED_SIZE                   ///< size varies between max and min
  } ;

  /** status of size relative to scale thresholds */
  enum ScaleStatus {
    UNKNOWN_SCALE = 0,              ///< unknown
    TOO_SMALL,                      ///< actor below size threshold
    IN_SCALE,                       ///< actor size in visible range
    TOO_LARGE                       ///< actor size larger than scale
  } ;
}



/*******************************************************************************
lhpMultiscaleActor:

Utility class for lhpOpMultiscaleExplore.
This is a container for a vtk actor.

The class also contains flags for:
(a) the type of multiscale actor - visualization data or token
(b) the current status of the actor relative to the scale thresholds
(c) the behaviour of the screen size (tokens only)
(d) whether the actor has current attention (ie needs to stay visible)

This class is POD, and allocates no memory.

*******************************************************************************/

class lhpMultiscaleActor
{
public:
  /** Constructor */
  lhpMultiscaleActor(lhpMultiscalePipeline *pipeline, lhpMultiscale::MultiscaleActorType actortype) ;

  /** Print self */
  void PrintSelf(std::ostream& os, vtkIndent indent) ;

  /** Get the vtk actor.
  Do not use this method to set the visibility */
  vtkActor* GetActor() {return m_pipeline->GetActor() ;}

  /** Set visibility of actor */
  void SetVisibility(int visibility) {m_pipeline->SetVisibility(visibility) ;}

  /** Get visibility of actor */
  int GetVisibility() {return m_pipeline->GetVisibility() ;}

  /** Get the type of actor - data or token */
  lhpMultiscale::MultiscaleActorType GetActorType() {return m_type ;}

  /** Get/set the status of the actor relative to the size threshold */
  lhpMultiscale::ScaleStatus GetScaleStatus() {return m_ScaleStatus ;}
  void SetScaleStatus(lhpMultiscale::ScaleStatus status) {m_ScaleStatus = status ;}

  /** Get/set the behaviour of the actor's screen size to fixed, variable or limited */
  lhpMultiscale::ScreenSizeMode GetScreenSizeMode() {return m_ScreenSizeMode ;}
  void SetScreenSizeMode(lhpMultiscale::ScreenSizeMode mode) {m_ScreenSizeMode = mode ;}

  /** Get/set attention flag.
  The attention flag distinguishes actors which you are currently interested in from those you are not.
  If true, it stops the actor disappearing when it becomes too large. */
  bool GetAttention() {return m_attention ;}
  void SetAttention(bool attention) {m_attention = attention ;}

private:
  lhpMultiscalePipeline *m_pipeline ;           ///< pointer to vtk pipeline containing actor
  lhpMultiscale::MultiscaleActorType m_type ;                  ///< type of actor - data-actor, token or group
  lhpMultiscale::ScaleStatus m_ScaleStatus ;                   ///< status of size relative to scale thresholds
  lhpMultiscale::ScreenSizeMode m_ScreenSizeMode ;             ///< controls behaviour of screen size
  bool m_attention ;                            ///< flag indicating if actor has current attention
} ;

#endif