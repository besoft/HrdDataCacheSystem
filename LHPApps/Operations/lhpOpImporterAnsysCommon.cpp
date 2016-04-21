/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpImporterAnsysCommon.cpp,v $
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

#include "lhpBuilderDecl.h"
#include "lhpUtils.h"

#include "lhpOpImporterAnsysCommon.h"

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
#include "wx/filename.h"

//buffer size 1024*1024 
#define READ_BUFFER_SIZE 1048576 
#define min(a,b)  (((a) < (b)) ? (a) : (b))


//----------------------------------------------------------------------------
lhpOpImporterAnsysCommon::lhpOpImporterAnsysCommon(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{ 
  m_OpType  = OPTYPE_IMPORTER;
  m_Canundo = true;
  m_ImporterType = 0;
  m_ImportedVmeMesh = NULL;
  m_Output=NULL;

  m_NodesFileName = "";
  m_ElementsFileName = "";
  m_MaterialsFileName = "";

	m_DataDir = lhpUtils::lhpGetApplicationDirectory() + "\\Data";
  m_CacheDir = m_DataDir + "\\AnsysReaderCache";
  m_AnsysInputFileNameFullPath		= "";
  m_FileDir = lhpUtils::lhpGetApplicationDirectory() + "/Data/External/";

  m_BusyInfo = NULL;
}

//----------------------------------------------------------------------------
lhpOpImporterAnsysCommon::~lhpOpImporterAnsysCommon()
//----------------------------------------------------------------------------
{
  mafDEL(m_ImportedVmeMesh);
}
//----------------------------------------------------------------------------
bool lhpOpImporterAnsysCommon::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return true;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void lhpOpImporterAnsysCommon::OpRun()   
//----------------------------------------------------------------------------
{  
  mafString wildcard = GetWildcard();

  int result = OP_RUN_CANCEL;
  m_AnsysInputFileNameFullPath = "";

  wxString f;
  f = mafGetOpenFile(m_FileDir, wildcard).c_str(); 
  if(!f.IsEmpty() && wxFileExists(f))
  {
    m_AnsysInputFileNameFullPath = f;
    Import();
    result = OP_RUN_OK;
  }
  mafEventMacro(mafEvent(this,result));  
}

//---------------------------------------------------------------------------
void lhpOpImporterAnsysCommon::InitProgressBar(wxString label ="")
  //----------------------------------------------------------------------------
{
  m_OperationProgress = 0;

  if (GetTestMode() == false)
  {
    m_BusyInfo = new wxBusyInfo(label);
    mafEventMacro(mafEvent(this,PROGRESSBAR_SHOW));
  }
  else
  {
    printf("\n" + label + "\n");
    printf("%c", 179);
  }
}
//---------------------------------------------------------------------------
void lhpOpImporterAnsysCommon::CloseProgressBar()
  //----------------------------------------------------------------------------
{
  if (GetTestMode() == false)
  {
    cppDEL(m_BusyInfo);
    mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));
  }
  else
  {
    printf("%c\n", 179);
  }

  m_OperationProgress = 0;
}
//---------------------------------------------------------------------------
void lhpOpImporterAnsysCommon::UpdateProgressBar(long progress)
  //----------------------------------------------------------------------------
{
  if (GetTestMode() == false)
  {
    if(progress != m_OperationProgress)
    {
      m_OperationProgress = progress;
      mafEventMacro(mafEvent(this,PROGRESSBAR_SET_VALUE,progress));
    }
  }
  else
  {
    while(progress - m_OperationProgress > 2)
    {
      m_OperationProgress += 2;
      printf("%c", 177);
    }
  }
}

//----------------------------------------------------------------------------
void lhpOpImporterAnsysCommon::OnEvent(mafEventBase *maf_event) 
  //----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
    case wxOK:
      {
        this->Import();
        this->OpStop(OP_RUN_OK);
      }
      break;
    case wxCANCEL:
      {
        this->OpStop(OP_RUN_CANCEL);
      }
      break;
    default:
      mafEventMacro(*e);
      break;
    }	
  }
}
//----------------------------------------------------------------------------
int lhpOpImporterAnsysCommon::Import()
//----------------------------------------------------------------------------
{
  mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );

  m_NodesFileName = m_CacheDir + "\\nodes.lis" ;
  m_ElementsFileName = m_CacheDir + "\\elements.lis" ;
  m_MaterialsFileName = m_CacheDir + "\\materials.lis" ;

  // Create tmp path
  mkdir(m_DataDir);
	
	if(!wxDirExists(m_DataDir))
	{
		mafLogMessage("Cloud not create \"Data\" Directory");
		return MAF_ERROR;
	}

	mkdir(m_CacheDir);
	if(!wxDirExists(m_DataDir))
	{
		mafLogMessage("Cloud not create Read Cache Directory");
		return MAF_ERROR;
	}



  // Parsing Ansys File
  if(ParseAnsysFile(m_AnsysInputFileNameFullPath.c_str()) == MAF_ERROR)
  {
    return MAF_ERROR;
  }

  if (GetTestMode() == false)
  {
    m_BusyInfo = new wxBusyInfo("Please wait importing VME Mesh AnsysText...");
  }

  mafVMEMeshAnsysTextImporter *reader = new mafVMEMeshAnsysTextImporter;
	reader->SetNodesFileName(m_NodesFileName.c_str());
  reader->SetElementsFileName(m_ElementsFileName.c_str());
  reader->SetMaterialsFileName(m_MaterialsFileName.c_str());
	int returnValue = reader->Read();

  if (returnValue == MAF_ERROR)
  {
    if (!m_TestMode)
    {
      mafMessage(_("Error parsing input files! See log window for details..."),_("Error"));
    }
  } 
  else if (returnValue == MAF_OK)
  {
    wxString name, path, ext;
    wxFileName::SplitPath(m_AnsysInputFileNameFullPath, &path, &name, &ext);

    mafNEW(m_ImportedVmeMesh);
    m_ImportedVmeMesh->SetName(name);
	  m_ImportedVmeMesh->SetDataByDetaching(reader->GetOutput()->GetUnstructuredGridOutput()->GetVTKData(),0);

    mafTagItem tag_Nature;
    tag_Nature.SetName("VME_NATURE");
    tag_Nature.SetValue("NATURAL");
    m_ImportedVmeMesh->GetTagArray()->SetTag(tag_Nature);

    m_Output = m_ImportedVmeMesh;
  }

  if (GetTestMode() == false)
  {
    cppDEL(m_BusyInfo);
  }

  delete reader;
  return returnValue;
}

//----------------------------------------------------------------------------
int lhpOpImporterAnsysCommon::ReadNBLOCK(FILE *outFile)
//----------------------------------------------------------------------------
{
  char blockName[254];
  int maxId = 0, numElements = 0, nodeId, nodeSolidModelEntityId, nodeLine;
  double nodeX, nodeY, nodeZ;

  // NBLOCK,6,SOLID,   2596567,    25   or
  // NBLOCK,6,SOLID

  int nReaded = sscanf(m_Line, "%s %d, %d", blockName, &maxId, &numElements);

  GetLine(m_FilePointer, m_Line); // (3i8,6e20.13) Line ignored

  if(nReaded == 1)
  {
    // CDB from Hypermesh

    while (GetLine(m_FilePointer, m_Line) != 0 && strncmp (m_Line,"N,R5",4) != 0)
    {
      nodeId = nodeSolidModelEntityId = nodeLine = 0;
      nodeX = nodeY = nodeZ = 0;

      //15239 0 0 112.48882247215 -174.4868225037 -378.3886770441 0.0 0.0 0.0
      sscanf(m_Line, "%d %d %d %lf %lf %lf", &nodeId, &nodeSolidModelEntityId, &nodeLine, &nodeX, &nodeY, &nodeZ);

      fprintf(outFile,"%d\t%.13E\t%.13E\t%.13E\n", nodeId, nodeX, nodeY, nodeZ);
    }
  }
  else
  {
    // CDB from Ansys

    for (int i = 0; i < numElements; i++)
    {
      if(GetLine(m_FilePointer, m_Line) != 0)
      {
        nodeId = nodeSolidModelEntityId = nodeLine = 0;
        nodeX = nodeY = nodeZ = 0;

        sscanf(m_Line, "%d %d %d %lf %lf %lf", &nodeId, &nodeSolidModelEntityId, &nodeLine, &nodeX, &nodeY, &nodeZ);

        fprintf(outFile,"%d\t%.13E\t%.13E\t%.13E\n", nodeId, nodeX, nodeY, nodeZ);
      }
    } 
  }

  return MAF_OK;
}
//----------------------------------------------------------------------------
int lhpOpImporterAnsysCommon::ReadEBLOCK(FILE *outFile)
//----------------------------------------------------------------------------
{
  char blockName[254];
  int maxId = 0, numElements = 0;
  int idMaterial,idTypeElement,idConstants,nNodes, elementNumber, nodes[16], unused;

  // EBLOCK,19,SOLID,   3436163,    234932   / EBLOCK,19,SOLID,   3436163     or 
  // EBLOCK,19,SOLID,
  int nReaded = sscanf(m_Line, "%s %d, %d", blockName, &maxId, &numElements);

  GetLine(m_FilePointer, m_Line); //(19i8) Line ignored
  
  if(nReaded < 3)
  {
    // CDB from Hypermesh

    while (GetLine(m_FilePointer, m_Line) != 0 )
    {
      int readedElem=sscanf(m_Line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &idMaterial,&idTypeElement,&idConstants,&unused,&unused,&unused,&unused,&unused,&nNodes,&unused,&elementNumber,nodes+0,nodes+1,nodes+2,nodes+3,nodes+4,nodes+5,nodes+6,nodes+7);

      if (readedElem < 2)
        break;

      if(nNodes > 8)
      {
        GetLine(m_FilePointer, m_Line);
        sscanf(m_Line, "%d %d %d %d %d %d %d %d", nodes+8,nodes+9,nodes+10,nodes+11,nodes+12,nodes+13,nodes+14,nodes+15);
      }

      fprintf(outFile,"%d\t%d\t%d\t%d\t%d\t%d", elementNumber,idMaterial,idTypeElement,idConstants,0,1);

      for (int j=0;j<nNodes;j++)
      {
        fprintf(outFile,"\t%d",nodes[j]);
      }

      fprintf(outFile,"\n");
    }
  }
  else
  {
    // CDB from Ansys

    for (int index = 0; index < numElements; index++)
    {
      if(GetLine(m_FilePointer, m_Line) != 0)
      {
        sscanf(m_Line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &idMaterial,&idTypeElement,&idConstants,&unused,&unused,&unused,&unused,&unused,&nNodes,&unused,&elementNumber,nodes+0,nodes+1,nodes+2,nodes+3,nodes+4,nodes+5,nodes+6,nodes+7);

        if(nNodes > 8)
        {
          GetLine(m_FilePointer, m_Line);
          sscanf(m_Line, "%d %d %d %d %d %d %d %d", nodes+8,nodes+9,nodes+10,nodes+11,nodes+12,nodes+13,nodes+14,nodes+15);
        }

        fprintf(outFile,"%d\t%d\t%d\t%d\t%d\t%d", elementNumber,idMaterial,idTypeElement,idConstants,0,1);

        for (int j=0;j<nNodes;j++)
        {
          fprintf(outFile,"\t%d",nodes[j]);
        }

        fprintf(outFile,"\n");
      }
    } 
  }  

  return MAF_OK;
}
//----------------------------------------------------------------------------
int lhpOpImporterAnsysCommon::ReadMPDATA(FILE *outFile)
  //----------------------------------------------------------------------------
{
  char matName[254], unusedStr[254], matValue[254];
  int matId;

  //MPDATA,EX  ,2,1,          1000.0  or MP,EX  ,2,1,          1000.0
  //MPDATA,R5.0, 1,EX  ,       1, 1,  26630.9000 
  int commaNum = ReplaceInString(m_Line, ',', ' ');

  if(commaNum <= 4)
  {
    //MPDATA EX   2 1           1000.0  or //MP EX   2 1           1000.0   
    sscanf(m_Line, "%s %s %d %s", unusedStr, matName, &matId, matValue);
  }
  else
  {
    //MPDATA R5.0  1 EX          1  1   26630.9000    
    sscanf(m_Line, "%s %s %s %s %d %s %s", unusedStr, unusedStr, unusedStr, matName, &matId, unusedStr, matValue);
  }

  if(matId != m_CurrentMatId)
  {  
    if(m_CurrentMatId != -1)
      fprintf(outFile,"\n");

    fprintf(outFile,"MATERIAL NUMBER =      %d EVALUATED AT TEMPERATURE OF   0.0  \n", matId);
    m_CurrentMatId = matId;
  }

  fprintf(outFile,"%s = %s\n", matName, matValue);

  return MAF_OK; 
}


int lhpOpImporterAnsysCommon::GetLine(FILE *fp, char *lineBuffer)
{
  char readValue;
  int readedChars=0;

  do 
  {
		if (m_BufferLeft == 0)
		{
			m_BufferLeft = fread(m_Buffer,sizeof(char),READ_BUFFER_SIZE,m_FilePointer);
			m_BufferPointer = 0;
			//Breaks if EOF is reached
			if(m_BufferLeft==0)
				break;
		}

		lineBuffer[readedChars]=readValue=m_Buffer[m_BufferPointer];
		readedChars++;
		m_BufferPointer++;
		m_BufferLeft--;
  } while (readValue != '\n');

  lineBuffer[readedChars]=0;

	//Windows translate CR/LF into CR char we need to count a char more for each newline
	m_BytesReaded+=readedChars+1; 
  UpdateProgressBar(((double)m_BytesReaded) * 100 / m_FileSize);

  return readedChars;
}
//----------------------------------------------------------------------------
int lhpOpImporterAnsysCommon::ReplaceInString(char *str, char from, char to)
//----------------------------------------------------------------------------
{
  int count=0;

  for (int i=0; str[i]!= '\0'; i++)
  {
    if (str[i] == from)
    {
      str[i]=to;
      count++;
    }
  }

  return count;
}
//----------------------------------------------------------------------------
int lhpOpImporterAnsysCommon::ReadInit(mafString &fileName)
//----------------------------------------------------------------------------
{
	m_FilePointer = fopen(fileName.GetCStr(), "r");

	if (m_FilePointer == NULL)
	{
		mafMessage(_("Error parsing input files! File not found."),_("Error"));
		return MAF_ERROR;
	}

	// Calculate file size
	fseek(m_FilePointer, 0L, SEEK_END);
	m_FileSize = ftell(m_FilePointer);
	fseek(m_FilePointer, 0L, SEEK_SET);

	m_Buffer=new char[READ_BUFFER_SIZE];
	m_BytesReaded = m_BufferLeft = m_BufferPointer = 0;
}
//----------------------------------------------------------------------------
void lhpOpImporterAnsysCommon::ReadFinalize()
//----------------------------------------------------------------------------
{
	delete [] m_Buffer;
	fclose(m_FilePointer);
}

