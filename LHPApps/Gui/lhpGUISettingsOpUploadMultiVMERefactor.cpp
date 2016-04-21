/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpGUISettingsOpUploadMultiVMERefactor.cpp,v $
Language:  C++
Date:      $Date: 2009-07-17 12:59:19 $
Version:   $Revision: 1.1.2.1 $
Authors:   Stefano Perticoni
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

#include "lhpGUISettingsOpUploadMultiVMERefactor.h"

#include "mafDecl.h"
#include "mafGUI.h" 

//----------------------------------------------------------------------------
lhpGUISettingsOpUploadMultiVMERefactor::lhpGUISettingsOpUploadMultiVMERefactor(mafObserver *Listener, const mafString &label):
mafGUISettings(Listener, label)
//----------------------------------------------------------------------------
{
	m_SleepTimeBetweenTwoVMEUploadInSeconds = 30;

	InitializeSettings();
}
//----------------------------------------------------------------------------
lhpGUISettingsOpUploadMultiVMERefactor::~lhpGUISettingsOpUploadMultiVMERefactor()
//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
void lhpGUISettingsOpUploadMultiVMERefactor::CreateGui()
//----------------------------------------------------------------------------
{
	m_Gui = new mafGUI(this);
  m_Gui->Divider();
  m_Gui->Label("Sleep time between two uploads in seconds: ");
  m_Gui->Double(ID_STEP,_(""),&m_SleepTimeBetweenTwoVMEUploadInSeconds);
	m_Gui->Divider(1);

	m_Gui->Update();
}

//----------------------------------------------------------------------------
void lhpGUISettingsOpUploadMultiVMERefactor::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
	switch(maf_event->GetId())
	{
    case ID_STEP:
    {
      m_Config->Write("SleepTimeBetweenTwoVMEUploadInSeconds",m_SleepTimeBetweenTwoVMEUploadInSeconds);
    }
    break;
	}
	
	m_Config->Flush();
}
//----------------------------------------------------------------------------
void lhpGUISettingsOpUploadMultiVMERefactor::InitializeSettings()
//----------------------------------------------------------------------------
{	
  double sleepTime;

  if(m_Config->Read("SleepTimeBetweenTwoVMEUploadInSeconds", &sleepTime))
  {
    m_SleepTimeBetweenTwoVMEUploadInSeconds=sleepTime;
  }
  else
  {
    m_Config->Write("SleepTimeBetweenTwoVMEUploadInSeconds",m_SleepTimeBetweenTwoVMEUploadInSeconds);
  }
	m_Config->Flush();
}
