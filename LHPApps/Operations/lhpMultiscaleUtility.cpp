/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleUtility.cpp,v $
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
#include "vtkMatrix4x4.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkLight.h"
#include "vtkLightCollection.h"
#include "vtkActorCollection.h"
#include "vtkIdType.h"

#include "lhpMultiscaleActor.h"
#include "lhpMultiscaleActorCoordsUtility.h"
#include "lhpMultiscaleCameraParams.h"
#include "lhpMultiscaleCameraUtility.h"
#include "lhpMultiscaleUtility.h"
#include "lhpMultiscaleVisualPipes.h"

#include <cmath>
#include <iostream>
#include <assert.h>


//------------------------------------------------------------------------------
using namespace lhpMultiscale ;
//------------------------------------------------------------------------------




//------------------------------------------------------------------------------
// Constructor
lhpMultiscaleUtility::lhpMultiscaleUtility()
//------------------------------------------------------------------------------
{
  m_actorCoordsUtility = new lhpMultiscaleActorCoordsUtility ;
  m_cameraUtility = new lhpMultiscaleCameraUtility ;
}



//------------------------------------------------------------------------------
// Destructor
lhpMultiscaleUtility::~lhpMultiscaleUtility()
//------------------------------------------------------------------------------
{
  delete m_actorCoordsUtility  ;
  delete m_cameraUtility ;
}


//------------------------------------------------------------------------------
// print self
void lhpMultiscaleUtility::PrintSelf(std::ostream& os, vtkIndent indent)
//------------------------------------------------------------------------------
{
  int i ;

  os << indent << "Multiscale Utility" << std::endl << std::endl ;

  // print list of multiscale actors
  os << indent << "multiscale actors" << std::endl ;
  for (i = 0 ;  i < GetNumberOfActors() ;  i++){
    os << indent << i << " " ;
    GetMultiscaleActor(i)->PrintSelf(os, indent) ;
  }
  os << std::endl ;

  // print list of actor-token pairs
  os << indent << "actor-token pairs" << std::endl ;
  for (i = 0 ;  i < GetNumberOfActors() ;  i++){
    os << indent << i << " " << m_actorPairs[i] << std::endl ;
  }
  os << std::endl ;

  // print list of touching tokens
  os << indent << "touching tokens" << std::endl ;
  for (i = 0 ;  i < (int)m_touchingTokens.size() ;  i++){
    ActorIdPair idpair = m_touchingTokens.at(i) ;
    os << indent << idpair.GetId0() << " " << idpair.GetId1() << std::endl ;
  }
  os << std::endl ;

  // print saved camera states
  os << indent << "saved camera states (starting with most recent)" << std::endl ;
  for (i = (int)m_savedViewStates.size()-1 ;  i >= 0 ;  i--){
    ViewState VS = m_savedViewStates.at(i) ;
    VS.cameraParams.PrintSelf(os, indent) ;
  }
  os << std::endl << std::endl ;
}


//------------------------------------------------------------------------------
// Create multiscale actor from given actor and add to list
void lhpMultiscaleUtility::AddMultiscaleActor(lhpMultiscalePipeline *pipeline, lhpMultiscale::MultiscaleActorType type)
//------------------------------------------------------------------------------
{
  lhpMultiscaleActor mso(pipeline, type) ;
  m_multiscaleActors.push_back(mso) ;
}




//------------------------------------------------------------------------------
// Get multiscale actor/mapper
lhpMultiscaleActor* lhpMultiscaleUtility::GetMultiscaleActor(int i)
//------------------------------------------------------------------------------
{
  return &(m_multiscaleActors.at(i)) ;
}




//------------------------------------------------------------------------------
// Get the token actor which corresponds to data-actor i
int lhpMultiscaleUtility::GetTokenCorrespondingToDataActor(int i)
//------------------------------------------------------------------------------
{
  // check that input is a data actor
  if (GetMultiscaleActor(i)->GetActorType() != MSCALE_DATA_ACTOR){
    std::cout << "GetTokenCorrespondingToDataActor(): actor " << i << " incorrect type" << std::endl ;
    assert(false) ;
  }

  int j = m_actorPairs[i] ;

  // check that output is a token
  if (GetMultiscaleActor(j)->GetActorType() != MSCALE_TOKEN){
    std::cout << "GetTokenCorrespondingToDataActor(): actor " << j << " incorrect type" << std::endl ;
    assert(false) ;
  }

  // return the token id
  return j ;
}


//------------------------------------------------------------------------------
// Get the data-actor which corresponds to token i
int lhpMultiscaleUtility::GetDataActorCorrespondingToToken(int i)
//------------------------------------------------------------------------------
{
  // check that input is a token
  if (GetMultiscaleActor(i)->GetActorType() != MSCALE_TOKEN){
    std::cout << "GetDataActorCorrespondingToToken(): actor " << i << " incorrect type" << std::endl ;
    assert(false) ;
  }

  int j = m_actorPairs[i] ;

  // check that output is a data actor
  if (GetMultiscaleActor(j)->GetActorType() != MSCALE_DATA_ACTOR){
    std::cout << "GetDataActorCorrespondingToToken(): actor " << j << " incorrect type" << std::endl ;
    assert(false) ;
  }

  // return the actor id
  return j ;
}


//------------------------------------------------------------------------------
// Set actors i and j as a data-actor/token pair
void lhpMultiscaleUtility::SetActorTokenPair(int i, int j)
//------------------------------------------------------------------------------
{
  int typei = GetMultiscaleActor(i)->GetActorType() ;
  int typej = GetMultiscaleActor(j)->GetActorType() ;
  if (!((typei == MSCALE_DATA_ACTOR && typej == MSCALE_TOKEN) || (typei == MSCALE_TOKEN && typej == MSCALE_DATA_ACTOR))){
    std::cout << "SetActorTokenPair(): actors " << i << " and " << j << " must be data-actor and token" << std::endl ;
    assert(false) ;
  }

  // get lists for each actor
  m_actorPairs[i] = j ;
  m_actorPairs[j] = i ;
}



//------------------------------------------------------------------------------
// Save current view state.  
// Saves camera parameters and attention flags so that we can return to a previous state.
// We save the attention flags as well as the camera state so we remember which actor(s) we we looking at.
//------------------------------------------------------------------------------
void lhpMultiscaleUtility::SaveView(vtkRenderer *renderer)
{
  // save the camera parameters
  lhpMultiscaleCameraParams cp(renderer) ;     // construct camera params from renderer
  ViewState state ;
  state.cameraParams = cp ;

  // save the attention flags of the actors
  state.attentionFlags.clear() ;
  for (int i = 0 ;  i < GetNumberOfActors() ;  i++){
    bool attention = GetMultiscaleActor(i)->GetAttention() ;
    state.attentionFlags.push_back(attention) ;
  }

  m_savedViewStates.push_back(state) ;  // push onto stack
}


//------------------------------------------------------------------------------
// Save current view state as intial view
// Saves camera parameters and attention flags so that we can return to a previous state.
// If you add more actors, you must call this again.
void lhpMultiscaleUtility::SaveInitialView(vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  // Clear all previous saves
  while (m_savedViewStates.size() > 0)
    m_savedViewStates.pop_back() ;

  // Save current view
  SaveView(renderer) ;
}


//------------------------------------------------------------------------------
// Restore view state (camera and attention flags)
// If the states stored are O, A, B, C, and our current state is C' (ie C plus camera movement)
// then restore will delete C and return the view to B.
void lhpMultiscaleUtility::RestoreView(vtkRenderer* renderer)
//------------------------------------------------------------------------------
{
  // delete top of stack, unless there is only one left
  if (m_savedViewStates.size() > 1)
    m_savedViewStates.pop_back() ;

  // get the previously saved state
  ViewState state = m_savedViewStates.back() ;
 
  // restore the camera settings
  state.cameraParams.RestoreCamera(renderer) ;

  // copy the attention flags
  for (int i = 0 ;  i < GetNumberOfActors() ;  i++){
    bool attention = state.attentionFlags.at(i) ;
    GetMultiscaleActor(i)->SetAttention(attention) ;
  }
}



//------------------------------------------------------------------------------
// Restore view to last state without deleting saved state
// If the states stored are O, A, B, C, and our current state is C' (ie C plus camera movement)
// this will return the state to C, without deleting C.
void lhpMultiscaleUtility::RestoreViewWithoutDelete(vtkRenderer* renderer)
//------------------------------------------------------------------------------
{
  // get the last saved state
  ViewState state = m_savedViewStates.back() ;

  // restore the camera settings
  state.cameraParams.RestoreCamera(renderer) ;

  // copy the attention flags
  for (int i = 0 ;  i < GetNumberOfActors() ;  i++){
    bool attention = state.attentionFlags.at(i) ;
    GetMultiscaleActor(i)->SetAttention(attention) ;
  }

}


//------------------------------------------------------------------------------
// Clear all view saves except the intial state
void lhpMultiscaleUtility::ClearViewSaves()
//------------------------------------------------------------------------------
{
  // delete stack until there is only one item left
  while (m_savedViewStates.size() > 1)
    m_savedViewStates.pop_back() ;
}


//------------------------------------------------------------------------------
// Has camera changed since last save
bool lhpMultiscaleUtility::CameraSameSinceLastSave(vtkRenderer *renderer)
//------------------------------------------------------------------------------
{

  // get current and saved params
  lhpMultiscaleCameraParams currentParams(renderer) ;
  lhpMultiscaleCameraParams savedParams = ((ViewState)m_savedViewStates.back()).cameraParams ;

  return (currentParams == savedParams) ;
}


//------------------------------------------------------------------------------
// Reset camera to fit all actors
void lhpMultiscaleUtility::ResetCameraFitAll(vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  // Get all the data actors as a collection
  vtkActorCollection *AC = vtkActorCollection::New() ;
  for (int i = 0 ;  i < GetNumberOfActors() ;  i++){
    if (GetMultiscaleActor(i)->GetActorType() == MSCALE_DATA_ACTOR){
      vtkActor *actor = GetMultiscaleActor(i)->GetActor() ;
      AC->AddItem(actor) ;
    }
  }

  // Reposition the camera on the collection
  // If actors have been introduced behind the camera, zooming might not be enough -
  // we have to change the position as well.
  GetCameraUtility()->RepositionCameraOnActorCollection(AC, renderer) ;

  AC->Delete() ;
}



//------------------------------------------------------------------------------
// convert scale value to tidy units, eg 0.342 m -> 300 mm, or 0.6 cm -> 60 mm
// scale is the input value, and baseUnits is the units which the value is expressed in.
void lhpMultiscaleUtility::ConvertScaleToTidyUnits(double scale, int baseUnits, int *iscale, std::ostrstream& units)
//------------------------------------------------------------------------------
{
  // Convert scale to metres
  switch(baseUnits){
    case ID_METRES:
      break ;
    case ID_CENTIMETRES:
      scale /= 1E2 ;
      break ;
    case ID_MILLIMETRES:
      scale /= 1E3 ;
      break ;
    case ID_MICRONS:
      scale /= 1E6 ;
      break ;
    case ID_NANOMETRES:
      scale /= 1E9 ;
      break ;
    default:
      mafLogMessage("unknown base unit in lhpMultiscaleUtility::ConvertScaleToTidyUnits()") ;
      assert(false) ;
      break ;
  }


  // get log base 10 of scale
  double logscale = std::log10(scale) ;
  int ilogscale = (int)logscale ;
  if (logscale < 0.0)
    ilogscale -= 1 ;


  if (ilogscale >= 0){
    units << "m" << std::ends ;
    *iscale = (int)(scale / pow(10.0,ilogscale) + 0.5) ;	//BES: 3.3.2008 - 10 => 10.0
    *iscale *= std::pow(10.0,ilogscale) ;					//BES: 3.3.2008 - 10 => 10.0
    return ;
  }

  switch(ilogscale){
    case -1:
      units << "mm" << std::ends ;
      *iscale = (int)(scale * 1E1 + 0.5) ;
      *iscale *= 100 ;
      break ;
    case -2:
      units << "mm" << std::ends ;
      *iscale = (int)(scale * 1E2 + 0.5) ;
      *iscale *= 10 ;
      break ;
    case -3:
      units << "mm" << std::ends ;
      *iscale = (int)(scale * 1E3 + 0.5) ;
      break ;
    case -4:
      units << "micron" << std::ends ;
      *iscale = (int)(scale * 1E4 + 0.5) ;
      *iscale *= 100 ;
      break ;
    case -5:
      units << "micron." << std::ends ;
      *iscale = (int)(scale * 1E5 + 0.5) ;
      *iscale *= 10 ;
      break ;
    case -6:
      units << "micron." << std::ends ;
      *iscale = (int)(scale * 1E6 + 0.5) ;
      break ;
    case -7:
      units << "nm" << std::ends ;
      *iscale = (int)(scale * 1E7 + 0.5) ;
      *iscale *= 100 ;
      break ;
    case -8:
      units << "nm" << std::ends ;
      *iscale = (int)(scale * 1E8 + 0.5) ;
      *iscale *= 10 ;
      break ;
    case -9:
      units << "nm" << std::ends ;
      *iscale = (int)(scale * 1E9 + 0.5) ;
      break ;
    default:
      units << "nm" << std::ends ;
      *iscale = 0 ;
  }
}


//------------------------------------------------------------------------------
// Set attention flag of all actors
void lhpMultiscaleUtility::SetAttentionAll(bool attention)
//------------------------------------------------------------------------------
{
  for (int i = 0 ;  i < GetNumberOfActors() ;  i++)
    GetMultiscaleActor(i)->SetAttention(attention) ;
}


//------------------------------------------------------------------------------
// set attention of all actors in list
void lhpMultiscaleUtility::SetAttentionToList(const std::vector<int> &idlist, bool attention)
//------------------------------------------------------------------------------
{
  for (int i = 0 ;  i < (int)idlist.size() ;  i++){
    int actorId = idlist.at(i) ;
    GetMultiscaleActor(actorId)->SetAttention(attention) ;
  }
}


//------------------------------------------------------------------------------
// get the bounds of all the actors with current attention
void lhpMultiscaleUtility::GetAttentionBounds(double *bounds)
//------------------------------------------------------------------------------
{
  vtkActorCollection *AC = vtkActorCollection::New() ;

  for (int i = 0 ;  i < GetNumberOfActors() ;  i++){
    if (GetMultiscaleActor(i)->GetAttention())
      AC->AddItem(GetMultiscaleActor(i)->GetActor()) ;
  }

  GetActorCoordsUtility()->GetBoundsOfCollection(AC, bounds) ;

  AC->Delete() ;
}


//------------------------------------------------------------------------------
// Add a pair of touching tokens to the list
void lhpMultiscaleUtility::AddTouchingTokens(int tokenId0, int tokenId1)
//------------------------------------------------------------------------------
{
  // check that both actors are tokens
  if ((GetMultiscaleActor(tokenId0)->GetActorType() != MSCALE_TOKEN) || (GetMultiscaleActor(tokenId1)->GetActorType() != MSCALE_TOKEN)){
    std::cout << "AddTouchingTokens(): actor id's must be tokens" << std::endl ;
    assert(false) ;
  }

  ActorIdPair idPair(tokenId0, tokenId1) ;
  m_touchingTokens.push_back(idPair) ;
}


//------------------------------------------------------------------------------
// Find pair of touching tokens in the list
// Returns -1 if not found
int lhpMultiscaleUtility::FindTouchingTokens(int tokenId0, int tokenId1)
//------------------------------------------------------------------------------
{
  // check that both actors are tokens
  if ((GetMultiscaleActor(tokenId0)->GetActorType() != MSCALE_TOKEN) || (GetMultiscaleActor(tokenId1)->GetActorType() != MSCALE_TOKEN)){
    std::cout << "FindTouchingTokens(): actor id's must be tokens" << std::endl ;
    assert(false) ;
  }

  ActorIdPair idPair(tokenId0, tokenId1) ;
  for (int i = 0 ;  i < (int)m_touchingTokens.size() ;  i++){
    if (idPair == m_touchingTokens.at(i))
      return i ;
  }

  return -1 ;
}


//------------------------------------------------------------------------------
// Are tokens in list of touching pairs
bool lhpMultiscaleUtility::TokensListedAsTouching(int tokenId0, int tokenId1)
//------------------------------------------------------------------------------
{
  // check that both actors are tokens
  if ((GetMultiscaleActor(tokenId0)->GetActorType() != MSCALE_TOKEN) || (GetMultiscaleActor(tokenId1)->GetActorType() != MSCALE_TOKEN)){
    std::cout << "TokensListedAsTouching(): actor id's must be tokens" << std::endl ;
    assert(false) ;
  }

  // try to find the pair
  if (FindTouchingTokens(tokenId0, tokenId1) == -1)
    return false ;
  else
    return true ;
}


//------------------------------------------------------------------------------
// Remove a pair of touching tokens from the list
void lhpMultiscaleUtility::RemoveTouchingTokens(int tokenId0, int tokenId1)
//------------------------------------------------------------------------------
{
  // check that both actors are tokens
  if ((GetMultiscaleActor(tokenId0)->GetActorType() != MSCALE_TOKEN) || (GetMultiscaleActor(tokenId1)->GetActorType() != MSCALE_TOKEN)){
    std::cout << "RemoveTouchingTokens(): actor id's must be tokens" << std::endl ;
    assert(false) ;
  }

  // find the pair in the list
  ActorIdPair idPair(tokenId0, tokenId1) ;
  int i = FindTouchingTokens(tokenId0, tokenId1) ;
  if (i == -1){
    std::cout << "RemoveTouchingTokens(): can't find pair " << tokenId0 << " " << tokenId1 << std::endl ;
    assert(false) ;
  }

  std::vector<ActorIdPair>::iterator it = m_touchingTokens.begin() + i ;
  m_touchingTokens.erase(it) ;
}


//------------------------------------------------------------------------------
// Get number of tokens which touch this one
int lhpMultiscaleUtility::GetNumberOfTouchingTokens(int tokenId)
//------------------------------------------------------------------------------
{
  // check that actor is a token
  if (GetMultiscaleActor(tokenId)->GetActorType() != MSCALE_TOKEN){
    std::cout << "GetNumberOfTouchingTokens(): actor id must be token" << std::endl ;
    assert(false) ;
  }

  std::vector<int> idlist ;
  GetTouchingDataActorsAsIdList(tokenId, idlist) ;
  return((int)idlist.size() - 1) ;
}


//------------------------------------------------------------------------------
// Get number of touching tokens whose data actors are larger
int lhpMultiscaleUtility::GetNumberOfTouchingTokensLarger(int tokenId)
//------------------------------------------------------------------------------
{
  int i, sum ;

  // check that actor is a token
  if (GetMultiscaleActor(tokenId)->GetActorType() != MSCALE_TOKEN){
    std::cout << "GetNumberOfTouchingTokens(): actor id must be token" << std::endl ;
    assert(false) ;
  }

  // get size of input data actor
  int idIn = this->GetDataActorCorrespondingToToken(tokenId) ;
  vtkActor *actorIn = GetMultiscaleActor(idIn)->GetActor() ;
  double actorInSize = this->GetActorCoordsUtility()->GetMaxSizeWorld(actorIn) ;

  // get list of touching tokens
  std::vector<int> idlist ;
  GetTouchingDataActorsAsIdList(tokenId, idlist) ;

  for (i = 0, sum = 0 ;  i < (int)idlist.size() ;  i++){
    // get id's of data actor and token
    int id = idlist.at(i) ;
    int tkid = this->GetTokenCorrespondingToDataActor(id) ;

    if (tkid != tokenId){
      // get size of data actor
      vtkActor *actor = GetMultiscaleActor(id)->GetActor() ;
      double actorSize = this->GetActorCoordsUtility()->GetMaxSizeWorld(actor) ;

      // compare with input actor and increment sum if larger
      // nb if actors are same size, the lower index is considered larger.  See ScaleCallback::OnTokensOverlap().
      if (actorSize > actorInSize)
        sum++ ;
      else if ((actorSize == actorInSize) && (id < idIn))
        sum++ ;
    }
  }
  
  return sum ;
}


//------------------------------------------------------------------------------
// Print list of touching tokens
void lhpMultiscaleUtility::PrintTouchingTokensList()
//------------------------------------------------------------------------------
{
  for (int i = 0 ;  i < (int)m_touchingTokens.size() ;  i++){
    ActorIdPair idPair = m_touchingTokens.at(i) ;
    std::cout << idPair.GetId0() << " " << idPair.GetId1() << std::endl ;
  }
  std::cout << std::endl ;
}



//------------------------------------------------------------------------------
// Return actor collection consisting of the data actors of the tokens which touch this one
// The collection includes the input token
void lhpMultiscaleUtility::GetTouchingDataActorsAsCollection(int tokenId, vtkActorCollection *AC)
//------------------------------------------------------------------------------
{
  int actorId ;

  // empty the collection
  AC->RemoveAllItems() ;

  // put input token's data actor into collection
  actorId = GetDataActorCorrespondingToToken(tokenId) ;
  AC->AddItem(GetMultiscaleActor(actorId)->GetActor()) ;

  // add any data actors whose tokens touch it to the collection
  for (int i = 0 ;  i < (int)m_touchingTokens.size() ; i++){
    ActorIdPair idPair = m_touchingTokens.at(i) ;
    int otherId ;
    if (idPair.GetId0() == tokenId){
      otherId = idPair.GetId1() ;
      actorId = GetDataActorCorrespondingToToken(otherId) ;
      AC->AddItem(GetMultiscaleActor(actorId)->GetActor()) ;
    }
    else if (idPair.GetId1() == tokenId){
      otherId = idPair.GetId0() ;
      actorId = GetDataActorCorrespondingToToken(otherId) ;
      AC->AddItem(GetMultiscaleActor(actorId)->GetActor()) ;
    }
  }
}


//------------------------------------------------------------------------------
// Return actor collection consisting of the data actors of the tokens which touch this one
// The list includes the input token
void lhpMultiscaleUtility::GetTouchingDataActorsAsIdList(int tokenId, std::vector<int> &idlist)
//------------------------------------------------------------------------------
{
  int actorId ;

  // clear the list
  idlist.clear() ;

  // put input token's data actor into collection
  actorId = GetDataActorCorrespondingToToken(tokenId) ;
  idlist.push_back(actorId) ;

  // add any data actors whose tokens touch it to the collection
  for (int i = 0 ;  i < (int)m_touchingTokens.size() ; i++){
    ActorIdPair idPair = m_touchingTokens.at(i) ;
    int otherId ;
    if (idPair.GetId0() == tokenId){
      otherId = idPair.GetId1() ;
      actorId = GetDataActorCorrespondingToToken(otherId) ;
      idlist.push_back(actorId) ;
    }
    else if (idPair.GetId1() == tokenId){
      otherId = idPair.GetId0() ;
      actorId = GetDataActorCorrespondingToToken(otherId) ;
      idlist.push_back(actorId) ;
    }
  }
}


//------------------------------------------------------------------------------
// Get distance apart of two actors in pixel units
// This is not the 2D separation - it is the 3D distance in pixels, independent of view direction
double lhpMultiscaleUtility::DistanceApartPixelUnits(vtkActor *actor1, vtkActor *actor2, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  double wpFactor = GetCameraUtility()->CalculateWorldToPixelsFactor(renderer) ;  // get world to pixels conversion factor
  double distW = GetActorCoordsUtility()->DistanceApartWorld(actor1, actor2) ;    // get separation in world coords
  return wpFactor * distW ;                                                       // return distance in pixels
}