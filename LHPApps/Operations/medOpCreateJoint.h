/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: medOpCreateJoint.h,v $
  Language:  C++
  Date:      $Date: 2011-06-30 10:14:24 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011
 University of West Bohemia
=========================================================================*/

#ifndef __medOpCreateJoint_H__
#define __medOpCreateJoint_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// medOpCreateJoint :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT medOpCreateJoint: public mafOp
{
public:
  medOpCreateJoint(const wxString &label = "Create medVMEJoint");
  ~medOpCreateJoint(); 

  mafTypeMacro(medOpCreateJoint, mafOp);

  /*virtual*/ mafOp* Copy();
  /*virtual*/ bool Accept(mafNode *node);
  /*virtual*/ void OpRun();
  /*virtual*/ void OpDo();
	/*virtual*/ void OpUndo();
};
#endif //__medOpCreateJoint_H__