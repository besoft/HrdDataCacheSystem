/*=========================================================================
Program:   Multimod Applicatio Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelRunStaticOptimizationAPI.cpp,v $
Language:  C++
Date:      $Date: 2012-04-20 10:58:02 $
Version:   $Revision: 1.1.2.3 $
Authors:   Stefano Perticoni 
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include <string>
#include <iostream>
#include <OpenSim/Common/IO.h>
#include <OpenSim/Common/LoadOpenSimLibrary.h>
#include <OpenSim/Simulation/Model/Model.h>
#include <OpenSim/Simulation/Model/BodySet.h>
#include <OpenSim/Tools/AnalyzeTool.h>
#include <ctime>  // clock(), clock_t, CLOCKS_PER_SEC

using namespace OpenSim;
using namespace std;

#include "lhpBuilderDecl.h"

//----------------------------------------------------------------------------
// NOTE: Every CPP openSimFileName in the MAF must include "mafDefines.h" as first.
// This force to include Window, wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpModifyOpenSimModelRunStaticOptimizationAPI.h"

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
#include "wx/busyinfo.h"
#include "lhpOpImporterOpenSimIDSimulationResults.h"
using namespace xercesc;

#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelRunStaticOptimizationAPI);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateJoint_ID
{
	ID_CHOOSE_IK_TASK_SET_FILE = lhpOpModifyOpenSimModel::ID_LAST,
	ID_BUILD_MODEL,
	ID_CHOOSE_EXTERNAL_LOADS_FILE,
	ID_CHOOSE_COORDINATES_FILE,
	ID_EDIT_SIMULATION_SETUP_FILE,
	ID_IMPORT_RESULTS_INTO_MSF,
	ID_RUN_SIMULATION,
};

const string SimulationSetupLocalFileName = "setupStaticOptimization.xml";

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelRunStaticOptimizationAPI::lhpOpModifyOpenSimModelRunStaticOptimizationAPI(const wxString &label) :
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

	m_ExternalLoadsFile = "";
	m_CoordinatesFile = "";
	
	m_BuildModel = false;
	m_ImportResultsIntoMSF = false;
}

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelRunStaticOptimizationAPI::~lhpOpModifyOpenSimModelRunStaticOptimizationAPI( ) 
	//----------------------------------------------------------------------------
{
	assert(true);
}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelRunStaticOptimizationAPI::Copy()   
	//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelRunStaticOptimizationAPI(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelRunStaticOptimizationAPI::Accept(mafNode *node)
	//----------------------------------------------------------------------------
{
	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelRunStaticOptimizationAPI::OpRun()   
	//----------------------------------------------------------------------------
{	
	CreateGui();
	ShowGui();
}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelRunStaticOptimizationAPI::CreateGui()
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
	m_Gui->Button(ID_CHOOSE_EXTERNAL_LOADS_FILE, "External Loads File");
	m_Gui->Enable(ID_CHOOSE_EXTERNAL_LOADS_FILE, true);

	m_Gui->Label("");
	m_Gui->Button(ID_CHOOSE_COORDINATES_FILE, "Coordinates File");
	m_Gui->Enable(ID_CHOOSE_COORDINATES_FILE, false);

	m_Gui->Label("");
	m_Gui->Button(ID_EDIT_SIMULATION_SETUP_FILE, "Edit SO Setup File");
	m_Gui->Enable(ID_EDIT_SIMULATION_SETUP_FILE, false);

//	m_Gui->Label("");
//	m_Gui->Label("Import Results into MSF");
//	m_Gui->Bool(ID_IMPORT_RESULTS_INTO_MSF, "", &m_ImportResultsIntoMSF);
//	m_Gui->Enable(ID_BUILD_MODEL, true);

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



void lhpOpModifyOpenSimModelRunStaticOptimizationAPI::OnEvent(mafEventBase *maf_event)
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

	case ID_CHOOSE_EXTERNAL_LOADS_FILE:
		{
			OnChooseExternalLoadsData();
		}
		break;


	case ID_CHOOSE_COORDINATES_FILE:
		{
			OnChooseMotionFile();
		}
		break;


	case ID_EDIT_SIMULATION_SETUP_FILE:
		{
			OnEditSetupFile();
		}
		break;

	case ID_RUN_SIMULATION:
		{
			OnRunStaticOptimization();
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

void lhpOpModifyOpenSimModelRunStaticOptimizationAPI::OnChooseExternalLoadsData()
{
	wxString defaultDir = "";

	mafString wildc = "External Loads file (*.xml)|*.xml";
	mafString fileName;

	fileName = mafGetOpenFile(defaultDir, wildc, _("Choose External Loads file")).c_str();

	if(!fileName.IsEmpty())
	{
		m_ExternalLoadsFile = fileName;
		assert(wxFileExists(m_ExternalLoadsFile));

		wxString path, externalLoadsName, externalLoadsExtension;
		wxSplitPath(m_ExternalLoadsFile.c_str(),&path,&externalLoadsName,&externalLoadsExtension);

		wxString targetFile = GetOutputModelDirectory() + externalLoadsName + "." + externalLoadsExtension;
		
		if (m_ExternalLoadsFile != targetFile)
		{
			wxCopyFile(m_ExternalLoadsFile , targetFile);
		}

		m_Gui->Enable(ID_CHOOSE_COORDINATES_FILE, true);
	}	
}

void lhpOpModifyOpenSimModelRunStaticOptimizationAPI::OnChooseMotionFile()
{
	wxString defaultDir = "";

	mafString wildc = "Coordinates file (*.mot)|*.mot";
	mafString fileName;

	fileName = mafGetOpenFile(defaultDir, wildc, _("Choose Coordinates file")).c_str();

	if(!fileName.IsEmpty())
	{
		m_CoordinatesFile = fileName;
		assert(wxFileExists(m_CoordinatesFile));

		wxString path, name, ext;
		wxSplitPath(m_CoordinatesFile.c_str(),&path,&name,&ext);

		wxString targetFile = GetOutputModelDirectory() + name + "." + ext;

		if (m_CoordinatesFile != targetFile)
		{
			wxCopyFile(m_CoordinatesFile , targetFile);
		}

		m_Gui->Enable(ID_EDIT_SIMULATION_SETUP_FILE, true);
		m_Gui->Enable(ID_RUN_SIMULATION, true);

		CreateSetupFile();

	}
}


void lhpOpModifyOpenSimModelRunStaticOptimizationAPI::OnRunStaticOptimization()
{

	mafEvent event;
	event.SetSender(this);
	event.SetId(ID_REQUEST_OPENSIM_DIR);
	mafEventMacro(event);

	if(event.GetString())
	{
		m_OpenSimDir.Erase(0);
		m_OpenSimDir = event.GetString()->GetCStr();
	}

	bool openSimDirExists = wxDirExists(m_OpenSimDir.GetCStr());
	if (!openSimDirExists)
	{
		std::ostringstream stringStream;
		stringStream  << "OpenSim dir " << m_OpenSimDir.GetCStr() << " not found. Please check your Tools -> Options -> OpenSim Settings " << std::endl;
		wxMessageBox(stringStream.str().c_str());
		mafLogMessage(stringStream.str().c_str());
		return;
	}

	wxString oldDir = wxGetCwd();
	wxSetWorkingDirectory(GetOutputModelDirectory());

	wxBusyInfo *waitUpdateModel = NULL;
	
	//------------------------------------------

	if (m_BuildModel == true)
	{
		wxString outputModelABSFileName = GetOutputModelDirectory() + GetOutputModelFileName();
		wxRemoveFile(outputModelABSFileName);
		assert(wxFileExists(outputModelABSFileName) == false);

		waitUpdateModel = new wxBusyInfo("Updating OpenSim model and running ID simulation. Please wait...");
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
		waitUpdateModel = new wxBusyInfo("Running static optimization. Please wait...");
		mafSleep(2000);
	}

	//------------------------------------------

	wxString logFileName = "out.log";
	wxString logABSFileName = GetOutputModelDirectory() + logFileName;

	wxRemoveFile(logABSFileName);
	assert(wxFileExists(logABSFileName) == false);

	wxString errFileName = "err.log";
	wxString errABSFileName = GetOutputModelDirectory() + errFileName;

	wxRemoveFile(errABSFileName);
	assert(wxFileExists(errABSFileName) == false);

	wxString outputStaticOptimizationControlsAbsFileName = GetOutputModelDirectory() + "Output_StaticOptimization_controls.xml";
	wxRemoveFile(outputStaticOptimizationControlsAbsFileName);
	assert(wxFileExists(outputStaticOptimizationControlsAbsFileName) == false);

	wxString outputStaticOptimizationForceAbsFileName = GetOutputModelDirectory() + "Output_StaticOptimization_force.sto";
	wxRemoveFile(outputStaticOptimizationForceAbsFileName);
	assert(wxFileExists(outputStaticOptimizationForceAbsFileName) == false);

	wxString outputStaticOptimizationActivationAbsFileName = GetOutputModelDirectory() + "Output_StaticOptimization_activation.sto";
	wxRemoveFile(outputStaticOptimizationActivationAbsFileName);
	assert(wxFileExists(outputStaticOptimizationActivationAbsFileName) == false);

	wxString simulationSetupFile = m_SetupFile;

	wxString setupFile = m_SetupFile;

	RunStaticOptimization(setupFile, logABSFileName, errABSFileName, outputStaticOptimizationControlsAbsFileName, outputStaticOptimizationForceAbsFileName, outputStaticOptimizationActivationAbsFileName);

	//----------------------------
	// return 0;

	////////////////////////////////////////////////////////

	if (wxFileExists(outputStaticOptimizationControlsAbsFileName) == false)
	{
		// Study how to print the log file with the API
		OpenFileWithNotepadPP(logABSFileName);
		OpenFileWithNotepadPP(errABSFileName);
	}
	else
	{
		// Study how to print the log file with the API
		OpenFileWithNotepadPP(logABSFileName);
		OpenFileWithNotepadPP(outputStaticOptimizationControlsAbsFileName);
		OpenFileWithNotepadPP(outputStaticOptimizationForceAbsFileName);
		OpenFileWithNotepadPP(outputStaticOptimizationActivationAbsFileName);
	}

	if (m_ImportResultsIntoMSF == true)
	{
		// import results into the msf
		lhpOpImporterOpenSimIDSimulationResults *importer=new lhpOpImporterOpenSimIDSimulationResults("importer");
		importer->SetInput(m_Input);
		importer->SetListener(this);
		//importer->TestModeOn();
		importer->SetFileName(outputStaticOptimizationControlsAbsFileName);
		importer->Import();
		mafDEL(importer);
	}
	
	wxSetWorkingDirectory(oldDir);
	
	cppDEL(waitUpdateModel);

}

void lhpOpModifyOpenSimModelRunStaticOptimizationAPI::WriteNMSBuilderDataToFile()
{
	assert(!m_CoordinatesFile.IsEmpty());
	assert(!m_ExternalLoadsFile.IsEmpty());

	// CreateIKSimulationSetupFile();
}


void lhpOpModifyOpenSimModelRunStaticOptimizationAPI::CreateSetupFile()
{
	
	//-------------------------------------------------------------------------------------------------------------
	wxString outpuModelABSFileName = GetOutputModelDirectory() + GetOutputModelFileName();
	assert(wxFileExists(outpuModelABSFileName));

	assert(m_CoordinatesFile);
	assert(m_ExternalLoadsFile);

	assert(wxFileExists(m_CoordinatesFile));
	assert(wxFileExists(m_ExternalLoadsFile));

	wxString pathMarkerSetFileABSPath, nameMarkerSetFileABSPath, extMarkerSetFileABSPath;
	wxSplitPath(m_CoordinatesFile.c_str(), &pathMarkerSetFileABSPath, &nameMarkerSetFileABSPath, &extMarkerSetFileABSPath);

	wxString pathIKTaskSetFileABSPath, nameIKTaskSetFileABSPath, extIKTaskSetFileABSPath;
	wxSplitPath(m_ExternalLoadsFile.c_str(), &pathIKTaskSetFileABSPath, &nameIKTaskSetFileABSPath, &extIKTaskSetFileABSPath);

	wxString currentDir = wxGetCwd();
	mafLogMessage(currentDir);

	//-------------------------------------------------

	ostringstream stringStream;

	stringStream << 

	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"   << std::endl << 
	"	<AnalyzeTool name=\"Output\">"   << std::endl << 
	"	<!--Name of the .osim file used to construct a model.--> "   << std::endl << 
	"	<model_file> outputModel.osim  </model_file> "   << std::endl << 
	"	<!--Replace the model's actuator set with sets specified in <force_set_files>? If false, the actuator set is appended.--> "   << std::endl << 
	"	<replace_force_set> false </replace_force_set> "   << std::endl << 
	"	<!--List of xml files used to construct an actuator set.--> "   << std::endl << 
	"	<force_set_files> </force_set_files> "   << std::endl << 
	"	<!--Directory used for writing results.--> "   << std::endl << 
	"	<results_directory>" << GetOutputModelDirectory().c_str() << "</results_directory> "   << std::endl << 
	"	<!--Output precision. It is 8 by default.--> "   << std::endl << 
	"	<output_precision> 20 </output_precision> "   << std::endl << 
	"	<!--Initial time for the simulation.--> "   << std::endl << 
	"	<initial_time> 0.4 </initial_time> "   << std::endl << 
	"	<!--Final time for the simulation.--> "   << std::endl << 
	"	<final_time> 1.6 </final_time> "   << std::endl << 
	"	<!--Set of analyses to be run during the investigation.--> "   << std::endl << 
	"	<AnalysisSet name=\"Analyses\"> "   << std::endl << 
	"	<objects> "   << std::endl << 
	"	<StaticOptimization name=\"StaticOptimization\"> "   << std::endl << 
	"	<on> true </on> "   << std::endl << 
	"	<step_interval> 1 </step_interval> "   << std::endl << 
	"	<use_model_force_set> false </use_model_force_set> "   << std::endl << 
	"	</StaticOptimization> "   << std::endl << 
	"	</objects> "   << std::endl << 
	"	</AnalysisSet> "   << std::endl << 
	"	<!--Motion file (.mot) containing the generalized coordinates for the model.--> "   << std::endl << 
	"   <coordinates_file> " << m_CoordinatesFile << " </coordinates_file>"  << std::endl << 

	"	<!--Low-pass cut-off frequency for filtering the model generalized coordinates. A negative value results in no filtering. The "   << std::endl << 
	"	default value is 1.0, so no filtering.--> "   << std::endl << 
	"	<lowpass_cutoff_frequency_for_coordinates> 6 </lowpass_cutoff_frequency_for_coordinates> "   << std::endl << 
	"	<!--XML file (.xml) containing the forces applied to the model as ExternalLoads --> "   << std::endl << 
	"	<external_loads_file>"  << m_ExternalLoadsFile << "</external_loads_file>"  << std::endl << 
	"	</AnalyzeTool> "   << std::endl; 

	//-------------------------------------------------

	ofstream myFile;

	wxString localFileName = SimulationSetupLocalFileName.c_str();
	m_SetupFile = GetOutputModelDirectory() + localFileName;

	myFile.open(m_SetupFile);
	myFile << stringStream.str().c_str();
	myFile.close();

	assert(wxFileExists(m_SetupFile));

}

void lhpOpModifyOpenSimModelRunStaticOptimizationAPI::OnEditSetupFile()
{
	OpenFileWithNotepadPP(m_SetupFile);
}

void lhpOpModifyOpenSimModelRunStaticOptimizationAPI::RunStaticOptimization( wxString setupFile, wxString logABSFileName, wxString errABSFileName, wxString outputStaticOptimizationControlsAbsFileName, wxString outputStaticOptimizationForceAbsFileName, wxString outputStaticOptimizationActivationAbsFileName )
{
	//----------------------
	// Surrounding try block
	//----------------------
	try {

		//// Load libraries first
		LoadOpenSimLibrary("osimAnalyses");

		//ISSUES:
		//Load dlls that register Built in Actuator classes.
		LoadOpenSimLibrary("osimActuators");

		// CONSTRUCT
		cout<<"Constructing tool from setup file "<< setupFile <<".\n\n";
		AnalyzeTool analyze(setupFile.c_str());

		// PRINT MODEL INFORMATION
		Model& model = analyze.getModel();
		cout<<"-----------------------------------------------------------------------\n";
		cout<<"Loaded library\n";
		cout<<"-----------------------------------------------------------------------\n";
		cout<<"-----------------------------------------------------------------------\n\n";

		// start timing
		std::clock_t startTime = std::clock();

		// Redirect cout.
		streambuf* oldCoutStreamBuf = cout.rdbuf();
		ostringstream strCout;
		cout.rdbuf( strCout.rdbuf() );

		streambuf* oldCerrStreamBuf = cerr.rdbuf();
		ostringstream strCerr;
		cerr.rdbuf( strCerr.rdbuf() );

		// RUN
		analyze.run();

		// Restore old cout.
		cout.rdbuf( oldCoutStreamBuf );

		// Restore old cerr.
		cerr.rdbuf( oldCerrStreamBuf );

		ofstream out;
		out.open (logABSFileName.c_str());
		out << strCout.str().c_str();
		out.close();

		ofstream err;
		err.open (errABSFileName.c_str());
		err << strCerr.str().c_str();
		err.close();

		std::cout << "Analyze compute time = " << 1.e3*(std::clock()-startTime)/CLOCKS_PER_SEC << "ms\n" << endl;

		std::ostringstream stringStream2;
		stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
		stringStream2 << "ID compute time = " << 1.e3*(std::clock()-startTime)/CLOCKS_PER_SEC << std::endl;          
		stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
		stringStream2 << "Output controls file: " << outputStaticOptimizationControlsAbsFileName.c_str() << std::endl;
		stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
		stringStream2 << "Output force file: " << outputStaticOptimizationForceAbsFileName.c_str() << std::endl;
		stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
		stringStream2 << "Output activation file: " << outputStaticOptimizationActivationAbsFileName.c_str() << std::endl;
		stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
		stringStream2 << "Simulation Log file: " << logABSFileName.c_str() << std::endl;
		stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
		stringStream2 << "Simulation Error file: " << errABSFileName.c_str() << std::endl;
		stringStream2 << "//**************************************************************************************" << std::endl;
		stringStream2 << "Static Optimization simulation ended" << std::endl;          
		stringStream2 << "//**************************************************************************************" << std::endl;
		mafLogMessage(stringStream2.str().c_str());

		mafSleep(2000);
		//----------------------------
		// Catch any thrown exceptions
		//----------------------------
	} catch(const std::exception& x) {
		cout << "Exception in analyze: " << x.what() << endl;
		// return -1;
	}


}

