/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpMSModelCheckTags.h,v $
  Language:  C++
  Date:      $Date: 2011-06-30 10:14:23 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011
 University of West Bohemia
=========================================================================*/

#ifndef __lhpOpMSModelCheckTags_H__
#define __lhpOpMSModelCheckTags_H__

#include "mafOp.h"
#include "lhpBuilderDecl.h"
#include "lhpOperationsDefines.h"

#if defined(VPHOP_WP10)
#include "medMSMGraph.h"

//----------------------------------------------------------------------------
// lhpOpMSModelCheckTags :
//----------------------------------------------------------------------------
/** Specifies a lower resolution VME for the current one. */
class LHP_OPERATIONS_EXPORT lhpOpMSModelCheckTags: public mafOp
{
public:
  lhpOpMSModelCheckTags(const wxString &label = "Check Atlas");
  ~lhpOpMSModelCheckTags(); 

  mafTypeMacro(lhpOpMSModelCheckTags, mafOp);

  /*virtual*/ mafOp* Copy();

  /** Return true for the acceptable vme type. */
  /*virtual*/ bool Accept(mafNode *node);

  /** Builds operation's interface. */
  /*virtual*/ void OpRun();

  /** Execute the operation. */
  /*virtual*/ void OpDo();

protected:
	/** Main routine to recursively check every node of the atlas. */
	void CheckGraphR(const medMSMGraph::MSMGraphNode* root, int indent = 0); 

	/** Copies tags from to to VME */
	void CopyTags(mafNode* from, mafNode* to);
};
#endif
#endif //__lhpOpMSModelCheckTags_H__