
/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpFilterPolyDataUsingExtApp.cpp,v $
  Language:  C++
  Date:      $Date: 2011-07-29 08:01:57 $
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

#include "lhpOpFilterPolyDataUsingExtApp.h"
#if defined(VPHOP_WP10)

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafVME.h"
#include "mafVMESurface.h"

#include "vtkMAFSmartPointer.h"

#include "lhpUtils.h"
#include "vtkPolyData.h"
#include "vtkSTLWriter.h"
#include "vtkSTLReader.h"
#include "mafDbg.h"


//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpFilterPolyDataUsingExtApp);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpFilterPolyDataUsingExtApp::lhpOpFilterPolyDataUsingExtApp(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;	
  m_Canundo = true;  

	m_OriginalPolydata = NULL;
	m_ResultPolydata = NULL;
}

//----------------------------------------------------------------------------
lhpOpFilterPolyDataUsingExtApp::~lhpOpFilterPolyDataUsingExtApp( ) 
//----------------------------------------------------------------------------
{
	vtkDEL(m_OriginalPolydata);
	vtkDEL(m_ResultPolydata);
}
//----------------------------------------------------------------------------
mafOp* lhpOpFilterPolyDataUsingExtApp::Copy()   
//----------------------------------------------------------------------------
{
	return new lhpOpFilterPolyDataUsingExtApp(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpFilterPolyDataUsingExtApp::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
	return (node && node->IsMAFType(mafVMESurface));
}
//----------------------------------------------------------------------------
void lhpOpFilterPolyDataUsingExtApp::OpRun()   
//----------------------------------------------------------------------------
{	
	//just a simple GUI to ask a few things
  
	//copy input		
	vtkNEW(m_OriginalPolydata);
	m_OriginalPolydata->DeepCopy((vtkPolyData*)((mafVME *)m_Input)->GetOutput()->GetVTKData());

	wxString szName = lhpUtils::lhpGetApplicationDirectory() + "\\temp.stl";

	//export muscle to STL format for polymender
	vtkMAFSmartPointer< vtkSTLWriter > stlw;
	stlw->SetInput(m_OriginalPolydata);
	stlw->SetFileName(szName.c_str());
	stlw->SetFileTypeToBinary();
	stlw->Update();

	stlw->SetInput(NULL);

	//inform the user about the export
	int result = OP_RUN_OK;
	if (::wxMessageBox(wxString::Format("Mesh was exported into '%s' file. "
		"Now, you can use an external editor to modify it (filter it). When you are finished, press OK.", 
		szName.c_str()), wxMessageBoxCaptionStr, wxOK | wxCANCEL | wxICON_INFORMATION) == wxCANCEL)
		result = OP_RUN_CANCEL;
	else
	{
		//read the data
		vtkNEW(m_ResultPolydata);
		vtkMAFSmartPointer< vtkSTLReader > stlr;
		stlr->SetOutput(m_ResultPolydata);
		stlr->SetFileName(szName.c_str());
		stlr->Update();

		stlr->SetOutput(NULL);
	}

	wxRemoveFile(szName);

	mafEventMacro(mafEvent(this,  result));
}

//----------------------------------------------------------------------------
void lhpOpFilterPolyDataUsingExtApp::OpDo()
//----------------------------------------------------------------------------
{
	((mafVMESurface *)m_Input)->SetData(m_ResultPolydata,((mafVME *)m_Input)->GetTimeStamp());
	mafEventMacro(mafEvent(this, CAMERA_UPDATE));
}

//----------------------------------------------------------------------------
void lhpOpFilterPolyDataUsingExtApp::OpUndo()
//----------------------------------------------------------------------------
{		
	((mafVMESurface *)m_Input)->SetData(m_OriginalPolydata,((mafVME *)m_Input)->GetTimeStamp());
	mafEventMacro(mafEvent(this, CAMERA_UPDATE));
}
#endif