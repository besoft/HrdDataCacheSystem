/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoTimeReduce.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __mmoTimeReduce_H__
#define __mmoTimeReduce_H__

//----------------------------------------------------------------------------
// Includes:
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVMEAFRefSys;
class mafGui;
class mafEvent;
class mafIntGraphHyer;
class mafVMELandmarkCloud;
class vtkPoints;

//----------------------------------------------------------------------------
// mmoRefSys :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT mmoTimeReduce: public mafOp
{
public:
  mmoTimeReduce(const wxString& label = "TimeReduce");
 ~mmoTimeReduce(); 

  virtual void OnEvent(mafEventBase *maf_event);
  mafOp* Copy();

  bool Accept(mafNode* vme);   
  void OpRun();
  void OpDo();
  void OpUndo();
  void CreateGui();

protected: 
  //void OpStop(int result);

private:
  int m_Delete;
  int m_Number;
};
#endif
