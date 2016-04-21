/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelRunStaticOptimizationAPI.h,v $
  Language:  C++
  Date:      $Date: 2012-04-19 13:26:30 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelRunStaticOptimizationAPI_H__
#define __lhpOpModifyOpenSimModelRunStaticOptimizationAPI_H__

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
// lhpOpModifyOpenSimModelRunStaticOptimizationAPI :
//----------------------------------------------------------------------------
/** 

Run OpenSim Static Optimization using the OpenSim API

*/

class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelRunStaticOptimizationAPI: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelRunStaticOptimizationAPI(const wxString &label = "lhpOpModifyOpenSimModelRunStaticOptimizationAPI");
 ~lhpOpModifyOpenSimModelRunStaticOptimizationAPI();

 mafTypeMacro(lhpOpModifyOpenSimModelRunStaticOptimizationAPI, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 /** Run static optimization simulation */
 void RunStaticOptimization( wxString setupFile, wxString logABSFileName, wxString errABSFileName, wxString outputStaticOptimizationControlsAbsFileName, wxString outputStaticOptimizationForceAbsFileName, wxString outputStaticOptimizationActivationAbsFileName );
 
protected:

	void CreateGui();

	void OnEvent(mafEventBase *maf_event);

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	/** Create the setup file for simulation */
	void CreateSetupFile();
	wxString m_SetupFile;

	void OnChooseExternalLoadsData();
	wxString m_ExternalLoadsFile;

	void OnChooseMotionFile();
	wxString m_CoordinatesFile;

	void OnEditSetupFile();

	void OnRunStaticOptimization();

	int m_BuildModel;
	int m_ImportResultsIntoMSF;


};
#endif

