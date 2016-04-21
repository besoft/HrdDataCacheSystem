/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleCallbacks.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpMultiscaleCallbacks_H__
#define __lhpMultiscaleCallbacks_H__

#include "vtkCommand.h"
#include "vtkObject.h"
#include "mafObserver.h"
#include "mafInteractor.h"


/*******************************************************************************
Callback to convert vtk event into a maf event.
Plugs into the renderer and responds to start render event.
Throws maf event id back to listener->OnEvent().
*******************************************************************************/
class vtkStartRenderCallback: public vtkCommand
{
public:
  static vtkStartRenderCallback *New() { return new vtkStartRenderCallback; }
  void Execute(vtkObject *caller, unsigned long, void*) ;

  /** Set the listener which is to receive the maf event */
  void SetListener(mafObserver *listener) {m_Listener = listener ;}

  /** This sets the id of the maf event to be sent to the listener. */
  // NB since there is only one listener, we let the listener set the id.
  // This way the listener can choose a valid id, and define it in the
  // same place as the rest of its id's.
  void SetMafEventId(int id) {m_id = id ;}

  vtkTypeMacro(vtkStartRenderCallback, vtkCommand);

private:
  mafObserver *m_Listener ; // required by mafEventMacro
  int m_id ;                // id of maf event
};


/*******************************************************************************
Callback to convert vtk event into a maf event.
Plugs into the interactor and responds to mouse click event.
Throws maf event id back to listener->OnEvent().
*******************************************************************************/
class vtkMouseClickCallback: public vtkCommand
{
public:
  static vtkMouseClickCallback *New() { return new vtkMouseClickCallback; }
  void Execute(vtkObject *caller, unsigned long, void*) ;

  /** Set the listener which is to receive the maf event */
  void SetListener(mafObserver *listener) {m_Listener = listener ;}

  /** This sets the id of the maf event to be sent to the listener. */
  // NB since there is only one listener, we let the listener set the id.
  // This way the listener can choose a valid id, and define it in the
  // same place as the rest of its id's.
  void SetMafEventId(int id) {m_id = id ;}

  vtkTypeMacro(vtkMouseClickCallback,vtkCommand);

private:
  mafObserver *m_Listener ; // required by mafEventMacro
  int m_id ;                // id of maf event
};




/*******************************************************************************
Callback to catch the double click event.
Throws maf event id back to listener->OnEvent().
*******************************************************************************/
class lhpMultiscaleDoubleClickCallback: public mafObserver
{
public:
  //mafTypeMacro(lhpMultiscaleDoubleClickCallback, mafObserver);

  /** Override the OnEvent() method */
  void OnEvent(mafEventBase *event) ;

  /** Set the listener which is to receive the maf event */
  void SetListener(mafObserver *listener) {m_Listener = listener ;}

  /** This sets the id of the maf event to be sent to the listener. */
  //NB since there is only one listener, we let the listener set the id.
  //This way the listener can choose a valid id, and define it in the
  //same place as the rest of its id's.
  void SetMafEventId(int id) {m_id = id ;}

private:
  mafObserver *m_Listener ; // required by mafEventMacro
  int m_id ;                // id of maf event
};




#endif
