/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoHelAxis.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __mmoHelAxis_H__
#define __mmoHelAxis_H__

//----------------------------------------------------------------------------
// Includes:
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "mafVME.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVMEHelAxis;
class mafGui;
class mafEvent;
class mafIntGraphHyer;
class mafVMELandmarkCloud;
class vtkPoints;

//----------------------------------------------------------------------------
// mmoHelAxis:
//----------------------------------------------------------------------------
class LHP_OPERATIONS_EXPORT mmoHelAxis: public mafOp
{
public:
  mmoHelAxis(wxString label = "HelAxis");
 ~mmoHelAxis(); 

  mafTypeMacro(mmoHelAxis, mafOp);

  virtual void OnEvent(mafEventBase *maf_event);
  mafOp* Copy();

  bool Accept(mafNode* vme);   
  void OpRun();
  void OpDo();
  void CreateGui();

  static double RegisterPoints(vtkPoints *pointsSource, vtkPoints *pointsTarget, int &numPoints, mafVMELandmarkCloud *src, float time1, float time2, vtkMatrix4x4 *res_matrix);
protected: 

  void OpStop(int result);

  mafVMEHelAxis   *m_HelicalSys;
  mafIntGraphHyer *m_Hierarchy;
  wxString         m_DictionaryFName;
};
#endif
