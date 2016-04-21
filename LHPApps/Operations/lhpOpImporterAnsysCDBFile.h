/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpImporterAnsysCDBFile.h,v $
Language:  C++
Date:      $Date: 2010-11-23 16:50:26 $
Version:   $Revision: 1.1.1.1.2.3 $
Authors:   Daniele Giunchi , Stefano Perticoni, Gianluigi Crimi
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpImporterAnsysCDBFile_H__
#define __lhpOpImporterAnsysCDBFile_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include "lhpOpImporterAnsysCommon.h"

//----------------------------------------------------------------------------
// lhpOpImporterAnsysCDBFile :
//----------------------------------------------------------------------------
/** 
Importer for Ansys CDB Input files
*/
class LHP_OPERATIONS_EXPORT lhpOpImporterAnsysCDBFile : public lhpOpImporterAnsysCommon
{
public:

	lhpOpImporterAnsysCDBFile(const wxString &label = "MeshImporter");
	~lhpOpImporterAnsysCDBFile(); 

	mafTypeMacro(lhpOpImporterAnsysCDBFile, mafOp);

	mafOp* Copy();

protected:
 
  mafString GetWildcard();

	int ParseAnsysFile(mafString fileName);
};
#endif
