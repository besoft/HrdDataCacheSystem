/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelRunIKSimulation.cpp,v $
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

#include "lhpOpModifyOpenSimModelRunIKSimulation.h"

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
#include "lhpOpImporterOpenSimIKSimulationResults.h"
using namespace xercesc;
using namespace std;

#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999



#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelRunIKSimulation);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateJoint_ID
{
	ID_CHOOSE_IK_TASK_SET_FILE = lhpOpModifyOpenSimModel::ID_LAST,
	ID_CHOOSE_MARKER_FILE,
	ID_EDIT_SIMULATION_SETUP_FILE,
	ID_RUN_SIMULATION,
};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelRunIKSimulation::lhpOpModifyOpenSimModelRunIKSimulation(const wxString &label) :
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
	m_IKSolverRelativePath = "\\bin\\ik.exe";
	m_IKSolverABSPath = "c:\\OpenSim2.2.1\\bin\\ik.exe";
	m_IKSimulationSetupFileName = "setupIK.xml";
}

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelRunIKSimulation::~lhpOpModifyOpenSimModelRunIKSimulation( ) 
	//----------------------------------------------------------------------------
{
	assert(true);
}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelRunIKSimulation::Copy()   
	//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelRunIKSimulation(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelRunIKSimulation::Accept(mafNode *node)
	//----------------------------------------------------------------------------
{
	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelRunIKSimulation::OpRun()   
	//----------------------------------------------------------------------------
{	

	CreateGui();
	ShowGui();

}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelRunIKSimulation::CreateGui()
	//----------------------------------------------------------------------------
{
	m_Gui = new mafGUI(this);
	m_Gui->SetListener(this);

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



void lhpOpModifyOpenSimModelRunIKSimulation::OnEvent(mafEventBase *maf_event)
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch(e->GetId())
		{

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

void lhpOpModifyOpenSimModelRunIKSimulation::OnChooseMarkerSetFile()
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

		if (m_MarkerSetFileABSPath != targetFile)
		{
			wxCopyFile(m_MarkerSetFileABSPath , targetFile);
		}

		m_Gui->Enable(ID_EDIT_SIMULATION_SETUP_FILE, true);
		m_Gui->Enable(ID_RUN_SIMULATION, true);

		CreateIKSimulationSetupFile();

	}
}

void lhpOpModifyOpenSimModelRunIKSimulation::OnChooseIKTaskSetFile()
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

		m_Gui->Enable(ID_CHOOSE_MARKER_FILE, true);
	}	
}

void lhpOpModifyOpenSimModelRunIKSimulation::OnRunSimulation()
{

	wxString oldDir = wxGetCwd();
	wxSetWorkingDirectory(GetOutputModelDirectory());

	wxString outputModelABSFileName = GetOutputModelDirectory() + GetOutputModelFileName();
	wxRemoveFile(outputModelABSFileName);
	assert(wxFileExists(outputModelABSFileName) == false);

	assert(wxFileExists(m_IKSolverABSPath));

	wxBusyInfo waitUpdateModel("Updating OpenSim model. Please wait...");

	BuildOpenSIMFile();

	wxBusyInfo runSimulation("Running IK simulation. Please wait...");

	if (wxFileExists(outputModelABSFileName) == false)
	{
		std::string msg = std::string("Model generation for simulation failed, please check your model");
		wxMessageBox(msg.c_str(),_(""),wxOK|wxICON_ERROR);
		return;
	}
	

	wxString outputIKFile = "output_ik.mot";
	
	wxString outputIKABSFileName = GetOutputModelDirectory() + outputIKFile;
	wxRemoveFile(outputIKABSFileName);
	assert(wxFileExists(outputIKABSFileName) == false);

	//-----------------------------------

	wxString simulationDOSFile = GetOutputModelDirectory() + "runIKSimulation.bat";

	wxRemoveFile(simulationDOSFile);

	assert(wxFileExists(simulationDOSFile) == false);

	std::ofstream simBatchFile(simulationDOSFile.c_str(), std::ios::out);

	if (simBatchFile.fail())
	{
		wxMessageBox("Error creating file");
		return;
	}

	wxString ikLogFileName = "IKSimulationOutputLog.txt";
	wxString ikLogABSFileName = GetOutputModelDirectory() + ikLogFileName;

	wxRemoveFile(ikLogABSFileName);
	assert(wxFileExists(ikLogABSFileName) == false);

    m_IKSolverABSPath = m_OpenSimDir + m_IKSolverRelativePath;
	assert(wxFileExists(m_IKSolverABSPath));

	simBatchFile << m_IKSolverABSPath.c_str() << " -S " << m_IKSimulationSetupFileName.c_str() << " >> " << ikLogFileName.c_str() << std::endl;
	// simBatchFile << "PAUSE" << std::endl;
	simBatchFile.close();

	assert(wxFileExists(simulationDOSFile.c_str()));

	//-----------------------------------

	wxString command2execute;
	command2execute = simulationDOSFile;

	wxArrayString output;
	wxArrayString errors;
	long pid = -1;

	pid = wxExecute(command2execute, output, errors, wxEXEC_SYNC);	

	mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

	if (wxFileExists(outputIKABSFileName) == false)
	{
		wxMessageBox("Error performing the simulation: check the simulation log", "Error.", wxOK | wxCENTRE | wxICON_ERROR);
		OpenFileWithNotepadPP(GetOutputModelDirectory() + ikLogFileName);
	}
	else
	{
		OpenFileWithNotepadPP(GetOutputModelDirectory() + ikLogFileName);
		OpenFileWithNotepadPP(outputIKABSFileName);

		// import results into the msf
		lhpOpImporterOpenSimIKSimulationResults *importer=new lhpOpImporterOpenSimIKSimulationResults("importer");
		importer->SetInput(m_Input);
		importer->SetListener(this);
		//importer->TestModeOn();
		importer->SetFileName(outputIKABSFileName);
		importer->Import();
		mafDEL(importer);
	}
	
	wxSetWorkingDirectory(oldDir);

}

void lhpOpModifyOpenSimModelRunIKSimulation::WriteNMSBuilderDataToFile()
{
	assert(!m_MarkerSetFileABSPath.IsEmpty());
	assert(!m_IKTaskSetFileABSPath.IsEmpty());

	// CreateIKSimulationSetupFile();
}


void lhpOpModifyOpenSimModelRunIKSimulation::CreateIKSimulationSetupFile()
{
	
	//-------------------------------------------------------------------------------------------------------------
	/*
	<?xml version="1.0" encoding="UTF-8"?>

		<IKTool name="subject01">

		<model_file> outputModel.osim </model_file>

		<IKTaskSet file="gait2354_IK_Tasks.xml"/>

			<IKTrialSet name="">

				<objects>

					<IKTrial name="first trial">
						<marker_file> subject01_walk1.trc </marker_file>
						<coordinate_file> subject01_walk1.mot </coordinate_file>

						<time_range> 0.4 2.0 </time_range>

						<output_motion_file> subject01_walk1_ik.mot </output_motion_file>

					</IKTrial>

				</objects>
		
			</IKTrialSet>

	</IKTool>
*/
	//-------------------------------------------------------------------------------------------------------------

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

#ifdef USE_OPENSIM_API

		//-----------------------------------------
		// OpenSim 3.0 setup file version
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

#else

    //-----------------------------------------
	// OpenSim 2.2.1 and 2.4.0 setup file version
	//-----------------------------------------

	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"  << std::endl <<

		"	<IKTool name=\"subject01\">"  << std::endl <<

		"	<model_file> outputModel.osim </model_file>"  << std::endl <<

		"	<IKTaskSet file=\"" << nameIKTaskSetFileABSPath.c_str() << "." << extIKTaskSetFileABSPath.c_str() << "\"/>"  << std::endl <<

		"		<IKTrialSet name=\"\">"  << std::endl <<

		"			<objects>" << std::endl <<

		"				<IKTrial name=\"first trial\">" << std::endl <<
		"					<marker_file> " <<  nameMarkerSetFileABSPath.c_str() << "." << extMarkerSetFileABSPath.c_str() << " </marker_file>" << std::endl <<

		"					<coordinate_file></coordinate_file>" << std::endl <<

		"					<time_range> 0.4 2.0 </time_range>" << std::endl <<

		"					<output_motion_file> output_ik.mot </output_motion_file>" << std::endl <<

		"				</IKTrial>" << std::endl <<

		"			</objects>" << std::endl <<

		"		</IKTrialSet>" << std::endl <<

		"   </IKTool>" << std::endl;

	//-----------------------------------------

#endif

	ofstream myFile;

	wxString ikSimulationSetupFileABSPath = GetOutputModelDirectory() + m_IKSimulationSetupFileName.c_str();

	myFile.open(ikSimulationSetupFileABSPath);
	myFile << stringStream.str().c_str();
	myFile.close();

	assert(wxFileExists(ikSimulationSetupFileABSPath));

}

void lhpOpModifyOpenSimModelRunIKSimulation::OnEditSimulationSetupFile()
{
	OpenFileWithNotepadPP(GetOutputModelDirectory() + m_IKSimulationSetupFileName);
}
