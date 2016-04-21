/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpMergeSurfaces.h,v $
  Language:  C++
  Date:      $Date: 2011-06-30 10:14:23 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011
 University of West Bohemia
=========================================================================*/



#ifndef __lhpOpMergeSurfaces_H__
#define __lhpOpMergeSurfaces_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include "lhpBuilderDecl.h"
#if defined(VPHOP_WP10)
#include <vector>

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;

//----------------------------------------------------------------------------
// lhpOpMergeSurfaces :
//----------------------------------------------------------------------------
/** Specifies a lower resolution VME for the current one. */
class LHP_OPERATIONS_EXPORT lhpOpMergeSurfaces: public mafOp
{
public:
  lhpOpMergeSurfaces(const wxString &label = "Merge Surface VMEs");
  ~lhpOpMergeSurfaces(); 

  mafTypeMacro(lhpOpMergeSurfaces, mafOp);	

  /*virtual*/ mafOp* Copy();

  /** Return true for the acceptable vme type. */
  /*virtual*/ bool Accept(mafNode *node);

  /** Builds operation's interface. */
  /*virtual*/ void OpRun();

  /** Execute the operation. */
  /*virtual*/ void OpDo();

    /** Execute the operation. */
  /*virtual*/ void OpUndo();
  
protected:
	std::vector< mafNode* > m_SelectedNodes; ///<nodes to be merged
protected:
	/** Callback for VME_CHOOSE that accepts any surface VME */
  static bool SelectVMECallback(mafNode *node);    	
};
#endif
#endif //__lhpOpMergeSurfaces_H__