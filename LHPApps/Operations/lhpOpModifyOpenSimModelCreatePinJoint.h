/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelCreatePinJoint.h,v $
  Language:  C++
  Date:      $Date: 2012-03-20 12:14:37 $
  Version:   $Revision: 1.1.2.4 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelCreatePinJoint_H__
#define __lhpOpModifyOpenSimModelCreatePinJoint_H__

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
// lhpOpModifyOpenSimModelCreatePinJoint :
//----------------------------------------------------------------------------
/** 

WORK IN PROGRESS

Create OpenSim Pin Joint from msf tree

User Guide:
https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit#heading=h.t9i50amlajqp

*/

class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelCreatePinJoint: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelCreatePinJoint(const wxString &label = "lhpOpModifyOpenSimModelCreatePinJoint");
 ~lhpOpModifyOpenSimModelCreatePinJoint();

 mafTypeMacro(lhpOpModifyOpenSimModelCreatePinJoint, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 void SetParentBodyVME(mafVME *parent);

 /** Vme child for body generation */
 void SetChildBodyVME(mafVME *child);

 /** Vme to be used as refsys */
 void SetRefSysVMEInParent(mafVME *refSys);
 void SetRefSysVMEInChild( mafVME *refSys );

 /** Use the ground as parent body*/
 void SetUseGroundAsParent(int useGroundAsParent);

 protected:

	void CreateGui();
	void OnEvent(mafEventBase *maf_event);

	/** choose the parent vme for joint generation */
	void OnChooseParentBodyVMESurface();

	mafVME *ChooseBodyVME( mafString title );

	/** choose the child vme for joint generation */
	void OnChooseChildBodyVMESurface();

	/** accept a vme landmark */
	static bool IsVMELandmark(mafNode *node);
	
	void OnGenerateJoint();

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	/** fill direction cosines ivar from rotation axis origin and endpoint (old implementation)*/
	void RotationAxisDirectionCosinesFromLandmarks();
	
	wxString m_ComponentToConfigureJointBetween2BodiesFileName;

	wxString m_ComponentToConfigureJointBetweenBodyAndGroundFileName;
	
	/** manual gui test stuff */
	void InitGUITestStuff();
	
	void OnChooseGroundAsParent();
	void OnChooseRefSysVme();
	
	/** Orientation X , Y , Z */
	void GetOrientationXYZ(const mafMatrix &in_matrix,double orientation[3]);

	/** Vme parent for body generation */
	mafVME *m_ParentBodyVME;

	/** Vme child for body generation */
	mafVME *m_ChildBodyVME;

	mafVME *m_RefSysVMEInParent;

	mafVME *m_RefSysVMEInChild;

	/** Use the ground as parent body*/
	int m_UseGroundAsParent;


};
#endif
