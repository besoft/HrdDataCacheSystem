/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelRunIDSimulationAPI.h,v $
  Language:  C++
  Date:      $Date: 2012-04-19 13:26:30 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelRunIDSimulationAPI_H__
#define __lhpOpModifyOpenSimModelRunIDSimulationAPI_H__

using namespace std;

#include "lhpOpModifyOpenSimModel.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafOp;
class mafEvent;
class mafVME;

//----------------------------------------------------------------------------
// lhpOpModifyOpenSimModelRunIDSimulationAPI :
//----------------------------------------------------------------------------
/** 

Run OpenSim Inverse Dynamics simulation using the OpenSim API

*/

class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelRunIDSimulationAPI: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelRunIDSimulationAPI(const wxString &label = "lhpOpModifyOpenSimModelAssignMarkerSet");
 ~lhpOpModifyOpenSimModelRunIDSimulationAPI();

 mafTypeMacro(lhpOpModifyOpenSimModelRunIDSimulationAPI, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 void RunSimulation( wxString simulationSetupFile, wxString idLogABSFileName, wxString idErrABSFileName, wxString outputIDAbsFileName );

 protected:

	void CreateGui();

	void OnEvent(mafEventBase *maf_event);

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	/** Create the setup file for simulation */
	void CreateSimulationSetupFile();
	wxString m_SimulationSetupFile;

	void OnChooseExternalLoadsFile();
	wxString m_ExternalLoadsFile;

	void OnChooseCoordinatesFile();
	wxString m_CoordinatesFile;

	void OnEditSimulationSetupFile();

	void OnRunSimulation();

	int m_BuildModel;
	int m_ImportResultsIntoMSF;


};
#endif

