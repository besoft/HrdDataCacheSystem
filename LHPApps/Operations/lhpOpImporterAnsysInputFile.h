/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpImporterAnsysInputFile.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Stefano Perticoni   
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpImporterAnsysInputFile_H__
#define __lhpOpImporterAnsysInputFile_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include "lhpOpImporterAnsysCommon.h"

//----------------------------------------------------------------------------
// lhpOpImporterAnsysInputFile :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT lhpOpImporterAnsysInputFile : public lhpOpImporterAnsysCommon
{
public:
	lhpOpImporterAnsysInputFile(const wxString &label = "MeshImporter");
	~lhpOpImporterAnsysInputFile(); 
	
  mafTypeMacro(lhpOpImporterAnsysInputFile, mafOp);

  mafOp* Copy();

protected:

  mafString GetWildcard();

  int ParseAnsysFile(mafString fileName);

  int UpdateNodesFile(FILE *outFile);
  int UpdateElementsFile(FILE *outFile);
};
#endif
