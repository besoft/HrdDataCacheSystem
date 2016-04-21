/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: medOpCreateParticle.h,v $
  Language:  C++
  Date:      $Date: 2012-04-04 16:02:03 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Yubo Tao
==========================================================================
 Copyright (c) 2012
 University of Bedfordshire
=========================================================================*/

#ifndef __medOpCreateParticle_H__
#define __medOpCreateParticle_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// medOpCreateParticle :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT medOpCreateParticle: public mafOp
{
public:
  medOpCreateParticle(const wxString &label = "Create mafVMEOutputLandmarkCloud");
  ~medOpCreateParticle(); 

  mafTypeMacro(medOpCreateParticle, mafOp);

  /*virtual*/ mafOp* Copy();
  /*virtual*/ bool Accept(mafNode *node);
  /*virtual*/ void OpRun();
  /*virtual*/ void OpDo();
  /*virtual*/ void OpUndo();
};
#endif //__medOpCreateParticle_H__