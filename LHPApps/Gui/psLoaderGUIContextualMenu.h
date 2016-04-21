/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: psLoaderGUIContextualMenu.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:52 $
  Version:   $Revision: 1.1 $
  Authors:   Daniele Giunchi    
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __medGUIContextualMenu_H__
#define __medGUIContextualMenu_H__

#include "lhpGuiDefines.h"

//----------------------------------------------------------------------------
// forward references;
//----------------------------------------------------------------------------
class mafView;
class mafEvent;
class mafObserver;

//----------------------------------------------------------------------------
// medGUIContextualMenu :
//----------------------------------------------------------------------------
class LHP_GUI_EXPORT psLoaderGUIContextualMenu : public wxMenu
{
public:
  psLoaderGUIContextualMenu();
  virtual ~psLoaderGUIContextualMenu();
  void SetListener(mafObserver *Listener) {m_Listener = Listener;};

	/** 
  Visualize contextual menù for the MDI child and selected view. */
  void ShowContextualMenu(wxFrame *child, mafView *view, bool vme_menu);		

protected:
  wxFrame     *m_ChildViewActive;
  mafView     *m_ViewActive;
  mafObserver *m_Listener;

	/** 
  Answer contextual menù's selection. */
	void OnContextualViewMenu(wxCommandEvent& event);
  DECLARE_EVENT_TABLE()
};
#endif
