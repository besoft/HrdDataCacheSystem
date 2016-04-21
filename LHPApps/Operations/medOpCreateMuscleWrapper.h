/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: medOpCreateMuscleWrapper.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Josef Kohout
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __medOpCreateMuscleWrapper_H__
#define __medOpCreateMuscleWrapper_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class medVMEMuscleWrapper;
class mafGui;
class mafEvent;
//----------------------------------------------------------------------------
// medOpCreateMuscleWrapper :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT medOpCreateMuscleWrapper: public mafOp
{
public:
  medOpCreateMuscleWrapper(const wxString &label = "CreateMuscleWrappedMeter");
  ~medOpCreateMuscleWrapper(); 

  mafTypeMacro(medOpCreateMuscleWrapper, mafOp);

  mafOp* Copy();

  bool Accept(mafNode *node);
  void OpRun();
  void OpDo();

protected: 
  medVMEMuscleWrapper *m_Meter;
};
#endif //__medOpCreateMuscleWrapper_H__
