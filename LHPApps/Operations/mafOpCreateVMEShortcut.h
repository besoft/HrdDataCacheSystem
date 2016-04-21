/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafOpCreateVMEShortcut.h,v $
  Language:  C++
  Date:      $Date: 2011-03-03 11:37:46 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Josef Kohout
==========================================================================
 Copyright (c) 2011
 University of West Bohemia
=========================================================================*/

#ifndef __mafOpCreateVMEShortcut_H__
#define __mafOpCreateVMEShortcut_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVMEShortcut;
class mafGui;
class mafEvent;
//----------------------------------------------------------------------------
// mafOpCreateVMEShortcut :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT mafOpCreateVMEShortcut: public mafOp
{
public:
  mafOpCreateVMEShortcut(const wxString &label = "CreateVMEShortcut");
  ~mafOpCreateVMEShortcut(); 

  mafTypeMacro(mafOpCreateVMEShortcut, mafOp);

  mafOp* Copy();

  bool Accept(mafNode *node);
  void OpRun();
  void OpDo();

protected: 
  mafVMEShortcut *m_Shortcut;
};
#endif //__mafOpCreateVMEShortcut_H__
