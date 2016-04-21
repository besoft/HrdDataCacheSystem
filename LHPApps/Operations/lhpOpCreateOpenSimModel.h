/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpCreateOpenSimModel.h,v $
  Language:  C++
  Date:      $Date: 2011-06-22 17:42:30 $
  Version:   $Revision: 1.1.2.3 $
  Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpCreateOpenSimModel_H__
#define __lhpOpCreateOpenSimModel_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVMEExternalData;
class mafNode;

//----------------------------------------------------------------------------
// lhpOpCreateOpenSimModel :
//----------------------------------------------------------------------------
/** Create the openSim model which is a vme external data with a .osim linked text file :P */
class LHP_OPERATIONS_EXPORT lhpOpCreateOpenSimModel: public mafOp
{
public:
  lhpOpCreateOpenSimModel(const wxString &label = "CreateGroup");
  ~lhpOpCreateOpenSimModel(); 

  mafTypeMacro(lhpOpCreateOpenSimModel, mafOp);

  mafOp* Copy();

  bool Accept(mafNode *node);
  void OpRun();

  void OpDo();

  
  /** Build a VME OpenSim model from a text string: the text will be copied in vme external data and the
  vme will be added to the tree as inputVme child */
  static wxString GenerateOpenSimModelFromText( wxString inText, mafNode *inputVme );

  /** Build a VME OpenSim model from a text file: the text will be copied in vme external data and the
  vme will be added to the tree as inputVme child */
  static wxString GenerateOpenSimModelFromFile( wxString &inModelFileABSName , mafNode *inputVme );

protected: 

	static std::string GenerateRandomString(int length, bool letters, bool numbers, bool symbols);
	static bool wxMakePath(const wxString& path);

};
#endif
