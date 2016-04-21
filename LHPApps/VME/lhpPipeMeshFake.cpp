/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpPipeMeshFake.cpp,v $
  Language:  C++
  Date:      $Date: 2010-09-10 15:45:52 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Gianluigi Crimi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time_Array error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "lhpPipeMeshFake.h"
#include "mafDecl.h"
#include "mafSceneNode.h"
#include "mafGUI.h"
#include "mafGUICheckListBox.h"

#include "mafVMEScalarMatrix.h"
#include "mafTagArray.h"
#include "mafTagItem.h"
#include "mafVMEOutputScalar.h"
#include "mafVMEOutputScalarMatrix.h"
#include "mafEventSource.h"

#include "vtkTextProperty.h"
#include "vtkDoubleArray.h"
#include "vtkXYPlotActor.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkRenderer.h"
#include "vtkPointData.h"
#include "vtkRectilinearGrid.h"
#include "vtkMAFSmartPointer.h"
#include "mafString.h"

#include "vtkLookupTable.h"
#include <vtkStructuredGrid.h>
#include <vtkGeometryFilter.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkAxes.h>
#include <vtkTubeFilter.h>
#include <vtkAxisActor2D.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkUnsignedCharArray.h>
#include <vtkLine.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkVectorText.h>
#include <vtkFollower.h>
#include <vtkPolyDataMapper2D.h>
#include <wx\combobox.h>

#include "mafGUIMaterialButton.h"
#include "mafGUILutSwatch.h"
#include "vtkUnstructuredGrid.h"
#include "mafVMEOutputMesh.h"
#include "mmaMaterial.h"
#include "mafPipeMesh.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpPipeMeshFake);
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
lhpPipeMeshFake::lhpPipeMeshFake()
:mafPipeMesh()
//----------------------------------------------------------------------------
{
	vtkNEW(m_Table);
}
//----------------------------------------------------------------------------
lhpPipeMeshFake::~lhpPipeMeshFake()
//----------------------------------------------------------------------------
{
	m_Vme = NULL;
	m_Border = NULL;
	m_Wireframe = NULL;
	m_BorderElementsWiredActor = NULL;
	m_UseVTKProperty  = NULL;
	m_ScalarMapActive = NULL;
	m_ScalarIndex = NULL;
	m_NumberOfArrays = NULL;
	m_ScalarsInComboBoxNames = NULL;
	m_ScalarComboBox = NULL;
	vtkDEL(m_Table);
}

//----------------------------------------------------------------------------
mafGUI *lhpPipeMeshFake::CreateGui()
//----------------------------------------------------------------------------
{
	assert(m_Gui == NULL);
	m_Gui = new mafGUI(this);

	m_Gui->Bool(ID_WIREFRAME,_("Wireframe"), &m_Wireframe, 1);
	m_Gui->FloatSlider(ID_BORDER_CHANGE,_("Thickness"),&m_Border,1.0,5.0);
	m_Gui->Enable(ID_BORDER_CHANGE,m_Wireframe);
	m_Gui->Divider(2);

	m_Gui->Bool(ID_WIRED_ACTOR_VISIBILITY,_("Element Edges"), &m_BorderElementsWiredActor, 1);
	m_Gui->Enable(ID_WIRED_ACTOR_VISIBILITY,!m_Wireframe);

	m_Gui->Divider(2);
	m_Gui->Bool(ID_USE_VTK_PROPERTY,"Property",&m_UseVTKProperty, 1);
	m_MaterialButton = new mafGUIMaterialButton(m_Vme,this);
	m_Gui->AddGui(m_MaterialButton->GetGui());
	m_MaterialButton->Enable(m_UseVTKProperty != 0);

	m_Gui->Divider(2);
	m_Gui->Bool(ID_SCALAR_MAP_ACTIVE,_("Enable scalar field mapping"), &m_ScalarMapActive, 1);
	m_ScalarComboBox=m_Gui->Combo(ID_SCALARS,"",&m_ScalarIndex,m_NumberOfArrays,m_ScalarsInComboBoxNames);	
	m_LutSwatch=m_Gui->Lut(ID_LUT,"Lut",m_Table);

	m_Gui->Enable(ID_SCALARS, m_ScalarMapActive != 0);
	m_Gui->Enable(ID_LUT, m_ScalarMapActive != 0);

	m_Gui->Divider();
	m_Gui->Label("");
	m_Gui->Update();
	return m_Gui;
}



//----------------------------------------------------------------------------
void lhpPipeMeshFake::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
		case ID_WIREFRAME:
			{
				e->SetSender(this);
				e->SetBool(m_Wireframe);
				mafEventMacro(*e);

				m_Gui->Enable(ID_BORDER_CHANGE,m_Wireframe);
				m_Gui->Enable(ID_WIRED_ACTOR_VISIBILITY,!m_Wireframe);
				m_MaterialButton->UpdateMaterialIcon();
				m_MaterialButton->GetGui()->Update();
				m_Gui->Update();
			}
		break;
		case ID_BORDER_CHANGE:
			{
				e->SetSender(this);
				e->SetDouble(m_Border);
				mafEventMacro(*e);

				m_MaterialButton->UpdateMaterialIcon();
				m_MaterialButton->GetGui()->Update();
				m_Gui->Update();
			}
		case ID_SCALAR_MAP_ACTIVE:
			{
				e->SetSender(this);
				e->SetBool(m_ScalarMapActive);
				m_Gui->Enable(ID_SCALARS, m_ScalarMapActive != 0);
				m_Gui->Enable(ID_LUT, m_ScalarMapActive != 0);
				m_Gui->Update();
				mafEventMacro(*e);
			}
		break;
		case ID_USE_VTK_PROPERTY:
			{
				e->SetSender(this);
				e->SetBool(m_UseVTKProperty);
				mafEventMacro(*e);
				
				m_MaterialButton->Enable(m_UseVTKProperty != 0);
				m_MaterialButton->UpdateMaterialIcon();
				m_MaterialButton->GetGui()->Update();
				m_Gui->Update();
			}
		break;
		case ID_WIRED_ACTOR_VISIBILITY:
			{
				e->SetSender(this);
				e->SetBool(m_BorderElementsWiredActor);
				mafEventMacro(*e);
			}
			break;
		case VME_CHOOSE_MATERIAL:
			{
				mafEventMacro(*e);
				mmaMaterial *material = (mmaMaterial *)m_Vme->GetAttribute("MaterialAttributes");
				if(material && material->m_Prop )
				{
					bool newWireframe=(material->m_Prop->GetRepresentation() == VTK_WIREFRAME);
					if (newWireframe!=m_Wireframe)
					{
						mafEvent newEvent;
						m_Wireframe=newWireframe;
						newEvent.SetId(ID_WIREFRAME);
						newEvent.SetSender(this);
						newEvent.SetBool(m_Wireframe);
						m_Gui->Enable(ID_BORDER_CHANGE,m_Wireframe);
						m_Gui->Enable(ID_WIRED_ACTOR_VISIBILITY,!m_Wireframe);
						m_Gui->Update();
						mafEventMacro(newEvent);
					}
				}
			}
		break;	
		case ID_LUT:
			{
				e->SetSender(this);
				e->SetVtkObj((vtkObject*)m_Table);
				mafEventMacro(*e);
			}
		break;
		case ID_SCALARS:
			{
				e->SetSender(this);
				e->SetDouble(m_ScalarIndex);
				mafEventMacro(*e);
				m_Table->DeepCopy(m_MeshMaterial->m_ColorLut);
			}
		
    }
	
	
  }
 
}

void lhpPipeMeshFake::CopyGuiInfoFromPipe(mafPipeMesh *pipe)
{
	lhpPipeMeshFake *pipeFakeCopy=(lhpPipeMeshFake *)pipe;
	m_Vme=pipeFakeCopy->m_Vme;
	m_Border=pipeFakeCopy->m_Border;
	m_Wireframe = pipeFakeCopy->m_Wireframe;
	m_BorderElementsWiredActor = pipeFakeCopy->m_BorderElementsWiredActor;
	m_UseVTKProperty  = pipeFakeCopy->m_UseVTKProperty;
	m_ScalarMapActive = pipeFakeCopy->m_ScalarMapActive;
	m_ScalarIndex = pipeFakeCopy->m_ScalarIndex;
	m_NumberOfArrays=pipeFakeCopy->m_NumberOfArrays;
	m_ScalarsInComboBoxNames=pipeFakeCopy->m_ScalarsInComboBoxNames;
	m_MeshMaterial=pipeFakeCopy->m_MeshMaterial;
	m_Table->DeepCopy(pipeFakeCopy->m_Table);

	if(m_Gui)
	{
		m_Gui->Enable(ID_WIRED_ACTOR_VISIBILITY,!m_Wireframe);
		m_Gui->Enable(ID_SCALARS,m_ScalarMapActive);
		m_Gui->Enable(ID_LUT, m_ScalarMapActive != 0);

		m_LutSwatch->SetLut(m_Table);
		
		m_MaterialButton->SetVME(m_Vme);
		m_MaterialButton->UpdateMaterialIcon();
		
		m_ScalarComboBox->Clear();
		for(int i=0;i<m_NumberOfArrays;i++)
			m_ScalarComboBox->Append(m_ScalarsInComboBoxNames[i]);
			
		m_Gui->Update();
	}
}
