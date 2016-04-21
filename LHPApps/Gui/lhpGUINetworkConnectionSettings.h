/*=========================================================================
Program:   LHPBuilder
Module:    $RCSfile: lhpGUINetworkConnectionSettings.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:52 $
Version:   $Revision: 1.1 $
Authors:   Daniele Giunchi
==========================================================================
Copyright (c) 2008
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#ifndef __lhpGUINetworkConnectionSettings_H__
#define __lhpGUINetworkConnectionSettings_H__

#include "mafGUISettings.h"

//----------------------------------------------------------------------------
// forward reference
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// lhpGUINetworkConnectionSettings :
//----------------------------------------------------------------------------
/**
  This class represents the panel attached to settings dialog, regarding Network Connection.
  It Contains:
  - Proxy parameters
*/
class lhpGUINetworkConnectionSettings : public mafGUISettings
{
public:
	lhpGUINetworkConnectionSettings(mafObserver *Listener, const mafString &label = _("Network Connection"));
	~lhpGUINetworkConnectionSettings(); 

  enum APP_SETTINGS_WIDGET_ID
  {
    ID_PROXY_FLAG = MINID,
    ID_PROXY_HOST,
    ID_PROXY_PORT,
  };

  /** Answer to the messages coming from interface. */
  void OnEvent(mafEventBase *maf_event);

  /** Select image type during saving of the views*/
  int GetProxyFlag(){return m_ProxyFlag;};

  /** Set Port in proxy connection*/
  void SetProxyFlag(int flag){m_ProxyFlag = flag;};

  /** Select image type during saving of the views*/
  mafString &GetProxyHost();

  /** Set Port in proxy connection*/
  void SetProxyHost(const char *proxyHost){m_ProxyHost = proxyHost;};

	/** Get Port in proxy connection*/
	int GetProxyPort();

  /** Set Port in proxy connection*/
  void SetProxyPort(int proxyPort){m_ProxyPort = proxyPort;};


protected:
  /** Create the GUI for the setting panel.*/
  void CreateGui();

  /** Initialize the application settings.*/
  void InitializeSettings();

  /** Used to enable/disable items according to the current widgets state.*/
  void EnableItems();

  // Connection variables
  int         m_ProxyFlag;///< Flag used to for enable proxy. 
  mafString   m_ProxyHost;///< Proxy Host
  int         m_ProxyPort;///< Proxy Host
  
};
#endif
