/*=========================================================================

 Program: NMSBuilder
 Module: lhpOpModifyOpenSimModelCreateCustomJoint
 Authors: Stefano Perticoni
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelCreateCustomJoint_H__
#define __lhpOpModifyOpenSimModelCreateCustomJoint_H__

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
// lhpOpModifyOpenSimModelCreateCustomJoint :
//----------------------------------------------------------------------------
/** 

Create OpenSim Custom Joint

*/

class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelCreateCustomJoint: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelCreateCustomJoint(const wxString &label = "lhpOpModifyOpenSimModelCreateCustomJoint");
 ~lhpOpModifyOpenSimModelCreateCustomJoint();

 mafTypeMacro(lhpOpModifyOpenSimModelCreateCustomJoint, lhpOpModifyOpenSimModel);

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
	mafVME *m_ParentBodyVME;

	/** Vme parent for body generation */
	mafVME *m_ChildBodyVME;

	/** Use the ground as parent body*/
	int m_UseGroundAsParent;

	wxString m_ComponentToConfigureJointBetween2BodiesFileName;

	wxString m_ComponentToConfigureJointBetweenBodyAndGroundFileName;
	
	void OnChooseGroundAsParent();
	void OnChooseParentBodyVMESurface();
	void OnChooseChildBodyVMESurface();
	void WriteNMSBuilderDataToFile();

	void EnableGui(bool enable);

	mafVME *m_RefSysVMEInParent;
	mafVME *m_RefSysVMEInChild;
	
};
#endif
