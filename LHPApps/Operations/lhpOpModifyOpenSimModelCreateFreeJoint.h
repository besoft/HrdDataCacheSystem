/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelCreateFreeJoint.h,v $
  Language:  C++
  Date:      $Date: 2012-03-12 14:58:46 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelCreateFreeJoint_H__
#define __lhpOpModifyOpenSimModelCreateFreeJoint_H__

using namespace std;

#include "lhpOpModifyOpenSimModel.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafOp;
class mafEvent;
class mafVME;
class mafVMELandmark;

//----------------------------------------------------------------------------
// lhpOpModifyOpenSimModelCreateFreeJoint :
//----------------------------------------------------------------------------
/** 

WORK IN PROGRESS

Create OpenSim Free Joint between selected body and ground

User Guide:
https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit#heading=h.t9i50amlajqp

*/
class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelCreateFreeJoint: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelCreateFreeJoint(const wxString &label = "lhpOpModifyOpenSimModelCreateJoint");
 ~lhpOpModifyOpenSimModelCreateFreeJoint();

 mafTypeMacro(lhpOpModifyOpenSimModelCreateFreeJoint, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 /** Set child surface vme*/
 void SetChildBodyVME(mafVME *surface);

 protected:

	void CreateGui();

	void OnEvent(mafEventBase *maf_event);

	/** choose the articular centre */
	void OnChooseArticularCentreVME();
	
	/** choose the rotation axis origin */
	void OnChooseRotationAxisOriginVME();
	
	/** choose the rotation axis end point */
	void OnChooseRotationAxisEndVME();

	/** choose the parent vme for joint generation */
	void OnChooseParentVMESurface();

	/** choose the child vme for joint generation */
	void OnChooseChildVMESurface();

	/** accept a vme landmark */
	static bool VmeSurfaceAcceptLandmark(mafNode *node);

	/** accept a vme surface that is valid for body generation (contains inertial parameters)*/
	static bool VmeSurfaceOpenSimBodyAccept(mafNode *node);
	
	void OnGenerateJoint();

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();
	/** Id of the type of joint to be created */
	int m_JointId;

	/** Check to use the ground as parent instead of choosing a vme */
	int m_UseGroundAsParent;

	/** Articular centre vme landmark */
	mafVMELandmark *m_ArticularCentreVME;

	/** Rotation axis origin landmark */
	mafVMELandmark *m_RotationAxisOriginVME;

	/** Rotation axis end landmark */
	mafVMELandmark *m_RotationAxisEndVME;

	/** Vme parent for body generation */
	mafVME *m_ParentBodyVME;

	/** Vme parent for body generation */
	mafVME *m_ChildBodyVME;

	/** Rotation axes direction cosines */
	double m_RotationAxisDirectionCosines[3];

	void ComputeRotationAxisDirectionCosines();
	
	/** Rotation axes direction cosines */
	wxString m_ComponentToConfigureJointBetween2BodiesFileName;

	/** Rotation axes direction cosines */
	wxString m_ComponentToConfigureJointBetweenGroundAndBodyFileName;

	/** manual gui test stuff */
	void InitGUITestStuff();

	enum JOINT_TEST_CASE {BODY_WITH_GROUND };
	int m_JointTestCase;
};
#endif
