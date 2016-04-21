/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpInteractorPERScalarInformation.h,v $
Language:  C++
Date:      $Date: 2010-04-01 13:15:24 $
Version:   $Revision: 1.1.2.1 $
Authors:   Matteo Giacomoni
==========================================================================
Copyright (c) 2010
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/
#ifndef __lhpInteractorPERScalarInformation_h
#define __lhpInteractorPERScalarInformation_h

#include "mafInteractorPER.h"

//----------------------------------------------------------------------------
// forward declarations
//----------------------------------------------------------------------------

/** Class implementing image info reporting on status bar when not interacting.
*/
class lhpInteractorPERScalarInformation : public mafInteractorPER
{
public: 
  mafTypeMacro(lhpInteractorPERScalarInformation,mafInteractorPER);

  virtual void OnEvent(mafEventBase *event);

protected:
  lhpInteractorPERScalarInformation();
  virtual ~lhpInteractorPERScalarInformation();

private:
  lhpInteractorPERScalarInformation(const lhpInteractorPERScalarInformation&);  // Not implemented.
  void operator=(const lhpInteractorPERScalarInformation&);  // Not implemented.
};
#endif 
