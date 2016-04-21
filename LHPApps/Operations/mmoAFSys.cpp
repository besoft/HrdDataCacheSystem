/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoAFSys.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "mmoAFSys.h"
#include "wx/busyinfo.h"
#include "wx/textfile.h"

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafGUI.h"
#include "mafDictionary.h"
#include "mafPlotMath.h"

#include "mafSmartPointer.h"

#include "mafVMEAFRefSys.h"
#include "mafVMELandmarkCloud.h"

//----------------------------------------------------------------------------
// Required for MSVC
//----------------------------------------------------------------------------
#ifdef _MSC_FULL_VER
#pragma warning (disable: 4786)
#endif

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Forward Refs
//----------------------------------------------------------------------------

#define ADD_PREDEF(name, boneID) m_predefinedScripts.push_back(PredefinedScripts(#name, std::vector<mafString>(&_##name[0], &_##name[0] + sizeof(_##name)/sizeof(_##name[0])), mafVMEAFRefSys::ID_AFS_##boneID))
void mmoAFSys::InitPredefined()
{
  mafString _IPE[] = {"ASSV PN0 RIAS", "ASSV PN1 LIAS", "ASSV PN2 RIPS", "ASSV PN3 LIPS", "DEFVI RIAC", "DEFVI LIAC", "LNCMB MIDDLEA 0.5 PN0 0.5 PN1", "LNCMB MIDDLEP 0.5 PN2 0.5 PN3", "ASSV P1 PN0", "ASSV P2 PN1", "ASSV P3 MIDDLEP", "ASSV P4 PN1", "ASSV P5 PN0", "LNCMB A 1 P2 -1 P1", "NRML A", "LNCMB B 1 P3 -1 P2", "NRML B", "CROSS X1 A B", "NRML X1", "LNCMB R 1 P5 -1 P4", "NRML R", "CROSS Y1 X1 R", "NRML Y1", "CROSS Z1 X1 Y1", "NRML Z1", "ASSV X Y1", "ASSV Y X1", "LNCMB Z -1 Z1 0 Y1", "ASSV P MIDDLEA"};
  mafString _LFT[] = {"ASSV PN0 LFCC", "ASSV PN1 LFM5", "ASSV PN2 LFM2", "ASSV PN3 LFM1", "ASSV P1 PN3", "ASSV P2 PN1", "ASSV P3 PN0", "ASSV P4 PN2", "ASSV P5 PN0", "LNCMB A 1 P2 -1 P1", "NRML A", "LNCMB B 1 P3 -1 P2", "NRML B", "CROSS X1 A B", "NRML X1", "LNCMB R 1 P5 -1 P4", "NRML R", "CROSS Y1 X1 R", "NRML Y1", "CROSS Z1 X1 Y1", "NRML Z1", "ASSV X Z1", "ASSV Y X1", "ASSV Z Y1", "ASSV P PN0"};
  mafString _LSH[] = {"ASSV PN0 LFAX", "ASSV PN1 LTTC", "ASSV PN2 LTAM", "ASSV PN3 LFAL", "LNCMB MIDDLE 0.5 PN2 0.5 PN3", "ASSV P1 PN2", "ASSV P2 PN3", "ASSV P3 PN0", "ASSV P4 MIDDLE", "ASSV P5 PN1", "LNCMB A 1 P2 -1 P1", "NRML A", "LNCMB B 1 P3 -1 P2", "NRML B", "CROSS X1 A B", "NRML X1", "LNCMB R 1 P5 -1 P4", "NRML R", "CROSS Y1 X1 R", "NRML Y1", "CROSS Z1 X1 Y1", "NRML Z1", "ASSV X X1", "LNCMB Y -1 Z1 0 Y1", "ASSV Z Y1", "ASSV P MIDDLE"};
  mafString _LSH_ISB[] = {"ASSV PN0 LTLR", "ASSV PN1 LTMR", "ASSV PN2 LFAL", "ASSV PN3 LTAM", "LNCMB MIDDLEUP 0.5 PN0 0.5 PN1", "LNCMB MIDDLEDN 0.5 PN2 0.5 PN3", "SUBV Z PN3 PN2", "NRML Z", "SUBV A MIDDLEUP MIDDLEDN", "NRML A", "CROSS X A Z", "NRML X", "CROSS Y Z X", "NRML Y", "ASSV P MIDDLEDN"};
  mafString _LTH[] = {"ASSV PN0 LFME", "ASSV PN1 LFLE", "ASSV PN2 LFCH", "LNCMB MIDDLE 0.5 PN0 0.5 PN1", "ASSV P1 PN0", "ASSV P2 PN1", "ASSV P3 PN2", "ASSV P4 MIDDLE", "ASSV P5 PN2", "LNCMB A 1 P2 -1 P1", "NRML A", "LNCMB B 1 P3 -1 P2", "NRML B", "CROSS X1 A B", "NRML X1", "LNCMB R 1 P5 -1 P4", "NRML R", "CROSS Y1 X1 R", "NRML Y1", "CROSS Z1 X1 Y1", "NRML Z1", "ASSV X X1", "LNCMB Y -1 Z1 0 Y1", "ASSV Z Y1", "ASSV P MIDDLE"};
  mafString _RFT[] = {"ASSV PN0 RFCC", "ASSV PN1 RFM1", "ASSV PN2 RFM2", "ASSV PN3 RFM5", "ASSV P1 PN3", "ASSV P2 PN1", "ASSV P3 PN0", "ASSV P4 PN2", "ASSV P5 PN0", "LNCMB A 1 P2 -1 P1", "NRML A", "LNCMB B 1 P3 -1 P2", "NRML B", "CROSS X1 A B", "NRML X1", "LNCMB R 1 P5 -1 P4", "NRML R", "CROSS Y1 X1 R", "NRML Y1", "CROSS Z1 X1 Y1", "NRML Z1", "ASSV X Z1", "ASSV Y X1", "ASSV Z Y1", "ASSV P PN0"};
  mafString _RSH[] = {"ASSV PN0 RFAX", "ASSV PN1 RTTC", "ASSV PN2 RFAL", "ASSV PN3 RTAM", "LNCMB MIDDLE 0.5 PN2 0.5 PN3", "ASSV P1 PN2", "ASSV P2 PN3", "ASSV P3 PN0", "ASSV P4 MIDDLE", "ASSV P5 PN1", "LNCMB A 1 P2 -1 P1", "NRML A", "LNCMB B 1 P3 -1 P2", "NRML B", "CROSS X1 A B", "NRML X1", "LNCMB R 1 P5 -1 P4", "NRML R", "CROSS Y1 X1 R", "NRML Y1", "CROSS Z1 X1 Y1", "NRML Z1", "ASSV X X1", "LNCMB Y -1 Z1 0 Y1", "ASSV Z Y1", "ASSV P MIDDLE"};
  mafString _RSH_ISB[] = {"ASSV PN0 RTLR", "ASSV PN1 RTMR", "ASSV PN2 RFAL", "ASSV PN3 RTAM", "LNCMB MIDDLEUP 0.5 PN0 0.5 PN1", "LNCMB MIDDLEDN 0.5 PN2 0.5 PN3", "SUBV Z PN2 PN3", "NRML Z", "SUBV A MIDDLEUP MIDDLEDN", "NRML A", "CROSS X A Z", "NRML X", "CROSS Y Z X", "NRML Y", "ASSV P MIDDLEDN"};
  mafString _RTH[] = {"ASSV PN0 RFLE", "ASSV PN1 RFME", "ASSV PN2 RFCH", "LNCMB MIDDLE 0.5 PN0 0.5 PN1", "ASSV P1 PN0", "ASSV P2 PN1", "ASSV P3 PN2", "ASSV P4 MIDDLE", "ASSV P5 PN2", "LNCMB A 1 P2 -1 P1", "NRML A", "LNCMB B 1 P3 -1 P2", "NRML B", "CROSS X1 A B", "NRML X1", "LNCMB R 1 P5 -1 P4", "NRML R", "CROSS Y1 X1 R", "NRML Y1", "CROSS Z1 X1 Y1", "NRML Z1", "ASSV X X1", "LNCMB Y -1 Z1 0 Y1", "ASSV Z Y1", "ASSV P MIDDLE"};



  mafString _TRX[]   = {"ASSV PN0 MSXS", "ASSV PN1 MTV8", "ASSV PN2 MSJN", "ASSV PN3 MTV2", "LNCMB MIDDLEDN 0.5 PN0 0.5 PN1", "LNCMB MIDDLEUP 0.5 PN2 0.5 PN3", "SUBV Y MIDDLEUP MIDDLEDN", "NRML Y", "SUBV A PN2 PN3", "NRML A", "CROSS Z A Y", "NRML Z", "CROSS X Y Z", "NRML X", "ASSV P PN2"};
  mafString _RCLV[]  = {"ASSV PN0 MSXS", "ASSV PN1 MTV8", "ASSV PN2 MSJN", "ASSV PN3 MTV2", "ASSV PN4 RCAS", "ASSV PN5 RCAJ", "LNCMB MIDDLEDN 0.5 PN0 0.5 PN1", "LNCMB MIDDLEUP 0.5 PN2 0.5 PN3", "SUBV YT MIDDLEUP MIDDLEDN", "NRML YT", "LNCMB MIDDLE 0.5 PN1 0.5 PN2", "SUBV Z PN5 PN4", "NRML Z", "CROSS X YT Z", "NRML X", "CROSS Y Z X", "NRML Y", "ASSV P PN4"};
  mafString _LCLV[]  = {"ASSV PN0 MSXS", "ASSV PN1 MTV8", "ASSV PN2 MSJN", "ASSV PN3 MTV2", "ASSV PN4 LCAS", "ASSV PN5 LCAJ", "LNCMB MIDDLEDN 0.5 PN0 0.5 PN1", "LNCMB MIDDLEUP 0.5 PN2 0.5 PN3", "SUBV YT MIDDLEUP MIDDLEDN", "NRML YT", "LNCMB MIDDLE 0.5 PN1 0.5 PN2", "SUBV Z PN4 PN5", "NRML Z", "CROSS X YT Z", "NRML X", "CROSS Y Z X", "NRML Y", "ASSV P PN4"};
  mafString _RSCP[]  = {"ASSV PN0 RSAA", "ASSV PN1 RSRS", "ASSV PN2 RSIA", "SUBV Z PN0 PN1", "NRML Z", "SUBV A PN1 PN2", "NRML A", "CROSS X A Z", "NRML X", "CROSS Y Z X", "NRML Y", "ASSV P PN0"};
  mafString _LSCP[]  = {"ASSV PN0 LSAA", "ASSV PN1 LSRS", "ASSV PN2 LSIA", "SUBV Z PN1 PN0", "NRML Z", "SUBV A PN1 PN2", "NRML A", "CROSS X A Z", "NRML X", "CROSS Y Z X", "NRML Y", "ASSV P PN0"};
  mafString _RHUM1[] = {"ASSV PN0 RHGH", "ASSV PN1 RHLE", "ASSV PN2 RHME", "LNCMB MIDDLE 0.5 PN1 0.5 PN2", "SUBV Y PN0 MIDDLE", "NRML Y", "SUBV A PN1 PN2", "NRML A", "CROSS X Y A", "NRML X", "CROSS Z X Y", "NRML Z", "ASSV P PN0"};
  mafString _LHUM1[] = {"ASSV PN0 LHGH", "ASSV PN1 LHLE", "ASSV PN2 LHME", "LNCMB MIDDLE 0.5 PN1 0.5 PN2", "SUBV Y PN0 MIDDLE", "NRML Y", "SUBV A PN2 PN1", "NRML A", "CROSS X Y A", "NRML X", "CROSS Z X Y", "NRML Z", "ASSV P PN0"};
  mafString _RHUM2[] = {"ASSV PN0 RHGH", "ASSV PN1 RHLE", "ASSV PN2 RHME", "ASSV PN4 RUSP", "LNCMB MIDDLE 0.5 PN1 0.5 PN2", "SUBV Y PN0 MIDDLE", "NRML Y", "SUBV YF PN4 MIDDLE", "NRML YF", "CROSS Z Y YF", "NRML X", "CROSS X Y Z", "NRML X", "ASSV P PN0"};
  mafString _LHUM2[] = {"ASSV PN0 LHGH", "ASSV PN1 LHLE", "ASSV PN2 LHME", "ASSV PN4 LUSP", "LNCMB MIDDLE 0.5 PN1 0.5 PN2", "SUBV Y PN0 MIDDLE", "NRML Y", "SUBV YF PN4 MIDDLE", "NRML YF", "CROSS Z Y YF", "NRML X", "CROSS X Y Z", "NRML X", "ASSV P PN0"};
  mafString _RFRA[]  = {"ASSV PN0 RUSP", "ASSV PN1 RHLE", "ASSV PN2 RHME", "ASSV PN3 RRSP", "LNCMB MIDDLE 0.5 PN1 0.5 PN2", "SUBV Y PN0 MIDDLE", "NRML Y", "SUBV A PN3 PN0", "NRML A", "CROSS X Y A", "NRML X", "CROSS Z X Y", "NRML Z", "ASSV P PN0"};
  mafString _LFRA[]  = {"ASSV PN0 LUSP", "ASSV PN1 LHLE", "ASSV PN2 LHME", "ASSV PN3 LRSP", "LNCMB MIDDLE 0.5 PN1 0.5 PN2", "SUBV Y PN0 MIDDLE", "NRML Y", "SUBV A PN0 PN3", "NRML A", "CROSS X Y A", "NRML X", "CROSS Z X Y", "NRML Z", "ASSV P PN0"};

  mafString _3PNT_Y[] = {"ASSV PN0 PNT1", "ASSV PN1 PNT2", "ASSV PN2 PNT3", "DEFSI 0 t 1", "SUBS s 1 t", "LNCMB MIDDLE t PN0 s PN1", "ASSV P1 PN1", "ASSV P2 PN0", "ASSV P3 PN2", "ASSV P4 MIDDLE", "ASSV P5 PN2", "LNCMB A 1 P2 -1 P1", "NRML A", "LNCMB B 1 P3 -1 P2", "NRML B", "CROSS X1 A B", "NRML X1", "LNCMB R 1 P5 -1 P4", "NRML R", "CROSS Y1 X1 R", "NRML Y1", "CROSS Z1 X1 Y1", "NRML Z1", "ASSV X X1", "LNCMB Y -1 Z1 0 Y1", "ASSV Z Y1", "ASSV P MIDDLE"};
  mafString _3PNT_Z[] = {"ASSV PN0 PNT1", "ASSV PN1 PNT2", "ASSV PN2 PNT3", "DEFSI 0 t 1", "SUBS s 1 t", "LNCMB MIDDLE t PN0 s PN1", "SUBV Z PN1 PN0", "NRML Z", "SUBV Y1 MIDDLE PN2", "NRML Y1", "CROSS X Z Y1", "NRML X", "CROSS Y1 X Z", "LNCMB Y -1 Y1 0 Y1", "ASSV P MIDDLE"};
  mafString _4PNT_Y[] = {"ASSV PN0 PNT1", "ASSV PN1 PNT2", "ASSV PN2 PNT3", "ASSV PN3 PNT4", "DEFSI 0 t 1", "SUBS s 1 t", "LNCMB MIDDLE t PN0 s PN1", "ASSV P1 PN1", "ASSV P2 PN0", "ASSV P3 PN2", "ASSV P4 PN3", "ASSV P5 PN2", "LNCMB A 1 P2 -1 P1", "NRML A", "LNCMB B 1 P3 -1 P2", "NRML B", "CROSS X1 A B", "NRML X1", "LNCMB R 1 P5 -1 P4", "NRML R", "CROSS Y1 X1 R", "NRML Y1", "CROSS Z1 X1 Y1", "NRML Z1", "ASSV X X1", "LNCMB Y -1 Z1 0 Y1", "ASSV Z Y1", "ASSV P MIDDLE"};
  mafString _4PNT_Z[] = {"ASSV PN0 PNT1", "ASSV PN1 PNT2", "ASSV PN2 PNT3", "ASSV PN3 PNT4", "DEFSI 0 t 1", "SUBS s 1 t", "LNCMB MIDDLE t PN0 s PN1", "SUBV Z PN1 PN0", "NRML Z", "SUBV Y1 PN3 PN2", "NRML Y1", "CROSS X Z Y1", "NRML X", "CROSS Y1 X Z", "LNCMB Y -1 Y1 0 Y1", "ASSV P MIDDLE"};

  ADD_PREDEF(IPE, PELVIS);
  ADD_PREDEF(RTH, RTHIGH);
  ADD_PREDEF(LTH, LTHIGH);
  ADD_PREDEF(RSH, RSHANK);
  ADD_PREDEF(RSH_ISB, NOTDEFINED);
  ADD_PREDEF(LSH, LSHANK);
  ADD_PREDEF(LSH_ISB, NOTDEFINED);
  ADD_PREDEF(RFT, RFOOT);
  ADD_PREDEF(LFT, LFOOT);

  ADD_PREDEF(TRX, NOTDEFINED);
  ADD_PREDEF(RCLV, NOTDEFINED);
  ADD_PREDEF(LCLV, NOTDEFINED);
  ADD_PREDEF(RSCP, NOTDEFINED);
  ADD_PREDEF(LSCP, NOTDEFINED);
  ADD_PREDEF(RHUM1, NOTDEFINED);
  ADD_PREDEF(LHUM1, NOTDEFINED);
  ADD_PREDEF(RHUM2, NOTDEFINED);
  ADD_PREDEF(LHUM2, NOTDEFINED);
  ADD_PREDEF(RFRA, NOTDEFINED);
  ADD_PREDEF(LFRA, NOTDEFINED);



  ADD_PREDEF(3PNT_Y, NOTDEFINED);
  ADD_PREDEF(3PNT_Z, NOTDEFINED);
  ADD_PREDEF(4PNT_Y, NOTDEFINED);
  ADD_PREDEF(4PNT_Z, NOTDEFINED);
}

//----------------------------------------------------------------------------
mmoAFSys::mmoAFSys(wxString label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType   = OPTYPE_OP;
  m_Canundo  = true;
  m_RefSys   = NULL;
  InitPredefined();
  m_Radio    = m_predefinedScripts.size();
}

//----------------------------------------------------------------------------
mmoAFSys::~mmoAFSys()
//----------------------------------------------------------------------------
{
  mafDEL(m_RefSys);
}

//----------------------------------------------------------------------------
mafOp* mmoAFSys::Copy()   
//----------------------------------------------------------------------------
{
  return new mmoAFSys(m_Label);
}

//----------------------------------------------------------------------------
bool mmoAFSys::Accept(mafNode* vme)
//----------------------------------------------------------------------------
{
  if(!vme) return false;
  if(mafVMELandmarkCloud::SafeDownCast(vme) == NULL)
    return false;

  /*for(int i = 0; i < vme->GetNumberOfChildren(); i++)
  {
    if(vme->GetChild(i)->IsA("mafVMEAFRefSys"))
      return false;
  }*/
  
  return true;
}

//----------------------------------------------------------------------------
// widget id's
//----------------------------------------------------------------------------
enum 
{
  ID_DEFAULT = MINID,
  ID_RADIO_SCRIPT,
  ID_LOAD_DICTIONARY,
  ID_LOAD_SCRIPT,
  ID_LAST,
  ID_FORCED_DWORD = 0x7fffffff,
  ID_HELP
};

//----------------------------------------------------------------------------
void mmoAFSys::OpRun()   
//----------------------------------------------------------------------------
{
  mafString strBase(m_Input->GetName());
  mafNEW(m_RefSys);
  strBase += "_AF_Frame";
  mafString str = strBase;
  unsigned ind = 0;
  unsigned i;
  do
  {
    for(i = 0; i < m_Input->GetNumberOfChildren(); i++)
    {
      mafNode *node = m_Input->GetChild(i);
      if(strcmp(node->GetName(), str.GetCStr()) == 0)
      {
        str.Printf("%s_%u", strBase.GetCStr(), ind);
        ind++;
        break;
      }
    }
  }
  while(i < m_Input->GetNumberOfChildren() && ind != UINT_MAX);
  m_RefSys->SetName(str.GetCStr());
  for(unsigned nm = 0; nm < m_predefinedScripts.size(); nm++)
  {
    if(stricmp(m_predefinedScripts[nm].m_Name.GetCStr(), m_Input->GetName()) == 0)
    {
      m_Radio = nm;
      m_RefSys->SetScriptText(m_predefinedScripts[m_Radio].m_Script);
      m_RefSys->SetBoneID(m_predefinedScripts[m_Radio].m_BoneID);
      break;
    }
  }
  CreateGui();
}

//----------------------------------------------------------------------------
void mmoAFSys::CreateGui()
//----------------------------------------------------------------------------
{
  m_Gui = new mafGUI(this);
  m_Gui->SetListener(this);
	
  mafEvent buildHelpGui;
  buildHelpGui.SetSender(this);
  buildHelpGui.SetId(GET_BUILD_HELP_GUI);
  mafEventMacro(buildHelpGui);

  if (buildHelpGui.GetArg() == true)
  {
	  m_Gui->Button(ID_HELP, "Help","");	
  }

  std::vector<wxString> list;
  for(unsigned i = 0; i < m_predefinedScripts.size(); i++)
  {
    list.push_back(m_predefinedScripts[i].m_Name.GetCStr());
  }
  list.push_back("Custom");
  m_Gui->Radio(ID_RADIO_SCRIPT, "",&m_Radio, list.size(), &list[0]);
  m_Gui->FileOpen(ID_LOAD_SCRIPT, "Script", &m_ScriptFName);
  m_Gui->Label("");

  m_Gui->Enable(ID_LOAD_SCRIPT, m_Radio == m_predefinedScripts.size());
  m_Gui->OkCancel();
  ShowGui();
}

//----------------------------------------------------------------------------
void mmoAFSys::OpStop(int result)
//----------------------------------------------------------------------------
{
  if (result == OP_RUN_CANCEL)
  {
    HideGui();
    if(m_RefSys->GetParent())
    {
      mafEventMacro(mafEvent(this, VME_REMOVE, m_RefSys));
    }
    mafEventMacro(mafEvent(this,result));
  }
  else if (result == OP_RUN_OK)
  {
    if(m_Radio == m_predefinedScripts.size() && m_ScriptFName == "")
    {
      wxMessageBox("Method is not specified","Alert", wxOK , NULL);
      return;
    }
    HideGui();
    mafEventMacro(mafEvent(this,result));
  }
}

bool mmoAFSys::ReadScript(const mafString& filename, std::vector<mafString>& output)
{
  FILE *fp = fopen(filename, "rt");
  if(fp == NULL)
  {
    return false;
  }

  int const maxStrLen = 1000;
  char      sLine[maxStrLen];
  char      *pRet;

  while(true)
  {
    pRet = fgets(sLine, maxStrLen, fp);
    if(pRet == NULL)
      break;
    output.push_back(mafString(pRet));
  }
  fclose(fp);
  return true;
}


//----------------------------------------------------------------------------
void mmoAFSys::OnEvent(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{ 
  switch(maf_event->GetId())
  {
	case ID_HELP:
	{
		mafEvent helpEvent;
		helpEvent.SetSender(this);
		mafString operationLabel = this->m_Label;
		helpEvent.SetString(&operationLabel);
		helpEvent.SetId(OPEN_HELP_PAGE);
		mafEventMacro(helpEvent);
	}
	break;

    case wxOK:          
      OpStop(OP_RUN_OK);
      break;
    case wxCANCEL:
      OpStop(OP_RUN_CANCEL);
      break;
    case ID_LOAD_SCRIPT:
      {
        std::vector<mafString> tmp;
        if(m_ScriptFName != "" && ReadScript(m_ScriptFName, tmp))
        {
          m_RefSys->SetScriptText(tmp);
          m_RefSys->SetBoneID(mafVMEAFRefSys::ID_AFS_NOTDEFINED);
        }
      }
      break;
    case ID_RADIO_SCRIPT:
      {
        m_Gui->Enable(ID_LOAD_SCRIPT, m_Radio == m_predefinedScripts.size());
        if(m_Radio != m_predefinedScripts.size())
        {
          m_RefSys->SetScriptText(m_predefinedScripts[m_Radio].m_Script);
          m_RefSys->SetBoneID(m_predefinedScripts[m_Radio].m_BoneID);
        }
        else
        {
          std::vector<mafString> tmp;
          if(m_ScriptFName != "" && ReadScript(m_ScriptFName, tmp))
          {
            m_RefSys->SetScriptText(tmp);
            m_RefSys->SetBoneID(mafVMEAFRefSys::ID_AFS_NOTDEFINED);
          }
        }
        m_Gui->Update();
      }
      break;
    default:
      mafEventMacro(*maf_event); 
      break;
  }
}

//----------------------------------------------------------------------------
void mmoAFSys::OpDo()
//----------------------------------------------------------------------------
{
  wxBusyInfo wait("Please wait, working...");

  assert(m_RefSys);
  m_RefSys->ReparentTo(m_Input);
  m_RefSys->SetScaleFactor(100.0);
  mafEventMacro(mafEvent(this, VME_ADD, m_RefSys));
  m_RefSys->SetActive(1);
}
//----------------------------------------------------------------------------
void mmoAFSys::OpUndo()
//----------------------------------------------------------------------------
{
  assert(m_RefSys);
  mafEventMacro(mafEvent(this, VME_REMOVE, m_RefSys));
}

