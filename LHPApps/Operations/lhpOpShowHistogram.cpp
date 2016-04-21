/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpShowHistogram.cpp,v $
Language:  C++
Date:      $Date: 2012-02-07 09:29:44 $
Version:   $Revision: 1.1.2.6 $
Authors:   Gianluigi Crimi
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "medDefines.h" 

//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "mafVME.h"
#include "mafGUI.h"
#include "mafGUIDialog.h"
#include "lhpOpShowHistogram.h"
#include "lhpGUIHistogramWidget.h"
#include "vtkPointData.h"
#include "vtkDataSet.h"
#include <wx/busyinfo.h>

class wxString;

// RTTI support
mafCxxTypeMacro(lhpOpShowHistogram);

//----------------------------------------------------------------------------
// Constructor
lhpOpShowHistogram::lhpOpShowHistogram(wxString label):mafOp(label)
//----------------------------------------------------------------------------
{
  m_BinNumber=100;
  m_ShowPercentage=0;
}

//----------------------------------------------------------------------------
// Destructor
lhpOpShowHistogram::~lhpOpShowHistogram(void)
//----------------------------------------------------------------------------
{
}

mafOp * lhpOpShowHistogram::Copy()
{
	return new lhpOpShowHistogram();
}

//----------------------------------------------------------------------------
// Accept surface or volume data
bool lhpOpShowHistogram::Accept(mafNode * vme)
//----------------------------------------------------------------------------
{
	return (vme!=NULL && vme->IsMAFType(mafVME) && !vme->IsA("mafVMERoot") &&
		// Bug 2588 fix:
    // http://bugzilla.b3c.it/show_bug.cgi?id=2588
		// should not accept vme with no output
		!vme->IsA("mafVMELandmarkCloud") && !vme->IsA("mafVMEGroup") &&
		// end fix
    //Bug 2605 fix
    // http://bugzilla.b3c.it/show_bug.cgi
    // should not accept vme with no vtk data
    mafVME::SafeDownCast(vme)->GetOutput()->GetVTKData() != NULL &&
    // end fix
    mafVME::SafeDownCast(vme)->GetOutput()->GetVTKData()->GetPointData()->GetNumberOfArrays()>0 );
}



//----------------------------------------------------------------------------
void lhpOpShowHistogram::OnEvent(mafEventBase * e)
//----------------------------------------------------------------------------
{

	switch (e->GetId())
	{
    case ID_SCALARS_ARRAY_SELECTION:
      {
        UpdateHistogram();
        break;
      }
    case ID_BIN_NUMBER:
      {
        m_Histogram->SetBinNumber(m_BinNumber);
        break;
      }
    case ID_SHOW_PERCENTAGE:
      {
        m_Histogram->ShowPercentage(m_ShowPercentage);
        break;
      }
    case ID_RESET_ZOOM:
      {
        m_Histogram->ResetZoom();  
        break;
      }
		case wxOK: 
      {
        m_Dialog->EndModal(wxID_OK);
        break;
      }
    case wxCANCEL:
      {
        m_Dialog->EndModal(wxID_CANCEL);
        break;
      }
	}
	
}

//----------------------------------------------------------------------------
void lhpOpShowHistogram::OpRun()
//----------------------------------------------------------------------------
{
  m_VME=mafVME::SafeDownCast(m_Input);

  
	CreateOpDialog();
  UpdateHistogram();

  int result = m_Dialog->ShowModal() == wxID_OK ? OP_RUN_OK : OP_RUN_CANCEL;

  OpStop(result);
}

//----------------------------------------------------------------------------
void lhpOpShowHistogram::UpdateHistogram()
//----------------------------------------------------------------------------
{
  wxBusyInfo wait(_("Updating Histogram ..."));
  m_PointData->SetActiveScalars(m_PointData->GetArrayName(m_SelectedScalarsArray));
  m_Histogram->SetData(m_PointData->GetScalars());
  m_Histogram->ShowPercentage(m_ShowPercentage);
}

//----------------------------------------------------------------------------
void lhpOpShowHistogram::CreateOpDialog()
//----------------------------------------------------------------------------
{
  m_Dialog = new mafGUIDialog(m_Label, mafCLOSEWINDOW);
  m_Dialog->SetListener(this);

  m_GuiDialog = new mafGUI(this);
  m_GuiDialog->Reparent(m_Dialog);

  //Change default frame to our dialog
  wxWindow* oldFrame = mafGetFrame();
  mafSetFrame(m_Dialog);

  m_Dialog->SetMinSize(wxSize(800,200));

  wxBoxSizer * sizer = new wxBoxSizer(wxHORIZONTAL);

  
  //Setting up the histogram
  m_Histogram = new lhpGUIHistogramWidget(m_Dialog,-1,wxPoint(0,0),wxSize(800,250),wxTAB_TRAVERSAL);
  m_Histogram->SetListener(this);
  sizer->Add(m_Histogram);

  
  m_Dialog->Add(sizer,1);

  m_GuiDialog->Label("");

  m_PointData = m_VME->GetOutput()->GetVTKData()->GetPointData();
  wxArrayString scalarsArrayNames;
  m_GuiDialog->Button(ID_RESET_ZOOM,"Reset Zoom","");

  for(int i = 0; i < m_PointData->GetNumberOfArrays(); i++)
  {
    scalarsArrayNames.Add(m_PointData->GetArrayName(i));
  }
  m_SelectedScalarsArray=0;
  wxCArrayString scalarsCArrayNames = wxCArrayString(scalarsArrayNames); // Use this class because wxArrayString::GetStringsArray() is a deprecated method
  wxComboBox *cb=m_GuiDialog->Combo(ID_SCALARS_ARRAY_SELECTION,"Scalars:",&m_SelectedScalarsArray,scalarsCArrayNames.GetCount(),scalarsCArrayNames.GetStrings(),"Determine the visible scalars array");
  
  m_GuiDialog->Enable(ID_SCALARS_ARRAY_SELECTION, m_PointData->GetNumberOfArrays() > 1);

  m_GuiDialog->Integer(ID_BIN_NUMBER,"Bins:",&m_BinNumber,0,500,"The number of the bins");
    
  m_GuiDialog->Bool(ID_SHOW_PERCENTAGE,"Show Percentage",&m_ShowPercentage,1);
  
  wxStaticText *label = new wxStaticText(m_GuiDialog, -1, "Right button + move to zoom, left button + move to focus", wxDefaultPosition, wxSize(500,18), wxALIGN_LEFT | wxST_NO_AUTORESIZE );

  m_GuiDialog->Add(label,0,wxEXPAND | wxALL);

  m_GuiDialog->OkCancel();
  m_GuiDialog->FitGui();

  
  m_Dialog->Add(m_GuiDialog);

  m_Dialog->Fit();
  m_Dialog->Update();
}


//----------------------------------------------------------------------------
void lhpOpShowHistogram::OpDo()
//----------------------------------------------------------------------------
{
	Superclass::OpDo();
}

//----------------------------------------------------------------------------
void lhpOpShowHistogram::OpUndo()
//----------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------
void lhpOpShowHistogram::OpStop(int result)
//----------------------------------------------------------------------------
{
  mafEventMacro(mafEvent(this,result));
}


