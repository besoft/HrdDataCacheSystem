/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpKeyczarIntegrationTest.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Stefano Perticoni   
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpKeyczarIntegrationTest_H__
#define __lhpOpKeyczarIntegrationTest_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class mafVMEMesh;
class mafEvent;

//----------------------------------------------------------------------------
// lhpOpKeyczarIntegrationTest :
//----------------------------------------------------------------------------
/** */
class lhpOpKeyczarIntegrationTest : public mafOp
{
public:
	lhpOpKeyczarIntegrationTest(const wxString &label = "lhpOpKeyczarIntegrationTest");
	~lhpOpKeyczarIntegrationTest(); 
	
  mafTypeMacro(lhpOpKeyczarIntegrationTest, mafOp);

  virtual void OnEvent(mafEventBase *maf_event);

  mafOp* Copy();

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode *node);

  /** Builds operation's interface. */
	void OpRun();

protected:

  int Execute();
  virtual void CreateGui();  
  
  mafString m_PythonExe; //>python  executable
  mafString m_PythonwExe; //>pythonw  executable

};
#endif
