/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpMoveVMEDown.h,v $
  Language:  C++
  Date:      $Date: 2011-06-30 10:14:23 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011
 University of West Bohemia
=========================================================================*/

#ifndef __lhpOpMoveVMEDown_H__
#define __lhpOpMoveVMEDown_H__

#include "mafOp.h"
#include "lhpBuilderDecl.h"
#include "lhpOperationsDefines.h"

#if defined(VPHOP_WP10)

//----------------------------------------------------------------------------
// lhpOpMoveVMEDown :
//----------------------------------------------------------------------------
/** Specifies a lower resolution VME for the current one. */
class LHP_OPERATIONS_EXPORT lhpOpMoveVMEDown: public mafOp
{
public:
  lhpOpMoveVMEDown(const wxString &label = "Move VME Down");
  ~lhpOpMoveVMEDown(); 

  mafTypeMacro(lhpOpMoveVMEDown, mafOp);

  /*virtual*/ mafOp* Copy();

  /** Return true for the acceptable vme type. */
  /*virtual*/ bool Accept(mafNode *node);

  /** Builds operation's interface. */
  /*virtual*/ void OpRun();

  /** Execute the operation. */
  /*virtual*/ void OpDo();

protected:
	bool m_bPropagate;	///<true, if the operation should propagate to children
};
#endif
#endif //__lhpOpMoveVMEDown_H__