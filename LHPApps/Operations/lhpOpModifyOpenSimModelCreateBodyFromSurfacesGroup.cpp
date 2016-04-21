/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup.cpp,v $
Language:  C++
Date:      $Date: 2012-04-11 17:06:29 $
Version:   $Revision: 1.1.2.2 $
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

#include "lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup.h"

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
#include "mafTagArray.h"
#include "vtkMath.h"
#include "mafTransform.h"
#include "lhpOpModifyOpenSimModelCreateBodyFromSurface.h"
#include "mafVME.h"
#include "mafNode.h"
#include "../Operations/medOpComputeInertialTensor.h"

using namespace xercesc;
using namespace std;

#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999999

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateBody_ID
{
	ID_CHOOSE_SURFACES_GROUP = lhpOpModifyOpenSimModel::ID_LAST,
	ID_GENERATE_BODY,
};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup(const wxString &label) :
lhpOpModifyOpenSimModel(label)
//----------------------------------------------------------------------------
{
	m_OpType	= OPTYPE_OP;
	m_Canundo = false;
	m_Input		= NULL; 

	m_SurfacesGroup = NULL;

	m_DictionaryToConfigureFileName = "OSIM_CreateBodyFromSurfacesGroup_Dictionary.txt.in";
	m_DataFromNMSBuilderFileName = "OSIM_CreateBodyFromSurfacesGroup_DataFromNMSBuilder.txt";
	m_ConfiguredDictionaryFileName = "OSIM_CreateBodyFromSurfacesGroup_Dictionary.txt";

	m_ComponentToConfigureFileName = "OSIM_CreateBodyFromSurfacesGroup.cpp.in";
	m_ConfiguredComponentFileName = "OSIM_CreateBodyFromSurfacesGroup.cpp";

	#define UNDEFINED_STRING "UNDEFINED"

	m_Ixx = UNDEFINED_DOUBLE;
	m_Iyy = UNDEFINED_DOUBLE;
	m_Izz = UNDEFINED_DOUBLE;
	m_Ixy = UNDEFINED_DOUBLE;
	m_Ixz = UNDEFINED_DOUBLE;
	m_Iyz = UNDEFINED_DOUBLE;

	m_Mass = UNDEFINED_DOUBLE;
	
	m_CenterOfMass[0] = UNDEFINED_DOUBLE;
	m_CenterOfMass[1] = UNDEFINED_DOUBLE;
	m_CenterOfMass[2] = UNDEFINED_DOUBLE;


}
//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::~lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup( ) 
//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::Copy()   
//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
 	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::OpRun()   
//----------------------------------------------------------------------------
{	

	CreateGui();
	ShowGui();
}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::CreateGui()
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
		m_Gui->Button(ID_HELP, "Help","");	
	}

	m_Gui->Label("");
	m_Gui->Button(ID_CHOOSE_SURFACES_GROUP, "Choose surfaces group");
	m_Gui->Label("");
	m_Gui->Button(ID_GENERATE_BODY, "Generate OpenSim Body");
	m_Gui->Label("");
	m_Gui->Divider(2);
	m_Gui->Label("Generate Model", true);
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

	m_Gui->Enable(ID_GENERATE_BODY, false);

	m_Gui->OkCancel();
}



void lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::OnEvent(mafEventBase *maf_event)
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch(e->GetId())
		{
		
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

		case ID_CHOOSE_SURFACES_GROUP:
			{
				OnChooseSurfacesGroup();
			}
			break;

		case ID_GENERATE_BODY:
			{
				OnGenerateBody();
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

void lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::OnChooseSurfacesGroup()
{
	mafString title = mafString("Select a VME surfaces group:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMEGroupAcceptForBodyGeneration);
	mafEventMacro(e);
	mafVMEGroup *node = mafVMEGroup::SafeDownCast(e.GetVme());	
	
	if (node == NULL)
	{
		return;
	}

	m_SurfacesGroup = node;

	m_Gui->Enable(ID_GENERATE_BODY, true);
	
}

void lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::GenerateOpenSimComponentAPI()
{
	CreateCPPCodeTemplate();
	Superclass::GenerateOpenSimComponentAPI();
}

void lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::OnGenerateBody()
{
	CreateCPPCodeTemplate();
	GenerateOpenSimComponentAPIAndOpenItInNotepadPP();
}

void lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::WriteNMSBuilderDataToFile()
{
	// write data extracted from NMSBuilder
	ostringstream stringStream;
	stringStream << 
		"[***blockName_FromNMSB***]"  << "," << ReplaceSpaceWithUnderscore(m_SurfacesGroup->GetName()) << std::endl <<
	    "[***blockMass_FromNMSB***]" <<"," <<  m_Mass << std::endl <<
		"[***blockInertia_XX_FromNMSB***]" <<"," <<  m_Ixx << std::endl <<
		"[***blockInertia_YY_FromNMSB***]" << "," <<  m_Iyy << std::endl <<
		"[***blockInertia_ZZ_FromNMSB***]" <<"," <<  m_Izz << std::endl <<
		"[***blockInertia_XY_FromNMSB***]" <<"," <<  m_Ixy << std::endl <<
		"[***blockInertia_XZ_FromNMSB***]" << "," <<  m_Ixz << std::endl <<
		"[***blockInertia_YZ_FromNMSB***]" <<"," <<  m_Iyz << std::endl <<
		"[***blockMassCenter_X_FromNMSB***]" << "," <<  m_CenterOfMass[0] << std::endl <<
		"[***blockMassCenter_Y_FromNMSB***]" << "," <<  m_CenterOfMass[1] << std::endl <<
		"[***blockMassCenter_Z_FromNMSB***]" << "," <<  m_CenterOfMass[2] << std::endl;
	    
	ofstream myFile;
	myFile.open(GetOpenSimAPITemplatesDir() + m_DataFromNMSBuilderFileName.c_str());
	myFile << stringStream.str().c_str();
	myFile.close();
}


void lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::CreateCPPCodeTemplate()
{
	assert(m_SurfacesGroup);
	assert(lhpOpModifyOpenSimModel::VMEGroupAcceptForBodyGeneration(m_SurfacesGroup));

	  /* Inertial tensor
  | Ixx  Iyx  Izx |
  | Iyx  Iyy  Izy | 
  | Izx  Izy  Izz | */
  // [Ixx  Iyx  Izx ,  Iyx  Iyy  Izy , Izx  Izy  Izz]

	// get inertial attributes
	m_Ixx = m_SurfacesGroup->GetTagArray()->GetTag("INERTIAL_TENSOR_COMPONENTS")->GetComponentAsDouble(0);
	m_Iyy = m_SurfacesGroup->GetTagArray()->GetTag("INERTIAL_TENSOR_COMPONENTS")->GetComponentAsDouble(4);
	m_Izz = m_SurfacesGroup->GetTagArray()->GetTag("INERTIAL_TENSOR_COMPONENTS")->GetComponentAsDouble(8);
	m_Ixy = m_SurfacesGroup->GetTagArray()->GetTag("INERTIAL_TENSOR_COMPONENTS")->GetComponentAsDouble(1);
	m_Ixz = m_SurfacesGroup->GetTagArray()->GetTag("INERTIAL_TENSOR_COMPONENTS")->GetComponentAsDouble(2);
	m_Iyz = m_SurfacesGroup->GetTagArray()->GetTag("INERTIAL_TENSOR_COMPONENTS")->GetComponentAsDouble(5);

	m_Mass = m_SurfacesGroup->GetTagArray()->GetTag("SURFACE_MASS")->GetValueAsDouble();
	
	m_CenterOfMass[0] = m_SurfacesGroup->GetTagArray()->GetTag("LOCAL_CENTER_OF_MASS_COMPONENTS")->GetComponentAsDouble(0);
	m_CenterOfMass[1] = m_SurfacesGroup->GetTagArray()->GetTag("LOCAL_CENTER_OF_MASS_COMPONENTS")->GetComponentAsDouble(1);
	m_CenterOfMass[2] = m_SurfacesGroup->GetTagArray()->GetTag("LOCAL_CENTER_OF_MASS_COMPONENTS")->GetComponentAsDouble(2);

	double surfacesGroupCenterOfMass[3] = {UNDEFINED_DOUBLE , UNDEFINED_DOUBLE , UNDEFINED_DOUBLE};

	// ComputeMassCenterFromGroup(m_SurfacesGroup , m_CenterOfMass);

	ostringstream stringStream;

	stringStream << 

	std::endl <<
	"//------------------------------------------------------" << std::endl <<
	"//" << std::endl <<
	"// [blockName] OpenSim Block Body featuring the following " << std::endl <<
	"// inertial parameters from NMSBuilder: " << std::endl <<
	"//" << std::endl <<
	"// - Mass" << std::endl <<
	"// - Inertia" << std::endl <<
	"// - Centre of Mass" << std::endl <<
	"//" << std::endl <<
	"// Target model: osimModel" << std::endl <<
	"//------------------------------------------------------" << std::endl
	<< std::endl <<
	"// Specify the mass" << std::endl <<
	"double blockMass[blockName] = [blockMass] ; // from NMSB" << std::endl <<
	std::endl <<
	"// Specify the centre of mass" << std::endl <<
	"Vec3 blockMassCenter[blockName];" << std::endl <<
	"blockMassCenter[blockName][0] = " << m_CenterOfMass[0] << "; // from NMSB mm to OpenSim meters" << std::endl <<
	"blockMassCenter[blockName][1] = " << m_CenterOfMass[1] << "; // from NMSB mm to OpenSim meters" << std::endl <<
	"blockMassCenter[blockName][2] = " << m_CenterOfMass[2] << "; // from NMSB mm to OpenSim meters" << std::endl <<
	std::endl <<
	"// Specify the inertia" << std::endl <<
	"Inertia blockInertia[blockName]; // from NMSBuilder to OpenSim measures units" << std::endl <<
	"blockInertia[blockName].setInertia( [blockInertia_XX] , [blockInertia_YY] , [blockInertia_ZZ] , " << std::endl <<
	"[blockInertia_XY] , [blockInertia_XZ] , [blockInertia_YZ] );" << std::endl <<
	std::endl <<
	"// Create a new block body with specified properties (from NMSB)" << std::endl <<
	"OpenSim::Body *[blockName] = new OpenSim::Body(\"" << m_SurfacesGroup->GetName() << "\", blockMass[blockName], blockMassCenter[blockName], blockInertia[blockName]);" << std::endl <<
	std::endl <<
	"// Add display geometry to the block to visualize in the GUI (from NMSB)" << std::endl;
	
	//////////////////////// END TEMPLATE /////////////////////////
	
	// for all surfaces convert geometries to VTP and store to OpenSim output model directory 
	int numChildren = m_SurfacesGroup->GetNumberOfChildren();

	for (int i=0;i<numChildren;i++)
	{
		if (m_SurfacesGroup->GetChild(i)->IsMAFType(mafVMESurface))
		{
			mafVMESurface *surface = mafVMESurface::SafeDownCast(m_SurfacesGroup->GetChild(i));
			assert(surface);

			wxString surfaceVTKAbsFileName = GetVTKFileNameAbsPath(surface);
			assert(wxFileExists(surfaceVTKAbsFileName));

			wxString surfaceVTPFileName = surface->GetName() + wxString(".vtp");

			wxString outputAbsFileName = GetOutputModelDirectory() + surfaceVTPFileName;

			medOpExporterVTKXML *exporter=new medOpExporterVTKXML();
			exporter->SetInput(surface);
			exporter->TestModeOn();
			exporter->SetFileName(outputAbsFileName.c_str());
			exporter->ExportAsBynaryOn();
			exporter->ExportVTK();

			assert(wxFileExists(outputAbsFileName));

			vtkDEL(exporter);

			//////////////////////// START TEMPLATE /////////////////////////

			stringStream << "[blockName]->addDisplayGeometry(\"" << surfaceVTPFileName << "\");" << std::endl;

			//////////////////////// END TEMPLATE /////////////////////////
		}
	}
	//////////////////////// START TEMPLATE /////////////////////////
	
	stringStream << std::endl <<
	"Vec3 scaleGeometry[blockName];" << std::endl <<
	"scaleGeometry[blockName][0] = 0.001;" << std::endl <<
	"scaleGeometry[blockName][1] = 0.001;" << std::endl <<
	"scaleGeometry[blockName][2] = 0.001;" << std::endl <<
	std::endl <<
	"[blockName]->scale(scaleGeometry[blockName]);" << std::endl <<

	std::endl << 

	"// Add the block body to the model" << std::endl << 
	"osimModel.addBody([blockName]);" << std::endl << 


	std::endl << 
	"//-----------------END SNIPPET--------------" << std::endl;
	//////////////////////// END TEMPLATE /////////////////////////

	ofstream myFile;

	wxString componentToConfigureABSFileName = GetOpenSimAPITemplatesDir() + m_ComponentToConfigureFileName.c_str();
	
	myFile.open(componentToConfigureABSFileName);
	myFile << stringStream.str().c_str();
	myFile.close();

	assert(wxFileExists(componentToConfigureABSFileName));

}


void lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::ComputeMassCenterFromGroup(mafVMEGroup *inGroup , double *outCenterOfMass)
{
	mafVMEGroup* group = inGroup;
	assert(group);

	int numChildren = group->GetNumberOfChildren();
	int numSurfaces = 0;

	// get number of children surfaces (only direct child!)
	for (int i=0;i<numChildren;i++)
	{
		if (group->GetChild(i)->IsMAFType(mafVMESurface))
		{
			numSurfaces++;
		}
	}

	if (numSurfaces>0) 
	{
		wxString s;
		s << "Found " << numSurfaces << " surfaces: computing mass center for all of them ..";
		mafLogMessage(s.c_str());
	}
	else
	{
		mafLogMessage("no surfaces found in the group. Quit!");
		return;
	}

	double total[3] = {0,0,0};
	double totalMass = 0;

	for (int i=0;i<numChildren;i++)
	{
		if (group->GetChild(i)->IsMAFType(mafVMESurface))
		{
			mafVMESurface *surface = mafVMESurface::SafeDownCast(m_SurfacesGroup->GetChild(i));
			assert(surface);

			double currentSurfaceCenterOfMass[3] = {0,0,0};
			
			std::stringstream s;
			s << "Computing mass center for " << surface->GetName() << std::endl;
			mafLogMessage(s.str().c_str());
	
			surface->GetSurfaceOutput()->GetVTKData()->Update();
			vtkPoints *points = surface->GetSurfaceOutput()->GetSurfaceData()->GetPoints();
			assert(points);

			lhpOpModifyOpenSimModelCreateBodyFromSurface::ComputeCenterOfMass(surface->GetSurfaceOutput()->GetSurfaceData()->GetPoints(), currentSurfaceCenterOfMass);

			double currentMass = medOpComputeInertialTensor::GetMass(surface);
			totalMass += currentMass;

			mafTransform::MultiplyVectorByScalar(currentMass , currentSurfaceCenterOfMass , currentSurfaceCenterOfMass);
			mafTransform::AddVectors(total , currentSurfaceCenterOfMass , total);

		}

	}

	outCenterOfMass[0] = total[0] / totalMass;
	outCenterOfMass[1] = total[1] / totalMass;
	outCenterOfMass[2] = total[2] / totalMass;

}

void lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup::SetInputVMEForComponentAPIGeneration(mafNode *vme)
{
	m_SurfacesGroup = mafVMEGroup::SafeDownCast(vme);
}
