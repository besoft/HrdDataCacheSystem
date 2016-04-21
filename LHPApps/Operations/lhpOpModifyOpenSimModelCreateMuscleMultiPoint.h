/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelCreateMuscleMultiPoint.h,v $
  Language:  C++
  Date:      $Date: 2012-04-11 16:51:06 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelCreateMuscleMultiPoint_H__
#define __lhpOpModifyOpenSimModelCreateMuscleMultiPoint_H__

using namespace std;

#include "lhpOpModifyOpenSimModel.h"
#include "lhpOperationsDefines.h"
#include <map>

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafOp;
class mafEvent;
class mafVMESurface;
class mafVMELandmark;
class mafVME;

//----------------------------------------------------------------------------
// lhpOpModifyOpenSimModelCreateMuscleMultiPoint :
//----------------------------------------------------------------------------
/** 

Create an OpenSim muscle featuring multiple points.

User Guide:
https://docs.google.com/document/d/1sFmjzjO9YRS8k5WddSvVxXxQaXelK4TEpcuEQSId2V4/edit#heading=h.t9i50amlajqp

*/

class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelCreateMuscleMultiPoint: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelCreateMuscleMultiPoint(const wxString &label = "lhpOpModifyOpenSimModelAssignMarkerSet");
 ~lhpOpModifyOpenSimModelCreateMuscleMultiPoint();

 mafTypeMacro(lhpOpModifyOpenSimModelCreateMuscleMultiPoint, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 void SetListBox(wxListBox *listBox) {m_ListBox = listBox;};
 void SetLMNameLMVmeMap(std::map<std::string , mafVMELandmark *> &lmNameLMVmeMap) {m_LMNameLMVmeMap = lmNameLMVmeMap;};
 void SetLMNameBodyVmeMap(std::map<std::string , mafVME *> &lmNameBodyVmeMap) {m_LMNameBodyVmeMap = lmNameBodyVmeMap;};

 void GenerateOpenSimComponentAPI();

 protected:

	void CreateGui();

	void OnEvent(mafEventBase *maf_event);

	void OnRemovePoint();

	void OnAddPoint();

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	/** Create the C++ OpenSim API code template to be configured  */
	void CreateCPPCodeTemplate();

	void OnGenerateMuscle();

	wxListBox *m_ListBox;

	std::map<std::string , mafVMELandmark *> m_LMNameLMVmeMap;
	std::map<std::string , mafVME *> m_LMNameBodyVmeMap;

};
#endif