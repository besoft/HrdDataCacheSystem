/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpImporterAnalogASCII.cpp,v $
  Language:  C++
  Date:      $Date: 2009-11-03 13:00:35 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Alberto Losi
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
#include "lhpOpImporterAnalogASCII.h"

#include <wx/busyinfo.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>
#include <wx/wfstream.h>
#include "mafGUI.h"

#include "mafTagArray.h"
#include "lhpVMEScalarMatrix.h"

#include <fstream>
#include <iostream>
#include <string>

#include <vnl\vnl_matrix.h>
#include <vnl\vnl_vector.h>

using namespace std;

#define TAG_FORMAT "VPH2_ANALOG"
// #define TAG_NUM_T "num_t:"
// #define TAG_NUM_X "num_x:"
#define TAG_T "t:"
#define TAG_X "x:"

//----------------------------------------------------------------------------
lhpOpImporterAnalogASCII::lhpOpImporterAnalogASCII(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
	m_OpType	= OPTYPE_IMPORTER;
	m_Canundo	= true;
	m_File		= "";
	m_FileDir = (mafGetApplicationDirectory() + "/Data/External/").c_str();

  m_Scalar = NULL;
}
//----------------------------------------------------------------------------
lhpOpImporterAnalogASCII::~lhpOpImporterAnalogASCII()
//----------------------------------------------------------------------------
{
  mafDEL(m_Scalar);
}
//----------------------------------------------------------------------------
mafOp* lhpOpImporterAnalogASCII::Copy()   
//----------------------------------------------------------------------------
{
	lhpOpImporterAnalogASCII *cp = new lhpOpImporterAnalogASCII(m_Label);
	cp->m_File = m_File;
  cp->m_FileDir = m_FileDir;
	return cp;
}
//----------------------------------------------------------------------------
void lhpOpImporterAnalogASCII::OpRun()   
//----------------------------------------------------------------------------
{
	int result = OP_RUN_CANCEL;
	m_File = "";
	wxString pgd_wildc	= "ASCII File (*.*)|*.*";
  wxString f;
  f = mafGetOpenFile(m_FileDir,pgd_wildc).c_str(); 
	if(!f.IsEmpty() && wxFileExists(f))
	{
	  m_File = f;
    Read();
    result = OP_RUN_OK;
  }
  mafEventMacro(mafEvent(this,result));
}

//----------------------------------------------------------------------------
void lhpOpImporterAnalogASCII::Read()   
//----------------------------------------------------------------------------
{
  // TODO: add a trim to file lines
  if (!m_TestMode)
  {
	  wxBusyInfo wait("Please wait, working...");
	  mafEventMacro(mafEvent(this,PROGRESSBAR_SHOW));
  }
  
  mafNEW(m_Scalar);
  wxString path, name, ext;
  wxSplitPath(m_File.c_str(),&path,&name,&ext);
  m_Scalar->SetName(name);

  mafLogMessage(wxString::Format("start_import %s", name).c_str());

  mafTagItem tag_Nature;
  tag_Nature.SetName("VME_NATURE");
  tag_Nature.SetValue("NATURAL");

  m_Scalar->GetTagArray()->SetTag(tag_Nature);

  wxFileInputStream inputFile( m_File );
  wxTextInputStream text( inputFile );

  double val_scalar;
  wxString line;
  std::vector<double> xVec;
  std::vector<double> timeVec;

  // Check if file starts with the string "VPH2_ANALOG"
  line = text.ReadLine(); 
  if (line.CompareTo(TAG_FORMAT)!= 0)
  {
    mafErrorMessage("Invalid file format!");
    return;
  }

  //Put the t values in a vector of double
  int num_t;
  line = text.ReadLine();
  wxString str_t_values = line.SubString(strlen(TAG_T),line.Len()); // Skip "t:" tag
  wxStringTokenizer tkzTime(str_t_values,wxT(','),wxTOKEN_RET_EMPTY_ALL); // Read t values
  num_t = tkzTime.CountTokens();

  while (tkzTime.HasMoreTokens())
  {
    timeVec.push_back(atof(tkzTime.GetNextToken().c_str()));  // Convert to double
  }

  // Put the x values in a vector of string
  int num_x;
  line = text.ReadLine();
  wxString str_x_values = line.SubString(strlen(TAG_X),line.Len()); // Skip "x:" tag
  wxStringTokenizer tkzX(str_x_values,wxT(','),wxTOKEN_RET_EMPTY_ALL); // Read x values
  num_x = tkzX.CountTokens();

  while (tkzX.HasMoreTokens())
  {
    xVec.push_back(atof(tkzX.GetNextToken())); 
  }

  int space;
  wxString frame;



  vnl_vector<double> x_vector;
  x_vector.set_size(num_x);
	for(int j = 0; j < num_x; j++) // Fit the matrix's first row with x values! //time stamps
	{
		double x = xVec[j];
    x_vector.put(j,x);
	}

  vnl_matrix<double> y_matrix;
  y_matrix.set_size(num_x,num_t);
  
  for(int x = 0; x < num_x; x++)
  {
    int t = 0;
    line = text.ReadLine();
    line.Replace(","," ");
    space = line.Find(' ');
    frame = line.SubString(0,space - 1);
    wxStringTokenizer tkzY(line,wxT(' '),wxTOKEN_RET_EMPTY_ALL);

    while (tkzY.HasMoreTokens())
    {
      val_scalar = atof(tkzY.GetNextToken().c_str());
      if(!(t < num_t))
      {
        mafErrorMessage("Invalid file format!");
        m_Scalar = NULL;
        if (!m_TestMode)
        {
          mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));
        }
        return;
      }
      y_matrix.put(x,t,val_scalar); // Add scalar value to the vnl_matrix
      t++;
    }

    if (!m_TestMode)
    {
      mafEventMacro(mafEvent(this,PROGRESSBAR_SET_VALUE,(long)(((double) x)/((double) num_x)*100.)));
    }
    line.Replace(","," ");
  }

  for(int t = 0; t < num_t; t++)
  {
    vnl_matrix<double> matrix;
    matrix.set_size(2, num_x);
    matrix.set_row(0,x_vector);
    matrix.set_row(1,y_matrix.get_column(t));
    m_Scalar->SetData(matrix, timeVec[t]);
  }
  if (!m_TestMode)
  {
    mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));
  }
  m_Output = m_Scalar;
  m_Output->ReparentTo(m_Input);
  mafLogMessage("end_import");
}