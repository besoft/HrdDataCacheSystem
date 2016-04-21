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

#ifndef __lhpOpModifyOpenSimModelRunIKSimulationAPI_H__
#define __lhpOpModifyOpenSimModelRunIKSimulationAPI_H__

using namespace std;

#include "lhpOpModifyOpenSimModel.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafOp;
class mafEvent;
class mafVMESurface;
class mafVMELandmark;
class mafVME;
class mafVMELandmarkCloud;

//----------------------------------------------------------------------------
// lhpOpModifyOpenSimModelRunIKSimulationAPI :
//----------------------------------------------------------------------------
/** 

Create an OpenSim marker set and assign it to a body

User Guide:
https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit#heading=h.t9i50amlajqp

*/

class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelRunIKSimulationAPI: public lhpOpModifyOpenSimModel
{
public:

	  lhpOpModifyOpenSimModelRunIKSimulationAPI(const wxString &label = "lhpOpModifyOpenSimModelAssignMarkerSet");
	 ~lhpOpModifyOpenSimModelRunIKSimulationAPI();

	 mafTypeMacro(lhpOpModifyOpenSimModelRunIKSimulationAPI, lhpOpModifyOpenSimModel);

	 mafOp* Copy();

	 /** Return true for the acceptable vme type. */
	 bool Accept(mafNode* node);

	 /** Builds operation's interface. */
	 void OpRun();

	 /** Run the IK simulation */
	 void RunSimulation( wxString ikSimulationSetupFileABSPath, wxString ikResultsDirectory, wxString ikLogABSFileName, wxString ikErrABSFileName, wxString outputIKSimulationABSFileName );

 protected:

	void CreateGui();

	void OnEvent(mafEventBase *maf_event);

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	/** Create the setup file for IK simulation */
	void CreateIKSimulationSetupFile();

	void OnChooseIKTaskSetFile();
	void OnChooseMarkerSetFile();

	void OnEditSimulationSetupFile();

	void OnRunSimulation();

	wxString m_IKSolverABSPath;	
	wxString m_IKTaskSetFileABSPath;
	wxString m_MarkerSetFileABSPath;

	wxString m_IKSimulationSetupFileName;

	int m_BuildModel;
	int m_ImportResultsIntoMSF;
};
#endif
