/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleUtility.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpMultiscaleUtility_H__
#define __lhpMultiscaleUtility_H__

#include "vtkObject.h"
#include "vtkCommand.h"

#include "lhpMultiscaleActor.h"
#include "lhpMultiscaleCameraParams.h"
#include "lhpMultiscaleActorCoordsUtility.h"
#include "lhpMultiscaleCameraUtility.h"
#include "lhpMultiscaleVisualPipes.h"

#include <vector>
#include <iostream>


/*******************************************************************************
Utility methods for multiscale:
typedef ViewState           - helper struct
class ActorIdPair           - helper class
class lhpMultiscaleUtility  - methods and info blackboard for managing multiscale view
*******************************************************************************/

/*******************************************************************************
Helper struct to store state of view
*******************************************************************************/
typedef struct{
  lhpMultiscaleCameraParams cameraParams ;               // camera position
  std::vector<bool> attentionFlags ;        // attention flags of the actors (nb beware of vector<bool>)
} ViewState ;


/*******************************************************************************
Helper class containing pair of id's.
*******************************************************************************/
class ActorIdPair{
public:
  ActorIdPair(int id0, int id1) : m_id0(id0), m_id1(id1) {}
  int GetId0() {return m_id0 ;}
  int GetId1() {return m_id1 ;}

  // equality operator
  bool operator==(const ActorIdPair& idPair1) const 
  {
    return ((this->m_id0==idPair1.m_id0 && this->m_id1==idPair1.m_id1) ||
      (this->m_id0==idPair1.m_id1 && this->m_id1==idPair1.m_id0)) ;
  }
private:
  int m_id0 ;
  int m_id1 ;
} ;


namespace lhpMultiscale{
  /*******************************************************************************
  Id's of base units
  *******************************************************************************/
  enum BaseUnitsId
  {
    ID_METRES,
    ID_CENTIMETRES,
    ID_MILLIMETRES,
    ID_MICRONS,
    ID_NANOMETRES
  } ;
}



/*******************************************************************************
lhpMultiscaleUtility:
Methods and blackboard for managing multiscale view.

The class maintains a list of all the multiscale actors 
and the current state of the view.

The class also contains an ActorCoordsUtility and a CameraUtility for
manipulating actors and cameras.
*******************************************************************************/

class lhpMultiscaleUtility
{
public:
  lhpMultiscaleUtility() ;
  ~lhpMultiscaleUtility() ;


  //----------------------------------------------------------------------------
  // methods which get actor and camera utilities
  //----------------------------------------------------------------------------

  /** lhpMultiscaleActorCoordsUtility provides methods for manipulating the coords and size of actors */
  lhpMultiscaleActorCoordsUtility* GetActorCoordsUtility() {return m_actorCoordsUtility ;}

  /** lhpMultiscaleCameraUtility provides methods for manipulating the camera */
  lhpMultiscaleCameraUtility* GetCameraUtility() {return m_cameraUtility ;}


  //----------------------------------------------------------------------------
  // methods for maintaining list of multiscale actors
  //----------------------------------------------------------------------------

  /** Create multiscale actor from vtk pipe and add to list.
  Type is data actor or token.
  NB If you add actors after having saved the camera view, you must call SaveInitialView() again. */
  void AddMultiscaleActor(lhpMultiscalePipeline *pipeline, lhpMultiscale::MultiscaleActorType type) ;

  /** Get ith multiscale actor */
  lhpMultiscaleActor* GetMultiscaleActor(int i) ;

  /** Get number of multiscale actors */
  int GetNumberOfActors() {return (int)m_multiscaleActors.size() ;}

  /** Get the token actor which corresponds to data-actor i */
  int GetTokenCorrespondingToDataActor(int i) ;

  /** Get the data-actor which corresponds to token i */
  int GetDataActorCorrespondingToToken(int i) ;

  /** Set actors i and j as a data-actor/token pair.
  You can only call this once for each i and j. */
  void SetActorTokenPair(int i, int j) ;


  //----------------------------------------------------------------------------
  // Methods for attention.
  // Attention defines which actors you are currently looking at.
  // The focal point of the camera is centred on the actor(s) with attention.
  // Actors with attention do not vanish when they become too big.
  // The range of the slice position is set the bounds of the actor(s) with attention.
  //----------------------------------------------------------------------------

  /** set attention of all actors to true or false */
  void SetAttentionAll(bool attention) ;

  /** set attention to all actors in list */
  void SetAttentionToList(const std::vector<int> &idlist, bool attention) ;

  /** get the bounds of all the actors with current attention */
  void GetAttentionBounds(double *bounds) ;


  //----------------------------------------------------------------------------
  // methods for touching tokens
  //----------------------------------------------------------------------------

  /** Add pair of touching tokens to list */
  void AddTouchingTokens(int tokenid0, int tokenid1) ;

  /** Find and remove pair of touching tokens from list */
  void RemoveTouchingTokens(int tokenid0, int vid1) ;

  /** Get number of tokens which touch this one */
  int GetNumberOfTouchingTokens(int tokenId) ;

  /** Get number of touching tokens whose data actors are larger
  If actors are the same size, the lower token index is considered larger. */
  int GetNumberOfTouchingTokensLarger(int tokenId) ;

  /** Are tokens in list of touching pairs */
  bool TokensListedAsTouching(int tokenid0, int tokenid1) ;

  /** Print list of touching tokens */
  void PrintTouchingTokensList() ;

  /** Return actor collection consisting of the data actors of the tokens which touch this one
  The collection includes the input token */
  void GetTouchingDataActorsAsCollection(int tokenId, vtkActorCollection *AC) ;

  /** Return id list consisting of all the data actors of the tokens which touch this one
  The list includes the input token */
  void GetTouchingDataActorsAsIdList(int tokenId, std::vector<int> &idlist) ;

  /** Get distance apart of two actors in pixel units
  This is not the 2D separation - it is the 3D distance in pixels, independent of view direction */
  double DistanceApartPixelUnits(vtkActor *actor1, vtkActor *actor2, vtkRenderer *renderer) ;


  //----------------------------------------------------------------------------
  // methods for saving and restoring the view settings
  //----------------------------------------------------------------------------

  /** Save current view state.  
  Saves camera parameters and attention flags so that we can return to a previous state. */
  void SaveView(vtkRenderer *renderer) ;

  /** Save current view state as intial view
  Saves camera parameters and attention flags so that we can return to a previous state.
  If you add more actors, you must call this again. */
  void SaveInitialView(vtkRenderer *renderer) ;

  /** Restore view to previous state
  If the states stored are O, A, B, C, and our current state is C' (ie C plus camera movement)
  this will delete C and return the view to B. */
  void RestoreView(vtkRenderer* renderer) ;

  /** Restore view to last state without deleting saved state
  If the states stored are O, A, B, C, and our current state is C' (ie C plus camera movement)
  this will return the state to C, without deleting C. */
  void RestoreViewWithoutDelete(vtkRenderer* renderer) ;

  /** Clear all camera saves except the intial state */
  void ClearViewSaves() ;

  /** Has camera changed since last save */
  bool CameraSameSinceLastSave(vtkRenderer *renderer) ;

  /** Reset camera to fit all actors */
  void ResetCameraFitAll(vtkRenderer *renderer) ;


  //----------------------------------------------------------------------------
  // methods for scale units
  //----------------------------------------------------------------------------

  /** convert scale to tidy units, eg 0.342 m -> 300 mm */
  void ConvertScaleToTidyUnits(double scale, int baseUnits, int *iscale, std::ostrstream& units) ;


  //----------------------------------------------------------------------------
  // methods for debugging
  //----------------------------------------------------------------------------

  /** Print self */
  void PrintSelf(std::ostream& os, vtkIndent indent) ;


private:
  /** Find pair of touching tokens in the list
  Returns -1 if not found */
  int FindTouchingTokens(int tokenId0, int tokenId1) ;

  /** list of multiscale actors */
  std::vector<lhpMultiscaleActor> m_multiscaleActors ;

  /** table of correspondences between actors and tokens.
  if actor i is a data-actor,  m_actorPairs[i] is its token. 
  "    "   "  " " token,       m_actorPairs[i] is its data actor. */
  int m_actorPairs[100] ;

  /** table of pairs of touching tokens */
  std::vector<ActorIdPair> m_touchingTokens ;

  /** utilities (these are created and deleted by the class)
  it is convenient but not essential for this class to have pointers to these classes */
  lhpMultiscaleActorCoordsUtility *m_actorCoordsUtility ;
  lhpMultiscaleCameraUtility *m_cameraUtility ;

  /** stack of saved states */
  std::vector<ViewState> m_savedViewStates ;
} ;

#endif
