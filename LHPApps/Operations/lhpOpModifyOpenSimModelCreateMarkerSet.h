/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelCreateMarkerSet.h,v $
  Language:  C++
  Date:      $Date: 2012-04-11 16:51:06 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelCreateMarkerSet_H__
#define __lhpOpModifyOpenSimModelCreateMarkerSet_H__

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
class mafVMELandmarkCloud;

//----------------------------------------------------------------------------
// lhpOpModifyOpenSimModelCreateMarkerSet :
//----------------------------------------------------------------------------
/** 

Create an OpenSim marker set and assign it to a body

User Guide:
https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit#heading=h.t9i50amlajqp

*/

class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelCreateMarkerSet: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelCreateMarkerSet(const wxString &label = "lhpOpModifyOpenSimModelAssignMarkerSet");
 ~lhpOpModifyOpenSimModelCreateMarkerSet();

 mafTypeMacro(lhpOpModifyOpenSimModelCreateMarkerSet, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 void SetMarkerSetLandmarkCloud(mafVMELandmarkCloud *markerSetLandmarkCloud);

 void SetMarkerSetBody(mafVME *m_MarkerSetBody);

 void GenerateOpenSimComponentAPI();
 
 

 protected:

	void CreateGui();

	void OnEvent(mafEventBase *maf_event);

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	/** Create the C++ OpenSim API code template to be configured  */
	void CreateCPPCodeTemplate();

	void OnChooseMarkerSetLandmarkCloud();
	void OnChooseMarkerSetBody();
	void OnGenerateMarkerSet();

	mafVMELandmarkCloud *m_MarkerSetLandmarkCloud;
	mafVME *m_MarkerSetBody;

};
#endif
