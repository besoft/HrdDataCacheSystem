
/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpSetLowerRes.cpp,v $
  Language:  C++
  Date:      $Date: 2011-03-25 11:47:55 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Josef Kohout
==========================================================================
  Copyright (c) 2011
  University of West Bohemia
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpSetLowerRes.h"

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafGUIDialog.h"
#include "mafGUIValidator.h"
#include "mafVME.h"

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------
/*static const char* lhpOpSetLowerRes::LINK_NAME_LR = "LowerResVME";*/
/*static const char* lhpOpSetLowerRes::LINK_NAME_HULL = "HullVME";*/
//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpSetLowerRes);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpSetLowerRes::lhpOpSetLowerRes(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;
	m_SelectedLowResVME = NULL;
	m_SelectedHullVME = NULL;

	m_UndoLowResVMEId = m_UndoHullVMEId = -1;
	m_Dialog = NULL;
  m_Canundo = true;  
}
//----------------------------------------------------------------------------
lhpOpSetLowerRes::~lhpOpSetLowerRes( ) 
//----------------------------------------------------------------------------
{
  
}
//----------------------------------------------------------------------------
mafOp* lhpOpSetLowerRes::Copy()   
//----------------------------------------------------------------------------
{
	return new lhpOpSetLowerRes(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpSetLowerRes::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return (node && node->IsMAFType(mafVME));
}
//----------------------------------------------------------------------------
void lhpOpSetLowerRes::OpRun()   
//----------------------------------------------------------------------------
{ 		
	//initialize everything
	if (NULL != (m_SelectedLowResVME = mafVME::SafeDownCast(m_Input->GetLink(GetLinkNameLR()))))
	{
		m_SelectedLowResVMEName = m_SelectedLowResVME->GetName();
		m_UndoLowResVMEId = m_SelectedLowResVME->GetId();
	}
	else 
	{
		m_SelectedLowResVMEName = "";
		m_UndoLowResVMEId = -1;
	}

	if (NULL != (m_SelectedHullVME = mafVME::SafeDownCast(m_Input->GetLink(GetLinkNameHULL()))))
	{
		m_SelectedHullVMEName = m_SelectedHullVME->GetName();
		m_UndoHullVMEId = m_SelectedHullVME->GetId();
	}
	else 
	{
		m_SelectedHullVMEName = "";
		m_UndoHullVMEId = -1;
	}
	

	// interface:	
  CreateOpDialog();  

  int result = m_Dialog->ShowModal() == wxID_OK ? OP_RUN_OK : OP_RUN_CANCEL;
  DeleteOpDialog();

  mafEventMacro(mafEvent(this, result));
}


//----------------------------------------------------------------------------
//Creates GUI
/*virtual*/void lhpOpSetLowerRes::CreateOpDialog()
//----------------------------------------------------------------------------
{
  wxBusyCursor wait;

  //===== setup interface ====
  m_Dialog = new mafGUIDialog(m_Label, mafCLOSEWINDOW | mafRESIZABLE);  
    
  //The following code was originally generated using wxFormBuilder
  //and modified here to work with MAF
#pragma region GUI
	
	wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );	

	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer2->Add( new wxStaticText( m_Dialog, wxID_ANY, wxT("VME with Lower Res:"), wxDefaultPosition, wxSize( 120,-1 ), 0 ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );	
	wxTextCtrl* vmeLowerResCtrl = new wxTextCtrl( m_Dialog, ID_LOWER_RES_VME, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	vmeLowerResCtrl->SetToolTip( wxT("The VME currently associated with m_Dialog VME as its lower resolution VME.") );	
	bSizer2->Add( vmeLowerResCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer1->Add( bSizer2, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer8 = new wxBoxSizer( wxHORIZONTAL );		
	bSizer8->Add( new wxPanel( m_Dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL ), 1, wxEXPAND|wxALL|wxALIGN_RIGHT, 5 );	
	wxButton* bttnUpdLR = new wxButton( m_Dialog, ID_UPDATE_LR, wxT("Update"), wxDefaultPosition, wxDefaultSize, 0 );
	bttnUpdLR->SetToolTip( wxT("Set a new VME") );	
	bSizer8->Add( bttnUpdLR, 0, wxALL, 5 );	
	wxButton* bttnRemLR = new wxButton( m_Dialog, ID_REMOVE_LR, wxT("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	bttnRemLR->SetToolTip( wxT("Remove the link to the VME ") );	
	bSizer8->Add( bttnRemLR, 0, wxALL, 5 );
	
	bSizer1->Add( bSizer8, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer21 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer21->Add( new wxStaticText( m_Dialog, wxID_ANY, wxT("VME with Hull:"), wxDefaultPosition, wxSize( 120,-1 ), 0 ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );	
	wxTextCtrl* vmeHullCtrl = new wxTextCtrl( m_Dialog, ID_HULL_RES_VME, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	vmeHullCtrl->SetToolTip( wxT("The VME currently associated with m_Dialog VME as its lower resolution VME.") );	
	bSizer21->Add( vmeHullCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	bSizer1->Add( bSizer21, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer81 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer81->Add( new wxPanel( m_Dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL ), 1, wxEXPAND|wxALL|wxALIGN_RIGHT, 5 );	
	wxButton* bttnUpdHull = new wxButton( m_Dialog, ID_UPDATE_HULL, wxT("Update"), wxDefaultPosition, wxDefaultSize, 0 );
	bttnUpdHull->SetToolTip( wxT("Set a new VME") );	
	bSizer81->Add( bttnUpdHull, 0, wxALL, 5 );	
	wxButton* bttnRemHull = new wxButton( m_Dialog, ID_REMOVE_HULL, wxT("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	bttnRemHull->SetToolTip( wxT("Remove the link to the VME ") );	
	bSizer81->Add( bttnRemHull, 0, wxALL, 5 );
	
	bSizer1->Add( bSizer81, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer6 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer6->Add( new wxPanel( m_Dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL ), 1, wxEXPAND|wxALL|wxALIGN_CENTER_VERTICAL, 5 );	
	wxButton* bttnOK = new wxButton( m_Dialog, ID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( bttnOK, 0, wxALL|wxALIGN_BOTTOM, 5 );	
	wxButton* bttnCancel = new wxButton( m_Dialog, ID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( bttnCancel, 0, wxALL|wxALIGN_BOTTOM, 5 );
	
	bSizer1->Add( bSizer6, 1, wxEXPAND, 1 );
#pragma endregion GUI

	//set validators
	vmeLowerResCtrl->SetValidator(mafGUIValidator(this, ID_LOWER_RES_VME, vmeLowerResCtrl, &m_SelectedLowResVMEName));
	vmeHullCtrl->SetValidator(mafGUIValidator(this, ID_HULL_RES_VME, vmeHullCtrl, &m_SelectedHullVMEName));

	bttnUpdLR->SetValidator(mafGUIValidator(this, ID_UPDATE_LR, bttnUpdLR));
	bttnUpdHull->SetValidator(mafGUIValidator(this, ID_UPDATE_HULL, bttnUpdHull));
	bttnRemLR->SetValidator(mafGUIValidator(this, ID_REMOVE_LR, bttnRemLR));
	bttnRemHull->SetValidator(mafGUIValidator(this, ID_REMOVE_HULL, bttnRemHull));  

  bttnOK->SetValidator(mafGUIValidator(this, ID_OK, bttnOK));
  bttnCancel->SetValidator(mafGUIValidator(this, ID_CANCEL, bttnCancel));

  m_Dialog->Add(bSizer1, 1, wxEXPAND);
  
	//int x_pos, y_pos;
  //mafGetFrame()->GetPosition(&x_pos,&y_pos);  
  //m_Dialog->SetSize(x_pos+5,y_pos+5, 460, 225);   
	m_Dialog->SetMinSize(wxSize(460, 225));
}

//----------------------------------------------------------------------------
//Creates GUI including renderer window
/*virtual*/ void lhpOpSetLowerRes::DeleteOpDialog()
//----------------------------------------------------------------------------
{
  cppDEL(m_Dialog);
}

//------------------------------------------------------------------------
//Sets a new lower resolution VME and updates GUI 
/*virtual*/ void lhpOpSetLowerRes::SetNewLowerResVME(mafVME* vme)
//------------------------------------------------------------------------
{
	if (m_SelectedLowResVME == vme)
    return; //we do not have here a new vme

	if (NULL == (m_SelectedLowResVME = vme))
		m_SelectedLowResVMEName = "";
	else
		m_SelectedLowResVMEName = m_SelectedLowResVME->GetName();

	if (m_Dialog != NULL)
		m_Dialog->TransferDataToWindow();
}

//------------------------------------------------------------------------
//Sets a new hull VME and updates GUI 
/*virtual*/  void lhpOpSetLowerRes::SetNewHullVME(mafVME* vme)
//------------------------------------------------------------------------
{
	if (m_SelectedHullVME == vme)
    return; //we do not have here a new vme

	if (NULL == (m_SelectedHullVME = vme))
		m_SelectedHullVMEName = "";
	else
		m_SelectedHullVMEName = m_SelectedHullVME->GetName();

	if (m_Dialog != NULL)
		m_Dialog->TransferDataToWindow();
}

//----------------------------------------------------------------------------
void lhpOpSetLowerRes::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {      
		case ID_UPDATE_LR:
		case ID_UPDATE_HULL:
			OnUpdateLinkedVME(e->GetId() == ID_UPDATE_LR);
			break;

		case ID_REMOVE_LR:
		case ID_REMOVE_HULL:
			OnRemoveLinkedVME(e->GetId() == ID_REMOVE_LR);
			break;

    case ID_OK:       
      m_Dialog->EndModal(wxID_OK);      
      break;

    case ID_CANCEL:
      m_Dialog->EndModal(wxID_CANCEL);
      break;
    }
  }
}

#pragma region GUI Handlers
//------------------------------------------------------------------------
// Handles the selection of  lower resolution or hull VME 
/*virtual*/ void lhpOpSetLowerRes::OnUpdateLinkedVME(bool bLowResVME)
//------------------------------------------------------------------------
{
	mafString title = "Choose the VME to be linked";

	mafEvent ev(this, VME_CHOOSE);   
	ev.SetString(&title);

#pragma warning(suppress: 4311)
	ev.SetArg((long)&lhpOpSetLowerRes::SelectVMECallback);

	mafEventMacro(ev);
	mafVME* vme = mafVME::SafeDownCast(ev.GetVme()); 
	if (vme == m_Input)
		wxMessageBox(_("Cannot link the VME to itself. Select another VME."),
		_("Error"), wxOK | wxICON_EXCLAMATION);
	else if (vme != NULL)
	{
		if (bLowResVME)
			SetNewLowerResVME(vme);
		else
			SetNewHullVME(vme);
	}
}

//------------------------------------------------------------------------
// Handles the removal of  lower resolution or hull VME
/*virtual*/ void lhpOpSetLowerRes::OnRemoveLinkedVME(bool bLowResVME)
//------------------------------------------------------------------------
{
	if (bLowResVME)
		SetNewLowerResVME(NULL);
	else
		SetNewHullVME(NULL);
}

//------------------------------------------------------------------------
//Callback for VME_CHOOSE that accepts any VME
/*static*/ bool lhpOpSetLowerRes::SelectVMECallback(mafNode *node) 
//------------------------------------------------------------------------
{  
	mafVME* vme = mafVME::SafeDownCast(node);
	return vme != NULL && !vme->GetOutput()->IsA("mafVMEOutputNULL");	//cannot be GROUP, etc.
}

//----------------------------------------------------------------------------
void lhpOpSetLowerRes::OpDo()
//----------------------------------------------------------------------------
{	
	if (m_SelectedLowResVME != NULL)
		m_Input->SetLink(GetLinkNameLR(), m_SelectedLowResVME);
	else
		m_Input->RemoveLink(GetLinkNameLR());

	if (m_SelectedHullVME != NULL)
		m_Input->SetLink(GetLinkNameHULL(), m_SelectedHullVME);
	else
		m_Input->RemoveLink(GetLinkNameHULL());
}

//----------------------------------------------------------------------------
void lhpOpSetLowerRes::OpUndo()
//----------------------------------------------------------------------------
{		
	bool bsuccess = true;

	mafNode* newLink = NULL;
	if (m_UndoLowResVMEId < 0) 
		m_Input->RemoveLink(GetLinkNameLR());
	else
	{
		if (NULL != (newLink = m_Input->GetRoot()->FindInTreeById(m_UndoLowResVMEId)))
			m_Input->SetLink(GetLinkNameLR(), newLink);
		else	
		{
			m_Input->RemoveLink(GetLinkNameLR());
			bsuccess = false;
		}
	}

	newLink = NULL;
	if (m_UndoHullVMEId < 0) 
		m_Input->RemoveLink(GetLinkNameHULL());
	else
	{
		if (NULL != (newLink = m_Input->GetRoot()->FindInTreeById(m_UndoHullVMEId)))		
			m_Input->SetLink(GetLinkNameHULL(), newLink);
		else	
		{
			m_Input->RemoveLink(GetLinkNameHULL());
			bsuccess = false;
		}
	}		
		
	if (!bsuccess)
		mafLogMessage("Unable to complete UNDO operation - previously linked VME no longer exists.");
}

const char* lhpOpSetLowerRes::GetLinkNameLR()
{
  static const char* LINK_NAME_LR = "LowerResVME";	///< Link name to the Neck Output VME (if exists)
  return LINK_NAME_LR;
}

const char* lhpOpSetLowerRes::GetLinkNameHULL()
{
  static const char* LINK_NAME_HULL = "HullVME";	///< Link name to the Neck Output VME (if exists)
  return LINK_NAME_HULL;
}
