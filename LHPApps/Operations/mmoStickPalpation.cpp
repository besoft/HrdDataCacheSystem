/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoStickPalpation.cpp,v $
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

#include "mmoStickPalpation.h"

#include "wx/textfile.h"
#include "wx/arrimpl.cpp"
#include "wx/busyinfo.h"
#include <math.h>

#include "mafDecl.h"
#include "mafOp.h"
#include "mafEvent.h"
#include "mafGUI.h"

#include "mafDictionary.h"
#include "mafOpExplodeCollapse.h"

#include "mafSmartPointer.h"

#include "mafVME.h"
#include "mafVMEC3DData.h"
#include "mafVMESurface.h"
#include "mafVMELandmark.h"
#include "mafVMELandmarkCloud.h"

#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkWeightedLandmarkTransform.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"

//----------------------------------------------------------------------------
// Required for MSVC
//----------------------------------------------------------------------------
#ifdef _MSC_FULL_VER
#pragma warning (disable: 4786)
#endif

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------
enum 
{
  RIGID  = 0,
  SIMILARITY,
  AFFINE
};

#ifndef DIM
#define DIM(a)  (sizeof((a)) / sizeof(*(a)))
#endif


//----------------------------------------------------------------------------
// static persistent data:
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Forward Refs
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
inline bool LMCSetState(mafVMELandmarkCloud *cloud, mafObserver *listener, bool openState)
//----------------------------------------------------------------------------
{
  bool LMCOpened = cloud->IsOpen();
  if(LMCOpened == openState)
    return LMCOpened;
  if(openState)
  {
    mafOp *OpenOp = new mafOpExplodeCollapse("open cloud");
    OpenOp->SetInput(cloud);
    OpenOp->SetListener(listener);
    OpenOp->OpDo();
    cppDEL(OpenOp); 
    return LMCOpened;
  }
  {
    mafOp *CloseOp = new mafOpExplodeCollapse("close cloud");
    CloseOp->SetInput(cloud);
    CloseOp->SetListener(listener);
    CloseOp->OpDo();
    cppDEL(CloseOp); 
    return LMCOpened;
  }
}

//----------------------------------------------------------------------------
mmoStickPalpation::mmoStickPalpation(const wxString& label) : mafOp(label), m_DictionaryFName("")
//----------------------------------------------------------------------------
{
  m_OpType               = OPTYPE_OP;
  m_Canundo              = true;
  
  m_StickCalibration     = NULL;
  m_StickCalibrationName = "";
  
  m_StickDefinition      = NULL;
  m_StickDefinitionName  = "";
  m_LimbCalibration      = NULL;
  m_LimbCalibrationName  = "";
  m_LimbCloud            = NULL;
  m_LimbCloudName        = "";
  m_TrgMotion            = NULL;
  m_TrgMotionName        = "";
  m_NewLandmarkName      = "";

  m_WarnNamesNotMatched  = false;
  m_NewIndex             = 0;
  m_Registered           = NULL;
  m_pointsSource         = NULL;
  m_pointsTarget         = NULL;
  m_RegistrationMode     = SIMILARITY;
  m_weight               = NULL;
}

//----------------------------------------------------------------------------
mmoStickPalpation::~mmoStickPalpation() 
//----------------------------------------------------------------------------
{
  vtkDEL(m_Registered);
  vtkDEL(m_pointsSource);
  vtkDEL(m_pointsTarget);

  if(m_weight)
  {
    delete[] m_weight;
    m_weight = NULL;
  }

}

//----------------------------------------------------------------------------
mafOp* mmoStickPalpation::Copy()
//----------------------------------------------------------------------------
{
  return new mmoStickPalpation(m_Label);
}

//----------------------------------------------------------------------------
bool mmoStickPalpation::Accept(mafNode* vme)
//----------------------------------------------------------------------------
{
  if(!vme) return false;

  if(!vme->IsA("mafVMELandmarkCloud"))
  {
    return false;
  }

  return true;
}


//----------------------------------------------------------------------------
void mmoStickPalpation::OpRun()
//----------------------------------------------------------------------------
{
  m_LimbCloud     = mafVMELandmarkCloud::SafeDownCast(m_Input);
  SetNodeName(m_LimbCloud, &m_LimbCloudName);

  m_TrgMotion     = mafVME::SafeDownCast(m_Input->GetParent());
  SetNodeName(m_TrgMotion, &m_TrgMotionName);

  if(m_StickDefinition == NULL)
  {
    m_StickDefinition = (mafVMELandmarkCloud *)MatchCriterion((mafVME *)m_Input->GetRoot(), &mmoStickPalpation::MatchStickDefinition, NULL);
    if(m_StickDefinition)
      SetNodeName(m_StickDefinition, &m_StickDefinitionName);
  }

  CreateGui();
  assert(!m_pointsSource && !m_pointsTarget);
  m_pointsSource = vtkPoints::New();
  m_pointsTarget = vtkPoints::New();
}

//----------------------------------------------------------------------------
void mmoStickPalpation::CreateGui()
//----------------------------------------------------------------------------
{
  const wxString choices_string[] = {_("rigid"), _("similarity"), _("affine")}; 
  if(m_Gui == NULL)
  {
    m_Gui = new mafGUI(this);
    m_Gui->SetListener(this);
    m_Gui->Label("");
    m_Gui->Button(ID_STICK_DEFINITION, "Wand definition cloud", "", "Press to select wand cloud with wand definition." );  
    m_Gui->Label("Wnd d.VME",&m_StickDefinitionName);
    m_Gui->Button(ID_STICK_CALIBRATION, "Wand calibration cloud", "", "Press to select cloud for wand calibration." );  
    m_Gui->Label("Wnd c.VME",&m_StickCalibrationName);
    m_Gui->Button(ID_LIMB_CALIBRATION, "Target calibration cloud", "", "Press to select target cloud with wand calibration." );  
    m_Gui->Label("Trg c. VME:",&m_LimbCalibrationName);
    m_Gui->Button(ID_LIMB_CLOUD, "Target cloud", "", "Press to select target cloud." );  
    m_Gui->Label("Target VME: ",&m_LimbCloudName);

    m_Gui->Button(ID_TRG_MOT, "Target motion", "", "Press to select target motion" );  
    m_Gui->Label("Target VME: ",&m_TrgMotionName);


    m_Gui->Label("");
    m_Gui->FileOpen(ID_LOAD_SCRIPT, "Script",  &m_ScriptFName, "*.srp");
    m_Gui->Label("");
    m_Gui->Button(ID_CLEAN_SCRIPT, "Clean", "", "Press to cancel script using" );  
    m_Gui->Label("");
    m_Gui->FileOpen(ID_LOAD_DICTIONARY, "Dictionary",  &m_DictionaryFName);
    m_Gui->Label("");
    m_Gui->Combo(ID_REG_TYPE, _("reg. type"), &m_RegistrationMode, 3, choices_string); 
    m_Gui->OkCancel();
  }
  ShowGui();
}

//----------------------------------------------------------------------------
void mmoStickPalpation::OpStop(int result)
//----------------------------------------------------------------------------
{
  if (result == OP_RUN_CANCEL)
  {
    HideGui();
    mafEventMacro(mafEvent(this,result));
  }
  else if (result == OP_RUN_OK)
  {
    if(m_ScriptFName == "")
    {
      if(m_StickCalibration == NULL || m_StickDefinition == NULL || m_LimbCalibration == NULL || m_LimbCloud == NULL)
      {
        wxMessageBox("Not all data defined for operation.","Alert", wxOK , NULL);
        return;
      }
    }
    else
    {
      if(m_StickDefinition == NULL || (m_TrgMotion == NULL && m_LimbCloud == NULL))
      {
        wxMessageBox("Not all data defined for operation.","Alert", wxOK , NULL);
        return;
      }
    }
    HideGui();
    mafEventMacro(mafEvent(this,result));
  }
}
//----------------------------------------------------------------------------
void mmoStickPalpation::SetNodeName(mafVME *pVME, mafString *pName) 
//----------------------------------------------------------------------------
{
   *pName = pVME->GetName();
  if(pVME->GetParent() != NULL)
  {
    *pName = *pName + " parent:";
    *pName = *pName + pVME->GetParent()->GetName();
  }
}
//----------------------------------------------------------------------------
void mmoStickPalpation::OnEvent(mafEventBase *e) 
//----------------------------------------------------------------------------
{ 
  int  i;
  bool b;
  switch(e->GetId())
  {
    case wxOK:          
    { 
      OpStop(OP_RUN_OK);
      break;
    }
    case wxCANCEL:
    {    
      OpStop(OP_RUN_CANCEL);
      break;
    }
    case ID_REG_TYPE:
      {
        break;
      }
    case ID_LOAD_DICTIONARY:
    {
      break;
    }
    case ID_LOAD_SCRIPT:
    {
      if(m_ScriptFName != "")
      {
        ReadDictionary(&m_ScriptFName, m_LMDict);
      }
      break;
    }
    case ID_CLEAN_SCRIPT:
    {
      m_ScriptFName = "";
      m_Gui->Update();
      break;
    }
    case ID_STICK_CALIBRATION:
    {
      mafString s("Choose wand calibration cloud");
      mafEvent e(this,VME_CHOOSE, &s);
      mafEventMacro(e);
      if(e.GetVme() == NULL)
      {
        return;
      }
      if(!e.GetVme()->IsA("mafVMELandmarkCloud"))
      {
        wxMessageBox("Selected VME should be of mafVMELandmarkCloud type.","Warning", wxOK|wxICON_WARNING , NULL);
        return;
      }
      m_StickCalibration = (mafVMELandmarkCloud *)e.GetVme();
      SetNodeName(m_StickCalibration, &m_StickCalibrationName);
      m_Gui->Update();
      break;
    }
    case ID_STICK_DEFINITION:
    {
      mafString s("Choose wand definition cloud");
      mafEvent e(this,VME_CHOOSE, &s);
      mafEventMacro(e);
      if(e.GetVme() == NULL)
      {
        return;
      }
      if(!e.GetVme()->IsA("mafVMELandmarkCloud"))
      {
        wxMessageBox("Selected VME should be of mafVMELandmarkCloud type.","Warning", wxOK|wxICON_WARNING , NULL);
        return;
      }
      m_StickDefinition = (mafVMELandmarkCloud *)e.GetVme();
      SetNodeName(m_StickDefinition, &m_StickDefinitionName);
      m_Gui->Update();
      break;
    }
    case ID_LIMB_CALIBRATION:
    {
      mafString s("Choose  target calibration cloud");
      mafEvent e(this,VME_CHOOSE, &s);
      mafEventMacro(e);
      if(e.GetVme() == NULL)
      {
        return;
      }
      if(!e.GetVme()->IsA("mafVMELandmarkCloud"))
      {
        wxMessageBox("Selected VME should be of mafVMELandmarkCloud type.","Warning", wxOK|wxICON_WARNING , NULL);
        return;
      }
      m_LimbCalibration = (mafVMELandmarkCloud *)e.GetVme();
      SetNodeName(m_LimbCalibration, &m_LimbCalibrationName);
      m_Gui->Update();
      break;
    }
    case ID_LIMB_CLOUD:
    {
      mafString s("Choose target cloud");
      mafEvent e(this,VME_CHOOSE, &s);
      mafEventMacro(e);
      if(e.GetVme() == NULL)
      {
        return;
      }
      if(!e.GetVme()->IsA("mafVMELandmarkCloud"))
      {
        wxMessageBox("Selected VME should be of mafVMELandmarkCloud type.","Warning", wxOK|wxICON_WARNING , NULL);
        return;
      }
      m_LimbCloud = (mafVMELandmarkCloud *)e.GetVme();
      SetNodeName(m_LimbCloud, &m_LimbCloudName);
      m_Gui->Update();
      break;
    }
    case ID_TRG_MOT:
    {
      mafString s("Choose node with landmark clouds as children");
      mafEvent e(this,VME_CHOOSE, &s);
      mafEventMacro(e);
      if(e.GetVme() == NULL)
      {
        return;
      }
      b = false;
      for(i = 0; i < e.GetVme()->GetNumberOfChildren(); i++)
      {
        if(e.GetVme()->GetChild(i)->IsA("mafVMELandmarkCloud"))
        {
          b = true;
          break;
        }
      }
      if(!b)
      {
        wxMessageBox("Selected VME should contain mafVMELandmarkCloud as a child.","Warning", wxOK|wxICON_WARNING , NULL);
        return;
      }
      m_TrgMotion = mafVME::SafeDownCast(e.GetVme());
      SetNodeName(m_TrgMotion, &m_TrgMotionName);
      m_Gui->Update();
      break;
    }
    default:
    {
      mafEventMacro(*e); 
    }
    break;
  }
}

//----------------------------------------------------------------------------
void mmoStickPalpation::GetLandmark(mafVMELandmarkCloud  *pStickCloud, int nIDx, double vpPoint[4], mafTimeStamp t) const
//----------------------------------------------------------------------------
{
  pStickCloud->GetLandmark(nIDx, vpPoint, t);
  vpPoint[3] = 1.;
}

//----------------------------------------------------------------------------
mafVME *mmoStickPalpation::MatchStickDefinition(mafVME *pVME, const char *name) const
//----------------------------------------------------------------------------
{
  bool bStickDetected = false;
  bool bWandDetected  = false;
  bool bDefDetected   = false;
  mafVME *paDetArray[2];
  wxInt32 nI;
  char const *pVMEName;

  //supposed to have correct type
  if(!pVME->IsA("mafVMELandmarkCloud"))
  {
    return NULL;
  }
  //this cloud is supposed to have words "stick, "wand and "definitions" somewhere in it's or it's parent name
  paDetArray[0] = pVME;
  paDetArray[1] = pVME->GetParent();
  for(nI = 0; nI < DIM(paDetArray); nI++)
  {
    if(paDetArray[nI] == NULL)
    {
      continue;
    }
    pVMEName = paDetArray[nI]->GetName();
    if(strstr(pVMEName, "stick") != NULL || strstr(pVMEName, "Stick") != NULL || strstr(pVMEName, "STICK") != NULL)
    {
      bStickDetected = true;
    }
    if(strstr(pVMEName, "wand") != NULL || strstr(pVMEName, "Wand") != NULL || strstr(pVMEName, "WAND") != NULL)
    {
      bWandDetected = true;
    }
    if(strstr(pVMEName, "definition") != NULL || strstr(pVMEName, "Definition") != NULL || strstr(pVMEName, "DEFINITION") != NULL)
    {
      bDefDetected = true;
    }
  }
  if(bDefDetected && (bStickDetected || bWandDetected))
  {
    return pVME;
  }
  return NULL;
}

//----------------------------------------------------------------------------
mafVME *mmoStickPalpation::MatchStick(mafVME *pVME, const char *name) const
//----------------------------------------------------------------------------
{
  bool bStickDetected = false;
  bool bWandDetected  = false;
  bool bPlpDetected   = false;
  char const *pVMEName;

  if(pVME == NULL)
    return NULL;
  //supposed to have correct type
  if(!pVME->IsA("mafVMELandmarkCloud"))
  {
    return NULL;
  }
  pVMEName = pVME->GetName();
  if(strstr(pVMEName, "stick") != NULL || strstr(pVMEName, "Stick") != NULL || strstr(pVMEName, "STICK") != NULL)
  {
    bStickDetected = true;
  }
  if(strstr(pVMEName, "wand") != NULL || strstr(pVMEName, "Wand") != NULL || strstr(pVMEName, "WAND") != NULL)
  {
    bWandDetected = true;
  }
  if(strstr(pVMEName, "palpator") != NULL || strstr(pVMEName, "Palpator") != NULL || strstr(pVMEName, "PALPATOR") != NULL)
  {
    bPlpDetected = true;
  }

  if(bPlpDetected || bStickDetected || bWandDetected)
  {
    return pVME;
  }
  return NULL;
}
//----------------------------------------------------------------------------
mafVME *mmoStickPalpation::MatchWithName(mafVME *pVME, const char *name) const
//----------------------------------------------------------------------------
{
  if(pVME == NULL)
    return NULL;
  //supposed to have correct type
  if(!pVME->IsA("mafVMELandmarkCloud"))
  {
    return NULL;
  }
  if(strstr(pVME->GetName(), name) != NULL)
    return pVME;
  return NULL;
}
//----------------------------------------------------------------------------
mafVME *mmoStickPalpation::MatchCriterion(mafVME *pRoot, MatchName pCritFunc, const char *name) const
//----------------------------------------------------------------------------
{
  wxInt32 nI;
  mafVME  *pRetVME = NULL;

  if(pRoot == NULL)
  {
    return NULL;
  }
  pRetVME = (this->*pCritFunc)(pRoot, name);

  for(nI = 0; nI < pRoot->GetNumberOfChildren() && pRetVME == NULL; nI++)
  {
    pRetVME = MatchCriterion(mafVME::SafeDownCast(pRoot->GetChild(nI)), pCritFunc, name);
  }

  return pRetVME;
}

//----------------------------------------------------------------------------
void mmoStickPalpation::OpDo()
//----------------------------------------------------------------------------
{
  wxInt32 nL;
  mafVME  *vme;
  
  //modified by Stefano. 18-9-2003
  wxBusyInfo wait("Please wait, working...");

  if(m_ScriptFName == "")
  {
    ProcessSingleLM();
    return;
  }

  if(m_DictionaryFName == "")
    wxMessageBox("Dictionary for c3d import is not specified. Trying to use C3D_dictionary.txt","Alert", wxOK , NULL);


  wxString spath, sname, sext;
  wxSplitPath(m_ScriptFName.GetCStr(), &spath, &sname, &sext);

  for(nL = 0; nL < m_LMDict.size(); nL++)
  {
    wxString file(spath);
    wxString dict(m_DictionaryFName);

    file += "\\";
    file += m_LMDict[nL].first; 
    file += ".c3d";

    if(dict == "")
    {
      dict  = spath;
      dict += "\\C3D_dictionnary.txt";
    }

    m_LimbCloud        = NULL;
    m_LimbCalibration  = NULL;
    m_StickCalibration = NULL;

  
    mafVMEC3DData *reader;
    mafNEW(reader);
    reader->SetFileName(file);
    reader->SetDictionaryFileName(dict);

    reader->DictionaryOn();

    reader->Read();

    wxString path, name, ext;
    wxSplitPath(file.c_str(),&path,&name,&ext);

    vme = reader;
    vme->SetName(name);

    mafEventMacro(mafEvent(this,VME_ADD,vme));

#if _MSC_VER >= 1400
	//BES: 3.3.2008 - VS 2005+ and also standard C++ requires the fully qualified method name
	m_LimbCloud        = mafVMELandmarkCloud::SafeDownCast(MatchCriterion(m_TrgMotion, 
		&mmoStickPalpation::MatchWithName, m_LMDict[nL].second));
	m_LimbCalibration  = mafVMELandmarkCloud::SafeDownCast(MatchCriterion(vme, 
		&mmoStickPalpation::MatchWithName, m_LMDict[nL].second));
	m_StickCalibration = mafVMELandmarkCloud::SafeDownCast(MatchCriterion(vme, 
		&mmoStickPalpation::MatchStick, NULL));
#else
    m_LimbCloud        = mafVMELandmarkCloud::SafeDownCast(MatchCriterion(m_TrgMotion, MatchWithName, m_LMDict[nL].second));
    m_LimbCalibration  = mafVMELandmarkCloud::SafeDownCast(MatchCriterion(vme, MatchWithName, m_LMDict[nL].second));
    m_StickCalibration = mafVMELandmarkCloud::SafeDownCast(MatchCriterion(vme, MatchStick, NULL));
#endif

    if(m_LimbCloud != NULL && m_LimbCalibration != NULL && m_StickCalibration != NULL)
      ProcessSingleLM();
    mafEventMacro(mafEvent(this,VME_REMOVE,vme));
    mafDEL(vme);
  }
}

//----------------------------------------------------------------------------
void mmoStickPalpation::ProcessSingleLM()
//----------------------------------------------------------------------------
{
  wxInt32                   nI, nJ, nK;
  vtkMatrix4x4              *mVTK = NULL;
  std::vector<double>       tips;
  std::vector<int>          tipsIdx;
  std::vector<mafTimeStamp> kframes1;

  
  if(m_StickDefinition->GetNumberOfLandmarks() - 1 != m_StickCalibration->GetNumberOfLandmarks())
  {
    wxMessageBox("Wand definition and wand calibration have different landmark number.","Alert", wxOK , NULL);
    return;
  }
  if(m_LimbCalibration->GetNumberOfTimeStamps() != m_StickCalibration->GetNumberOfTimeStamps())
  {
    wxMessageBox("Wand calibration and target calibration have different number of frames.","Alert", wxOK , NULL);
    return;
  }

  if(m_LimbCalibration->GetNumberOfTimeStamps() == 0)
  {
    wxMessageBox("Target calibration have no frames.","Alert", wxOK , NULL);
    return;
  }

  bool bStDefState = LMCSetState(m_StickDefinition, m_Listener, true);
  
  //2. find Tip coods in this system
  int tipIndex = m_StickDefinition->FindLandmarkIndex("TIP");
  if(tipIndex == -1)
  {
    wxMessageBox("TIP mark not found on wand. Either revise your dictionary or rename tip to TIP. Scientist should know that wand without tip is useless.","Failure", wxOK , NULL);
    return;
  }
  m_StickDefinition->GetLandmark(tipIndex, m_LocalTip);
  m_LocalTip[3] = 1.0;



  //3. Map wand definition to each frame of wand calibration
  bool bCalState = LMCSetState(m_StickCalibration, m_Listener, true);

  //mafProgressBarShowMacro();
  //mafProgressBarSetTextMacro("Registering palpated landmark: step 1/3 mapping TIP to calibration...");
  m_StickCalibration->GetTimeStamps(kframes1);

  for(nI = 0; nI < kframes1.size(); nI++)
  {
    double deviation = 0.0;
    double result[4];
    mafTimeStamp  currTime  = kframes1[nI];
    vtkMatrix4x4 *t_matrix = vtkMatrix4x4::New();

    if(ExtractMatchingPoints(m_StickDefinition, m_StickCalibration, -1, currTime))
      deviation = RegisterPoints(t_matrix);
    else
      continue;

    double tipPnt[4] = {m_LocalTip[0], m_LocalTip[1], m_LocalTip[2], m_LocalTip[3]};

    t_matrix->MultiplyPoint(tipPnt, result);
    for(nJ = 0; nJ < 3; nJ++)
      tips.push_back(result[nJ]);
    tips.push_back(1.0);
    tipsIdx.push_back(nI);

    vtkDEL(t_matrix);

    //mafProgressBarSetValueMacro((100 * nI / m_LimbCalibration->GetNumberOfTimeStamps()));
  }
  //mafProgressBarHideMacro();  

  LMCSetState(m_StickCalibration, m_Listener, bCalState);
  LMCSetState(m_StickDefinition, m_Listener, bStDefState);

  if(tips.size() == 0)
  {
    wxMessageBox("Stick definition and stick calibration are incompatible.","Alert", wxOK , NULL);
    return;
  }

  m_LimbCalibration->GetTimeStamps(kframes1);

  bool bLCalClosed = LMCSetState(m_LimbCalibration, m_Listener, true);

  //create cloud for averaged positions
  mafVMELandmarkCloud *averagedCalibr = mafVMELandmarkCloud::New();
  averagedCalibr->Open();
  averagedCalibr->SetName("averaged landmark cloud");
  averagedCalibr->SetRadius(15);

  //mafProgressBarShowMacro();
  //mafProgressBarSetTextMacro("Registering palpated landmark: step 2/3 averaging calibration data...");

  //4. averaging calibration cloud
  std::vector<double> lmPositions;
  lmPositions.resize(3 * m_LimbCalibration->GetNumberOfLandmarks());
  for(nI = 0; nI < 3 * m_LimbCalibration->GetNumberOfLandmarks(); nI++)
    lmPositions[nI] = 0.0;

  for(nI = 0; nI < m_LimbCalibration->GetNumberOfLandmarks(); nI++)
  {
    m_LimbCalibration->GetLandmark(nI, lmPositions[3 * nI + 0], lmPositions[3 * nI + 1], lmPositions[3 * nI + 2], kframes1[tipsIdx[0]]);
  }

  m_LocalTip[0] = tips[0];
  m_LocalTip[1] = tips[1];
  m_LocalTip[2] = tips[2];
  m_LocalTip[3] = 1.0;

  int numberRegistered = 0;
  for(nI = 1; nI < tipsIdx.size(); nI++)
  {
    double deviation = 0.0;
    double tippos[4];
    double tipsI[4];
    mafTimeStamp  currTime  = kframes1[tipsIdx[nI]];
    vtkMatrix4x4 *t_matrix = vtkMatrix4x4::New();

    if(ExtractMatchingPoints(m_LimbCalibration, m_LimbCalibration, kframes1[tipsIdx[0]], currTime))
    {
      deviation = RegisterPoints(t_matrix);
    }
    else
    {
      vtkDEL(t_matrix);
      continue;
    }
    numberRegistered++;

    for(nK = 0; nK < 4; nK ++)
      tipsI[nK] = tips[4 * nI + nK];
    t_matrix->MultiplyPoint(tipsI, tippos);
    for(nK = 0; nK < 3; nK ++)
      m_LocalTip[nK] += tippos[nK];

    for(nJ = 0; nJ < m_LimbCalibration->GetNumberOfLandmarks(); nJ++)
    {
      double lmpos[4];
      double newpos[4];
      m_LimbCalibration->GetLandmark(nJ, lmpos, currTime);
      lmpos[3] = 1.0;
      
      t_matrix->MultiplyPoint(lmpos, newpos);
      for(nK = 0; nK < 3; nK ++)
        lmPositions[3 * nJ + nK] += newpos[nK];
    }
    vtkDEL(t_matrix);
    //mafProgressBarSetValueMacro((100 * nI / m_LimbCalibration->GetNumberOfTimeStamps()));
  }
  if(numberRegistered == 0)
  {
    wxMessageBox("Problem processing limb calibration.","Alert", wxOK , NULL);
    LMCSetState(m_LimbCalibration, m_Listener, bLCalClosed);
    vtkDEL(averagedCalibr);
    return;
  }
  for(nI = 0; nI < 3 * m_LimbCalibration->GetNumberOfLandmarks(); nI++)
    lmPositions[nI] /= numberRegistered;
  for(nK = 0; nK < 3; nK ++)
    m_LocalTip[nK] /= numberRegistered;



  //5. fill cloud with averaged positions
  for(nJ = 0; nJ < m_LimbCalibration->GetNumberOfLandmarks(); nJ++)
  {
    averagedCalibr->AppendLandmark(lmPositions[3 * nJ + 0], lmPositions[3 * nJ + 1], lmPositions[3 * nJ + 2], m_LimbCalibration->GetLandmarkName(nJ));
  }
  //mafProgressBarHideMacro();  

  LMCSetState(m_LimbCalibration, m_Listener, bLCalClosed);

  m_LimbCloud->GetTimeStamps(kframes1);

  bool bLimbClosed = LMCSetState(m_LimbCloud, m_Listener, false);

  //6. process registration of landmark
  //mafProgressBarShowMacro();
  //mafProgressBarSetTextMacro("Registering palpated landmark: step 3/3 final registration...");
  bool added = false;
  for (int t = 0; t < kframes1.size(); t++)
  {
    vtkMatrix4x4 *t_matrix = vtkMatrix4x4::New();
    mafTimeStamp currTime  = kframes1[t];
    double       deviation = 0.0;

    long         p         = t * 100 / kframes1.size();
    //mafProgressBarSetValueMacro(p);

    //calculate registration transform
    if(ExtractMatchingPoints(averagedCalibr, m_LimbCloud, -1, currTime))
      deviation = RegisterPoints(t_matrix);
    else
      continue;

    //add landmark if not existed
    if(!added)
    {
      char lmNameNew[1000];
      int index = 0;
      sprintf(lmNameNew, "%s", m_LimbCalibration->GetName());
      do 
      {
        m_NewIndex = m_LimbCloud->FindLandmarkIndex(lmNameNew);
        index++;
        sprintf(lmNameNew, "%s%d", m_LimbCalibration->GetName(), index);
      }
      while(m_NewIndex != -1);

      m_NewIndex = m_LimbCloud->AppendLandmark(m_LimbCalibration->GetParent()->GetName());
      m_LimbCloud->Modified();
      m_LimbCloud->Update();
      added = true;
    }

    {
      double result[4];
      t_matrix->MultiplyPoint(m_LocalTip, result);
      m_LimbCloud->SetLandmark(m_NewIndex, result[0], result[1], result[2], currTime);
    }
    vtkDEL(t_matrix);
  }
  //mafProgressBarHideMacro();
  m_UndoList.push_back(std::make_pair(m_LimbCloud, m_NewIndex));
  int y = m_UndoList.size();

  LMCSetState(m_LimbCloud, m_Listener, bLimbClosed);

  vtkDEL(averagedCalibr);
}


//----------------------------------------------------------------------------
void mmoStickPalpation::OpUndo()
//----------------------------------------------------------------------------
{
  for(int i = m_UndoList.size(); i > 0; i--)
  {
    mafVMELandmarkCloud *lmUpd   = m_UndoList[i - 1].first;
    wxInt32             lmUpdIdx = m_UndoList[i - 1].second;
    bool bCloudWasOpen = LMCSetState(lmUpd, m_Listener, false);

    lmUpd->RemoveLandmark(lmUpdIdx);
    lmUpd->Modified();

    LMCSetState(lmUpd, m_Listener, bCloudWasOpen);
  }
  m_UndoList.clear();
  //restore previous selection
  mafEventMacro(mafEvent(this,VME_SELECT,m_Input));
}

//----------------------------------------------------------------------------
int mmoStickPalpation::ExtractMatchingPoints(mafVMELandmarkCloud *src, mafVMELandmarkCloud *trg, mafTimeStamp srctime, mafTimeStamp trgtime)
//----------------------------------------------------------------------------
{
  double lmsrcd[3];
  double lmtrgd[3];
  int    i, j;
  int    npSource    = src->GetNumberOfLandmarks();
  int    npTarget    = trg->GetNumberOfLandmarks();
  int    ncp         = 0;
  int    foundNumber = 0;

  m_pointsSource->Reset();
  m_pointsTarget->Reset();

  //number of common points in src and trg

  for(i = 0; i < npSource; i++)
  {
    wxString SourceLandmarkName = src->GetLandmarkName(i);
    //search for landmark with the same name
    for(j = 0; j < npTarget; j++)
    {
      wxString TargetLandmarkName = trg->GetLandmarkName(j);
      if(SourceLandmarkName == TargetLandmarkName)
      {
        foundNumber++;
        break;
      }
    }
    if(j == npTarget)
      continue;

    if(!src->GetLandmarkVisibility(i,srctime) || !trg->GetLandmarkVisibility(j,trgtime))
      continue;

    //add new points to arrays if no one is zero (zero indicates that data are invalid)
    src->GetLandmark(i, lmsrcd, srctime);
    trg->GetLandmark(j, lmtrgd, trgtime);

    if(lmsrcd[0] == 0.0 && lmsrcd[1] == 0.0 && lmsrcd[2] == 0.0)
      continue;
    if(lmtrgd[0] == 0.0 && lmtrgd[1] == 0.0 && lmtrgd[2] == 0.0)
      continue;

    m_pointsSource->InsertNextPoint(lmsrcd);
    m_pointsTarget->InsertNextPoint(lmtrgd);
    ncp++;
  }

  if(foundNumber < 3)
  {
    wxLogMessage("Not enough matching landmarks found!");
  }
  else if(ncp < 3)
  {
    wxLogMessage("Not enough visible matching landmarks found at this timestamp!");
  }
  m_numPoints = ncp;
  return ncp;
}
//----------------------------------------------------------------------------
double mmoStickPalpation::RegisterPoints(vtkMatrix4x4 *res_matrix)
//----------------------------------------------------------------------------
{
  int i;
  double deviation = 0.0;
  double dx, dy, dz;
  assert(m_pointsSource && m_pointsTarget);

  vtkWeightedLandmarkTransform *RegisterTransform = vtkWeightedLandmarkTransform::New();

  //setup transform calculator
  RegisterTransform->SetSourceLandmarks(m_pointsSource);  
  RegisterTransform->SetTargetLandmarks(m_pointsTarget);  
  
  if(m_weight)
  {
    int number = m_numPoints;
    RegisterTransform->SetWeights(m_weight, number);  
  }

  switch (m_RegistrationMode)						
  {
  case RIGID:
    RegisterTransform->SetModeToRigidBody();
    break;
  case SIMILARITY:
    RegisterTransform->SetModeToSimilarity();
    break;
  case AFFINE:
    RegisterTransform->SetModeToAffine();
    break;
  }

  //calculate transform
  RegisterTransform->Update();
  RegisterTransform->GetMatrix(res_matrix);

  //calculate deviation
  for(i = 0; i < m_pointsSource->GetNumberOfPoints(); i++)
  {
    double coord[4];
    double result[4];
    double target[3];
    m_pointsSource->GetPoint(i, coord);
    coord[3] = 1.0;
    
    m_pointsTarget->GetPoint(i, target);

    //transform point
    res_matrix->MultiplyPoint(coord, result);

    dx = target[0] - result[0];
    dy = target[1] - result[1];
    dz = target[2] - result[2];

    deviation += dx * dx + dy * dy + dz * dz;
  }
  if(m_pointsSource->GetNumberOfPoints() != 0)
    deviation /= m_pointsSource->GetNumberOfPoints();
  deviation = sqrt(deviation);

  vtkDEL(RegisterTransform);
  return deviation;
}
