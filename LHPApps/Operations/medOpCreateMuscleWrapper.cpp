/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: medOpCreateMuscleWrapper.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Josef Kohout
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


#include "medOpCreateMuscleWrapper.h"
#include "mafDecl.h"
#include "mafEvent.h"

#include "../VME/medVMEMuscleWrapper.h"

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafCxxTypeMacro(medOpCreateMuscleWrapper);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
medOpCreateMuscleWrapper::medOpCreateMuscleWrapper(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;
  m_Canundo = true;
  m_Meter   = NULL;
}
//----------------------------------------------------------------------------
medOpCreateMuscleWrapper::~medOpCreateMuscleWrapper( ) 
//----------------------------------------------------------------------------
{
  mafDEL(m_Meter);
}
//----------------------------------------------------------------------------
mafOp* medOpCreateMuscleWrapper::Copy()   
//----------------------------------------------------------------------------
{
	return new medOpCreateMuscleWrapper(m_Label);
}
//----------------------------------------------------------------------------
bool medOpCreateMuscleWrapper::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return (node && node->IsMAFType(mafVME));
}
//----------------------------------------------------------------------------
void medOpCreateMuscleWrapper::OpRun()   
//----------------------------------------------------------------------------
{
  mafNEW(m_Meter);
  m_Meter->SetName("Muscle Wrapped Meter");
  m_Output = m_Meter;
  mafEventMacro(mafEvent(this,OP_RUN_OK));
}
//----------------------------------------------------------------------------
void medOpCreateMuscleWrapper::OpDo()
//----------------------------------------------------------------------------
{
  m_Meter->ReparentTo(m_Input);
}
