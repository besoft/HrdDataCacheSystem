/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpComputeGradientStructureTensor.h,v $
  Language:  C++
  Date:      $Date: 2011-10-20 14:50:44 $
  Version:   $Revision: 1.1.2.3 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpComputeGradientStructureTensor_H__
#define __lhpOpComputeGradientStructureTensor_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"
//#include "mafNode.h"
#include <iostream>
//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;

//----------------------------------------------------------------------------
// lhpOpComputeGradientStructureTensor :
//----------------------------------------------------------------------------
/** 

	Compute Gradient Structure Tensor primary eigenvectors from input mavVMEVolumeGray containing
	vtkStructuredPoints dataset.

	Use cases , tutorial and test data for the operation available here:
	https://docs.google.com/document/d/1LBTqLlJZTsW3dL0QC93RdREPdiz-wyct6ltSBG2cN4s/edit?hl=en_US

*/

class LHP_OPERATIONS_EXPORT lhpOpComputeGradientStructureTensor: public mafOp
{
public:

  lhpOpComputeGradientStructureTensor(wxString label);
  ~lhpOpComputeGradientStructureTensor(); 

  /** Set the Primary Eigen Vectors Output Image file abs path */
  void SetPrimaryEigenVectorOutputImageAbsPath(const char* name);

  /** Get the Primary Eigen Vectors Output Image file abs path */
  const char* GetPrimaryEigenVectorOutputImageAbsPath();

  /** Compute structure tensor eigenvectors: see shared documentation for more informations */
  void ComputeGradientStructureTensor();

  /** Create the operation graphical user interface*/
  void CreateGui();

  /** Overridden to accept a mafVMEVolumeGray made of vtkStructuredPoints as input */
  bool Accept(mafNode *node);

  /** Builds operation's interface. */
  void OpRun();

  mafOp* Copy();
  void OnEvent(mafEventBase *maf_event);
 
protected:

  /** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
  void OpStop(int result);
 
  /** Primary eigen vectors output file abs file name */
  mafString m_PrimaryEigenVectorOutputImageAbsPath;

};
#endif
