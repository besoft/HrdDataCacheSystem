/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelRunIKSimulation.h,v $
  Language:  C++
  Date:      $Date: 2012-04-19 13:26:30 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelRunIKSimulation_H__
#define __lhpOpModifyOpenSimModelRunIKSimulation_H__

using namespace std;

#include "lhpOpModifyOpenSimModel.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafOp;
class mafEvent;

//----------------------------------------------------------------------------
// lhpOpModifyOpenSimModelRunIKSimulation :
//----------------------------------------------------------------------------
/** 

Run OpenSim Inverse Kinematics simulation

*/

class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelRunIKSimulation: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelRunIKSimulation(const wxString &label = "lhpOpModifyOpenSimModelRunIKSimulation");
 ~lhpOpModifyOpenSimModelRunIKSimulation();

 mafTypeMacro(lhpOpModifyOpenSimModelRunIKSimulation, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 protected:

	void CreateGui();

	void OnEvent(mafEventBase *maf_event);

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	/** Create the setup file for IK simulation */
	void CreateIKSimulationSetupFile();

	void OnChooseIKTaskSetFile();
	void OnChooseMarkerSetFile();

	void OnRunSimulation();
	void OnEditSimulationSetupFile();
	wxString m_IKSolverABSPath;
	wxString m_IKTaskSetFileABSPath;
	wxString m_MarkerSetFileABSPath;

	wxString m_IKSimulationSetupFileName;

	wxString m_IKSolverRelativePath;
};
#endif
