/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpExporterAnsysCDBFile.h,v $
  Language:  C++
  Date:      $Date: 2010-11-26 16:46:11 $
  Version:   $Revision: 1.1.1.1.2.1 $
  Authors:   Gianluigi Crimi   
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpExporterAnsysCDBFile_H__
#define __lhpOpExporterAnsysCDBFile_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

#include "lhpOpExporterAnsysCommon.h"

//----------------------------------------------------------------------------
// lhpOpExporterAnsysInputFile :
//----------------------------------------------------------------------------

class LHP_OPERATIONS_EXPORT lhpOpExporterAnsysCDBFile : public lhpOpExporterAnsysCommon
{
public:
	lhpOpExporterAnsysCDBFile(const wxString &label = "lhpOpExporterAnsysCDBFile");
	~lhpOpExporterAnsysCDBFile(); 
	
  mafTypeMacro(lhpOpExporterAnsysCDBFile, mafOp);
  
  /** Export the input mesh by writing it in Ansys .cdb format */
  int Write();
  
  mafOp* Copy();

protected:

  mafString GetWildcard();
  
  int WriteHeaderFile(FILE *file);
  int WriteNodesFile(FILE *file);
  int WriteElementsFile(FILE *file);
  int WriteMaterialsFile(FILE *file);  
};
#endif
