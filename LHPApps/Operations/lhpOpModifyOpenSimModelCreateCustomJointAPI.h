/*=========================================================================

 Program: NMSBuilder
 Module: lhpOpModifyOpenSimModelCreateCustomJointAPI
 Authors: Stefano Perticoni
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelCreateCustomJointAPI_H__
#define __lhpOpModifyOpenSimModelCreateCustomJointAPI_H__

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

//----------------------------------------------------------------------------
// lhpOpModifyOpenSimModelCreateCustomJointAPI :
//----------------------------------------------------------------------------
/** 

WORK IN PROGRESS

Create OpenSim Custom Joint from msf tree

User Guide:
https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit#heading=h.t9i50amlajqp

*/

class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelCreateCustomJointAPI: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelCreateCustomJointAPI(const wxString &label = "lhpOpModifyOpenSimModelCreateCustomJointAPI");
 ~lhpOpModifyOpenSimModelCreateCustomJointAPI();

 mafTypeMacro(lhpOpModifyOpenSimModelCreateCustomJointAPI, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 protected:

	void CreateGui();
	void OnEvent(mafEventBase *maf_event);

	/** choose the parent vme for joint generation */
	void OnChooseParentBodyVME();

	/** choose the child vme for joint generation */
	void OnChooseChildBodyVME();

	/** accept a vme refsys */
	static bool VMERefSysOrVMESurfaceAccept(mafNode *node);

	/** accept a vme landmark */
	static bool IsVMELandmark(mafNode *node);

	/** accept a vme surface that is valid for body generation (contains inertial parameters)*/
	static bool IsVMESurfaceAndCanBeUsedAsOpenSimBody(mafNode *node);
	
	void OnGenerateJoint();

	/** Vme parent for body generation */
	mafVME *m_FemurBodyVME;

	/** Vme parent for body generation */
	mafVME *m_TibiaBodyVME;

	/** fill direction cosines ivar from rotation axis origin and endpoint (old implementation)*/
	void RotationAxisDirectionCosinesFromLandmarks();
	
	/** Use the ground as parent body*/
	int m_UseGroundAsParent;

	wxString m_ComponentToConfigureJointBetween2BodiesFileName;

	wxString m_ComponentToConfigureJointBetweenBodyAndGroundFileName;
	
	/** manual gui test stuff */
	void InitGUITestStuff();
	
	void OnChooseGroundAsParent();
	void OnChooseRefSysVme();
	
	void OnChooseRefSysVME();

	void Generate();

	/** Vme to be used as refsys */
	mafVME *m_RefSysVME;

	
};
#endif
