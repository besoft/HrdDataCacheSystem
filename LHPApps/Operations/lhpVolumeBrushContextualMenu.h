/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpVolumeBrushContextualMenu.h,v $
  Language:  C++
  Date:      $Date: 2010-06-03 08:33:28 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Youbing Zhao    
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpVolumeBrushContextualMenu_H__
#define __lhpVolumeBrushContextualMenu_H__

#include "medGUIContextualMenu.h"
//----------------------------------------------------------------------------
// forward references;
//----------------------------------------------------------------------------
class mafView;
class mafEvent;
class mafObserver;

class wxFrame;

/**
  Class Name: lhpVolumeBrushContextualMenu.
  Represents the contexctual menu' which compares when click right button 
  over a VolumeBrush operation viewport.
  adapted from medGUIContextualMenu
*/
class lhpVolumeBrushContextualMenu : public medGUIContextualMenu
{
public:
  /** constructor. */
  lhpVolumeBrushContextualMenu();
  /** destructor. */
  virtual ~lhpVolumeBrushContextualMenu();
  /** Set the listener object, i.e. the object receiving events sent by this object */
  void SetListener(mafObserver *Listener) {m_Listener = Listener;};

	/** 
  Visualize contextual menu for the MDI child and selected view. */
  void ShowContextualMenu(wxFrame *child, mafView *view, bool vme_menu);		

protected:
  //wxFrame     *m_ChildViewActive;
  //mafView     *m_ViewActive;
  //mafObserver *m_Listener;

	/** Answer contextual menu's selection. */
	//void OnContextualViewMenu(wxCommandEvent& event);

  /** Event Table Declaration. */
  DECLARE_EVENT_TABLE()
};
#endif
