/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafInpReader.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "mafINPReader.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkFloatArray.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkCxxRevisionMacro(mafINPReader, "$Revision: 1.1 $");
vtkStandardNewMacro(mafINPReader);

// Construct object with merging set to true.
//----------------------------------------------------------------------------
mafINPReader::mafINPReader()
//----------------------------------------------------------------------------
{
  m_FileName = NULL;
}

//----------------------------------------------------------------------------
mafINPReader::~mafINPReader()
//----------------------------------------------------------------------------
{
  if (this->m_FileName)
  {
    delete [] this->m_FileName;
    this->m_FileName = NULL;
  }
}

//----------------------------------------------------------------------------
void mafINPReader::PrintSelf(ostream& os, vtkIndent indent)
//----------------------------------------------------------------------------
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: " 
     << (this->m_FileName ? this->m_FileName : "(none)") << "\n";
  
}

//----------------------------------------------------------------------------
void mafINPReader::Execute()
//----------------------------------------------------------------------------
{
  FILE         *fp;
  vtkPoints    *newPts;
  vtkCellArray *newPolys;
  vtkPolyData  *output = this->GetOutput();
  
  // All of the data in the first piece.
  if(output->GetUpdatePiece() > 0)
  {
    return;
  }
  
  if(!this->m_FileName)
  {
    vtkErrorMacro(<<"A FileName must be specified.");
    return;
  }

  // Initialize
  //
  fp = fopen(m_FileName, "rt");
  if(fp == NULL)
  {
    vtkErrorMacro(<< "File " << this->m_FileName << " not found");
    return;
  }

  std::vector<double> pointsRead;
  std::vector<wxInt32> pointsIndRead;
  std::vector<vtkIdType> indRead;
  // Depending upon file type, read differently
  //
  if(this->ReadASCIIINP(fp,pointsRead,pointsIndRead,indRead) == FALSE)
  {
    return;
  }

  newPts = vtkPoints::New();
  newPts->Allocate(pointsRead.size() / 3,10000);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(indRead.size() / 3,20000);
  double x[3];
  for(unsigned i = 0; i < pointsRead.size() / 3; i++)
  {
    newPts->InsertNextPoint(x);
  }
  for(unsigned i = 0; i < pointsRead.size() / 3; i++)
  {
    newPts->SetPoint(pointsIndRead[i], pointsRead[3 * i + 0], pointsRead[3 * i + 1], pointsRead[3 * i + 2]);
  }
  for(unsigned i = 0; i < indRead.size() / 3; i++)
  {
    vtkIdType ind[3];
    for(unsigned j = 0; j < 3; j++)
      ind[j] = indRead[3 * i + j];
    newPolys->InsertNextCell(3, ind);
  }

  vtkDebugMacro(<< "Read: " 
  << newPts->GetNumberOfPoints() << " points, "
  << newPolys->GetNumberOfCells() << " triangles");

  fclose(fp);
  
//
// Update ourselves
//
  output->SetPoints(newPts);
  output->SetPolys(newPolys);
  output->Squeeze();
  newPts->Delete();
  newPolys->Delete();
}

#define MAX_LINE 1000
//----------------------------------------------------------------------------
int mafINPReader::ReadASCIIINP(FILE *fp, std::vector<double> &pointsRead, std::vector<int> &pointsIndRead, std::vector<vtkIdType> &indRead)//vtkPoints *newPts, vtkCellArray *newPolys)
//----------------------------------------------------------------------------
{
  double    x[3];
  vtkIdType pts[3];
  int       indices[3];
  wxInt32   nI, nJ;
  wxInt32   nPointID, nPointsRead;
  char      sLine[MAX_LINE];
  wxInt32   nPointsNumber, nTrgRead, nTrgID;
  wxInt32   nTrgNumber;
  wxInt32   nUnkNumber1;
  wxInt32   nUnkNumber2;
  wxInt32   nUnkNumber3;
  char      *pRet;

  vtkDebugMacro(<< " Reading ASCII STL file");

  //just read them line by line for first uncommented string
  nI = 0;
  while(TRUE)
  {
    pRet = fgets(sLine, MAX_LINE, fp);
    if(pRet == NULL)
    {
      return FALSE;
    }
    if(sLine[0] != '#')
    {
      break;
    }
    nI++;
  }

  sscanf(sLine, "%d %d %d %d %d\n", &nPointsNumber, &nTrgNumber, &nUnkNumber1, &nUnkNumber2, &nUnkNumber3);
  wxASSERT(nUnkNumber1 == 0 && nUnkNumber2 == 0 && nUnkNumber3 == 0);
  for(nJ = 0; nJ < nPointsNumber; nJ++)
  {
    pointsRead.push_back(x[0]);
    pointsRead.push_back(x[1]);
    pointsRead.push_back(x[2]);
  }
  //to first data
  nI++;
  nPointsRead = 0;
  while(TRUE)
  {
    pRet = fgets(sLine, MAX_LINE, fp);
    if(pRet == NULL)
    {
      break;
    }
    if(sLine[0] == '#')
    {
      nI++;
    }
    sscanf(sLine, "%d %lf %lf %lf", &nPointID, x, x+1, x+2);
    pointsRead[3 * nPointsRead + 0] = x[0];
    pointsRead[3 * nPointsRead + 1] = x[1];
    pointsRead[3 * nPointsRead + 2] = x[2];

    pointsIndRead.push_back(nPointID);

    nPointsRead++;
    nI++;
    if(nPointsRead >= nPointsNumber)
    {
      break;
    }
  }
  //read indices
  nTrgRead = 0;
  while(TRUE)
  {
    pRet = fgets(sLine, MAX_LINE, fp);
    if(pRet == NULL)
    {
      break;
    }
    if(sLine[0] == '#')
    {
      nI++;
    }
    sscanf(sLine, "%d %d %*s %d %d %d", &nTrgID, &nUnkNumber1, indices, indices + 1, indices + 2);
    //wxASSERT(nUnkNumber1 == 0);
    pts[0] = indices[0];
    pts[1] = indices[1];
    pts[2] = indices[2];

    indRead.push_back(pts[0]);
    indRead.push_back(pts[1]);
    indRead.push_back(pts[2]);

    nTrgRead++;
    nI++;
    if(nTrgRead >= nTrgNumber)
    {
      break;
    }
  } 
  return TRUE;
}
