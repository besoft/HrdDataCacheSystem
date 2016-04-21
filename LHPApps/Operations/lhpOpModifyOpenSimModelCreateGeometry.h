/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelCreateGeometry.h,v $
  Language:  C++
  Date:      $Date: 2012-02-10 14:33:34 $
  Version:   $Revision: 1.1.2.3 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelCreateGeometry_H__
#define __lhpOpModifyOpenSimModelCreateGeometry_H__

using namespace std;

#include "lhpOpModifyOpenSimModel.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafOp;
class mafEvent;
class mafVMESurface;

//----------------------------------------------------------------------------
// lhpOpModifyOpenSimModelCreateGeometry :
//----------------------------------------------------------------------------
/** 

Create OpenSim Geometry from surface VME

User Guide:
https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit#heading=h.t9i50amlajqp

Operation Screencast:
http://www.youtube.com/watch?v=Bau2XfpQ5Qo

*/
class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelCreateGeometry: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelCreateGeometry(const wxString &label = "lhpOpModifyOpenSimModelCreateGeometry");
 ~lhpOpModifyOpenSimModelCreateGeometry();

 mafTypeMacro(lhpOpModifyOpenSimModelCreateGeometry, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 protected:

	void CreateGui();
	void OnEvent(mafEventBase *maf_event);

	void OnChooseGeometryVME();
	void OnGenerateGeometry();

	static bool VmeSurfaceAcceptForGeometryGeneration(mafNode *node);

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	/** Vme body */
	mafVMESurface *m_Surface;

	/** Body vtk data path */ 
	wxString m_SurfaceVtkAbsFileName;

	/** Body VTP (new vtk file format for polydata) data path */ 
	wxString m_SurfaceVTPFileName;


};
#endif
