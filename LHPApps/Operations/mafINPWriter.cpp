/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafINPWriter.cpp,v $
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

#include "mafINPWriter.h"

#include "vtkByteSwap.h"
#include "vtkCellArray.h"
#include "vtkErrorCode.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkTriangle.h"

#if !defined(_WIN32) || defined(__CYGWIN__)
# include <unistd.h> /* unlink */
#else
# include <io.h> /* unlink */
#endif

vtkCxxRevisionMacro(mafINPWriter, "$Revision: 1.1 $");
vtkStandardNewMacro(mafINPWriter);

//----------------------------------------------------------------------------
mafINPWriter::mafINPWriter()
//----------------------------------------------------------------------------
{
  this->FileType = VTK_ASCII;
}

//----------------------------------------------------------------------------
void mafINPWriter::WriteData()
//----------------------------------------------------------------------------
{
  vtkPoints *pts;
  vtkCellArray *polys;
  vtkPolyData *input = this->GetInput();

  polys = input->GetPolys();
  pts = input->GetPoints();
  if (pts == NULL || polys == NULL )
    {
    vtkErrorMacro(<<"No data to write!");
    return;
    }

  if ( this->FileName == NULL)
    {
    vtkErrorMacro(<< "Please specify FileName to write");
    this->SetErrorCode(vtkErrorCode::NoFileNameError);
    return;
    }

  if ( this->FileType == VTK_BINARY )
    {
    this->WriteBinaryINP(pts,polys);
    if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError)
      {
      vtkErrorMacro("Ran out of disk space; deleting file: "
                    << this->FileName);
      unlink(this->FileName);
      }
    }
  else
    {
    this->WriteAsciiINP(pts,polys);
    if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError)
      {
      vtkErrorMacro("Ran out of disk space; deleting file: "
                    << this->FileName);
      unlink(this->FileName);
      }
    }
}

static char header[]="# AVS UCD file\n# written by lhpBuilder\n#\n";

//----------------------------------------------------------------------------
void mafINPWriter::WriteAsciiINP(vtkPoints *pts, vtkCellArray *polys)
//----------------------------------------------------------------------------
{
  FILE      *fp;
  double    v1[3];
  vtkIdType npts = 0;
  vtkIdType *indx = 0;
  
  if ((fp = fopen(this->FileName, "w")) == NULL)
  {
    vtkErrorMacro(<< "Couldn't open file: " << this->FileName);
    this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
    return;
  }
//
//  Write header
//
  vtkIdType npnts = pts->GetNumberOfPoints();
  vtkIdType nclls = polys->GetNumberOfCells();

  vtkDebugMacro("Writing ASCII INP file");
  if (fprintf (fp, "%s%d %d 0 0 0\n", header, npnts, nclls) < 0)
  {
    fclose(fp);
    this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
    return;
  }

    for(vtkIdType i = 0; i < npnts; i++)
    {
      pts->GetPoint(i,v1);
      if (fprintf (fp, "%d %.6g %.6g %.6g\n", i, v1[0], v1[1], v1[2]) < 0)
      {
        fclose(fp);
        this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
        return;
      }
    }

    int counter = 0;
    for (polys->InitTraversal(); polys->GetNextCell(npts,indx); )
    {

      if (fprintf (fp, "%d 0 tri %d %d %d \n", counter++, indx[0], indx[1], indx[2]) < 0)
      {
        fclose(fp);
        this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
        return;
      }
    }
  fclose (fp);
}

//----------------------------------------------------------------------------
void mafINPWriter::WriteBinaryINP(vtkPoints *pts, vtkCellArray *polys)
//----------------------------------------------------------------------------
{
  FILE      *fp;
  double    v1[3];
  vtkIdType npts = 0;
  vtkIdType *indx = 0;

  if ((fp = fopen(this->FileName, "wb")) == NULL)
  {
    vtkErrorMacro(<< "Couldn't open file: " << this->FileName);
    this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
    return;
  }
  
  //Write header
  vtkDebugMacro("Writing Binary INP file");
  if (fwrite(header, 1, 41, fp) < 41)
  {
    fclose(fp);
    this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
    return;
  }
  vtkIdType npnts = pts->GetNumberOfPoints();
  vtkIdType nclls = polys->GetNumberOfCells();
  if(fwrite(&npnts, sizeof(vtkIdType), 1, fp) < 1)
  {
    fclose(fp);
    this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
    return;
  }
  if(fwrite(&nclls, sizeof(vtkIdType), 1, fp) < 1)
  {
    fclose(fp);
    this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
    return;
  }

  for(vtkIdType i = 0; i < npnts; i++)
  {
    pts->GetPoint(i,v1);
    if(fwrite(&i, sizeof(vtkIdType), 1, fp) < 1)
    {
      fclose(fp);
      this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
      return;
    }
    if(fwrite(v1, sizeof(double), 3, fp) < 3)
    {
      fclose(fp);
      this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
      return;
    }
  }

  int counter = 0;
  for (polys->InitTraversal(); polys->GetNextCell(npts,indx); )
  {
    if (fwrite (&counter, sizeof(int), 1, fp) < 1)
    {
      counter++;
      fclose(fp);
      this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
      return;
    }
    counter++;
    if (fwrite (indx, sizeof(vtkIdType), 3, fp) < 3)
    {
      fclose(fp);
      this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
      return;
    }
  }
  fclose (fp);
}

//----------------------------------------------------------------------------
void mafINPWriter::PrintSelf(ostream& os, vtkIndent indent)
//----------------------------------------------------------------------------
{
  this->Superclass::PrintSelf(os,indent);
}
