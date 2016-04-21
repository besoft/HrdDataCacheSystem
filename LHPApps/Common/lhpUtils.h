/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpUtils.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:52 $
Version:   $Revision: 1.1 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/
#include "mafDefines.h"
#include "lhpCommonDefines.h"

#ifndef __lhpUtils_H__
#define __lhpUtils_H__

class LHP_COMMON_EXPORT lhpUtils
{
public:

  /** In DEBUG mode return LHPBuilder source code directory in WIN32 path format. 
  In RELEASE mode it returns the Application installation directory in WIN32 path format (same as mafGetApplicationDirectory)*/

  static wxString lhpGetApplicationDirectory();

};
#endif