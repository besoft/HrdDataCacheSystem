/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelCreateMuscle.h,v $
  Language:  C++
  Date:      $Date: 2012-03-30 14:39:56 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelCreateMuscle_H__
#define __lhpOpModifyOpenSimModelCreateMuscle_H__

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
// lhpOpModifyOpenSimModelCreateMuscle :
//----------------------------------------------------------------------------
/** 

Create OpenSim Muscle from MSF Tree

User Guide:
https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit#heading=h.t9i50amlajqp

*/

class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelCreateMuscle: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelCreateMuscle(const wxString &label = "lhpOpModifyOpenSimModelCreateMuscle");
 ~lhpOpModifyOpenSimModelCreateMuscle();

 mafTypeMacro(lhpOpModifyOpenSimModelCreateMuscle, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 /** Set input vme's for generating Origin-ViaPoint-InsertionPoint muscles */
 void SetInputVMEs(
	 mafVMESurface *originPointBodyVME,
	 mafVMELandmark *originPointLandmarkVME,
	 mafVMESurface *viaPointBodyVME,
	 mafVMELandmark *viaPointLandmarkVME,
	 mafVMESurface *insertionPointBodyVME,
	 mafVMELandmark *insertionPointLandmarkVME);

 protected:

	void CreateGui();
	void OnEvent(mafEventBase *maf_event);

	/** choose the parent vme for joint generation */
	void OnChooseOriginPointBody();

	/** choose the child vme for joint generation */
	void OnChooseViaPointBody();

	void OnGenerateMuscle();

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	mafVMESurface *m_OriginPointBodyVME;
	mafVMELandmark *m_OriginPointLandmarkVME;

	mafVMESurface *m_ViaPointBodyVME;
	mafVMELandmark *m_ViaPointLandmarkVME;

	mafVMESurface *m_InsertionPointBodyVME;
	mafVMELandmark *m_InsertionPointLandmarkVME;

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
		
	/** Vme to be used as refsys */
	mafVME *m_RefSysVME;

	/** Orientation X , Y , Z */
	void GetOrientationXYZ(const mafMatrix &in_matrix,double orientation[3]);

	void OnChooseInsertionPointBody();
	void OnChooseOriginPointLandmark();
	void OnChooseViaPointLandmark();
	void OnChooseInsertionPointLandmark();

};
#endif
