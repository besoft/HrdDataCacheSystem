/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpModifyOpenSimModelCreateMuscleFromMeter.h,v $
  Language:  C++
  Date:      $Date: 2012-04-11 16:51:06 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpModifyOpenSimModelCreateMuscleFromMeter_H__
#define __lhpOpModifyOpenSimModelCreateMuscleFromMeter_H__

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
class medVMEComputeWrapping;
class mafVMELandmarkCloud;
//----------------------------------------------------------------------------
// lhpOpModifyOpenSimModelCreateMuscleFromMeter :
//----------------------------------------------------------------------------
/** 

Create an OpenSim muscle from a meter (medVMEComputeWrapping)

*/

class LHP_OPERATIONS_EXPORT lhpOpModifyOpenSimModelCreateMuscleFromMeter: public lhpOpModifyOpenSimModel
{
public:
  lhpOpModifyOpenSimModelCreateMuscleFromMeter(const wxString &label = "lhpOpModifyOpenSimModelAssignMarkerSet");
 ~lhpOpModifyOpenSimModelCreateMuscleFromMeter();

 mafTypeMacro(lhpOpModifyOpenSimModelCreateMuscleFromMeter, lhpOpModifyOpenSimModel);

 mafOp* Copy();

 /** Return true for the acceptable vme type. */
 bool Accept(mafNode* node);

 /** Builds operation's interface. */
 void OpRun();

 void SetMeter(medVMEComputeWrapping *meter) {m_Meter = meter;};

 void GenerateOpenSimComponentAPI();

 protected:

	void CreateGui();

	void OnEvent(mafEventBase *maf_event);

	void Fill_m_Cloud_LandmarkIdVector(medVMEComputeWrapping *meterVme);

	/** Create data file from NMSBuilder: this will be used to configure the corresponding OpenSim Component dictionary */
	void WriteNMSBuilderDataToFile();

	/** Create the C++ OpenSim API code template to be configured  */
	void CreateCPPCodeTemplate();
	
	void OnGenerateMuscle();
	
	//------------------------------
	medVMEComputeWrapping *m_Meter;

	std::vector<pair<mafVMELandmarkCloud *, int >> m_Cloud_LandmarkIdVector;

};
#endif
