/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoAverageLM.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __mmoAverageLM_H__
#define __mmoAverageLM_H__

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
class LHP_OPERATIONS_EXPORT mmoAverageLM: public mafOp
{
public:
  mmoAverageLM(const wxString& label);
 ~mmoAverageLM(); 

  virtual void OnEvent(mafEventBase *maf_event);
  mafOp* Copy();

  bool Accept(mafNode* vme);   
  void OpRun();
  void OpDo();
  void OpUndo();
  void CreateGui();

protected: 

  void OpStop(int result);

  /// limb cloud from motion: animated, we will insert stick tip here
  mafVMELandmarkCloud  *m_LimbCloud;

private:
  /// index of new landmark for undo operation 
  wxInt32                       m_NewIndex;
};
#endif
