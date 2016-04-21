/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafVMEShortcut.cpp,v $
  Language:  C++
  Date:      $Date: 2011-04-11 07:17:59 $
  Version:   $Revision: 1.1.2.2 $
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


#include "mafVMEShortcut.h"
#include "mafVMEOutputNULL.h"
#include "mafGUI.h"
#include "mafGUIScrolledPanel.h"
#include "mafGUIValidator.h"
#include "mafEventSource.h"

/** Helper class to handle PAINT events and restore GUI - HACK */
class mafVMEShortcutPaintGuiEvnt : public wxObject
{
public:	
	mafVMEShortcut* m_pThis;

public:
	mafVMEShortcutPaintGuiEvnt(mafVMEShortcut* pThis) : m_pThis(pThis){
	}

	void OnPaintGui(wxEvent& e)
	{		
		((mafVMEShortcutPaintGuiEvnt*)e.m_callbackUserData)->m_pThis->OnPaintGui();
		e.Skip(true);
	}
};

//-------------------------------------------------------------------------
mafCxxTypeMacro(mafVMEShortcut)
//-------------------------------------------------------------------------

/*static*/ const char* mafVMEShortcut::ShortcutVME_Link = "ShortcutLink";

//-------------------------------------------------------------------------
mafVMEShortcut::mafVMEShortcut()
//-------------------------------------------------------------------------
{		
	DependsOnLinkedNodeOn();
}


//-------------------------------------------------------------------------
mafVMEShortcut::~mafVMEShortcut()
//-------------------------------------------------------------------------
{
  // data pipe destroyed in mafVME
  // data vector destroyed in mafVMEGeneric

	if (m_Gui != NULL)
	{		
		//stop observing wxEVT_PAINT
		m_Gui->Disconnect(wxEVT_PAINT, (wxObjectEventFunction)&mafVMEShortcutPaintGuiEvnt::OnPaintGui);  
				//, ((wxObject*)(mafObject::SafeDownCast(this))));

		UpdateGui(NULL);	//remove the target VME GUI from us
	}
}

//-------------------------------------------------------------------------
mafVMEOutput *mafVMEShortcut::GetOutput()
//-------------------------------------------------------------------------
{
	//check, if we are in the VME tree, if not, then get our output
	//not taget VME because probably we are in an invalid state
	//and calling GetTargetVME could lead to crash
	mafVME* vmeparent = GetParent();
	if (vmeparent == NULL)
		return __super::GetOutput();
	else
	{
		// allocate the right type of output on demand
		mafVME* vme = GetTargetVME();
		assert(mafVMEShortcut::SafeDownCast(vme) == NULL);

		if (vme != NULL)
			return vme->GetOutput();
		else
			return __super::GetOutput();
	}
}

//-------------------------------------------------------------------------
/*virtual*/ mafGUI* mafVMEShortcut::CreateGui()
	//-------------------------------------------------------------------------
{
	m_Gui = mafNode::CreateGui(); // Called to show info about vmes' type and name
	m_Gui->SetListener(this);

#pragma region GUI 
	wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer2->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Target:")), 0, wxALL, 5 );

	m_targetVMECtrl = new wxTextCtrl( m_Gui, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer2->Add( m_targetVMECtrl, 1, 0, 5 );

	bSizer1->Add( bSizer2, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer3 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer3->Add( new wxPanel( m_Gui), 1, wxALL, 5 );

	m_bttnUpdate = new wxButton( m_Gui, ID_UPDATE_TARGETLINK, wxT("Update"));
	bSizer3->Add( m_bttnUpdate, 0, wxALL, 5 );

	m_bttnDelete = new wxButton( m_Gui, ID_DELETE_TARGETLINK, wxT("Delete"));
	bSizer3->Add( m_bttnDelete, 0, wxALL, 5 );

	m_bttnGoto = new wxButton( m_Gui, ID_GOTO_TARGETLINK, wxT("Goto"));
	bSizer3->Add( m_bttnGoto, 0, wxALL, 5 );
	

	bSizer1->Add( bSizer3, 0, wxEXPAND, 5 );

	wxStaticBoxSizer* sbTargetVMEGui = new wxStaticBoxSizer( new wxStaticBox( m_Gui, wxID_ANY, wxT("Target VME Gui") ), wxVERTICAL );

	m_GUI_VME = new mafGUIScrolledPanel( m_Gui, wxID_ANY);  
	m_GUI_VME->SetMinSize(wxSize( m_GUI_VME->GetMinWidth(), 350));
  sbTargetVMEGui->Add( m_GUI_VME, 1, wxEXPAND | wxALL, 2 );

	mafVME* vme = GetTargetVME();
	if (vme != NULL)
	{
		UpdateGui(vme);		
	}
	else
	{
		m_bttnDelete->Enable(false);
		m_bttnGoto->Enable(false);
	}

	bSizer1->Add( sbTargetVMEGui, 1, wxEXPAND, 5 );

	m_Gui->Add(bSizer1);
#pragma endregion

	//validators
	m_bttnUpdate->SetValidator(mafGUIValidator(this, ID_UPDATE_TARGETLINK, m_bttnUpdate));
	m_bttnDelete->SetValidator(mafGUIValidator(this, ID_DELETE_TARGETLINK, m_bttnDelete));
	m_bttnGoto->SetValidator(mafGUIValidator(this, ID_GOTO_TARGETLINK, m_bttnGoto));

	m_Gui->Connect(wxEVT_PAINT, (wxObjectEventFunction)&mafVMEShortcutPaintGuiEvnt::OnPaintGui, new mafVMEShortcutPaintGuiEvnt(this));
	return m_Gui;
}

//----------------------------------------------------------------------------
void mafVMEShortcut::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{	
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{			
    switch (e->GetId())
    { 
			case mafVMEShortcut::ID_UPDATE_TARGETLINK:
				OnUpdateTargetLink();
				return;

			case mafVMEShortcut::ID_DELETE_TARGETLINK:
				OnDeleteTargetLink();
				return;

			case mafVMEShortcut::ID_GOTO_TARGETLINK:
				OnGotoTargetLink();
				return;
    }		
	}

	//the target VME sends to us these events
	switch (maf_event->GetId())
	{	
	case NODE_DESTROYED:
	case NODE_DETACHED_FROM_TREE:
		{
			mafVME *n = mafVME::SafeDownCast((mafObject *)maf_event->GetSender());
			if (n != NULL && n == GetTargetVME())
			{				
				if (m_Gui != NULL)
					UpdateGui(NULL);	//remove the GUI from us
			}			
		}
		break;	
	}
				
	//HACK: mafVMEGroup fails to forward NODE_ events, which leads into storing invalid links
	//and it also behaves strangely => rather call mafVME::OnEvent directly
	mafVME::OnEvent(maf_event); 	
}

//-------------------------------------------------------------------------
// This method is called upon redrawing the GUI. 
//It is here to ensure that the target VME GUI will be correctly displayed
void mafVMEShortcut::OnPaintGui()
//-------------------------------------------------------------------------
{
	//when the target VME is selected, its GUI, which is a child of our VME GUI,
	//is reparent by the core to be a child of the panel and after reselection
	//of this VME, the GUI is no longer our child => we need to 
	//reparent it once again. Unfortunately, there is no event raised 
	//when the target VME is selected, so we need this HACK
	mafVME* vme = GetTargetVME();
	if (vme != NULL && m_Gui != NULL)
	{
		mafGUI* vmeGUI = vme->GetGui();
		if (vmeGUI->GetParent() != m_GUI_VME) {
			UpdateGui(vme);	//reparent GUI, so it is again displayed correctly
		}
	}
}

//-------------------------------------------------------------------------
// Called when TargetLink is requested to be updated 
/*virtual*/ void mafVMEShortcut::OnUpdateTargetLink()
//-------------------------------------------------------------------------
{
	mafString title = "Choose a new VME as target";
	mafEvent ev(this, VME_CHOOSE, (long)&mafVMEShortcut::VMEAcceptTargetVME);
	ev.SetString(&title);
	ForwardUpEvent(ev);

	mafVME* vme = mafVME::SafeDownCast(ev.GetVme());
	if (vme != NULL)
	{
		SetTargetVME(vme);

		UpdateGui(vme);
		UpdateMAFGUIs();

		wxMessageBox(_T("Warning: you may need to close and reopen views to propagate the changes."), wxEmptyString, 
			wxOK | wxCENTER | wxICON_INFORMATION);
	}
}

//-------------------------------------------------------------------------
// Called to update VME GUI using the GUI provided by the passed vme (may be NULL)
/*virtual*/ void mafVMEShortcut::UpdateGui(mafVME* vme)
//-------------------------------------------------------------------------
{	
	bool valid = vme != NULL;
	m_bttnDelete->Enable(valid);
	m_bttnGoto->Enable(valid);		
	
	if (!valid)
	{
		vme = GetTargetVME();
		if (vme != NULL)
		{
			m_targetVMECtrl->SetLabel(_(""));  

			wxWindow *current_gui = vme->GetGui();
			m_GUI_VME->Remove(current_gui);   
			current_gui->Show(false);
			current_gui->Reparent(mafGetFrame());
		}		
	}

	if (valid)
	{	
		//as soon as the user selects the targeted VME and then selects anything else,
		//the VME GUI will be destroyed and we have probably no means to find out
		//After being reselected, our VME provides the caller with an invalid GUI and, even more
		//seriously, when the application terminates and our GUI is being destroyed there will be crash
		m_targetVMECtrl->SetLabel(vme->GetName());				

		mafGUI* vmeGUI = vme->GetGui();		
		vmeGUI->FitGui();            		

		vmeGUI->Reparent(m_GUI_VME);

		m_GUI_VME->Remove(vmeGUI);
		m_GUI_VME->Add(vmeGUI, 1, wxEXPAND | wxALL);			

		vmeGUI->Show(true);  //our GUI is visible (if we are here)
		vmeGUI->Update();
	}

	m_GUI_VME->Layout();
}

//-------------------------------------------------------------------------
/** Called when TargetLink is requested to be deleted 
/*virtual */ void mafVMEShortcut::OnDeleteTargetLink()
//-------------------------------------------------------------------------
{
	if (wxMessageBox(_T("Do you really want to remove the link?"), _("Confirmation Required"), 
		wxYES_NO | wxCENTER | wxICON_QUESTION)  == wxYES)
	{	
		mafVME* vme = GetTargetVME();
		if (vme != NULL)
		{
			UpdateGui(NULL);			

			//physical removal
			RemoveTargetVME();
			
			UpdateMAFGUIs();
		}
	}
}

//------------------------------------------------------------------------
//Called when TargetLink is requested to be selected 
/*virtual*/ void mafVMEShortcut::OnGotoTargetLink()
//------------------------------------------------------------------------
{
	mafEvent ev(this, VME_SELECT, (mafNode*)GetTargetVME());
	this->ForwardUpEvent(&ev);
}

//------------------------------------------------------------------------
/*static*/ bool mafVMEShortcut::VMEAcceptTargetVME(mafNode *node) 
//------------------------------------------------------------------------
{  
	//accept anything but instances of mafVMEShortcut
	return (mafVME::SafeDownCast(node) != NULL && mafVMEShortcut::SafeDownCast(node) == NULL);  
}


//------------------------------------------------------------------------
//This routine should be called after the target link has changed to make sure
//that GUIs, visual pipes, etc. update to  reflect the change. 
void mafVMEShortcut::UpdateMAFGUIs()
//------------------------------------------------------------------------
{		
	//selects root
	mafEvent ev(this, VME_SELECT, (long)this->GetRoot()->GetId());
	this->ForwardUpEvent(&ev);

	//reselects this VME => this should recreate visual pipe
	ev.SetVme(this);
	this->ForwardUpEvent(&ev);	
}

//-------------------------------------------------------------------------
char** mafVMEShortcut::GetIcon() 
//-------------------------------------------------------------------------
{
  #include "mafVMEShortcut.xpm"
  return mafVMEShortcut_xpm;
}
