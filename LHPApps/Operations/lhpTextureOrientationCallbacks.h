/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpTextureOrientationCallbacks.h,v $
Language:  C++
Date:      $Date: 2009-06-26 14:03:04 $
Version:   $Revision: 1.1.1.1.2.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpTextureOrientationCallbacks_H__
#define __lhpTextureOrientationCallbacks_H__

#include "vtkCommand.h"
#include "vtkObject.h"
#include "mafObserver.h"
#include "mafInteractor.h"


//------------------------------------------------------------------------------
/// Helper class for lhpOpTextureOrientation. \n
/// Callback to convert vtk event into a maf event. \n
/// Plugs into lhpTextureOrientationFilter and responds to user progress event. \n
/// Throws maf event id back to listener->OnEvent().
//------------------------------------------------------------------------------
class lhpTextureOrientationProgressCallback : public vtkCommand
{
public:
  static lhpTextureOrientationProgressCallback *New() { return new lhpTextureOrientationProgressCallback ; }
  void Execute(vtkObject *caller, unsigned long eventId, void* calldata) ;

  /// Set the listener which is to receive the maf event
  void SetListener(mafObserver *listener) {m_Listener = listener ;}

  /// This sets the id of the maf event to be sent to the listener. \n
  /// NB since there is only one listener, we let the listener set the id. \n
  /// This way the listener can choose a valid id, and define it in the \n
  /// same place as the rest of its id's.
  void SetMafEventId(int id) {m_id = id ;}

  vtkTypeMacro(lhpTextureOrientationProgressCallback,vtkCommand);

private:
  mafObserver *m_Listener ; ///< required by mafEventMacro
  int m_id ;                ///< id of maf event
};








#endif
