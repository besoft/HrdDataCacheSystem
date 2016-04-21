/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpImporterAnalogASCII.h,v $
  Language:  C++
  Date:      $Date: 2009-11-03 13:00:35 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Alberto Losi
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpImporterAnalogASCII_H__
#define __lhpOpImporterAnalogASCII_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include <vnl\vnl_matrix.h>

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------

class lhpVMEScalarMatrix;

// Ported from vph2OpImporterAnalog
//----------------------------------------------------------------------------
// lhpOpImporterAnalogASCII :
//----------------------------------------------------------------------------
/** Import analog signals in a medVMEAnalog VME*/
class LHP_OPERATIONS_EXPORT lhpOpImporterAnalogASCII : public mafOp
{
public:
	lhpOpImporterAnalogASCII(const wxString &label = "Analog Importer");
	~lhpOpImporterAnalogASCII(); 
	mafOp* Copy();


	/** Return true for the acceptable vme type. */
	bool Accept(mafNode* node) {return true;};

	/** Builds operation's interface. */
	void OpRun();

  /** Read the file.
  File format:

  OLD
  VPH2_ANALOG
  t: t1,t2,..tn
  x: x1,x2,...,xm
  y11,y12,...,y1n
  y21,y22,...,y2n
  ...
  ym1,ym2,...,ymn

  The format of the file admits some specifics.
  1) Check if the first line contains "VPH2_ANALOG" tag
  4) Read the second line containing time stamps (ignore "t:" tag)
  5) Read the third line containing x values (ignore "x:" tag)
  6) Read the other lines that are y values. Each line correspond to an x value and each column to a time stamp
  7) Put the values in a medVMEAnalog item 
  */
  void Read();

  /** Set the filename for the file to import */
  void SetFileName(const char *file_name){m_File = file_name;};

protected:
  lhpVMEScalarMatrix *m_Scalar;
  vnl_matrix<double> m_Matrix;
  wxString m_FileDir;
	wxString m_File;
};
#endif
