/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpCreateSurfaceScalar.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Paolo Quadrani
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpCreateSurfaceScalar_H__
#define __lhpOpCreateSurfaceScalar_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class lhpVMESurfaceScalarVarying;

//----------------------------------------------------------------------------
// lhpOpCreateSurfaceScalar :
//----------------------------------------------------------------------------
/** Operation used to create a new lhpVMESurfaceScalarVarying.
@sa lhpVMESurfaceScalarVarying lhpVisualPipeSurfaceScalar*/
class LHP_OPERATIONS_EXPORT lhpOpCreateSurfaceScalar: public mafOp
{
public:
  lhpOpCreateSurfaceScalar(const wxString &label = "Create surface scalar");
  ~lhpOpCreateSurfaceScalar(); 

  mafTypeMacro(lhpOpCreateSurfaceScalar, mafOp);

  mafOp* Copy();

  bool Accept(mafNode *node);
  void OpRun();

protected: 
  lhpVMESurfaceScalarVarying *m_SurfaceScalar;
};
#endif
