/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafGraphIDDesc.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:56 $
  Version:   $Revision: 1.1 $
  Authors:   Vladik Aranov    
  Purpose:   class for variables descriptions header 
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __mafGraphIDDesc_H___
#define __mafGraphIDDesc_H___
    
#include <wx/menu.h>
#include "mafVME.h"
#include "mafViewIntGraphWindow.h"

//----------------------------------------------------------------------------
// Constant
//----------------------------------------------------------------------------
enum mafGraphDescType
{
  GDT_NA     = -1,
  GDT_FRAME  = 0,

  //GTM
  GDT_GTM_POSX   ,
  GDT_GTM_POSY   ,
  GDT_GTM_POSZ   ,
  GDT_GTM_ROTX   ,
  GDT_GTM_ROTY   ,
  GDT_GTM_ROTZ   ,

  //LTM
  GDT_LTM_POSX   ,
  GDT_LTM_POSY   ,
  GDT_LTM_POSZ   ,
  GDT_LTM_ROTX   ,
  GDT_LTM_ROTY   ,
  GDT_LTM_ROTZ   ,

  //OVP
  GDT_OVP_POSX   ,
  GDT_OVP_POSY   ,
  GDT_OVP_POSZ   ,
  GDT_OVP_ROTX   ,
  GDT_OVP_ROTY   ,
  GDT_OVP_ROTZ   ,

  //GES
  GDT_GES_POSX   ,
  GDT_GES_POSY   ,
  GDT_GES_POSZ   ,
  GDT_GES_ROTX   ,
  GDT_GES_ROTY   ,
  GDT_GES_ROTZ   ,

  //Euler
  GDT_EUL_ROTXXYZs ,
  GDT_EUL_ROTYXYZs ,
  GDT_EUL_ROTZXYZs ,

  GDT_EUL_ROTXXYXs ,
  GDT_EUL_ROTYXYXs ,
  GDT_EUL_ROTZXYXs ,

  GDT_EUL_ROTXXZYs ,
  GDT_EUL_ROTYXZYs ,
  GDT_EUL_ROTZXZYs ,

  GDT_EUL_ROTXXZXs ,
  GDT_EUL_ROTYXZXs ,
  GDT_EUL_ROTZXZXs ,

  GDT_EUL_ROTXYZXs ,
  GDT_EUL_ROTYYZXs ,
  GDT_EUL_ROTZYZXs ,

  GDT_EUL_ROTXYZYs ,
  GDT_EUL_ROTYYZYs ,
  GDT_EUL_ROTZYZYs ,

  GDT_EUL_ROTXYXZs ,
  GDT_EUL_ROTYYXZs ,
  GDT_EUL_ROTZYXZs ,

  GDT_EUL_ROTXYXYs ,
  GDT_EUL_ROTYYXYs ,
  GDT_EUL_ROTZYXYs ,

  GDT_EUL_ROTXZXYs ,
  GDT_EUL_ROTYZXYs ,
  GDT_EUL_ROTZZXYs ,

  GDT_EUL_ROTXZXZs ,
  GDT_EUL_ROTYZXZs ,
  GDT_EUL_ROTZZXZs ,

  GDT_EUL_ROTXZYXs ,
  GDT_EUL_ROTYZYXs ,
  GDT_EUL_ROTZZYXs ,

  GDT_EUL_ROTXZYZs ,
  GDT_EUL_ROTYZYZs ,
  GDT_EUL_ROTZZYZs ,


  GDT_EUL_ROTXZYXr ,
  GDT_EUL_ROTYZYXr ,
  GDT_EUL_ROTZZYXr ,

  GDT_EUL_ROTXXYXr ,
  GDT_EUL_ROTYXYXr ,
  GDT_EUL_ROTZXYXr ,

  GDT_EUL_ROTXYZXr ,
  GDT_EUL_ROTYYZXr ,
  GDT_EUL_ROTZYZXr ,

  GDT_EUL_ROTXXZXr ,
  GDT_EUL_ROTYXZXr ,
  GDT_EUL_ROTZXZXr ,

  GDT_EUL_ROTXXZYr ,
  GDT_EUL_ROTYXZYr ,
  GDT_EUL_ROTZXZYr ,

  GDT_EUL_ROTXYZYr ,
  GDT_EUL_ROTYYZYr ,
  GDT_EUL_ROTZYZYr ,

  GDT_EUL_ROTXZXYr ,
  GDT_EUL_ROTYZXYr ,
  GDT_EUL_ROTZZXYr ,

  GDT_EUL_ROTXYXYr ,
  GDT_EUL_ROTYYXYr ,
  GDT_EUL_ROTZYXYr ,

  GDT_EUL_ROTXYXZr ,
  GDT_EUL_ROTYYXZr ,
  GDT_EUL_ROTZYXZr ,

  GDT_EUL_ROTXZXZr ,
  GDT_EUL_ROTYZXZr ,
  GDT_EUL_ROTZZXZr ,

  GDT_EUL_ROTXXYZr ,
  GDT_EUL_ROTYXYZr ,
  GDT_EUL_ROTZXYZr ,

  GDT_EUL_ROTXZYZr ,
  GDT_EUL_ROTYZYZr ,
  GDT_EUL_ROTZZYZr ,

  //Helical Axis
  GDT_HEL_ROTX   ,
  GDT_HEL_ROTY   ,
  GDT_HEL_ROTZ   ,

  //control
  GDT_LAST
};
//----------------------------------------------------------------------------
// Forward definitions
//----------------------------------------------------------------------------

class mafPipeIntGraph;
//----------------------------------------------------------------------------
// Classes
//----------------------------------------------------------------------------

WX_DEFINE_ARRAY(mafPipeIntGraph *, PipeArray);

class mafGraphIDDesc: public mafIDDesc
{
public:
  mafGraphIDDesc(PipeArray *vmpVME);
  void   GetIDDesc(const IDType& nID,char *sDescript,unsigned int nLength);
  wxMenu *GenerateMenu(mafMemoryGraph *pmgGraph, unsigned int nIDBase);
  bool   ProcessMenu(mafMemoryGraph *pmgGraph, unsigned int nIDCommand);
  IDType GetIDByCommand(unsigned int command);
  unsigned int GetCommandByID(const IDType& ID);
private:
  wxMenu *GenerateMenu(unsigned int nIDBase, unsigned int nExtended);
  ///array of currently active pipes
  PipeArray *m_currVME;
};

#endif // __mafGraphIDDesc_H___