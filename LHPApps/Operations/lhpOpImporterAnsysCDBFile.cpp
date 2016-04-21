/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpImporterAnsysCDBFile.cpp,v $
  Language:  C++
  Date:      $Date: 2010-11-23 16:50:26 $
  Version:   $Revision: 1.1.1.1.2.4 $
  Authors:   Daniele Giunchi, Stefano Perticoni, Gianluigi Crimi
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

#include "lhpUtils.h"
#include "lhpBuilderDecl.h"

#include "lhpOpImporterAnsysCDBFile.h"

#include "wx/busyinfo.h"

#include "mafDecl.h"
#include "mafGUI.h"

#include "mafSmartPointer.h"
#include "mafTagItem.h"
#include "mafTagArray.h"
#include "mafVME.h"

#include "mafVMEMeshAnsysTextImporter.h"

#include "vtkMAFSmartPointer.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpImporterAnsysCDBFile);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpImporterAnsysCDBFile::lhpOpImporterAnsysCDBFile(const wxString &label) :
lhpOpImporterAnsysCommon(label)
//----------------------------------------------------------------------------
{ 

}
//----------------------------------------------------------------------------
lhpOpImporterAnsysCDBFile::~lhpOpImporterAnsysCDBFile()
//----------------------------------------------------------------------------
{
  mafDEL(m_ImportedVmeMesh);
}

//----------------------------------------------------------------------------
mafOp* lhpOpImporterAnsysCDBFile::Copy()   
//----------------------------------------------------------------------------
{
  lhpOpImporterAnsysCDBFile *cp = new lhpOpImporterAnsysCDBFile(m_Label);
  return cp;
}
//----------------------------------------------------------------------------
mafString lhpOpImporterAnsysCDBFile::GetWildcard()
//----------------------------------------------------------------------------
{
  return "cdb files (*.cdb)|*.cdb|All Files (*.*)|*.*";
}

//----------------------------------------------------------------------------
int lhpOpImporterAnsysCDBFile::ParseAnsysFile(mafString fileName)
//----------------------------------------------------------------------------
{
  InitProgressBar("Please wait parsing CDBAnsys File...");
	
  ReadInit(fileName);

  FILE *nodesFile, *elementsFile, *materialsFile;
  nodesFile = fopen(m_NodesFileName, "w");
	if (!nodesFile)
	{
		mafLogMessage("Cannot Open: %s",m_NodesFileName.c_str());
		return MAF_ERROR;
	}
  elementsFile = fopen(m_ElementsFileName, "w");
	if (!elementsFile)
	{
		mafLogMessage("Cannot Open: %s",m_ElementsFileName.c_str());
		return MAF_ERROR;
	}
  materialsFile = fopen(m_MaterialsFileName, "w");
	if (!materialsFile)
	{
		mafLogMessage("Cannot Open: %s",m_MaterialsFileName.c_str());
		return MAF_ERROR;
	}

	int lineLenght;

  m_CurrentMatId = -1;

  while ((lineLenght = GetLine(m_FilePointer, m_Line)) != 0) 
  {
    if(strncmp (m_Line,"NBLOCK,",7) == 0)
    {
      ReadNBLOCK(nodesFile);
    }

    if(strncmp (m_Line,"EBLOCK,",7) == 0)
    {
      ReadEBLOCK(elementsFile);
    }

    if(strncmp (m_Line,"MPDATA,",7) == 0)
    {
      ReadMPDATA(materialsFile);
    }
  }

  fclose(nodesFile);
  fclose(elementsFile);
  fclose(materialsFile);

	ReadFinalize();
  CloseProgressBar();

  return MAF_OK;
}