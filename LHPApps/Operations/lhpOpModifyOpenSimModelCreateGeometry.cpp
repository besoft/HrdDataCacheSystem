/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelCreateGeometry.cpp,v $
Language:  C++
Date:      $Date: 2012-03-16 14:27:41 $
Version:   $Revision: 1.1.2.4 $
Authors:   Stefano Perticoni 
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "lhpBuilderDecl.h"

//----------------------------------------------------------------------------
// NOTE: Every CPP openSimFileName in the MAF must include "mafDefines.h" as first.
// This force to include Window, wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpModifyOpenSimModelCreateGeometry.h"

#include <wx/mimetype.h>

#include "mafDecl.h"
#include "mafEvent.h"

#include "mafVMEExternalData.h"
#include "lhpUtils.h"
#include "mafGUI.h"
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <iosfwd>
#include <string>
#include "mafVMESurface.h"
#include "mafDataVector.h"
#include "medOpImporterVTKXML.h"
#include "medOpExporterVTKXML.h"
#include "vtkPolyData.h"

using namespace xercesc;
using namespace std;

#define UNDEFINED_STRING "UNDEFINED"
//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelCreateGeometry);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateGeometry_ID
{
	ID_CHOOSE_GEOMETRY_VME = lhpOpModifyOpenSimModel::ID_LAST,
	ID_GENERATE_GEOMETRY,
	ID_HELP_BUTTON,
};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateGeometry::lhpOpModifyOpenSimModelCreateGeometry(const wxString &label) :
lhpOpModifyOpenSimModel(label)
//----------------------------------------------------------------------------
{
	m_OpType	= OPTYPE_OP;
	m_Canundo = false;
	m_Input		= NULL; 
	
	m_Surface = NULL;
	m_SurfaceVtkAbsFileName = UNDEFINED_STRING;

	m_DictionaryToConfigureFileName = "OSIM_CreateGeometry_Dictionary.txt.in";
	m_DataFromNMSBuilderFileName = "OSIM_CreateGeometry_DataFromNMSBuilder.txt";
	m_ConfiguredDictionaryFileName = "OSIM_CreateGeometry_Dictionary.txt";

	m_ComponentToConfigureFileName = "OSIM_CreateGeometry.cpp.in";
	m_ConfiguredComponentFileName = "OSIM_CreateGeometry.cpp";

}
//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateGeometry::~lhpOpModifyOpenSimModelCreateGeometry( ) 
//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelCreateGeometry::Copy()   
//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelCreateGeometry(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelCreateGeometry::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
 	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateGeometry::OpRun()   
//----------------------------------------------------------------------------
{	

	CreateGui();
	ShowGui();
}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateGeometry::CreateGui()
//----------------------------------------------------------------------------
{
	m_Gui = new mafGUI(this);
	m_Gui->SetListener(this);
	mafEvent buildHelpGui;
	buildHelpGui.SetSender(this);
	buildHelpGui.SetId(GET_BUILD_HELP_GUI);
	mafEventMacro(buildHelpGui);

	if (buildHelpGui.GetArg() == true)
	{
		m_Gui->Button(ID_HELP_BUTTON, "Help","");	
	}
	
	m_Gui->Label("");
	m_Gui->Label("Create geometry from VME", true);
	m_Gui->Label("");
	m_Gui->Button(ID_CHOOSE_GEOMETRY_VME, "Choose VME");
	m_Gui->Label("");
	m_Gui->Button(ID_GENERATE_GEOMETRY, "Generate OpenSim Geometry");
	m_Gui->Label("");
	m_Gui->Divider(2);
	m_Gui->Label("");
	m_Gui->Button(ID_EDIT_MODEL_SOURCE, "Edit Model Source");
	m_Gui->Label("");
	m_Gui->Button(ID_GENERATE_MODEL, "Generate Model");
	m_Gui->Label("");
	m_Gui->Divider(2);
	m_Gui->Label("Post processing facilities", true);
	m_Gui->Label("");
	m_Gui->Button(ID_OPEN_OUTPUT_MODEL_DIRECTORY, "Open output model files dir");
	m_Gui->Label("");
	m_Gui->Button(ID_OPEN_MSF_DIRECTORY, "Open MSF directory");
	m_Gui->Label("");
	m_Gui->Enable(ID_GENERATE_GEOMETRY, false);
	m_Gui->OkCancel();
}



void lhpOpModifyOpenSimModelCreateGeometry::OnEvent(mafEventBase *maf_event)
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch(e->GetId())
		{

		case ID_HELP_BUTTON:
			{
				mafEvent helpEvent;
				helpEvent.SetSender(this);
				mafString operationLabel = this->m_Label;
				helpEvent.SetString(&operationLabel);
				helpEvent.SetId(OPEN_HELP_PAGE);
				mafEventMacro(helpEvent);
			}
			break;
	
		case ID_CHOOSE_GEOMETRY_VME:
			{
				OnChooseGeometryVME();
			}
			break;

		case ID_GENERATE_GEOMETRY:
			{
				OnGenerateGeometry();
			}
			break;

		case ID_HELP:
			{
				mafEvent helpEvent;
				helpEvent.SetSender(this);
				mafString operationLabel = this->m_Label;
				helpEvent.SetString(&operationLabel);
				helpEvent.SetId(OPEN_HELP_PAGE);
				mafEventMacro(helpEvent);
			}
			break;

		//------------------------------------------------------------------
		//		from superclass
		//------------------------------------------------------------------
		case ID_EDIT_MODEL_SOURCE:
			{
				lhpOpModifyOpenSimModel::EditOpenSIMFile();
			}
			break;

		case ID_GENERATE_MODEL:
			{
				lhpOpModifyOpenSimModel::BuildOpenSIMFile();
			}
			break;

		case ID_VIEW_MODEL_IN_OPENSIM:
			{
				lhpOpModifyOpenSimModel::ViewModelInOpenSim();
			}
			break;


		case ID_OPEN_OUTPUT_MODEL_DIRECTORY:
			{
				lhpOpModifyOpenSimModel::OnOpenOutputModelDirectory();
			}
			break;

		case ID_OPEN_MSF_DIRECTORY:
			{
				lhpOpModifyOpenSimModel::OnOpenMSFDirectory();
			}
			break;

		case wxOK:
			{
				OpStop(OP_RUN_OK);
			}
			break;

		case wxCANCEL:
			{
				OpStop(OP_RUN_CANCEL);
			}
			break;

		}
	}
}

void lhpOpModifyOpenSimModelCreateGeometry::OnChooseGeometryVME()
{
	mafString title = mafString("Select a VME surface:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModelCreateGeometry::VmeSurfaceAcceptForGeometryGeneration);
	mafEventMacro(e);
	mafVME *vme = (mafVME *)e.GetVme();	
	
	if (vme == NULL)
	{
		return;
	}

	m_Gui->Enable(ID_GENERATE_GEOMETRY, true);

	assert(vme);
	m_Surface = mafVMESurface::SafeDownCast(vme);
	m_SurfaceVtkAbsFileName = GetVTKFileNameAbsPath(m_Surface);
	assert(wxFileExists(m_SurfaceVtkAbsFileName));
	
	m_SurfaceVTPFileName = m_Surface->GetName() + wxString(".vtp");

	wxString outputFileName = GetOutputModelDirectory() + m_SurfaceVTPFileName;

	medOpExporterVTKXML *exporter=new medOpExporterVTKXML();
	exporter->SetInput(vme);
	exporter->TestModeOn();
	exporter->SetFileName(outputFileName.c_str());
	exporter->ExportAsBynaryOn();
	exporter->ExportVTK();

	assert(wxFileExists(outputFileName));

	vtkDEL(exporter);
}

bool lhpOpModifyOpenSimModelCreateGeometry::VmeSurfaceAcceptForGeometryGeneration(mafNode *node) 
{  	
	return mafVMESurface::SafeDownCast(node) != NULL;
}

void lhpOpModifyOpenSimModelCreateGeometry::OnGenerateGeometry()
{
	GenerateOpenSimComponentAPIAndOpenItInNotepadPP();
}

void lhpOpModifyOpenSimModelCreateGeometry::WriteNMSBuilderDataToFile()
{
	assert(m_SurfaceVtkAbsFileName != UNDEFINED_STRING);

	// write data extracted from NMSBuilder
	ostringstream stringStream;
	stringStream << 
		"[VTPFileNameFromNMS]" << "," << m_SurfaceVTPFileName.c_str() << std::endl << \
		"[VMENameFromNMS]" << "," << m_Surface->GetName() << std::endl;

	ofstream myFile;
	myFile.open(GetOpenSimAPITemplatesDir() + m_DataFromNMSBuilderFileName.c_str());
	myFile << stringStream.str().c_str();
	myFile.close();
}

