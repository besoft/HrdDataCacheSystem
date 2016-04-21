/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpRemoveTracability.h,v $
  Language:  C++
  Date:      $Date: 2011-06-30 10:14:24 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011
 University of West Bohemia
=========================================================================*/



#ifndef __lhpOpRemoveTracability_H__
#define __lhpOpRemoveTracability_H__

#include "mafOp.h"
#include "lhpBuilderDecl.h"
#include "lhpOperationsDefines.h"
#if defined(VPHOP_WP10)
//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// lhpOpRemoveTracability :
//----------------------------------------------------------------------------
/** Specifies a lower resolution VME for the current one. */
class LHP_OPERATIONS_EXPORT lhpOpRemoveTracability: public mafOp
{
public:
  lhpOpRemoveTracability(const wxString &label = "Remove mafAttributeTraceability");
  ~lhpOpRemoveTracability(); 

  mafTypeMacro(lhpOpRemoveTracability, mafOp);

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
#endif //__lhpOpRemoveTracability_H__