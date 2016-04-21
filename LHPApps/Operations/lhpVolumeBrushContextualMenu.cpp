/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpVolumeBrushContextualMenu.cpp,v $
Language:  C++
Date:      $Date: 2010-06-03 16:03:24 $
Version:   $Revision: 1.1.2.2 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2009
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "lhpVolumeBrushContextualMenu.h"

#include "wx/frame.h"
#include "mafViewCompound.h"

// from medGUIContextualMenu.cpp
enum VIEW_CONTEXTUAL_MENU_ID
{
  CONTEXTUAL_VIEW_MENU_START = MINID,	
		CONTEXTUAL_MENU_RENAME_VIEW,				
		CONTEXTUAL_MENU_HIDE_VME,				
		CONTEXTUAL_MENU_DELETE_VME,
		CONTEXTUAL_MENU_TRANSFORM,
    CONTEXTUAL_MENU_VME_PIPE,
		CONTEXTUAL_MENU_QUIT_CHILD_VIEW,
		CONTEXTUAL_MENU_MAXIMIZE_CHILD_VIEW,
    CONTEXTUAL_MENU_MAXIMIZE_CHILD_SUB_VIEW,
		CONTEXTUAL_MENU_NORMAL_SIZE_CHILD_VIEW,
    CONTEXTUAL_MENU_NORMAL_SIZE_CHILD_SUB_VIEW,
		CONTEXTUAL_MENU_SAVE_AS_IMAGE,
    CONTEXTUAL_MENU_SAVE_ALL_AS_IMAGE,
    CONTEXTUAL_MENU_EXPORT_AS_VRML,
    //CONTEXTUAL_MENU_EXTERNAL_INTERNAL_VIEW,
	CONTEXTUAL_VIEW_MENU_STOP
};

BEGIN_EVENT_TABLE(lhpVolumeBrushContextualMenu, medGUIContextualMenu)
  EVT_MENU_RANGE(CONTEXTUAL_MENU_NORMAL_SIZE_CHILD_VIEW, CONTEXTUAL_MENU_MAXIMIZE_CHILD_SUB_VIEW, medGUIContextualMenu::OnContextualViewMenu)
END_EVENT_TABLE()

lhpVolumeBrushContextualMenu::lhpVolumeBrushContextualMenu()
:medGUIContextualMenu()
//----------------------------------------------------------------------------
{
	//m_ChildViewActive = NULL;
	//m_ViewActive      = NULL;
	//m_Listener        = NULL;
}

//----------------------------------------------------------------------------
lhpVolumeBrushContextualMenu::~lhpVolumeBrushContextualMenu()
//----------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------
void lhpVolumeBrushContextualMenu::ShowContextualMenu(wxFrame *child, mafView *view, bool vme_menu)
//----------------------------------------------------------------------------
{
	m_ChildViewActive = child;
	m_ViewActive      = view;

	m_ChildViewActive->SetFocus();

	/*
	if(vme_menu)
	{
		this->Append(CONTEXTUAL_MENU_HIDE_VME, "Hide");
		this->Append(CONTEXTUAL_MENU_DELETE_VME, "Delete");
		this->Append(CONTEXTUAL_MENU_TRANSFORM, "Move\tCtrl+T");
    this->AppendSeparator();
    this->Append(CONTEXTUAL_MENU_VME_PIPE,"visual props");
    this->AppendSeparator();
  }
  */
	//this->Append(CONTEXTUAL_MENU_RENAME_VIEW, "Rename View");
  mafViewCompound *vc = mafViewCompound::SafeDownCast(m_ViewActive);
#ifdef WIN32  
	//this->AppendSeparator();
	if (m_ChildViewActive->IsMaximized())
	{
		this->Append(CONTEXTUAL_MENU_NORMAL_SIZE_CHILD_VIEW, "Normal Size");
	}
	else
	{
		this->Append(CONTEXTUAL_MENU_MAXIMIZE_CHILD_VIEW, "Maximize");
	}
	if (vc)
	{
		if (vc->IsSubViewMaximized())
		{
			this->Append(CONTEXTUAL_MENU_NORMAL_SIZE_CHILD_SUB_VIEW, "Normal Size SubView");
		}
		else
		{
			this->Append(CONTEXTUAL_MENU_MAXIMIZE_CHILD_SUB_VIEW, "Maximize SubView");
		}
	}
#endif
  //this->Append(CONTEXTUAL_MENU_EXTERNAL_INTERNAL_VIEW, "External", "Switch view visualization between external/internal", TRUE);
  //this->FindItem(CONTEXTUAL_MENU_EXTERNAL_INTERNAL_VIEW)->Check(m_ViewActive->IsExternal());
	/*	
  this->AppendSeparator();
	this->Append(CONTEXTUAL_MENU_SAVE_AS_IMAGE, _("Save as Image"));

  if (vc)
  {
    this->Append(CONTEXTUAL_MENU_SAVE_ALL_AS_IMAGE, _("Save All as Image"));
  }  
  
  
  this->Append(CONTEXTUAL_MENU_EXPORT_AS_VRML, _("Export Scene (VRML)"));
  */

	int x,y;
	::wxGetMousePosition(&x, &y);
	m_ChildViewActive->ScreenToClient(&x, &y);

	m_ViewActive->GetWindow()->PopupMenu(this, wxPoint(x, y));
		
	m_ChildViewActive->Refresh();
}

/*
void lhpVolumeBrushContextualMenu::OnContextualViewMenu(wxCommandEvent& event)
{
	medGUIContextualMenu::OnContextualViewMenu(event);
}
*/
