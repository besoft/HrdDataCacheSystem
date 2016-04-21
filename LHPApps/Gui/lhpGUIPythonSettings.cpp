/*=========================================================================
Program:   LHPBuilder
Module:    $RCSfile: lhpGUIPythonSettings.cpp,v $
Language:  C++
Date:      $Date: 2010-02-23 15:53:07 $
Version:   $Revision: 1.1.1.1.2.4 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2008
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpGUIPythonSettings.h"
#include "mafCrypt.h"

#include "mafDecl.h"
#include "mafGUI.h"
#include "lhpUtils.h"

lhpGUIPythonSettings::lhpGUIPythonSettings(mafObserver *Listener, const mafString &label):
mafGUISettings(Listener, label)
{
  // Default interpreter will be set at application startup
  //   m_Gui = NULL;

  m_EmbeddedPythonInterpreterAvailable = false;
  m_ForceEmbeddedPythonRuntime = 1;

  m_PythonExe = "UNSET PYTHON.EXE INTERPRETER";
  m_PythonwExe = "UNSET PYTHONW.EXE INTERPRETER";

  // first check for application embedded python runtime
  mafString embeddedPythonInterpreter = (lhpUtils::lhpGetApplicationDirectory() + "\\Python25\\python.exe").c_str();

  mafString embeddedPythonwInterpreter = (lhpUtils::lhpGetApplicationDirectory() + "\\Python25\\pythonw.exe").c_str();

  // otherwise try to find Python in its standard installation dir
  mafString defaultPythonDir  = "C:\\Python25";
  m_PythonExeDefaultInterpreter = defaultPythonDir + "\\python.exe";
  m_PythonwExeDefaultInterpreter = defaultPythonDir + "\\pythonw.exe";

  bool embeddedPythonAvailable = false;
 
  if (wxFileExists(embeddedPythonInterpreter.GetCStr()))
  {
    m_PythonExe = embeddedPythonInterpreter.GetCStr();
    embeddedPythonAvailable = true;
  }
  else if (wxFileExists(m_PythonExeDefaultInterpreter.GetCStr()))
  {
    m_PythonExe = m_PythonExeDefaultInterpreter.GetCStr();
  }
  else
  {
    wxMessageBox("No Python.exe Interpreter defined! Several app features will not work");
  }
  
  bool embeddedPythonwAvailable = false;

  if (wxFileExists(embeddedPythonwInterpreter.GetCStr()))
  {
    m_PythonwExe = embeddedPythonwInterpreter.GetCStr();
    embeddedPythonwAvailable = true;
  }
  else if (wxFileExists(m_PythonExeDefaultInterpreter.GetCStr()))
  {
    m_PythonwExe = m_PythonwExeDefaultInterpreter.GetCStr();
  }
  else
  {
    wxMessageBox("No Pythonw.exe Interpreter defined! Several app features will not work");
  }
  
  if (embeddedPythonAvailable && embeddedPythonwAvailable)
  {
    m_EmbeddedPythonInterpreterAvailable = true;
  }

  // read the configuration file: this will overwrite the 
  // previous python.exe guess
  InitializeSettings();
}

lhpGUIPythonSettings::~lhpGUIPythonSettings()
{
}

void lhpGUIPythonSettings::CreateGui()
{
  m_Gui = new mafGUI(this);
  m_Gui->Label(_("Python Settings"));
  m_Gui->Label(_(""));
  m_Gui->Label(_("python.exe interpreter full path"));
  m_Gui->String(ID_PYTHON_EXE,_(""),&m_PythonExe,1);
  m_Gui->Label(_(""));
  m_Gui->Label(_("pythonw.exe interpreter full path"));
  m_Gui->String(ID_PYTHONW_EXE,_(""),&m_PythonwExe,1);
  m_Gui->Label(_(""));
  m_Gui->Label(_("force embedded python interpreter"));
  m_Gui->Label(_("at application startup"));
  m_Gui->Bool(ID_FORCE_EMBEDDED_PYTHON, "" , &m_ForceEmbeddedPythonRuntime, 1);
  m_Gui->Divider(2);
  EnableItems(true);
}

void lhpGUIPythonSettings::EnableItems( bool enable )
{
  m_Gui->Enable(ID_PYTHON_EXE, enable);
  m_Gui->Enable(ID_PYTHONW_EXE, enable);
}

void lhpGUIPythonSettings::OnEvent(mafEventBase *maf_event)
{
  switch(maf_event->GetId())
  {
    case ID_PYTHON_EXE:
      if (Is_m_PythonExe_Existent())
      {
        m_Config->Write("m_PythonExe",m_PythonExe.GetCStr());
      }
        
    break;

    case ID_PYTHONW_EXE:
      if (Is_m_PythonwExe_Existent())
      {
        m_Config->Write("m_PythonwExe",m_PythonwExe.GetCStr());
      }
        
    case ID_FORCE_EMBEDDED_PYTHON:
      {
        m_Config->Write("m_ForceEmbeddedPythonRuntime", m_ForceEmbeddedPythonRuntime);
      }

    break;
    default:
      mafEventMacro(*maf_event);
    break; 
  }
  
  m_Config->Flush();
}

void lhpGUIPythonSettings::InitializeSettings()
{
  int intItem;

  if(m_Config->Read("m_ForceEmbeddedPythonRuntime", &intItem))
  {
    m_ForceEmbeddedPythonRuntime = intItem;
  }
  else
  {
    m_Config->Write("m_ForceEmbeddedPythonRuntime",m_ForceEmbeddedPythonRuntime);
  }

  if (m_EmbeddedPythonInterpreterAvailable && (m_ForceEmbeddedPythonRuntime == 1))
  {
    // mafLogMessage("Changing python runtime to the embedded one");  
    return;
  }

  wxString stringItem;

  if(m_Config->Read("m_PythonExe", &stringItem))
  {
    m_PythonExe = stringItem.c_str();
    Is_m_PythonExe_Existent();
   }
  else
  {
    m_Config->Write("m_PythonExe",m_PythonExe);
  }
   
  if(m_Config->Read("m_PythonwExe", &stringItem))
  {
    m_PythonwExe = stringItem.c_str();
    Is_m_PythonwExe_Existent();

  }
  else
  {
    m_Config->Write("m_PythonwExe",m_PythonwExe.GetCStr());
  }

  m_Config->Flush();
}

bool lhpGUIPythonSettings::Is_m_PythonExe_Existent()
{
  bool pythonInterpreterExists = wxFileExists(m_PythonExe.GetCStr());
  if (!pythonInterpreterExists)
  {
    std::ostringstream stringStream;
    stringStream  << "Python Settings "<< m_PythonExe.GetCStr() << " interpreter not found. \
Setting interpreter to default: " << m_PythonExeDefaultInterpreter.GetCStr() << std::endl;
    mafLogMessage(stringStream.str().c_str());

    m_PythonExe = m_PythonExeDefaultInterpreter;
    m_Config->Write("m_PythonExe",m_PythonExe);
    if (m_Gui)
    {
      m_Gui->Update();
    }
  }
  
  return pythonInterpreterExists;
}

bool lhpGUIPythonSettings::Is_m_PythonwExe_Existent()
{
  bool pythonwInterpreterExists = wxFileExists(m_PythonwExe.GetCStr());
  if (!pythonwInterpreterExists)
  {
    std::ostringstream stringStream;
    stringStream  << "Pythonw Settings " << m_PythonwExe.GetCStr() << " interpreter not found. \
Setting interpreter to default: " << m_PythonwExeDefaultInterpreter.GetCStr() << std::endl;
    mafLogMessage(stringStream.str().c_str());

    m_PythonwExe = m_PythonwExeDefaultInterpreter;
    m_Config->Write("m_PythonwExe",m_PythonwExe);
    if (m_Gui)
    {
      m_Gui->Update();
    }   
  }

  return pythonwInterpreterExists;
}