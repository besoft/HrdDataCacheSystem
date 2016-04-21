/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpCreateSurfaceScalar.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Paolo Quadrani
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "lhpOpCreateSurfaceScalar.h"
#include "mafDecl.h"

#include "lhpVMESurfaceScalarVarying.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpCreateSurfaceScalar);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpCreateSurfaceScalar::lhpOpCreateSurfaceScalar(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;
  m_Canundo = true;
  m_SurfaceScalar   = NULL;
}
//----------------------------------------------------------------------------
lhpOpCreateSurfaceScalar::~lhpOpCreateSurfaceScalar()
//----------------------------------------------------------------------------
{
  mafDEL(m_SurfaceScalar);
}
//----------------------------------------------------------------------------
mafOp* lhpOpCreateSurfaceScalar::Copy()   
//----------------------------------------------------------------------------
{
	return new lhpOpCreateSurfaceScalar(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpCreateSurfaceScalar::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return (node && node->IsMAFType(mafVME));
}
//----------------------------------------------------------------------------
void lhpOpCreateSurfaceScalar::OpRun()
//----------------------------------------------------------------------------
{
  mafNEW(m_SurfaceScalar);
  m_SurfaceScalar->SetName("Surface Scalar");
  m_Output = m_SurfaceScalar;
  mafEventMacro(mafEvent(this,OP_RUN_OK));
}
