/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpMultiReparentTo.h,v $
  Language:  C++
  Date:      $Date: 2011-06-30 10:14:24 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Josef Kohout
==========================================================================
  Copyright (c) 2011
  University of West Bohemia
=========================================================================*/

#ifndef __lhpOpMultiReparentTo_H__
#define __lhpOpMultiReparentTo_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOpReparentTo.h"
#include "lhpBuilderDecl.h"
#include "lhpOperationsDefines.h"

#if defined(VPHOP_WP10)

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafNode;
class mafVME;

//----------------------------------------------------------------------------
// lhpOpMultiReparentTo :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT lhpOpMultiReparentTo: public mafOp
{
public:
  lhpOpMultiReparentTo(const wxString &label = "ReparentTo");
 ~lhpOpMultiReparentTo(); 
  
  mafTypeMacro(lhpOpMultiReparentTo, mafOp);

  mafOp* Copy();

	/** Return true for the acceptable vme type. */
  bool Accept(mafNode *node);

	/** Builds operation's interface. */
  void OpRun();

	/** Execute the operation. */
  void OpDo();

  /** Sets a new input VME.
   If vme is NULL, it removes VME at the given index */
  void SetInputVME(mafVME* vme, int index);

  /** Set the target vme (used to be called without using interface)*/
  void SetTargetVme(mafVME *target);
  

protected:
	std::vector< mafVME* > m_InputVmes;
	mafVME *m_TargetVme;	
};

#endif
#endif



