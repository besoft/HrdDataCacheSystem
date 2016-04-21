/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelRunIDSimulation.cpp,v $
Language:  C++
Date:      $Date: 2012-04-20 10:58:02 $
Version:   $Revision: 1.1.2.3 $
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

#include "lhpOpModifyOpenSimModelRunIDSimulation.h"

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
using namespace std;

#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelRunIDSimulation);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateJoint_ID
{
	ID_CHOOSE_IK_TASK_SET_FILE = lhpOpModifyOpenSimModel::ID_LAST,
	ID_BUILD_MODEL,
	ID_CHOOSE_EXTERNAL_LOADS_FILE,
	ID_CHOOSE_COORDINATES_FILE,
	ID_EDIT_SIMULATION_SETUP_FILE,
	ID_RUN_SIMULATION,
};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelRunIDSimulation::lhpOpModifyOpenSimModelRunIDSimulation(const wxString &label) :
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
	m_SolverFileABSFileName = "c:\\OpenSim2.2.1\\bin\\analyze.exe";
	m_SolverFileRelativeFileName = "\\bin\\analyze.exe";
	m_SimulationSetupFile = "setupInverseDynamics.xml";
}

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelRunIDSimulation::~lhpOpModifyOpenSimModelRunIDSimulation( ) 
	//----------------------------------------------------------------------------
{
	assert(true);
}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelRunIDSimulation::Copy()   
	//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelRunIDSimulation(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelRunIDSimulation::Accept(mafNode *node)
	//----------------------------------------------------------------------------
{
	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelRunIDSimulation::OpRun()   
	//----------------------------------------------------------------------------
{	

	CreateGui();
	ShowGui();

}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelRunIDSimulation::CreateGui()
	//----------------------------------------------------------------------------
{
	m_Gui = new mafGUI(this);
	m_Gui->SetListener(this);

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



void lhpOpModifyOpenSimModelRunIDSimulation::OnEvent(mafEventBase *maf_event)
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch(e->GetId())
		{

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

void lhpOpModifyOpenSimModelRunIDSimulation::OnChooseExternalLoadsFile()
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

void lhpOpModifyOpenSimModelRunIDSimulation::OnChooseCoordinatesFile()
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


void lhpOpModifyOpenSimModelRunIDSimulation::OnRunSimulation()
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

	wxString outputModelABSFileName = GetOutputModelDirectory() + GetOutputModelFileName();
	wxRemoveFile(outputModelABSFileName);
	assert(wxFileExists(outputModelABSFileName) == false);

	m_SolverFileABSFileName = m_OpenSimDir + m_SolverFileRelativeFileName;
	assert(wxFileExists(m_SolverFileABSFileName));

	wxBusyInfo waitUpdateModel("Updating OpenSim model. Please wait...");
	mafSleep(2000);

	BuildOpenSIMFile();

	if (wxFileExists(outputModelABSFileName) == false)
	{
		std::string msg = std::string("Model generation for simulation failed, please check your model");
		wxMessageBox(msg.c_str(),_(""),wxOK|wxICON_ERROR);
		return;
	}
	
	//-----------------------------------

	wxString simulationDOSFile = GetOutputModelDirectory() + "runIDSimulation.bat";

	wxRemoveFile(simulationDOSFile);

	assert(wxFileExists(simulationDOSFile) == false);

	std::ofstream simBatchFile(simulationDOSFile.c_str(), std::ios::out);

	if (simBatchFile.fail())
	{
		wxMessageBox("Error creating file");
		return;
	}

	wxString LogFileName = "InverseDynamicsSimulationOutputLog.txt";
	wxString LogABSFileName = GetOutputModelDirectory() + LogFileName;

	wxRemoveFile(LogABSFileName);
	assert(wxFileExists(LogABSFileName) == false);

	simBatchFile << m_SolverFileABSFileName.c_str() << " -S " << m_SimulationSetupFile.c_str() << " >> " << LogFileName.c_str() << std::endl;
	// simBatchFile << "PAUSE" << std::endl;
	simBatchFile.close();

	assert(wxFileExists(simulationDOSFile.c_str()));

	//-----------------------------------

	wxBusyInfo waitRunIDSimulation("Run ID simulation. Please wait...");

	wxString command2execute;
	command2execute = simulationDOSFile;

	wxArrayString output;
	wxArrayString errors;
	long pid = -1;

	pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC);	

	mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

	wxString outputIDFile = GetOutputModelDirectory() + "InverseDynamics_Analysis1.sto";

	if (wxFileExists(outputIDFile) == false)
	{
		OpenFileWithNotepadPP(GetOutputModelDirectory() + LogFileName);
	}
	else
	{
		OpenFileWithNotepadPP(GetOutputModelDirectory() + LogFileName);
		OpenFileWithNotepadPP(outputIDFile);
	}
	
	// import results into the msf
	lhpOpImporterOpenSimIDSimulationResults *importer=new lhpOpImporterOpenSimIDSimulationResults("importer");
	importer->SetInput(m_Input);
	importer->SetListener(this);
	//importer->TestModeOn();
	importer->SetFileName(outputIDFile);
	importer->Import();
	mafDEL(importer);

	wxSetWorkingDirectory(oldDir);
	

}

void lhpOpModifyOpenSimModelRunIDSimulation::WriteNMSBuilderDataToFile()
{
	assert(!m_CoordinatesFile.IsEmpty());
	assert(!m_ExternalLoadsFile.IsEmpty());

	// CreateIKSimulationSetupFile();
}


void lhpOpModifyOpenSimModelRunIDSimulation::CreateSimulationSetupFile()
{

	assert(m_CoordinatesFile);
	assert(m_ExternalLoadsFile);

	assert(wxFileExists(m_CoordinatesFile));
	assert(wxFileExists(m_ExternalLoadsFile));

	wxString pathMarkerSetFileABSPath, nameMarkerSetFileABSPath, extMarkerSetFileABSPath;
	wxSplitPath(m_CoordinatesFile.c_str(), &pathMarkerSetFileABSPath, &nameMarkerSetFileABSPath, &extMarkerSetFileABSPath);

	wxString pathIKTaskSetFileABSPath, nameIKTaskSetFileABSPath, extIKTaskSetFileABSPath;
	wxSplitPath(m_ExternalLoadsFile.c_str(), &pathIKTaskSetFileABSPath, &nameIKTaskSetFileABSPath, &extIKTaskSetFileABSPath);

	ostringstream stringStream;

	stringStream << 

#ifdef USE_OPENSIM_API

		//-----------------------------------------
		// OpenSim 3 setup file version
		//-----------------------------------------

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

#else

		//-----------------------------------------
		// OpenSim 2.2.1 and 2.4 setup file version
		//-----------------------------------------

		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl <<

		"<AnalyzeTool name=\"InverseDynamics\">" << std::endl <<

		"<!--Name of the .osim file used to construct a model.-->" << std::endl <<

		"<model_file> outputModel.osim </model_file>" << std::endl <<

		"<!--Directory used for writing results.-->" << std::endl <<
		
		"<results_directory>" << GetOutputModelDirectory().c_str() << " </results_directory>" << std::endl <<

		"<!--Output precision.  It is 20 by default.-->" << std::endl <<
		
		"<output_precision> 20 </output_precision>" << std::endl <<
		
		"<!--Initial time for the simulation.-->" << std::endl <<
		
		"<initial_time> 0.4 </initial_time>" << std::endl <<
		
		"<!--Final time for the simulation.-->" << std::endl <<
		
		"<final_time> 1.175 </final_time>" << std::endl <<
		
		"<!--Set of analyses to be run during the investigation.-->" << std::endl <<
		
		"<external_loads_file> " << m_ExternalLoadsFile << " </external_loads_file>" << std::endl <<

		"<AnalysisSet name=\"Analyses\">" << std::endl <<
		"	<objects>" << std::endl <<
		"		<InverseDynamics name=\"Analysis1\">" << std::endl <<
		"			<!--Flag (true or false) specifying whether whether on. True by default.-->" << std::endl <<
		"			<on> true </on>" << std::endl <<
		"			<!--Specifies how often to store results during a simulation. More" << std::endl <<
		"				specifically, the interval (a positive integer) specifies how many" << std::endl <<
		"				successful integration steps should be taken before results are" << std::endl <<
		"				recorded again.-->" << std::endl <<
		"			<step_interval> 1 </step_interval>" << std::endl <<
		"			<!--Flag (true or false) indicating whether the results are in degrees or" << std::endl <<
		"				not.-->" << std::endl <<
		"			<in_degrees> false </in_degrees>" << std::endl <<
		"			<!--If true, the model's own actuator set will be used in the inverse" << std::endl <<
		"				dynamics computation.  Otherwise, inverse dynamics generalized forces" << std::endl <<
		"				will be computed for all unconstrained degrees of freedom.-->" << std::endl <<
		"			<use_model_force_set> false </use_model_force_set>" << std::endl <<
		"		</InverseDynamics>" << std::endl <<
		"	</objects>" << std::endl <<
		"	<groups/>" << std::endl <<
		"</AnalysisSet>" << std::endl <<

		"<!--Motion file (.mot) or storage file (.sto) containing the time history" << std::endl <<
		"	of the generalized coordinates for the model. These can be specified" << std::endl <<
		"	in place of the states file.-->" << std::endl <<
		"<coordinates_file> " << m_CoordinatesFile << " </coordinates_file>" << std::endl <<
		"<!--Low-pass cut-off frequency for filtering the coordinates_file data" << std::endl <<
		"	(currently does not apply to states_file or speeds_file). A negative" << std::endl <<
		"	value results in no filtering. The default value is -1.0, so no" << std::endl <<
		"	filtering.-->" << std::endl <<
		"<lowpass_cutoff_frequency_for_coordinates> -1.0 </lowpass_cutoff_frequency_for_coordinates>" << std::endl <<
		"</AnalyzeTool>" << std::endl;

	//-------------------------------------------------
	
#endif

	ofstream myFile;

	wxString idSimulationSetupFileABSPath = GetOutputModelDirectory() + m_SimulationSetupFile.c_str();

	myFile.open(idSimulationSetupFileABSPath);
	myFile << stringStream.str().c_str();
	myFile.close();

	assert(wxFileExists(idSimulationSetupFileABSPath));

}

void lhpOpModifyOpenSimModelRunIDSimulation::OnEditSimulationSetupFile()
{
	OpenFileWithNotepadPP(GetOutputModelDirectory() + m_SimulationSetupFile);
}
