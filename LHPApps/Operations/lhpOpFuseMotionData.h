/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpFuseMotionData.h,v $
  Language:  C++
  Date:      $Date: 2011-04-11 07:18:22 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011
 University of West Bohemia
=========================================================================*/

#ifndef __lhpOpFuseMotionData_H__
#define __lhpOpFuseMotionData_H__

#include "lhpOpConvertMotionData.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// lhpOpFuseMotionData :
//----------------------------------------------------------------------------
/** Specifies a lower resolution VME for the current one. */
class LHP_OPERATIONS_EXPORT lhpOpFuseMotionData: public lhpOpConvertMotionData
{
public:
  lhpOpFuseMotionData(const wxString &label = "Motion Fusion");
  ~lhpOpFuseMotionData(); 

  mafTypeMacro(lhpOpFuseMotionData, mafOp);	

	/** Return true for the acceptable vme type. */
  /*virtual*/ bool Accept(mafNode *node);

  /*virtual*/ mafOp* Copy();

  /** Builds operation's interface. */
  /*virtual*/ void OpRun();

  /** Execute the operation. */
  /*virtual*/ void OpDo();

	/*Sets a new musculoskeletal mode VME
	NB. you need to call OpDo to confirm the changes. */
	/*virtual*/ void SetMusculoskeletalModelVME(mafVME* vme);

	/*Sets a new MotionData VME
	NB. you need to call OpDo to confirm the changes. */
	inline /*virtual*/ void SetMotionDataVME(mafVME* vme) {
		m_MotionDataVME = vme;
	}
protected:	
	/** Returns true, if the VME represented by the given msm_node should be DeepCopied*/
	/*virtual*/ bool CanBeDeepCopied(const medMSMGraph::MSMGraphNode* msm_node);

	/** Returns true, if for the VME represented by the given msm_node a shortcut should be created*/
	/*virtual*/ bool CanBeLinkCopied(const medMSMGraph::MSMGraphNode* msm_node);
};

#endif //__lhpOpFuseMotionData_H__