/*=========================================================================
  Program:   VPH2
  Module:    $RCSfile: lhpVMEScalarMatrix.h,v $
  Language:  C++
  Date:      $Date: 2009-11-03 12:58:03 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Alberto Losi
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/
#ifndef __lhpVMEScalarMatrix_h
#define __lhpVMEScalarMatrix_h
//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafVMEScalarMatrix.h"
#include "lhpVMEDefines.h"

//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------
class mafVMEScalarMatrix;

/** lhpVMEScalarMatrix */
class LHP_VME_EXPORT lhpVMEScalarMatrix : public mafVMEScalarMatrix
{
public:
  mafTypeMacro(lhpVMEScalarMatrix,mafVMEScalarMatrix);

  /** return icon */
  static char** GetIcon();

protected:
  lhpVMEScalarMatrix();
  virtual ~lhpVMEScalarMatrix();

  /** Internally used to create a new instance of the GUI.*/
  virtual mafGUI *CreateGui();
};
#endif
