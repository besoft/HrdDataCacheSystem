/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpViewOrthoSlice.cpp,v $
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

#include "lhpViewOrthoSlice.h"
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

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpViewOrthoSlice);
//----------------------------------------------------------------------------

#define FIRST_VIEW_SLICE 1

//----------------------------------------------------------------------------
lhpViewOrthoSlice::lhpViewOrthoSlice(wxString label):mafViewOrthoSlice(label)
	//----------------------------------------------------------------------------
{
	m_MeshFakePipe = new lhpPipeMeshFake();
	m_MeshFakePipe->SetListener(this);
	m_IgnoreCameraUpdate=false;
}
//----------------------------------------------------------------------------
lhpViewOrthoSlice::~lhpViewOrthoSlice()
	//----------------------------------------------------------------------------
{
	delete m_MeshFakePipe;
}
//----------------------------------------------------------------------------
//mafView *lhpViewOrthoSlice::Copy(mafObserver *Listener)
mafView*  lhpViewOrthoSlice::Copy(mafObserver *Listener, bool lightCopyEnabled)
	//----------------------------------------------------------------------------
{
	lhpViewOrthoSlice *v = new lhpViewOrthoSlice(m_Label);
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
void lhpViewOrthoSlice::OnEvent(mafEventBase *maf_event)
	//----------------------------------------------------------------------------
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{

		//Avoid 
		if(e->GetId()==CAMERA_UPDATE && m_IgnoreCameraUpdate)
			return;

		if (e->GetSender() == m_MeshFakePipe)
		{
			m_IgnoreCameraUpdate=true;
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
			mafViewOrthoSlice::OnEvent(maf_event);
	}
	else
		mafEventMacro(*maf_event);
}

mafPipe* lhpViewOrthoSlice::GetNodePipe(mafNode *vme)
{
	m_CurrentMesh = NULL;
	if (!vme->IsA("mafVMEMesh"))
		return NULL;

	mafPipeMesh *meshPipe=mafPipeMesh::SafeDownCast(GetSubView(PERSPECTIVE_VIEW)->GetNodePipe(vme));

	if (!meshPipe)
		return NULL;

	m_CurrentMesh=mafVMEMesh::SafeDownCast(vme);

	m_MeshFakePipe->CopyGuiInfoFromPipe(meshPipe);

	return m_MeshFakePipe;

}

void lhpViewOrthoSlice::OnWireframe(mafEvent * e)
{
	for(int i = 0; i<VIEWS_NUMBER;i++)
	{
		mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		if (e->GetBool())
		{
			if(pipeSlice)
				pipeSlice->SetWireframeOn();
			else if(pipeMesh)
				pipeMesh->SetWireframeOn();
		}
		else
		{
			if(pipeSlice)
				pipeSlice->SetWireframeOff();
			else if(pipeMesh)
				pipeMesh->SetWireframeOff();
		}
	}
}

void lhpViewOrthoSlice::OnScalars(mafEvent * e)
{
	int selectedScalars=e->GetDouble();

	for(int i = 0; i<VIEWS_NUMBER;i++)
	{
		mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		if(pipeSlice)
		{
			e->SetId(mafPipeMeshSlice::ID_SCALARS);
			pipeSlice->SetActiveScalar(selectedScalars);
			pipeSlice->OnEvent(e);
		}
		else if(pipeMesh)
		{
			e->SetId(mafPipeMesh::ID_SCALARS);
			pipeMesh->SetActiveScalar(selectedScalars);
			pipeMesh->OnEvent(e);
		}
	}
}

void lhpViewOrthoSlice::OnUseVtkProperty(mafEvent * e)
{
	int useProprty=e->GetBool();

	for(int i = 0; i<VIEWS_NUMBER;i++)
	{
		mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		if(pipeSlice)
		{
			e->SetId(mafPipeMeshSlice::ID_USE_VTK_PROPERTY);
			pipeSlice->SetUseVTKProperty(useProprty);
			pipeSlice->OnEvent(e);
		}
		else if(pipeMesh)
		{
			e->SetId(mafPipeMesh::ID_USE_VTK_PROPERTY);
			pipeMesh->SetUseVTKProperty(useProprty);
			pipeMesh->OnEvent(e);
		}
	}
}

void lhpViewOrthoSlice::OnScalarMapActive(mafEvent * e)
{
	int scalarMapActive=e->GetBool();

	for(int i = 0; i<VIEWS_NUMBER;i++)
	{
		mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		if(pipeSlice)
		{
			e->SetId(mafPipeMeshSlice::ID_SCALAR_MAP_ACTIVE);
			pipeSlice->SetScalarMapActive(scalarMapActive);
			pipeSlice->OnEvent(e);
		}
		else if(pipeMesh)
		{
			e->SetId(mafPipeMesh::ID_SCALAR_MAP_ACTIVE);
			pipeMesh->SetScalarMapActive(scalarMapActive);
			pipeMesh->OnEvent(e);
		}
	}
}

void lhpViewOrthoSlice::OnBorderChange(mafEvent * e)
{
	double borderSize=e->GetDouble();

	for(int i = 0; i<VIEWS_NUMBER;i++)
	{
		mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		if(pipeSlice)
			pipeSlice->SetThickness(borderSize);
		if(pipeMesh)
			pipeMesh->SetThickness(borderSize);
	}
}

void lhpViewOrthoSlice::OnWiredActorVisibility(mafEvent * e)
{
	for(int i = 0; i<VIEWS_NUMBER;i++)
	{
		mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		if (e->GetBool())
		{
			if(pipeSlice)
				pipeSlice->SetWiredActorVisibilityOn();
			else if(pipeMesh)
				pipeMesh->SetWiredActorVisibilityOn();
		}
		else
		{
			if(pipeSlice)
				pipeSlice->SetWiredActorVisibilityOff();
			else if(pipeMesh)
				pipeMesh->SetWiredActorVisibilityOff();
		}
	}
}

void lhpViewOrthoSlice::onLutChange(mafEvent * e)
{
	vtkLookupTable *lookupTable=(vtkLookupTable *)e->GetVtkObj();

	for(int i = 0; i<VIEWS_NUMBER;i++)
	{
		mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		mafPipeMesh *pipeMesh=mafPipeMesh::SafeDownCast(GetSubView(i)->GetNodePipe(m_CurrentMesh));
		if(pipeSlice)
		{
			e->SetId(mafPipeMeshSlice::ID_LUT);
			pipeSlice->SetLookupTable(lookupTable);
			pipeSlice->OnEvent(e);
		}
		else if(pipeMesh)
		{
			e->SetId(mafPipeMesh::ID_LUT);
			pipeMesh->SetLookupTable(lookupTable);
			pipeMesh->OnEvent(e);
		}
	}
}

void lhpViewOrthoSlice::VmeShow(mafNode *node, bool show)
{
	Superclass::VmeShow(node,show);

	if(show && node->IsA("mafVMEVolume"))
	{
		 for (int i=0;i<m_VMElist.size();i++)
			if(m_VMElist[i]->IsA("mafVMEMesh"))
				CopyPipeInfoFromPerspectiveToSlices(m_VMElist[i]);
	}
}

void lhpViewOrthoSlice::CopyPipeInfoFromPerspectiveToSlices(mafNode *vme)
{
		lhpPipeMeshFake *pipeMesh=(lhpPipeMeshFake *) mafPipeMesh::SafeDownCast(m_ChildViewList[PERSPECTIVE_VIEW]->GetNodePipe(vme));

		int selectedScalars=pipeMesh->GetScalarIndex();

		m_IgnoreCameraUpdate=true;
		
		for(int i=XN_VIEW; i<VIEWS_NUMBER; i++)
		{
			mafPipeMeshSlice *pipeSlice=mafPipeMeshSlice::SafeDownCast(GetSubView(i)->GetNodePipe(vme));

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
		}

		m_IgnoreCameraUpdate=false;
}
