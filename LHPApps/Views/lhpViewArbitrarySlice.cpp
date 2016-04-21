/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpViewArbitrarySlice.cpp,v $
Language:  C++
Date:      $Date: 2011-05-27 07:52:38 $
Version:   $Revision: 1.1.2.2 $
Authors:   Gianluigi Crimi
==========================================================================
Copyright (c) 2008
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#include "lhpDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpViewArbitrarySlice.h"
#include "mafEventSource.h"
#include "mafViewSlice.h"
#include "lhpPipeMeshFake.h"
#include "mafPipeMeshSlice.h"
#include "mafPipeMesh.h"
#include "mafGUI.h"
#include "mafVMEMesh.h"
#include "mafNode.h"
#include "vtkDataSet.h"
#include "vtkPointData.h"


#include <string>
#include "mafNodeIterator.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpViewArbitrarySlice);
//----------------------------------------------------------------------------


enum ARBITRARY_SUBVIEW_ID
{
	ARBITRARY_VIEW = 0,
	SLICE_VIEW,
};

//----------------------------------------------------------------------------
lhpViewArbitrarySlice::lhpViewArbitrarySlice(wxString label):mafViewArbitrarySlice(label)
	//----------------------------------------------------------------------------
{
	m_MeshFakePipe = new lhpPipeMeshFake();
	m_MeshFakePipe->SetListener(this);
	m_IgnoreCameraUpdate=false;
}
//----------------------------------------------------------------------------
lhpViewArbitrarySlice::~lhpViewArbitrarySlice()
	//----------------------------------------------------------------------------
{
	delete m_MeshFakePipe;
}
//----------------------------------------------------------------------------
//mafView *lhpViewArbitrarySlice::Copy(mafObserver *Listener)
mafView*  lhpViewArbitrarySlice::Copy(mafObserver *Listener, bool lightCopyEnabled)
	//----------------------------------------------------------------------------
{
	lhpViewArbitrarySlice *v = new lhpViewArbitrarySlice(m_Label);
	v->m_Listener = Listener;
	v->m_Id = m_Id;
	for (int i=0;i<m_PluggedChildViewList.size();i++)
	{
		v->m_PluggedChildViewList.push_back(m_PluggedChildViewList[i]->Copy(this));
	}
	v->m_NumOfPluggedChildren = m_NumOfPluggedChildren;
	v->Create();
	return v;
}

//----------------------------------------------------------------------------
void lhpViewArbitrarySlice::OnEvent(mafEventBase *maf_event)
	//----------------------------------------------------------------------------
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{

		//Avoid 
		if(e->GetId()==CAMERA_UPDATE && m_IgnoreCameraUpdate)
			return;

		if (e->GetSender() == m_MeshFakePipe)
		{
			//m_IgnoreCameraUpdate=true;
			switch(e->GetId())
			{
			case lhpPipeMeshFake::ID_WIREFRAME:
				OnWireframe(e);
				break;
			case lhpPipeMeshFake::ID_SCALARS:
				OnScalars(e);
				break;
			case lhpPipeMeshFake::ID_USE_VTK_PROPERTY:
				OnUseVtkProperty(e);
				break;
			case lhpPipeMeshFake::ID_SCALAR_MAP_ACTIVE:
				OnScalarMapActive(e);
				break;
			case lhpPipeMeshFake::ID_BORDER_CHANGE:
				OnBorderChange(e);
				break;
			case lhpPipeMeshFake::ID_WIRED_ACTOR_VISIBILITY:
				OnWiredActorVisibility(e);
				break;
			case lhpPipeMeshFake::ID_LUT:
				onLutChange(e);
				break;
			}
			m_IgnoreCameraUpdate=false;
			mafEventMacro(mafEvent(this,CAMERA_UPDATE));
		}
		else
			mafViewArbitrarySlice::OnEvent(maf_event);
	}
	else
		mafEventMacro(*maf_event);
}

mafPipe* lhpViewArbitrarySlice::GetNodePipe(mafNode *vme)
{
	m_CurrentMesh = NULL;
	if (!vme->IsA("mafVMEMesh"))
		return NULL;
	
	mafPipeMesh *meshPipe=mafPipeMesh::SafeDownCast(m_ChildViewList[ARBITRARY_VIEW]->GetNodePipe(vme));

	if (!meshPipe)
		return NULL;

	m_CurrentMesh=mafVMEMesh::SafeDownCast(vme);
	m_MeshFakePipe->CopyGuiInfoFromPipe(meshPipe);

	return m_MeshFakePipe;

}

void lhpViewArbitrarySlice::OnWireframe(mafEvent * e)
{
	mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(m_ChildViewList[SLICE_VIEW]->GetNodePipe(m_CurrentMesh));
	mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(m_ChildViewList[ARBITRARY_VIEW]->GetNodePipe(m_CurrentMesh));
	if (e->GetBool())
	{
		if(pipeMesh)
			pipeMesh->SetWireframeOn();
		if(pipeSlice)
			pipeSlice->SetWireframeOn();
	}
	else
	{
		if(pipeMesh)
			pipeMesh->SetWireframeOff();
		if(pipeSlice)
			pipeSlice->SetWireframeOff();
	}
}

void lhpViewArbitrarySlice::OnScalars(mafEvent * e)
{
	int selectedScalars=e->GetDouble();

	mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(m_ChildViewList[SLICE_VIEW]->GetNodePipe(m_CurrentMesh));
	mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(m_ChildViewList[ARBITRARY_VIEW]->GetNodePipe(m_CurrentMesh));
	if(pipeMesh)
	{
		e->SetId(mafPipeMesh::ID_SCALARS);
		pipeMesh->SetActiveScalar(selectedScalars);
		pipeMesh->OnEvent(e);
	}
	if(pipeSlice)
	{
		e->SetId(mafPipeMeshSlice::ID_SCALARS);
		pipeSlice->SetActiveScalar(selectedScalars);
		pipeSlice->OnEvent(e);
	}

}

void lhpViewArbitrarySlice::OnUseVtkProperty(mafEvent * e)
{
	int useProprty=e->GetBool();

		mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(m_ChildViewList[SLICE_VIEW]->GetNodePipe(m_CurrentMesh));
		mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(m_ChildViewList[ARBITRARY_VIEW]->GetNodePipe(m_CurrentMesh));
		if(pipeMesh)
		{
			e->SetId(mafPipeMesh::ID_USE_VTK_PROPERTY);
			pipeMesh->SetUseVTKProperty(useProprty);
			pipeMesh->OnEvent(e);
		}
		if(pipeSlice)
		{
			e->SetId(mafPipeMeshSlice::ID_USE_VTK_PROPERTY);
			pipeSlice->SetUseVTKProperty(useProprty);
			pipeSlice->OnEvent(e);
		}

}

void lhpViewArbitrarySlice::OnScalarMapActive(mafEvent * e)
{
	int scalarMapActive=e->GetBool();


	mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(m_ChildViewList[SLICE_VIEW]->GetNodePipe(m_CurrentMesh));
	mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(m_ChildViewList[ARBITRARY_VIEW]->GetNodePipe(m_CurrentMesh));
	if(pipeMesh)
	{
		e->SetId(mafPipeMesh::ID_SCALAR_MAP_ACTIVE);
		pipeMesh->SetScalarMapActive(scalarMapActive);
		pipeMesh->OnEvent(e);
	}
	if(pipeSlice)
	{
		e->SetId(mafPipeMeshSlice::ID_SCALAR_MAP_ACTIVE);
		pipeSlice->SetScalarMapActive(scalarMapActive);
		pipeSlice->OnEvent(e);
	}

}

void lhpViewArbitrarySlice::OnBorderChange(mafEvent * e)
{
	double borderSize=e->GetDouble();

	mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(m_ChildViewList[SLICE_VIEW]->GetNodePipe(m_CurrentMesh));
	mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(m_ChildViewList[ARBITRARY_VIEW]->GetNodePipe(m_CurrentMesh));
	if(pipeMesh)
		pipeMesh->SetThickness(borderSize);
	if(pipeSlice)
		pipeSlice->SetThickness(borderSize);

}

void lhpViewArbitrarySlice::OnWiredActorVisibility(mafEvent * e)
{

	mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(m_ChildViewList[SLICE_VIEW]->GetNodePipe(m_CurrentMesh));
	mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(m_ChildViewList[ARBITRARY_VIEW]->GetNodePipe(m_CurrentMesh));
	if (e->GetBool())
	{
		if(pipeMesh)
			pipeMesh->SetWiredActorVisibilityOn();
		if(pipeSlice)
			pipeSlice->SetWiredActorVisibilityOn();
	}
	else
	{
		if(pipeMesh)
			pipeMesh->SetWiredActorVisibilityOff();
		if(pipeSlice)
			pipeSlice->SetWiredActorVisibilityOff();
	}
}

void lhpViewArbitrarySlice::onLutChange(mafEvent * e)
{
	vtkLookupTable *lookupTable=(vtkLookupTable *)e->GetVtkObj();

	
	mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(m_ChildViewList[SLICE_VIEW]->GetNodePipe(m_CurrentMesh));
	mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(m_ChildViewList[ARBITRARY_VIEW]->GetNodePipe(m_CurrentMesh));
	if(pipeMesh)
	{
		e->SetId(mafPipeMesh::ID_LUT);
		pipeMesh->SetLookupTable(lookupTable);
		pipeMesh->OnEvent(e);
	}
	if(pipeSlice)
	{
		e->SetId(mafPipeMeshSlice::ID_LUT);
		pipeSlice->SetLookupTable(lookupTable);
		pipeSlice->OnEvent(e);
	}

}

void lhpViewArbitrarySlice::VmeShow(mafNode *node, bool show)
{
	Superclass::VmeShow(node,show);

	if(show && node->IsA("mafVMEVolume"))
	{
		mafNode *root=m_CurrentVolume->GetRoot();
		mafNodeIterator *iter = root->NewIterator();
		for (mafNode *node = iter->GetFirstNode(); node; node = iter->GetNextNode())
		{
			if(node->IsA("mafVMEMesh") && mafPipeMesh::SafeDownCast(m_ChildViewList[ARBITRARY_VIEW]->GetNodePipe(node)))
				CopyPipeInfoFromPerspectiveToSlices((mafNode *)node);
		}
	}
}

void lhpViewArbitrarySlice::CopyPipeInfoFromPerspectiveToSlices(mafNode *vme)
{
		lhpPipeMeshFake *pipeMesh=(lhpPipeMeshFake *) mafPipeMesh::SafeDownCast(m_ChildViewList[ARBITRARY_VIEW]->GetNodePipe(vme));

		int selectedScalars=pipeMesh->GetScalarIndex();

		m_IgnoreCameraUpdate=true;
		
	
		mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(m_ChildViewList[SLICE_VIEW]->GetNodePipe(vme));

		if(!pipeMesh->GetWiredActorVisible());
			pipeSlice->SetWiredActorVisibilityOff();

		if (pipeMesh->GetScalarMapActive())
		{
				
			pipeSlice->SetLookupTable(pipeMesh->GetLookupTable());
				
			mafEvent e(this,mafPipeMeshSlice::ID_SCALARS);
			pipeSlice->SetActiveScalar(selectedScalars);
			pipeSlice->OnEvent(&e);

			e.SetId(mafPipeMeshSlice::ID_SCALAR_MAP_ACTIVE);
			pipeSlice->SetScalarMapActive(true);
			pipeSlice->OnEvent(&e);
		}

		m_IgnoreCameraUpdate=false;
}
