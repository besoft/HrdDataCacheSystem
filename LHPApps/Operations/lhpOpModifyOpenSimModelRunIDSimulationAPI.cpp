/*=========================================================================
Program:   Multimod Applicatio Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelRunIDSimulationAPI.cpp,v $
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
#include <OpenSim/Tools/InverseDynamicsTool.h>

#include "lhpBuilderDecl.h"

//----------------------------------------------------------------------------
// NOTE: Every CPP openSimFileName in the MAF must include "mafDefines.h" as first.
// This force to include Window, wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include <ctime>  // clock(), clock_t, CLOCKS_PER_SEC

using namespace OpenSim;
using namespace std;

#include "lhpOpModifyOpenSimModelRunIDSimulationAPI.h"

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
mafCxxTypeMacro(lhpOpModifyOpenSimModelRunIDSimulationAPI);
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

const string SimulationSetupLocalFileName = "setupInverseDynamics.xml";

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelRunIDSimulationAPI::lhpOpModifyOpenSimModelRunIDSimulationAPI(const wxString &label) :
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
lhpOpModifyOpenSimModelRunIDSimulationAPI::~lhpOpModifyOpenSimModelRunIDSimulationAPI( ) 
	//----------------------------------------------------------------------------
{
	assert(true);
}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelRunIDSimulationAPI::Copy()   
	//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelRunIDSimulationAPI(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelRunIDSimulationAPI::Accept(mafNode *node)
	//----------------------------------------------------------------------------
{
	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelRunIDSimulationAPI::OpRun()   
	//----------------------------------------------------------------------------
{	

	CreateGui();
	ShowGui();

}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelRunIDSimulationAPI::CreateGui()
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
	m_Gui->Button(ID_EDIT_SIMULATION_SETUP_FILE, "Edit ID Setup File");
	m_Gui->Enable(ID_EDIT_SIMULATION_SETUP_FILE, false);

	m_Gui->Label("");
	m_Gui->Label("Import Results into MSF");
	m_Gui->Bool(ID_IMPORT_RESULTS_INTO_MSF, "", &m_ImportResultsIntoMSF);
	m_Gui->Enable(ID_BUILD_MODEL, true);

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



void lhpOpModifyOpenSimModelRunIDSimulationAPI::OnEvent(mafEventBase *maf_event)
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
			OnChooseExternalLoadsFile();
		}
		break;


	case ID_CHOOSE_COORDINATES_FILE:
		{
			OnChooseCoordinatesFile();
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

void lhpOpModifyOpenSimModelRunIDSimulationAPI::OnChooseExternalLoadsFile()
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

void lhpOpModifyOpenSimModelRunIDSimulationAPI::OnChooseCoordinatesFile()
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

		CreateSimulationSetupFile();

	}
}


void lhpOpModifyOpenSimModelRunIDSimulationAPI::OnRunSimulation()
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
		waitUpdateModel = new wxBusyInfo("Running ID simulation. Please wait...");
		mafSleep(2000);
	}

	//------------------------------------------

	wxString idLogFileName = "out.log";
	wxString idLogABSFileName = GetOutputModelDirectory() + idLogFileName;

	wxRemoveFile(idLogABSFileName);
	assert(wxFileExists(idLogABSFileName) == false);

	wxString idErrFileName = "err.log";
	wxString idErrABSFileName = GetOutputModelDirectory() + idErrFileName;

	wxRemoveFile(idErrABSFileName);
	assert(wxFileExists(idErrABSFileName) == false);

	wxString outputIDAbsFileName = GetOutputModelDirectory() + "Analysis1.sto";
	wxRemoveFile(outputIDAbsFileName);
	assert(wxFileExists(outputIDAbsFileName) == false);

	wxString simulationSetupFile = m_SimulationSetupFile;

	//----------------------
	// Surrounding try block
	//----------------------
	try {
		
		RunSimulation(simulationSetupFile, idLogABSFileName, idErrABSFileName, outputIDAbsFileName);


		//----------------------------
		// Catch any thrown exceptions
		//----------------------------
	} catch(const std::exception& x) {

		cout << "Exception in ID: " << x.what() << endl;

	}

	if (wxFileExists(outputIDAbsFileName) == false)
	{
		// Study how to print the log file with the API
		OpenFileWithNotepadPP(idLogABSFileName);
		OpenFileWithNotepadPP(idErrABSFileName);
	}
	else
	{
		// Study how to print the log file with the API
		OpenFileWithNotepadPP(idLogABSFileName);
		OpenFileWithNotepadPP(outputIDAbsFileName);
	}

	if (m_ImportResultsIntoMSF == true)
	{
		// import results into the msf
		lhpOpImporterOpenSimIDSimulationResults *importer=new lhpOpImporterOpenSimIDSimulationResults("importer");
		importer->SetInput(m_Input);
		importer->SetListener(this);
		//importer->TestModeOn();
		bool idSimulationOutputExists = wxFileExists(outputIDAbsFileName.c_str());

		if (idSimulationOutputExists == false)
		{
			wxString message = "cannnot find: " + outputIDAbsFileName + " , exiting results importer";
			mafLogMessage(message);
		}

		importer->SetFileName(outputIDAbsFileName);
		importer->Import();
		mafDEL(importer);
	}
	
	wxSetWorkingDirectory(oldDir);
	
	cppDEL(waitUpdateModel);

}

void lhpOpModifyOpenSimModelRunIDSimulationAPI::WriteNMSBuilderDataToFile()
{
	assert(!m_CoordinatesFile.IsEmpty());
	assert(!m_ExternalLoadsFile.IsEmpty());

	// CreateIKSimulationSetupFile();
}


void lhpOpModifyOpenSimModelRunIDSimulationAPI::CreateSimulationSetupFile()
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

	ostringstream stringStream;

	stringStream << 

	//-------------------------------------------------
	
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"  << std::endl << 
	"	<OpenSimDocument Version=\"30000\">"  << std::endl << 
	"	    <InverseDynamicsTool name=\"subject01_walk1\">"  << std::endl << 
	"		    <!--Directory used for writing results.-->"  << std::endl << 
	"		    <results_directory>" << GetOutputModelDirectory().c_str() << "</results_directory>"  << std::endl << 
	"		    <!--Directory for input files-->"  << std::endl << 
	"		    <input_directory />"  << std::endl << 
	"		    <!--Name of the .osim file used to construct a model.-->"  << std::endl << 
	"		    <model_file> outputModel.osim </model_file>"  << std::endl << 
	"		    <!--Time range over which the inverse dynamics problem is solved.-->"  << std::endl << 
	"		    <time_range> 0.4 1.6</time_range>"  << std::endl << 
	"		    <!--List of forces by individual or grouping name (e.g. All, actuators, muscles, ...) to be excluded when computing model dynamics.-->"  << std::endl << 
	"		    <forces_to_exclude> Muscles</forces_to_exclude>"  << std::endl << 
	"		    <!--XML file (.xml) containing the external loads applied to the model as a set of ExternalForce(s).-->"  << std::endl << 
	"		    <external_loads_file>"  << m_ExternalLoadsFile << "</external_loads_file>"  << std::endl << 
	"		    <!--The name of the file containing coordinate data. Can be a motion (.mot) or a states (.sto) file.-->"  << std::endl << 
	"		    <coordinates_file> " << m_CoordinatesFile << " </coordinates_file>"  << std::endl << 
	"		    <!--Low-pass cut-off frequency for filtering the coordinates_file data (currently does not apply to states_file or speeds_file). A negative value results in no filtering. The default value is -1.0, so no filtering.-->"  << std::endl << 
	"		    <lowpass_cutoff_frequency_for_coordinates>6</lowpass_cutoff_frequency_for_coordinates>"  << std::endl << 
	"		    <!--Time range over which the inverse dynamics problem is solved.-->"  << std::endl << 
	"		    <time_range> 0.4 1.6</time_range>"  << std::endl << 
	"		    <!--Name of the storage file (.sto) to which the generalized forces are written.-->"  << std::endl << 
	"		    <output_gen_force_file> Analysis1 </output_gen_force_file>"  << std::endl << 
	"	    </InverseDynamicsTool>"  << std::endl << 
	"	</OpenSimDocument>"  << std::endl;


	//-------------------------------------------------


	ofstream myFile;

	wxString localFileName = SimulationSetupLocalFileName.c_str();
	m_SimulationSetupFile = GetOutputModelDirectory() + localFileName;

	myFile.open(m_SimulationSetupFile);
	myFile << stringStream.str().c_str();
	myFile.close();

	assert(wxFileExists(m_SimulationSetupFile));

}

void lhpOpModifyOpenSimModelRunIDSimulationAPI::OnEditSimulationSetupFile()
{
	OpenFileWithNotepadPP(m_SimulationSetupFile);
}

void lhpOpModifyOpenSimModelRunIDSimulationAPI::RunSimulation( wxString simulationSetupFile, wxString idLogABSFileName, wxString idErrABSFileName, wxString outputIDAbsFileName )
{
	//----------------------
	// CONSTRUCT
	cout<<"Constructing tool from setup file "<< simulationSetupFile <<".\n\n";
	InverseDynamicsTool id(simulationSetupFile.c_str());

	cout<<"-----------------------------------------------------------------------"<<endl;
	cout<<"Starting Inverse Dynamics\n";
	cout<<"-----------------------------------------------------------------------"<<endl;
	cout<<"-----------------------------------------------------------------------"<<endl<<endl;

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
	id.run();

	// Restore old cout.
	cout.rdbuf( oldCoutStreamBuf );

	// Restore old cerr.
	cerr.rdbuf( oldCerrStreamBuf );

	ofstream out;
	out.open (idLogABSFileName.c_str());
	out << strCout.str().c_str();
	out.close();

	ofstream err;
	err.open (idErrABSFileName.c_str());
	err << strCerr.str().c_str();
	err.close();

	std::cout << "Inverse dynamics compute time = " << 1.e3*(std::clock()-startTime)/CLOCKS_PER_SEC << "ms\n" << endl;

	std::ostringstream stringStream2;
	stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
	stringStream2 << "ID compute time = " << 1.e3*(std::clock()-startTime)/CLOCKS_PER_SEC << std::endl;          
	stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
	stringStream2 << "Output simulation file: " << outputIDAbsFileName.c_str() << std::endl;
	stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
	stringStream2 << "Simulation Log file: " << idLogABSFileName.c_str() << std::endl;
	stringStream2 << "//--------------------------------------------------------------------------------------" << std::endl;
	stringStream2 << "Simulation Error file: " << idErrABSFileName.c_str() << std::endl;
	stringStream2 << "//**************************************************************************************" << std::endl;
	stringStream2 << "IK simulation ended" << std::endl;          
	stringStream2 << "//**************************************************************************************" << std::endl;
	mafLogMessage(stringStream2.str().c_str());
}