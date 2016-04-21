/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpMoveVMEUp.h,v $
  Language:  C++
  Date:      $Date: 2011-06-30 10:14:24 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011
 University of West Bohemia
=========================================================================*/

#ifndef __lhpOpMoveVMEUp_H__
#define __lhpOpMoveVMEUp_H__

#include "mafOp.h"
#include "lhpBuilderDecl.h"
#include "lhpOperationsDefines.h"
#if defined(VPHOP_WP10)

//----------------------------------------------------------------------------
// lhpOpMoveVMEUp :
//----------------------------------------------------------------------------
/** Specifies a lower resolution VME for the current one. */
class LHP_OPERATIONS_EXPORT lhpOpMoveVMEUp: public mafOp
{
public:
  lhpOpMoveVMEUp(const wxString &label = "Move VME Up");
  ~lhpOpMoveVMEUp(); 

  mafTypeMacro(lhpOpMoveVMEUp, mafOp);

  /*virtual*/ mafOp* Copy();

  /** Return true for the acceptable vme type. */
  /*virtual*/ bool Accept(mafNode *node);

  /** Builds operation's interface. */
  /*virtual*/ void OpRun();

  /** Execute the operation. */
  /*virtual*/ void OpDo();
};
#endif
#endif //__lhpOpMoveVMEUp_H__