/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpFilterPolyDataUsingExtApp.h,v $
  Language:  C++
  Date:      $Date: 2011-07-29 08:01:57 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011
 University of West Bohemia
=========================================================================*/



#ifndef __lhpOpFilterPolyDataUsingExtApp_H__
#define __lhpOpFilterPolyDataUsingExtApp_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include "lhpBuilderDecl.h"
#if defined(VPHOP_WP10)

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class vtkPolyData;

//----------------------------------------------------------------------------
// lhpOpFilterPolyDataUsingExtApp :
//----------------------------------------------------------------------------
/** 
	This operation exports automatically the selected mesh into STL format and
	let the user to launch an external STL filtering application to edit the exported data
	and export it into the original VME (so every links, tags are preserved) .*/
class LHP_OPERATIONS_EXPORT lhpOpFilterPolyDataUsingExtApp: public mafOp
{
protected:
  //----------------------------------------------------------------------------
  // Constants:
  //----------------------------------------------------------------------------
  

public:
  lhpOpFilterPolyDataUsingExtApp(const wxString &label = "Filter Mesh Using External STL Editor");
  ~lhpOpFilterPolyDataUsingExtApp(); 

  mafTypeMacro(lhpOpFilterPolyDataUsingExtApp, mafOp);	

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

	vtkPolyData*	m_OriginalPolydata;			///<the stored original input (when Execute was called)
  vtkPolyData*	m_ResultPolydata;				///<the output
};
#endif
#endif //__lhpOpFilterPolyDataUsingExtApp_H__