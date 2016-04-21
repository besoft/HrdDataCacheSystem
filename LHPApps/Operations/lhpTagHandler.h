/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpTagHandler.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Stefano Perticoni - Daniele Giunchi
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/
#ifndef __lhpTagHandler_h
#define __lhpTagHandler_h

//----------------------------------------------------------------------------
// forward references:
//----------------------------------------------------------------------------
class mafVME;

//----------------------------------------------------------------------------
// includes :
//----------------------------------------------------------------------------
#include "mafObject.h"
#include "mafString.h"
#include "lhpUser.h"

class lhpTagHandlerInputOutputParametersCargo : public mafObject
{
public:
  mafTypeMacro(lhpTagHandlerInputOutputParametersCargo, mafObject);
  
  /** Set the input vme */
  void SetInputVme(mafVME* vme) {m_InputVme = vme;};
  mafVME *GetInputVme(){return m_InputVme;};

  /** Set the input user */
  void SetInputUser(mafUser* user) {m_InputUser = user;};
  mafUser *GetInputUser(){return m_InputUser;};

	/** Set the input storage */
	void SetInputMSF(const char *msfFile) {m_InputMSF = msfFile;};
	const char *GetInputMSF(){return m_InputMSF.GetCStr();};
  
  /** Called by the tag handler code: fill the auto string to be written in XML*/
  void SetTagHandlerGeneratedString(mafString tagHandlerGeneratedString) {m_TagHandlerGeneratedString = tagHandlerGeneratedString;};
  mafString GetTagHandlerGeneratedString() {return m_TagHandlerGeneratedString;};

private:
  mafVME* m_InputVme;
  mafUser* m_InputUser;
	mafString m_InputMSF;
  mafString m_TagHandlerGeneratedString;
  lhpTagHandlerInputOutputParametersCargo();

};

class lhpTagHandler : public mafObject
{
public:
  mafTypeMacro(lhpTagHandler,mafObject);

  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo){mafLogMessage("Please write handling code for this tag!");};  
  const char *GetTagName(){return m_TagName;};
  
    /** Set python.exe interpreter used at runtime*/
  void SetPythonExe(const char *pythonExe){m_PythonExe = pythonExe;};
  
  /** Get python.exe interpreter used at runtime*/
  mafString &GetPythonExe(){return m_PythonExe;};

  /** Set pythonw.exe interpreter used at runtime*/
  void SetPythonwExe(const char *pythonwExe){m_PythonwExe = pythonwExe;};

	/** Get pythonw.exe interpreter used at runtime*/
  mafString &GetPythonwExe() {return m_PythonwExe;};

  
protected:
  virtual void ExtractTagName();
  mafString m_TagName;

  mafString   m_PythonExe;///< python.exe full path
  mafString   m_PythonwExe;///< pythonw.exe full path

	mafString m_VMEUploaderDownloaderDir;

  lhpTagHandler();
};
#endif