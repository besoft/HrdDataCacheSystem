/*=========================================================================

 Program: NMSBuilder
 Module: lhpOpModifyOpenSimModelRunIKSimulationAPI
 Authors: Stefano Perticoni
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// INCLUDES
#include <string>
//#include <OpenSim/version.h>
#include <OpenSim/Common/Storage.h>
#include <OpenSim/Common/ScaleSet.h>
#include <OpenSim/Common/IO.h>
#include <OpenSim/Common/LoadOpenSimLibrary.h>

#include <OpenSim/Simulation/Model/Model.h>
#include <OpenSim/Simulation/Model/MarkerSet.h>
#include <OpenSim/Tools/ScaleTool.h>
#include <OpenSim/Simulation/Model/Marker.h>
#include <OpenSim/Tools/InverseKinematicsTool.h>

#include <ctime>  // clock(), clock_t, CLOCKS_PER_SEC

using namespace std;
using namespace OpenSim;

#include "lhpBuilderDecl.h"

//----------------------------------------------------------------------------
// NOTE: Every CPP openSimFileName in the MAF must include "mafDefines.h" as first.
// This force to include Window, wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpModifyOpenSimModelRunIKSimulationAPI.h"

#include "mafJointAnalysis.h"
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
#include "mafVMELandmark.h"
#include "mafTagArray.h"
#include "vtkMath.h"
#include "mafTransform.h"
#include "mafVMERefSys.h"
#include "mafMatrix3x3.h"
#include "wx/busyinfo.h"
#include "mafOpImporterExternalFile.h"
#include "lhpOpImporterOpenSimIKSimulationResults.h"
using namespace xercesc;
using namespace std;

#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelRunIKSimulationAPI);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateJoint_ID
{
	ID_CHOOSE_IK_TASK_SET_FILE = lhpOpModifyOpenSimModel::ID_LAST,
	ID_CHOOSE_MARKER_FILE,
	ID_EDIT_SIMULATION_SETUP_FILE,
	ID_RUN_SIMULATION,
	ID_BUILD_MODEL,
	ID_IMPORT_RESULTS_INTO_MSF,
};

enum {X = 0, Y= 1 , Z = 2};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelRunIKSimulationAPI::lhpOpModifyOpenSimModelRunIKSimulationAPI(const wxString &label) :
lhpOpModifyOpenSimModel(label)
//----------------------------------------------------------------------------
{
	//------------------------------------
	m_TestMode = false;  // default to false
	//------------------------------------

	m_OpType	= OPTYPE_OP;
	m_Canundo = false;
	m_Input		= NULL; 
	m_Debug = 0;

	m_IKTaskSetFileABSPath = "";
	m_MarkerSetFileABSPath = "";
	m_IKSolverABSPath = "c:\\OpenSim3InstallDir\\OpenSim\\bin\\ik.exe";
	m_IKSimulationSetupFileName = "setupIK.xml";
	m_BuildModel = false;
	m_ImportResultsIntoMSF = false;
}

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelRunIKSimulationAPI::~lhpOpModifyOpenSimModelRunIKSimulationAPI( ) 
	//----------------------------------------------------------------------------
{
	assert(true);
}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelRunIKSimulationAPI::Copy()   
//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelRunIKSimulationAPI(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelRunIKSimulationAPI::Accept(mafNode *node)
	//----------------------------------------------------------------------------
{
	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelRunIKSimulationAPI::OpRun()   
	//----------------------------------------------------------------------------
{	

	CreateGui();
	ShowGui();

}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelRunIKSimulationAPI::CreateGui()
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

	m_Gui->Label("Update model");
	m_Gui->Bool(ID_BUILD_MODEL, "", &m_BuildModel);
	m_Gui->Enable(ID_BUILD_MODEL, true);

	m_Gui->Label("");
	m_Gui->Button(ID_CHOOSE_IK_TASK_SET_FILE, "IK Task Set File");
	m_Gui->Enable(ID_CHOOSE_IK_TASK_SET_FILE, true);

	m_Gui->Label("");
	m_Gui->Button(ID_CHOOSE_MARKER_FILE, "Marker File");
	m_Gui->Enable(ID_CHOOSE_MARKER_FILE, false);

	m_Gui->Label("");
	m_Gui->Button(ID_EDIT_SIMULATION_SETUP_FILE, "Edit IK Setup File");
	m_Gui->Enable(ID_EDIT_SIMULATION_SETUP_FILE, false);

	m_Gui->Label("");
	m_Gui->Label("Import Results into MSF");
	m_Gui->Bool(ID_IMPORT_RESULTS_INTO_MSF, "", &m_ImportResultsIntoMSF);
	m_Gui->Enable(ID_IMPORT_RESULTS_INTO_MSF, true);

	m_Gui->Label("");
	m_Gui->Button(ID_RUN_SIMULATION, "Run Simulation");
	m_Gui->Enable(ID_RUN_SIMULATION, false);
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
	m_Gui->Enable(ID_RUN_SIMULATION, false);
	m_Gui->OkCancel();
}



void lhpOpModifyOpenSimModelRunIKSimulationAPI::OnEvent(mafEventBase *maf_event)
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

	case ID_CHOOSE_IK_TASK_SET_FILE:
		{
			OnChooseIKTaskSetFile();
		}
		break;


	case ID_CHOOSE_MARKER_FILE:
		{
			OnChooseMarkerSetFile();
		}
		break;


	case ID_EDIT_SIMULATION_SETUP_FILE:
		{
			OnEditSimulationSetupFile();
		}
		break;

	case ID_RUN_SIMULATION:
		{
			OnRunSimulation();
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

void lhpOpModifyOpenSimModelRunIKSimulationAPI::OnChooseMarkerSetFile()
{
	wxString defaultDir = "";

	mafString wildc = "Marker file (*.trc)|*.trc";
	mafString fileName;

	fileName = mafGetOpenFile(defaultDir, wildc, _("Choose Marker file")).c_str();

	if(!fileName.IsEmpty())
	{
		m_MarkerSetFileABSPath = fileName;
		assert(wxFileExists(m_MarkerSetFileABSPath));

		wxString path, name, ext;
		wxSplitPath(m_MarkerSetFileABSPath.c_str(),&path,&name,&ext);

		wxString targetFile = GetOutputModelDirectory() + name + "." + ext;

		if (targetFile != m_MarkerSetFileABSPath)
		{
			wxCopyFile(m_MarkerSetFileABSPath , targetFile);
		}
		
		assert(wxFileExists(targetFile));

		m_Gui->Enable(ID_EDIT_SIMULATION_SETUP_FILE, true);
		m_Gui->Enable(ID_RUN_SIMULATION, true);

		CreateIKSimulationSetupFile();

	}
}

void lhpOpModifyOpenSimModelRunIKSimulationAPI::OnChooseIKTaskSetFile()
{
	wxString defaultDir = "";

	mafString wildc = "IKTaskSet file (*.xml)|*.xml";
	mafString fileName;
	
	fileName = mafGetOpenFile(defaultDir, wildc, _("Choose IKTaskSet file")).c_str();
	
	if(!fileName.IsEmpty())
	{
		m_IKTaskSetFileABSPath = fileName;
		assert(wxFileExists(m_IKTaskSetFileABSPath));

		wxString path, name, ext;
		wxSplitPath(m_IKTaskSetFileABSPath.c_str(),&path,&name,&ext);

		wxString targetFile = GetOutputModelDirectory() + name + "." + ext;
		
		if (m_IKTaskSetFileABSPath != targetFile)
		{
			wxCopyFile(m_IKTaskSetFileABSPath , targetFile);
		}		
			
		assert(wxFileExists(targetFile));

		m_Gui->Enable(ID_CHOOSE_MARKER_FILE, true);
	}	
}

void lhpOpModifyOpenSimModelRunIKSimulationAPI::WriteNMSBuilderDataToFile()
{
	assert(!m_MarkerSetFileABSPath.IsEmpty());
	assert(!m_IKTaskSetFileABSPath.IsEmpty());

	// CreateIKSimulationSetupFile();
}


void lhpOpModifyOpenSimModelRunIKSimulationAPI::CreateIKSimulationSetupFile()
{
	
	assert(m_MarkerSetFileABSPath);
	assert(m_IKTaskSetFileABSPath);

	assert(wxFileExists(m_MarkerSetFileABSPath));
	assert(wxFileExists(m_IKTaskSetFileABSPath));

	wxString pathMarkerSetFileABSPath, nameMarkerSetFileABSPath, extMarkerSetFileABSPath;
	wxSplitPath(m_MarkerSetFileABSPath.c_str(), &pathMarkerSetFileABSPath, &nameMarkerSetFileABSPath, &extMarkerSetFileABSPath);

	wxString pathIKTaskSetFileABSPath, nameIKTaskSetFileABSPath, extIKTaskSetFileABSPath;
	wxSplitPath(m_IKTaskSetFileABSPath.c_str(), &pathIKTaskSetFileABSPath, &nameIKTaskSetFileABSPath, &extIKTaskSetFileABSPath);

	ostringstream stringStream;

	stringStream << 

	//-----------------------------------------
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl <<
	"<OpenSimDocument Version=\"20302\">"  << std::endl <<
	"	<InverseKinemtaticsTool name=\"subject01\">"  << std::endl <<
	"		<!--Name of the .osim file used to construct a model.-->"  << std::endl <<
	"		<model_file> outputModel.osim </model_file>" << std::endl <<
	"		<!--Specify which optimizer to use (ipopt or cfsqp or jacobian).-->"  << std::endl <<
	"		<!--Task set used to specify IK weights.-->"  << std::endl <<
	"	    <IKTaskSet file=\"" << nameIKTaskSetFileABSPath.c_str() << "." << extIKTaskSetFileABSPath.c_str() << "\"/>"  << std::endl <<
	"		<!--Parameters for solving the IK problem for each trial. Each trial"  << std::endl <<
	"		should get a seperate SimmIKTril block.-->"  << std::endl <<
	"		<!--TRC file (.trc) containing the time history of experimental marker"  << std::endl <<
	"					positions.-->"  << std::endl <<
	"		<marker_file> " <<  nameMarkerSetFileABSPath.c_str() << "." << extMarkerSetFileABSPath.c_str() << " </marker_file>" << std::endl <<
	"		<!--Name of file containing the joint angles used to set the initial"  << std::endl <<
	"					configuration of the model for the purpose of placing the markers."  << std::endl <<
	"					These coordinate values can also be included in the optimization"  << std::endl <<
	"					problem used to place the markers. Before the model markers are"  << std::endl <<
	"					placed, a single frame of an inverse kinematics (IK) problem is"  << std::endl <<
	"					solved. The IK problem can be solved simply by matching marker"  << std::endl <<
	"					positions, but if the model markers are not in the correct locations,"  << std::endl <<
	"					the IK solution will not be very good and neither will marker"  << std::endl <<
	"					placement. Alternatively, coordinate values (specified in this file)"  << std::endl <<
	"					can be specified and used to influence the IK solution. This is"  << std::endl <<
	"					valuable particularly if you have high confidence in the coordinate"  << std::endl <<
	"					values. For example, you know for the static trial the subject was"  << std::endl <<
	"					standing will all joint angles close to zero. If the coordinate set"  << std::endl <<
	"					(see the CoordinateSet property) contains non-zero weights for"  << std::endl <<
	"					coordinates, the IK solution will try to match not only the marker"  << std::endl <<
	"					positions, but also the coordinates in this file. Least-squared error"  << std::endl <<
	"					is used to solve the IK problem.-->"  << std::endl <<
	"		<coordinate_file></coordinate_file>"  << std::endl <<
	"		<!--Time range over which the IK problem is solved.-->"  << std::endl <<
	"		<time_range>0.4 1.60</time_range>"  << std::endl <<
	"		<!--Name of the motion file (.mot) to which the results should be written.-->"  << std::endl <<
	"		<output_motion_file>output_ik.mot</output_motion_file>"  << std::endl <<
	"		<!--A positive scalar that is used to weight the importance of satisfying constraints.A weighting of 'Infinity' or if it is unassigned results in the constraints being strictly enforced.-->"  << std::endl <<
	"		<constraint_weight>20.0</constraint_weight>"  << std::endl <<
	"		<!--The accuracy of the solution in absolute terms. I.e. the number of significantdigits to which the solution can be trusted.-->"  << std::endl <<
	"		<accuracy>1e-5</accuracy>"  << std::endl <<
	"	</InverseKinemtaticsTool>"  << std::endl <<
	"</OpenSimDocument>"  << std::endl;


	//-----------------------------------------

	ofstream myFile;

	wxString ikSimulationSetupFileABSPath = GetOutputModelDirectory() + m_IKSimulationSetupFileName.c_str();

	myFile.open(ikSimulationSetupFileABSPath);
	myFile << stringStream.str().c_str();
	myFile.close();

	assert(wxFileExists(ikSimulationSetupFileABSPath));

}

void lhpOpModifyOpenSimModelRunIKSimulationAPI::OnEditSimulationSetupFile()
{
	OpenFileWithNotepadPP(GetOutputModelDirectory() + m_IKSimulationSetupFileName);
}

void lhpOpModifyOpenSimModelRunIKSimulationAPI::OnRunSimulation()
{

	wxString oldDir = wxGetCwd();
	wxSetWorkingDirectory(GetOutputModelDirectory());
	
	wxBusyInfo *waitUpdateModel = NULL;
	
	if (m_BuildModel == true)
	{

		wxString outputModelABSFileName = GetOutputModelDirectory() + GetOutputModelFileName();
		wxRemoveFile(outputModelABSFileName);
		assert(wxFileExists(outputModelABSFileName) == false);

		waitUpdateModel = new wxBusyInfo("Updating OpenSim model and running IK simulation. Please wait...");
		mafSleep(2000);

		BuildOpenSIMFile();

		if (wxFileExists(outputModelABSFileName) == false)
		{
			cppDEL(waitUpdateModel);
			std::string msg = std::string("Model generation for simulation failed, please check your model");
			wxMessageBox(msg.c_str(),_(""),wxOK|wxICON_ERROR);
			return;
		}
	}
	else
	{
		waitUpdateModel = new wxBusyInfo("Running IK simulation. Please wait...");
		mafSleep(2000);
	}

	wxString outputIKSimulationFileName = "output_ik.mot";

	wxString outputIKSimulationABSFileName = GetOutputModelDirectory() + outputIKSimulationFileName;
	wxRemoveFile(outputIKSimulationABSFileName);
	assert(wxFileExists(outputIKSimulationABSFileName) == false);

	wxString ikSimulationSetupFileABSPath = GetOutputModelDirectory() + m_IKSimulationSetupFileName.c_str();
	wxString ikResultsDirectory = GetOutputModelDirectory().c_str();

	wxString ikLogFileName = "out.log";
	wxString ikErrFileName = "err.log";			

	wxString ikLogABSFileName = GetOutputModelDirectory() + ikLogFileName;
	wxString ikErrABSFileName = GetOutputModelDirectory() + ikErrFileName;

	wxRemoveFile(ikLogABSFileName);
	assert(wxFileExists(ikLogABSFileName) == false);

	wxRemoveFile(ikErrABSFileName);
	assert(wxFileExists(ikErrABSFileName) == false);

	///////////////////

	try
	{
		RunSimulation(ikSimulationSetupFileABSPath, ikResultsDirectory, ikLogABSFileName, ikErrABSFileName, outputIKSimulationABSFileName);

	} catch(const std::exception& x) {

		std::ostringstream stringStream;
		stringStream << "Exception in IK: " << x.what() << endl;          
		mafLogMessage( stringStream.str().c_str());
		wxMessageBox("Error running simulation");
	}


	///////////////////
	
	if (wxFileExists(outputIKSimulationABSFileName) == false)
	{
		// Study how to print the log file with the API
		OpenFileWithNotepadPP(ikLogABSFileName);
		OpenFileWithNotepadPP(ikErrABSFileName);
	}
	else
	{
		OpenFileWithNotepadPP(ikLogABSFileName);
		OpenFileWithNotepadPP(outputIKSimulationABSFileName);
	}

	if (m_ImportResultsIntoMSF == true)
	{
		lhpOpImporterOpenSimIKSimulationResults *importer=new lhpOpImporterOpenSimIKSimulationResults("importer");
		importer->SetInput(m_Input->GetRoot());
		assert(wxFileExists(outputIKSimulationABSFileName.c_str()));
		importer->SetFileName(outputIKSimulationABSFileName.c_str());
		importer->Import();
		mafDEL(importer);
	}

	wxSetWorkingDirectory(oldDir);
	cppDEL(waitUpdateModel);

}


void lhpOpModifyOpenSimModelRunIKSimulationAPI::RunSimulation( wxString ikSimulationSetupFileABSPath, wxString ikResultsDirectory, wxString ikLogABSFileName, wxString ikErrABSFileName, wxString outputIKSimulationABSFileName )
{
	// CONSTRUCT

	std::ostringstream stringStream;
	stringStream << "//--------------------------------------------------------------------------------------" << std::endl;
	stringStream <<"Constructing tool from setup file "<< ikSimulationSetupFileABSPath << std::endl;          
	stringStream <<"Performing simulation. Please wait..."<< std::endl;          
	mafLogMessage(stringStream.str().c_str());

	InverseKinematicsTool ik(ikSimulationSetupFileABSPath.c_str());
	ik.print("ik_setup_check.xml");

	ik.setResultsDir(ikResultsDirectory.c_str());

	// PRINT MODEL INFORMATION

	// start timing
	std::clock_t startTime = std::clock();

	// RUN

	// Redirect cout.
	streambuf* oldCoutStreamBuf = cout.rdbuf();
	ostringstream strCout;
	cout.rdbuf( strCout.rdbuf() );

	streambuf* oldCerrStreamBuf = cerr.rdbuf();
	ostringstream strCerr;
	cerr.rdbuf( strCerr.rdbuf() );

	ik.run();

	// Restore old cout.
	cout.rdbuf( oldCoutStreamBuf );

	// Restore old cerr.
	cerr.rdbuf( oldCerrStreamBuf );

	ofstream out;
	out.open (ikLogABSFileName.c_str());
	out << strCout.str().c_str();
	out.close();

	ofstream err;
	err.open (ikErrABSFileName.c_str());
	err << strCerr.str().c_str();
	err.close();

	//----------------------------
	// Catch any thrown exceptions
	//----------------------------

	std::ostringstream stringStream2;

	stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
	stringStream2 << "IK compute time = " << 1.e3*(std::clock()-startTime)/CLOCKS_PER_SEC << std::endl;          
	stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
	stringStream2 << "Output simulation file: " << outputIKSimulationABSFileName.c_str() << std::endl;
	stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
	stringStream2 << "Simulation Log file: " << ikLogABSFileName.c_str() << std::endl;
	stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
	stringStream2 << "Simulation Error file: " << ikErrABSFileName.c_str() << std::endl;
	stringStream2 << "//**************************************************************************************" << std::endl;
	stringStream2 << "IK simulation ended" << std::endl;          
	stringStream2 << "//**************************************************************************************" << std::endl;

	mafLogMessage(stringStream2.str().c_str());
}
