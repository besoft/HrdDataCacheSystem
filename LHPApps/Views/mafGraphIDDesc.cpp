/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafGraphIDDesc.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:56 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev/Vladik Aranov
==========================================================================
  Copyright (c) 2001/2005 
  ULB - Universite Libre de Bruxelles
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "mafGraphIDDesc.h"
#include "mafPipeIntGraph.h"

#ifdef _MSC_FULL_VER
#pragma warning (disable: 4786)
#endif


#define MAXTEXTLEN           1024 

enum 
{
  EVT_GROUP_SET_X,
  EVT_GROUP_ADD_Y,
  EVT_GROUP_ADD_SET,
  EVT_GROUP_REM_Y
};

#define AddEulerSubMenu(Convention, extend, index) do{        hSubEuler  = new wxMenu();\
        hEuler->Append(0, #Convention , hSubEuler );\
        if(extend > 0) \
          hSubEuler->Append(nIDBase + GDT_LAST * nI + extend + index, "All angles");\
        hSubEuler->Append(nIDBase + GDT_LAST * nI + GDT_EUL_ROTX##Convention, "Angle 1");\
        hSubEuler->Append(nIDBase + GDT_LAST * nI + GDT_EUL_ROTY##Convention, "Angle 2");\
        hSubEuler->Append(nIDBase + GDT_LAST * nI + GDT_EUL_ROTZ##Convention, "Angle 3");\
}while(0)

#define EulerAnglesDescr(Convention) "Euler "#Convention" Angle1, deg", "Euler "#Convention" Angle2, deg", "Euler "#Convention" Angle3, deg"




#define AddSetOfVars(Convention) static bool _addSetOf##Convention(mafMemoryGraph *graph, unsigned int setIndex) {\
  bool var, result;\
  IDType ID;\
  result = false;\
  ID[0] = setIndex;\
  ID[1] = Convention##X;\
  var       = graph->AddYVar(ID);\
  result = result || var;\
  ID[0] = setIndex;\
  ID[1] = Convention##Y;\
  var       = graph->AddYVar(ID);\
  result = result || var;\
  ID[0] = setIndex;\
  ID[1] = Convention##Z;\
  var       = graph->AddYVar(ID);\
  result = result || var;\
  return result;\
}
#define AddSetOfVarsRef(Convention) _addSetOf##Convention

#define AddSetOfEulVars(Convention) static bool _addSetOfEul##Convention(mafMemoryGraph *graph, unsigned int setIndex){\
  bool var, result;\
  IDType ID;\
  result = false;\
  ID[0] = setIndex;\
  ID[1] = GDT_EUL_ROTX##Convention;\
  var = graph->AddYVar(ID);\
  result = result || var;\
  ID[0] = setIndex;\
  ID[1] = GDT_EUL_ROTY##Convention;\
  var = graph->AddYVar(ID);\
  result = result || var;\
  ID[0] = setIndex;\
  ID[1] = GDT_EUL_ROTZ##Convention;\
  var = graph->AddYVar(ID);\
  result = result || var;\
  return result;\
}

#define AddSetOfEulVarsRef(Convention) _addSetOfEul##Convention


AddSetOfVars(GDT_GTM_POS)
AddSetOfVars(GDT_GTM_ROT)
AddSetOfVars(GDT_HEL_ROT)
AddSetOfVars(GDT_LTM_POS)
AddSetOfVars(GDT_LTM_ROT)
AddSetOfEulVars(XYZs)
AddSetOfEulVars(XYXs)
AddSetOfEulVars(XZYs)
AddSetOfEulVars(XZXs)
AddSetOfEulVars(YZXs)
AddSetOfEulVars(YZYs)
AddSetOfEulVars(YXZs)
AddSetOfEulVars(YXYs)
AddSetOfEulVars(ZXYs)
AddSetOfEulVars(ZXZs)
AddSetOfEulVars(ZYXs)
AddSetOfEulVars(ZYZs)
AddSetOfEulVars(ZYXr)
AddSetOfEulVars(XYXr)
AddSetOfEulVars(YZXr)
AddSetOfEulVars(XZXr)
AddSetOfEulVars(XZYr)
AddSetOfEulVars(YZYr)
AddSetOfEulVars(ZXYr)
AddSetOfEulVars(YXYr)
AddSetOfEulVars(YXZr)
AddSetOfEulVars(ZXZr)
AddSetOfEulVars(XYZr)
AddSetOfEulVars(ZYZr)
AddSetOfVars(GDT_OVP_POS)
AddSetOfVars(GDT_OVP_ROT)
AddSetOfVars(GDT_GES_POS)
AddSetOfVars(GDT_GES_ROT)



typedef bool (*addSet)(mafMemoryGraph *graph, unsigned int);

addSet _addSetArray[]={
  AddSetOfVarsRef(GDT_GTM_POS),
  AddSetOfVarsRef(GDT_GTM_ROT),
  AddSetOfVarsRef(GDT_HEL_ROT),
  AddSetOfVarsRef(GDT_LTM_POS),
  AddSetOfVarsRef(GDT_LTM_ROT),
  AddSetOfEulVarsRef(XYZs),
  AddSetOfEulVarsRef(XYXs),
  AddSetOfEulVarsRef(XZYs),
  AddSetOfEulVarsRef(XZXs),
  AddSetOfEulVarsRef(YZXs),
  AddSetOfEulVarsRef(YZYs),
  AddSetOfEulVarsRef(YXZs),
  AddSetOfEulVarsRef(YXYs),
  AddSetOfEulVarsRef(ZXYs),
  AddSetOfEulVarsRef(ZXZs),
  AddSetOfEulVarsRef(ZYXs),
  AddSetOfEulVarsRef(ZYZs),
  AddSetOfEulVarsRef(ZYXr),
  AddSetOfEulVarsRef(XYXr),
  AddSetOfEulVarsRef(YZXr),
  AddSetOfEulVarsRef(XZXr),
  AddSetOfEulVarsRef(XZYr),
  AddSetOfEulVarsRef(YZYr),
  AddSetOfEulVarsRef(ZXYr),
  AddSetOfEulVarsRef(YXYr),
  AddSetOfEulVarsRef(YXZr),
  AddSetOfEulVarsRef(ZXZr),
  AddSetOfEulVarsRef(XYZr),
  AddSetOfEulVarsRef(ZYZr),
  AddSetOfVarsRef(GDT_OVP_POS),
  AddSetOfVarsRef(GDT_OVP_ROT),
  AddSetOfVarsRef(GDT_GES_POS),
  AddSetOfVarsRef(GDT_GES_ROT),
};



//----------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------

mafGraphIDDesc::mafGraphIDDesc(PipeArray  *vmpVME)
{
  m_currVME   = vmpVME;
}

//----------------------------------------------------------------------------
void mafGraphIDDesc::GetIDDesc(const IDType& nID,char *sDescript,unsigned int nLength)
{
  const char *sMainDesc;
  const char *saAddDesc[] = {"Pos X, mm" ,"Pos Y, mm" ,"Pos Z, mm",
                             "Ori X, deg","Ori Y, deg","Ori Z, deg",

                             "Local Pos X, mm" ,"Local Pos Y, mm" ,"Local Pos Z, mm",
                             "Local Ori X, deg","Local Ori Y, deg","Local Ori Z, deg",

                             "OVP Pos X, mm","OVP Pos Y, mm","OVP Pos Z, mm",
                             "OVP Ori X, deg","OVP Ori Y, deg","OVP Ori Z, deg",

                             "GES Pos X, mm","GES Pos Y, mm","GES Pos Z, mm",
                             "GES Ori X, deg","GES Ori Y, deg","GES Ori Z, deg",

                             EulerAnglesDescr(XYZs),
                             EulerAnglesDescr(XYXs),
                             EulerAnglesDescr(XZYs),
                             EulerAnglesDescr(XZXs),
                             EulerAnglesDescr(YZXs),
                             EulerAnglesDescr(YZYs),
                             EulerAnglesDescr(YXZs),
                             EulerAnglesDescr(YXYs),
                             EulerAnglesDescr(ZXYs),
                             EulerAnglesDescr(ZXZs),
                             EulerAnglesDescr(ZYXs),
                             EulerAnglesDescr(ZYZs),

                             EulerAnglesDescr(ZYXr),
                             EulerAnglesDescr(XYXr),
                             EulerAnglesDescr(YZXr),
                             EulerAnglesDescr(XZXr),
                             EulerAnglesDescr(XZYr),
                             EulerAnglesDescr(YZYr),
                             EulerAnglesDescr(ZXYr),
                             EulerAnglesDescr(YXYr),
                             EulerAnglesDescr(YXZr),
                             EulerAnglesDescr(ZXZr),
                             EulerAnglesDescr(XYZr),
                             EulerAnglesDescr(ZYZr),
                             "Hel Ori X, deg","Hel Ori Y, deg", "Hel Ori Z, deg"};
  if(nID.isZero())
  {
    _snprintf(sDescript, nLength, "Frames");
    return;
  }
  sMainDesc = (m_currVME->Item(nID[0]))->m_Vme->GetName(); ///from VME
  _snprintf(sDescript,nLength,"%s %s",sMainDesc, saAddDesc[nID[1] - 1]);
  return;
}


wxMenu *mafGraphIDDesc::GenerateMenu(mafMemoryGraph *pmgGraph, unsigned int nIDBase)
{
  wxMenu       *pTempMenu, *pManageMenu = NULL;
  unsigned int nYCount;
  char         sCaption[MAXTEXTLEN + 1];
  unsigned     varNum = m_currVME->size() * GDT_LAST;

  if(varNum > 0)
  {
    if(pManageMenu == NULL)
      pManageMenu = new wxMenu();
    pTempMenu = GenerateMenu(nIDBase, 0);
    pManageMenu->Append(0, "Set X variable", pTempMenu);
    pTempMenu = GenerateMenu(nIDBase + varNum, varNum);
    pManageMenu->Append(0, "Add Y variable", pTempMenu);
  }

  nYCount = pmgGraph->GetYDim();
  if(nYCount > 1)
  {
    if(pManageMenu == NULL)
      pManageMenu = new wxMenu();
    pTempMenu = new wxMenu();
    for(unsigned int nI = 0; nI < nYCount; nI++)
    {
      unsigned int nYNumber = pmgGraph->GetYIndex(nI);
      IDType       nID;
      nID = pmgGraph->GetID(nYNumber);
      GetIDDesc(nID, sCaption, MAXTEXTLEN);
      pTempMenu->Append(nIDBase + 3 * varNum + GetCommandByID(nID), sCaption);
    }
    pManageMenu->Append(0, "Remove Y variable", pTempMenu);
  }
  return pManageMenu;
}
bool mafGraphIDDesc::ProcessMenu(mafMemoryGraph *pmgGraph, unsigned int nIDCommand)
{
  unsigned int nEventGroup;
  unsigned int nCurSelection;
  unsigned     varNum = m_currVME->size() * GDT_LAST;

  nEventGroup   = nIDCommand / varNum;
  nCurSelection = nIDCommand % varNum;

  switch(nEventGroup)
  {
  case EVT_GROUP_SET_X://set X
    {
      if(!pmgGraph->SetXVar(0, GetIDByCommand(nCurSelection)))
        return false;
      break;
    }
  case EVT_GROUP_ADD_Y://add Y
    {
      if(!pmgGraph->AddYVar(GetIDByCommand(nCurSelection)))
        return false;
      break;
    }
  case EVT_GROUP_ADD_SET:
    {
      if(!_addSetArray[nCurSelection % GDT_LAST](pmgGraph, nCurSelection / GDT_LAST))
        return false;
      break;
    }
  case EVT_GROUP_REM_Y://rem Y
    {
      if(!pmgGraph->RemYVar(GetIDByCommand(nCurSelection)))
        return false;
      break;
    }
  default://unknown
    {
      wxASSERT(false);
      return false;
    }
  }
  return true;
}


//----------------------------------------------------------------------------
wxMenu *mafGraphIDDesc::GenerateMenu(unsigned int nIDBase, unsigned int nExtended)//nIDBase is base for ID of menu commands 
//----------------------------------------------------------------------------
{                                                 //it will be added to each command id in generated menu
  wxMenu *Root, *Desc, *hGlobal, *hLocal, *hOVP, *hGES, *hEuler, *hSubEuler, *hHelical;
  const char *sMainDesc;
  int nI;
  int nDescNumber = m_currVME->Count(); // take from VME

  Root = new wxMenu();
  Root->Append(nIDBase, "Frames");
  for(nI = 0; nI < nDescNumber; nI++)
  {
    sMainDesc = (m_currVME->Item(nI))->m_Vme->GetName();//from VME

    Desc    = new wxMenu();
    
    //GTM is always available
    hGlobal = new wxMenu();
    Desc->Append(0, "Global", hGlobal);
    
    if(nExtended > 0)
      hGlobal->Append(nIDBase + GDT_LAST * nI + nExtended + 0,  "All Pos");

    hGlobal->Append(nIDBase + GDT_LAST * nI + GDT_GTM_POSX,  "PosX");
    hGlobal->Append(nIDBase + GDT_LAST * nI + GDT_GTM_POSY,  "PosY");
    hGlobal->Append(nIDBase + GDT_LAST * nI + GDT_GTM_POSZ,  "PosZ");

    if(nExtended > 0)
      hGlobal->Append(nIDBase + GDT_LAST * nI + nExtended + 1,  "All Ori");

    hGlobal->Append(nIDBase + GDT_LAST * nI + GDT_GTM_ROTX,  "OriX");
    hGlobal->Append(nIDBase + GDT_LAST * nI + GDT_GTM_ROTY,  "OriY");
    hGlobal->Append(nIDBase + GDT_LAST * nI + GDT_GTM_ROTZ,  "OriZ");
    
    ///along with helical
    hHelical = new wxMenu();
    Desc->Append(0, "Helical", hHelical);

    if(nExtended > 0)
      hHelical->Append(nIDBase + GDT_LAST * nI + nExtended + 2,  "All Ori");
    
    hHelical->Append(nIDBase + GDT_LAST * nI + GDT_HEL_ROTX,  "OriX");
    hHelical->Append(nIDBase + GDT_LAST * nI + GDT_HEL_ROTY,  "OriY");
    hHelical->Append(nIDBase + GDT_LAST * nI + GDT_HEL_ROTZ,  "OriZ");
    
    {
      {
        //LTM available only when parent is present      
        hLocal  = new wxMenu();
        Desc->Append(0, "Local" , hLocal );

        if(nExtended > 0)
          hLocal->Append(nIDBase + GDT_LAST * nI + nExtended + 3,  "All Pos");
    
        hLocal->Append(nIDBase + GDT_LAST * nI + GDT_LTM_POSX,  "PosX");
        hLocal->Append(nIDBase + GDT_LAST * nI + GDT_LTM_POSY,  "PosY");
        hLocal->Append(nIDBase + GDT_LAST * nI + GDT_LTM_POSZ,  "PosZ");

        if(nExtended > 0)
          hLocal->Append(nIDBase + GDT_LAST * nI + nExtended + 4,  "All Ori");
    
        hLocal->Append(nIDBase + GDT_LAST * nI + GDT_LTM_ROTX,  "OriX");
        hLocal->Append(nIDBase + GDT_LAST * nI + GDT_LTM_ROTY,  "OriY");
        hLocal->Append(nIDBase + GDT_LAST * nI + GDT_LTM_ROTZ,  "OriZ");
      }
      
      //if(fpFrame->GetParent() == NULL || (fpFrame->GetParent() != NULL && fpFrame->GetParent()->GetVME() != NULL))
      {
        //Euler needs only parent: but without it build over GTM    
        hEuler  = new wxMenu();
        Desc->Append(0, "Euler" , hEuler );

        AddEulerSubMenu(XYZs, nExtended,  5);
        AddEulerSubMenu(XYXs, nExtended,  6);
        AddEulerSubMenu(XZYs, nExtended,  7);
        AddEulerSubMenu(XZXs, nExtended,  8);
        AddEulerSubMenu(YZXs, nExtended,  9);
        AddEulerSubMenu(YZYs, nExtended, 10);
        AddEulerSubMenu(YXZs, nExtended, 11);
        AddEulerSubMenu(YXYs, nExtended, 12);
        AddEulerSubMenu(ZXYs, nExtended, 13);
        AddEulerSubMenu(ZXZs, nExtended, 14);
        AddEulerSubMenu(ZYXs, nExtended, 15);
        AddEulerSubMenu(ZYZs, nExtended, 16);

        AddEulerSubMenu(ZYXr, nExtended, 17);
        AddEulerSubMenu(XYXr, nExtended, 18);
        AddEulerSubMenu(YZXr, nExtended, 19);
        AddEulerSubMenu(XZXr, nExtended, 20);
        AddEulerSubMenu(XZYr, nExtended, 21);
        AddEulerSubMenu(YZYr, nExtended, 22);
        AddEulerSubMenu(ZXYr, nExtended, 23);
        AddEulerSubMenu(YXYr, nExtended, 24);
        AddEulerSubMenu(YXZr, nExtended, 25);
        AddEulerSubMenu(ZXZr, nExtended, 26);
        AddEulerSubMenu(XYZr, nExtended, 27);
        AddEulerSubMenu(ZYZr, nExtended, 28);

        //OVP angles available when parent is present but we need landmarks for positions     
        hOVP    = new wxMenu();
        Desc->Append(0, "OVP"   , hOVP   );

        if(nExtended > 0)
          hOVP->Append(nIDBase + GDT_LAST * nI + nExtended + 29,  "All Pos");

        hOVP->Append(nIDBase + GDT_LAST * nI + GDT_OVP_POSX, "PosX");
        hOVP->Append(nIDBase + GDT_LAST * nI + GDT_OVP_POSY, "PosY");
        hOVP->Append(nIDBase + GDT_LAST * nI + GDT_OVP_POSZ, "PosZ");

        if(nExtended > 0)
          hOVP->Append(nIDBase + GDT_LAST * nI + nExtended + 30,  "All Ori");

        hOVP->Append(nIDBase + GDT_LAST * nI + GDT_OVP_ROTX, "OriX");
        hOVP->Append(nIDBase + GDT_LAST * nI + GDT_OVP_ROTY, "OriY");
        hOVP->Append(nIDBase + GDT_LAST * nI + GDT_OVP_ROTZ, "OriZ");

        //GES angles available when parent is present but we need landmarks for positions     
        hGES    = new wxMenu();
        Desc->Append(0, "GES"   , hGES   );

        if(nExtended > 0)
          hGES->Append(nIDBase + GDT_LAST * nI + nExtended + 31,  "All Pos");

        hGES->Append(nIDBase + GDT_LAST * nI + GDT_GES_POSX, "PosX");
        hGES->Append(nIDBase + GDT_LAST * nI + GDT_GES_POSY, "PosY");
        hGES->Append(nIDBase + GDT_LAST * nI + GDT_GES_POSZ, "PosZ");

        if(nExtended > 0)
          hGES->Append(nIDBase + GDT_LAST * nI + nExtended + 32,  "All Ori");

        hGES->Append(nIDBase + GDT_LAST * nI + GDT_GES_ROTX, "OriX");
        hGES->Append(nIDBase + GDT_LAST * nI + GDT_GES_ROTY, "OriY");
        hGES->Append(nIDBase + GDT_LAST * nI + GDT_GES_ROTZ, "OriZ");
      }
    }

    Root->Append(0, sMainDesc, Desc);
  }
  return (Root);
}

IDType mafGraphIDDesc::GetIDByCommand(unsigned int command)
{
  IDType ID;
  ID[0] = command / GDT_LAST;
  ID[1] = command % GDT_LAST;
  return ID;
}
unsigned int mafGraphIDDesc::GetCommandByID(const IDType& ID)
{
  return GDT_LAST * ID[0] + ID[1];
}
