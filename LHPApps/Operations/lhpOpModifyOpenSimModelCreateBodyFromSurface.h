/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelCreateBodyFromSurface.h,v $
  Language:  C++
  Date:      $Date: 2012-04-11 16:40:13 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelCreateBody_H__
#define __lhpOpModifyOpenSimModelCreateBody_H__

using namespace std;

#include "lhpOpModifyOpenSimModel.h"
#include "lhpOperationsDefines.h"



//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafOp;
class mafEvent;
class mafVME;
class mafVMESurface;
class vtkPoints;
class vtkDataArray;

//----------------------------------------------------------------------------
// lhpOpModifyOpenSimModelCreateBody :
//----------------------------------------------------------------------------
/** 

Create OpenSim Body from a single surface VME and Inertial Parameters

User Guide:
https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit#heading=h.t9i50amlajqp

*/
class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelCreateBodyFromSurface: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelCreateBodyFromSurface(const wxString &label = "lhpOpModifyOpenSimModelCreateBody");
 ~lhpOpModifyOpenSimModelCreateBodyFromSurface();

 mafTypeMacro(lhpOpModifyOpenSimModelCreateBodyFromSurface, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 /** Compute the center of mass using surface points */
 static void ComputeCenterOfMass(vtkPoints* points, double center[3]);

 /** Set the input vme for opensim component API generation */
 void SetInputVMEForComponentAPIGeneration(mafNode *vme);

 protected:

	void CreateGui();
	void OnEvent(mafEventBase *maf_event);

	void OnChooseBodyVME();

	void OnSetInputVME( mafVME * vme );

	void OnGenerateBody();

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	/** Centre of mass for current surface */
	double m_CenterOfMass[3];

	/** Vme body */
	mafVMESurface *m_Surface;

	/** Body vtk data path */ 
	wxString m_SurfaceVtkAbsFileName;

	/** Body VTP (new vtk file format for polydata) data path */ 
	wxString m_SurfaceVTPFileName;

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
