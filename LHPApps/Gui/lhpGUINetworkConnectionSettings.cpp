/*=========================================================================
Program:   LHPBuilder
Module:    $RCSfile: lhpGUINetworkConnectionSettings.cpp,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:52 $
Version:   $Revision: 1.1 $
Authors:   Daniele Giunchi
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

#include "lhpGUINetworkConnectionSettings.h"
#include "mafCrypt.h"

#include "mafDecl.h"
#include "mafGUI.h"

//----------------------------------------------------------------------------
lhpGUINetworkConnectionSettings::lhpGUINetworkConnectionSettings(mafObserver *Listener, const mafString &label):
mafGUISettings(Listener, label)
//----------------------------------------------------------------------------
{
  // Default values for the application.
  m_ProxyFlag = 0;
  m_ProxyHost = "";
  m_ProxyPort = 0;

  InitializeSettings();
}
//----------------------------------------------------------------------------
lhpGUINetworkConnectionSettings::~lhpGUINetworkConnectionSettings()
//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
void lhpGUINetworkConnectionSettings::CreateGui()
//----------------------------------------------------------------------------
{
  m_Gui = new mafGUI(this);
  m_Gui->Label(_("Connection Settings"));
  m_Gui->Bool(ID_PROXY_FLAG,_("using proxy"),&m_ProxyFlag,1);
  m_Gui->String(ID_PROXY_HOST,_("Host"),&m_ProxyHost,"");
  m_Gui->Integer(ID_PROXY_PORT,_("Port"),&m_ProxyPort,1);
  m_Gui->Label(_(""));
  m_Gui->Divider(2);
  EnableItems();
}
//----------------------------------------------------------------------------
void lhpGUINetworkConnectionSettings::EnableItems()
//----------------------------------------------------------------------------
{
  m_Gui->Enable(ID_PROXY_HOST,m_ProxyFlag != 0);
  m_Gui->Enable(ID_PROXY_PORT,m_ProxyFlag != 0);
}
//----------------------------------------------------------------------------
void lhpGUINetworkConnectionSettings::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  switch(maf_event->GetId())
  {
    case ID_PROXY_FLAG:
      m_Config->Write("EnableProxy",m_ProxyFlag);
    break;
    case ID_PROXY_HOST:
      m_Config->Write("ProxyHost",m_ProxyHost.GetCStr());
    break;
    case ID_PROXY_PORT:
      m_Config->Write("ProxyPort",m_ProxyPort);
    break;
    default:
      mafEventMacro(*maf_event);
    break; 
  }
  EnableItems();
  m_Config->Flush();
}
//----------------------------------------------------------------------------
void lhpGUINetworkConnectionSettings::InitializeSettings()
//----------------------------------------------------------------------------
{
  wxString string_item;
  long long_item;
  if(m_Config->Read("EnableProxy", &long_item))
  {
    m_ProxyFlag = long_item;
  }
  else
  {
    m_Config->Write("EnableProxy",m_ProxyFlag);
  }

  if(m_Config->Read("ProxyHost", &string_item))
  {
    m_ProxyHost = string_item.c_str();
  }
  else
  {
    m_Config->Write("ProxyHost",m_ProxyHost.GetCStr());
  }

  if(m_Config->Read("ProxyPort", &long_item))
  {
    m_ProxyPort = long_item;
  }
  else
  {
    m_Config->Write("ProxyPort",m_ProxyPort);
  }
  m_Config->Flush();
}
//----------------------------------------------------------------------------
mafString &lhpGUINetworkConnectionSettings::GetProxyHost()
//----------------------------------------------------------------------------
{
  return m_ProxyHost;
}
//----------------------------------------------------------------------------
int lhpGUINetworkConnectionSettings::GetProxyPort()
//----------------------------------------------------------------------------
{ 
  return m_ProxyPort;
}