
/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpMergeSurfaces.cpp,v $
  Language:  C++
  Date:      $Date: 2011-06-30 10:14:23 $
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

#include "lhpOpMergeSurfaces.h"
#if defined(VPHOP_WP10)

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafGUIDialog.h"
#include "mafGUIValidator.h"
#include "mafVME.h"
#include "mafVMESurface.h"

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpMergeSurfaces);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpMergeSurfaces::lhpOpMergeSurfaces(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;	
  m_Canundo = true;  
}
//----------------------------------------------------------------------------
lhpOpMergeSurfaces::~lhpOpMergeSurfaces( ) 
//----------------------------------------------------------------------------
{
  
}
//----------------------------------------------------------------------------
mafOp* lhpOpMergeSurfaces::Copy()   
//----------------------------------------------------------------------------
{
	return new lhpOpMergeSurfaces(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpMergeSurfaces::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return (node && node->IsMAFType(mafVME));
}
//----------------------------------------------------------------------------
void lhpOpMergeSurfaces::OpRun()   
//----------------------------------------------------------------------------
{ 		
	mafString title = "Choose the surface VMEs to be merged";

	mafEvent ev(this, VME_CHOOSE);   
	ev.SetString(&title);
	ev.SetBool(true);
#pragma warning(suppress: 4311)
	ev.SetArg((long)&lhpOpMergeSurfaces::SelectVMECallback);

	mafEventMacro(ev);
	m_SelectedNodes = ev.GetVmeVector();

	mafEventMacro(mafEvent(this, m_SelectedNodes.size() != 0 ? OP_RUN_OK : OP_RUN_CANCEL));
}

//------------------------------------------------------------------------
//Callback for VME_CHOOSE that accepts any VME
/*static*/ bool lhpOpMergeSurfaces::SelectVMECallback(mafNode *node) 
//------------------------------------------------------------------------
{  	
	return mafVMESurface::SafeDownCast(node) != NULL;
}

#include "vtkPolyData.h"
#include "vtkAppendPolyData.h"

//----------------------------------------------------------------------------
void lhpOpMergeSurfaces::OpDo()
//----------------------------------------------------------------------------
{	
	vtkAppendPolyData* filter = vtkAppendPolyData::New();
	int nCount = (int)m_SelectedNodes.size();
	for (int i = 0; i < nCount; i++)
	{
		mafVMESurface* vme = mafVMESurface::SafeDownCast(m_SelectedNodes[i]);
		if (vme != NULL)
		{
			vme->GetOutput()->Update();
			vtkPolyData* poly = vtkPolyData::SafeDownCast(vme->GetOutput()->GetVTKData());
			if (poly != NULL)
				filter->AddInput(poly);
		}
	}

	mafVMESurface* vmeResult;
	mafNEW(vmeResult);
	vmeResult->SetName("Merged VMEs");
	vmeResult->SetDataByDetaching(filter->GetOutput(), 
		mafVME::SafeDownCast(m_SelectedNodes[0])->GetTimeStamp());

	m_SelectedNodes[0]->GetParent()->AddChild(		
		m_Output = vmeResult
		);

	filter->Delete();
}

//----------------------------------------------------------------------------
void lhpOpMergeSurfaces::OpUndo()
//----------------------------------------------------------------------------
{		
	if (m_Output != NULL)
	{
		m_Output->ReparentTo(NULL);
		mafDEL(m_Output);		
	}
}

#endif