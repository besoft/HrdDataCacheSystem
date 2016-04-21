/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup.h,v $
  Language:  C++
  Date:      $Date: 2012-04-11 16:51:06 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup_H__
#define __lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup_H__

using namespace std;

#include "lhpOpModifyOpenSimModel.h"
#include "lhpOperationsDefines.h"



//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafOp;
class mafEvent;
class mafVMESurface;
class mafVMEGroup;
class vtkPoints;
class vtkDataArray;

//----------------------------------------------------------------------------
// lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup :
//----------------------------------------------------------------------------
/** 

Create OpenSim Body from a vme group containing surfaces

User Guide:
https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit#heading=h.t9i50amlajqp

*/
class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup(const wxString &label = "lhpOpModifyOpenSimModelCreateBody");
 ~lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup();

 mafTypeMacro(lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 /** Set the input vme for opensim component API generation */
 void SetInputVMEForComponentAPIGeneration(mafNode *vme);

 /** Generate the OpenSim component C++ code */
 void GenerateOpenSimComponentAPI();

 protected:

	void CreateGui();
	void OnEvent(mafEventBase *maf_event);

	void OnChooseSurfacesGroup();
	void OnGenerateBody();

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	void CreateCPPCodeTemplate();

	void ComputeMassCenterFromGroup(mafVMEGroup *inGroup , double *outCenterOfMass);

	/** Centre of mass for the group of surfaces */
	double m_CenterOfMass[3];

	/** Vme body */
	mafVMEGroup *m_SurfacesGroup;

	/** Parameters needed by the operation*/
	double m_Ixx;
	double m_Iyy;
	double m_Izz;
	double m_Ixy;
	double m_Ixz;
	double m_Iyz;

	double m_Mass;
};
#endif
