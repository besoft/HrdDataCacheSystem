/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelCreateBallJoint.h,v $
  Language:  C++
  Date:      $Date: 2012-03-26 09:08:21 $
  Version:   $Revision: 1.1.2.4 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelCreateBallJoint_H__
#define __lhpOpModifyOpenSimModelCreateBallJoint_H__

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
// lhpOpModifyOpenSimModelCreateBallJoint :
//----------------------------------------------------------------------------
/** 

WORK IN PROGRESS

Create OpenSim Ball Joint from msf tree

User Guide:
https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit#heading=h.t9i50amlajqp

*/

class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelCreateBallJoint: public lhpOpModifyOpenSimModel
	{
public:
  lhpOpModifyOpenSimModelCreateBallJoint(const wxString &label = "lhpOpModifyOpenSimModelCreateBallJoint");
 ~lhpOpModifyOpenSimModelCreateBallJoint();

 mafTypeMacro(lhpOpModifyOpenSimModelCreateBallJoint, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 void SetUseGroundAsParent( int useGroundAsParent );

 void SetParentBodyVME( mafVME *parent );

 void SetChildBodyVME( mafVME *child );

 void SetRefSysVMEInParent( mafVME *refSys );

 void SetRefSysVMEInChild( mafVME *refSys );

 protected:

	void CreateGui();
	void OnEvent(mafEventBase *maf_event);

	/** choose the parent vme for joint generation */
	void OnChooseParentBodyVMESurface();

	/** choose the child vme for joint generation */
	void OnChooseChildBodyVMESurface();

	/** accept a vme refsys */
	static bool VMERefSysOrVMESurfaceAccept(mafNode *node);

	/** accept a vme landmark */
	static bool IsVMELandmark(mafNode *node);

	/** accept a vme surface that is valid for body generation (contains inertial parameters)*/
	static bool IsVMESurfaceAndCanBeUsedAsOpenSimBody(mafNode *node);
	
	void OnGenerateJoint();

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	/** Vme parent for body generation */
	mafVME *m_ParentBodyVME;

	/** Vme child for body generation */
	mafVME *m_ChildBodyVME;

	/** fill direction cosines ivar from rotation axis origin and endpoint (old implementation)*/
	void RotationAxisDirectionCosinesFromLandmarks();
	
	/** Use the ground as parent body*/
	int m_UseGroundAsParent;

	wxString m_ComponentToConfigureJointBetween2BodiesFileName;

	wxString m_ComponentToConfigureJointBetweenBodyAndGroundFileName;
	
	/** manual gui test stuff */
	void InitGUITestStuff();
	
	void OnChooseGroundAsParent();

	void EnableGui(bool enable);

	mafVME *m_RefSysVMEInParent;

	mafVME *m_RefSysVMEInChild;

};
#endif
