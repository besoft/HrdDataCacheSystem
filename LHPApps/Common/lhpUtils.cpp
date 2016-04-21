/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpUtils.cpp,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:52 $
Version:   $Revision: 1.1 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDecl.h"

#include "lhpUtils.h"
#include "lhpDefines.h"

wxString lhpUtils::lhpGetApplicationDirectory()
{
#ifndef _DEBUG
             // RELEASE 
             wxString applicationDirectory = mafGetApplicationDirectory().c_str();
             return applicationDirectory;

#else
             // DEBUG
             wxString sourceDir = LHP_SOURCE_DIR;

             for (unsigned int i=0;i<sourceDir.Length();i++)
             {
               if (sourceDir[i]=='/')
                 sourceDir[i]='\\';
             }

             return sourceDir;

#endif
}