/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoBuildHierarchy.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev
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

#include "mmoBuildHierarchy.h"
#include "wx/textfile.h"
#include "wx/arrimpl.cpp"
#include "wx/busyinfo.h"
#include <math.h>

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafGUI.h"

#include "mafOpExplodeCollapse.h"
#include "mafDictionary.h"

#include "mafTransformFrame.h"
#include "mmuTimeSet.h"
#include "mafVMELandmarkCloud.h"
#include "mafAbsMatrixPipe.h"
#include "mafSmartPointer.h"
#include "mafVMESurface.h"
#include "mafVMELandmark.h"

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
//----------------------------------------------------------------------------
// widget id's
//----------------------------------------------------------------------------
enum 
{
  ID_DEFAULT = MINID,
  ID_LOAD_HIERARCHY ,
  ID_LOAD_DICTIONARY,
  ID_LAST,
  ID_FORCED_DWORD = 0x7fffffff
};


//----------------------------------------------------------------------------
mmoBuildHierarchy::mmoBuildHierarchy(wxString label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_OP;
  m_Canundo = false;
  m_root    = NULL;
}

//----------------------------------------------------------------------------
mmoBuildHierarchy::~mmoBuildHierarchy()
//----------------------------------------------------------------------------
{
  Destroy(&m_root);
}

//----------------------------------------------------------------------------
mafOp* mmoBuildHierarchy::Copy()   
//----------------------------------------------------------------------------
{
  return new mmoBuildHierarchy(m_Label);
}

//----------------------------------------------------------------------------
bool mmoBuildHierarchy::Accept(mafNode* vme)
//----------------------------------------------------------------------------
{
  if(!vme) return false;

  if(!vme->IsA("mafVME"))
  {
    return false;
  }
  
  return true;
}


//----------------------------------------------------------------------------
void mmoBuildHierarchy::OpRun()   
//----------------------------------------------------------------------------
{
  CreateGui();
}

//----------------------------------------------------------------------------
void mmoBuildHierarchy::CreateGui()
//----------------------------------------------------------------------------
{
  if(m_Gui == NULL)
  {
    m_Gui = new mafGUI(this);
    m_Gui->SetListener(this);
    m_Gui->FileOpen(ID_LOAD_DICTIONARY, _("Dictionary"),  &m_DictionaryFName);
    m_Gui->Label("");
    m_Gui->FileOpen(ID_LOAD_HIERARCHY, _("Hierarchy"),  &m_HierarchyFName);
    m_Gui->Label("");
    m_Gui->OkCancel();
  }
  ShowGui();
}

//----------------------------------------------------------------------------
/*void mmoBuildHierarchy::OpStop(int result)
//----------------------------------------------------------------------------
{
  if (result == OP_RUN_CANCEL)
  {
    HideGui();
    mafEventMacro(mafEvent(this,result));
  }
  else if (result == OP_RUN_OK)
  {
    HideGui();
    mafEventMacro(mafEvent(this,result));
  }
}*/
//----------------------------------------------------------------------------
void mmoBuildHierarchy::OnEvent(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{ 
  switch(maf_event->GetId())
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
    case ID_LOAD_DICTIONARY:
    {
      if(m_DictionaryFName != "")
      {
        ReadDictionary(&m_DictionaryFName, m_dictionary);
      }
      break;
    }
    case ID_LOAD_HIERARCHY:
    {
      if(m_HierarchyFName != "")
      {
        ReadFromFile(m_HierarchyFName.GetCStr());
      }
      break;
    }
    default:
    {
      mafEventMacro(*maf_event); 
    }
    break;
  }
}

#define OLDVERSION
//----------------------------------------------------------------------------
static void makeReparent(mafVME *child, mafVME *newParent)
//----------------------------------------------------------------------------
{
  int num, t;
  mmuTimeVector input_time;
  mmuTimeVector target_time;
  mmuTimeVector time;
  mafTimeStamp  cTime, startTime;
  mafVME        *oldParent;

  oldParent = mafVME::SafeDownCast(child->GetParent());

  child->GetAbsTimeStamps(input_time);
  newParent->GetAbsTimeStamps(target_time);
  mmuTimeSet::Merge(input_time,target_time,time);
  num = time.size();

  startTime = newParent->GetTimeStamp();

  std::vector< mafAutoPointer<mafMatrix> > new_input_pose;
  new_input_pose.resize(num);

  for (t = 0; t < num; t++)
  {
    new_input_pose[t] = mafMatrix::New();
  }

  //change reference system
  mafSmartPointer<mafTransformFrame> transform;
  for (t = 0; t < num; t++)
  {
    cTime = time[t];

    child->SetTimeStamp(cTime);
    newParent->SetTimeStamp(cTime);
    oldParent->SetTimeStamp(cTime);
#ifdef OLDVERSION
    transform->SetTimeStamp(cTime);
    transform->SetInput(child->GetMatrixPipe());
    transform->SetInputFrame(oldParent->GetAbsMatrixPipe());
    transform->SetTargetFrame(newParent->GetAbsMatrixPipe());
    transform->Update();

    new_input_pose[t]->DeepCopy(transform->GetMatrixPointer());
#else
    mafMatrix mtr;
    child->GetOutput()->GetAbsMatrix(mtr, cTime);
    new_input_pose[t]->DeepCopy(&mtr);
#endif

  }

  child->SetTimeStamp(startTime);
  newParent->SetTimeStamp(startTime);
  oldParent->SetTimeStamp(startTime);
#ifdef OLDVERSION
  for (t = 0; t < num; t++)
  {
    child->SetMatrix(*new_input_pose[t]);
  }
#endif
  if (child->ReparentTo(newParent) == MAF_OK)
  {
    //mafEventMacro(mafEvent(this,CAMERA_UPDATE));
  }
  else
  {
    mafLogMessage("Something went wrong while reparenting (bad pointer or memory errors)"); 
  }
#ifndef OLDVERSION
  for (t = 0; t < num; t++)
  {
    child->SetAbsMatrix(*new_input_pose[t], time[t]);
  }
#endif
}

//----------------------------------------------------------------------------
static void reparentAll(mmoBuildHierarchy::mafFrame *pRoot, mafVME *root)
//----------------------------------------------------------------------------
{
  mmoBuildHierarchy::mafFrame *pNext;
  if(pRoot == NULL)
    return;
  if(pRoot->GetVME() != NULL)
    makeReparent(pRoot->GetVME(), root);

  reparentAll(pRoot->GetChild(), root);

  for(pNext = pRoot->GetNext(); pNext != NULL; pNext = pNext->GetNext())
  {
    reparentAll(pNext, root);
  }
}


//----------------------------------------------------------------------------
static bool searchVMEInTree(mmoBuildHierarchy::mafFrame *pRoot, mafVME *search)
//----------------------------------------------------------------------------
{
  mmoBuildHierarchy::mafFrame *pNext;
  if(pRoot == NULL)
    return false;
  if(pRoot->GetVME() == search)
    return true;
  if(searchVMEInTree(pRoot->GetChild(), search))
    return true;

  for(pNext = pRoot->GetNext(); pNext != NULL; pNext = pNext->GetNext())
  {
    if(searchVMEInTree(pNext, search))
      return true;
  }
  return false;
}

//----------------------------------------------------------------------------
static void restoreRootPlaces(mmoBuildHierarchy::mafFrame *pRoot, mafVME *input)
//----------------------------------------------------------------------------
{
  mmoBuildHierarchy::mafFrame *pNext;
  if(pRoot == NULL)
    return;
  if(pRoot->GetVME() == NULL)
    restoreRootPlaces(pRoot->GetChild(), input);
  else
  {
    if(!searchVMEInTree(pRoot->GetChild(), pRoot->GetParentVME()))
      makeReparent(pRoot->GetVME(), pRoot->GetParentVME());
    if(!searchVMEInTree(pRoot->GetChild(), input))
      makeReparent(pRoot->GetVME(), input);
  }

  for(pNext = pRoot->GetNext(); pNext != NULL; pNext = pNext->GetNext())
  {
    restoreRootPlaces(pNext, input);
  }
}
//----------------------------------------------------------------------------
static bool checkPossibility(mmoBuildHierarchy::mafFrame *pRoot)
//----------------------------------------------------------------------------
{
  mmoBuildHierarchy::mafFrame *pNext;
  if(pRoot == NULL)
    return true;
  if(pRoot->GetVME() == NULL)
  {
    if(pRoot->GetParent() != NULL && pRoot->GetParent()->GetVME() != NULL && !pRoot->GetVME()->CanReparentTo(pRoot->GetParent()->GetVME()))
      return false;
  }
  if(!checkPossibility(pRoot->GetChild()))
    return false;

  for(pNext = pRoot->GetNext(); pNext != NULL; pNext = pNext->GetNext())
  {
    if(!checkPossibility(pNext))
      return false;
  }
  return true;
}

//----------------------------------------------------------------------------
static void hierarchyReparent(mmoBuildHierarchy::mafFrame *pRoot, mafVME *parent)
//----------------------------------------------------------------------------
{
  mmoBuildHierarchy::mafFrame *pNext;
  if(pRoot == NULL)
    return;

  if(parent != NULL)
  if(pRoot->GetVME() != NULL)
    makeReparent(pRoot->GetVME(), parent);

  hierarchyReparent(pRoot->GetChild(), pRoot->GetVME());

  for(pNext = pRoot->GetNext(); pNext != NULL; pNext = pNext->GetNext())
  {
    hierarchyReparent(pNext, parent);
  }
}


//----------------------------------------------------------------------------
void mmoBuildHierarchy::OpDo()
//----------------------------------------------------------------------------
{
  BindToVME((mafVME*)m_Input, m_root);

  mafVME *pVMERoot = (mafVME *)m_Input->GetRoot();

  if(!checkPossibility(m_root))
  {
    wxMessageBox(wxString::Format("Hierarchy contains impossible relation."), "Warning.", wxOK | wxCENTRE | wxICON_WARNING);
    return;
  }

  reparentAll(m_root, pVMERoot);
  hierarchyReparent(m_root, NULL);
  restoreRootPlaces(m_root, (mafVME*)m_Input);
  mafEventMacro(mafEvent(this,CAMERA_UPDATE));
  return;
}

//----------------------------------------------------------------------------
void mmoBuildHierarchy::OpUndo()
//----------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------
bool mmoBuildHierarchy::ReadFromFile(const wxString& fileName)
//----------------------------------------------------------------------------
{
  wxInt32                                      nI; 
  mafFrame                                     *pParentFrame;
  mafFrame                                     *pChildFrame;
  mafString                                    fname(fileName);
  std::vector<std::pair<wxString, wxString> >  hierContent;

  if(m_dictionary.size() == 0)
  {
    if(wxNO == wxMessageBox(wxString::Format("Dictionary is not loaded yet. Are you sure you want to continue?"), "Warning.", wxYES_NO | wxCENTRE | wxICON_WARNING))
    {
      return false;
    }
  }

  ReadDictionary(&fname, hierContent);
  if(hierContent.size() == 0)
  {
    wxMessageBox(wxString::Format("Hierarchy is empty, nothing to do."), "Warning.", wxOK | wxCENTRE | wxICON_WARNING);
    return false;
  }

  //cleanup old if any
  Destroy(&m_root);

  for(nI = 0; nI < hierContent.size(); )
  {
    if(hierContent[nI].second == "" || hierContent[nI].first == "")
    {
      //consider string in valid
      nI++;
      continue;
    }
    //create parent if needed
    bool addRoot = false;
    pParentFrame = FindFrame(hierContent[nI].first, FALSE);
    if((pParentFrame == NULL) ^ (m_root == NULL))
    {
      addRoot = true;
    }
    pParentFrame = FindFrame(hierContent[nI].first, TRUE);
    pChildFrame  = FindFrame(hierContent[nI].second, TRUE);
    if(pChildFrame->GetNext() != NULL  || pChildFrame->GetChild() != NULL)
    {
      wxMessageBox(wxString::Format("Error in line %d: Bone %s already in hierarchy", nI + 1, hierContent[nI].second.GetData()), "Error.", wxOK | wxCENTRE | wxICON_ERROR);
      cppDEL(pChildFrame);
      cppDEL(pParentFrame);
      Destroy(&m_root);
      return true;
    }
    if(pChildFrame->GetParent() != NULL)
    {
      wxMessageBox(wxString::Format("Error in line %d: Bone %s already have a parent", nI + 1, hierContent[nI].second.GetData()), "Error.", wxOK | wxCENTRE | wxICON_ERROR);
      cppDEL(pChildFrame);
      cppDEL(pParentFrame);
      Destroy(&m_root);
      return true;
    }
    if(m_root == NULL)
    {
      m_root = pParentFrame;
    }
    else if(addRoot)
    {
      pParentFrame->SetNext(m_root);
      m_root = pParentFrame;
    }
    if(pParentFrame->GetChild() == NULL)
    {
      //we are first child
      pParentFrame->SetChild(pChildFrame);
    }
    else
    {
      pChildFrame->SetNext(pParentFrame->GetChild());
      pParentFrame->SetChild(pChildFrame);
    }
    pChildFrame->SetParent(pParentFrame);
    // to next
    nI++;
  }
  return true;
}

//----------------------------------------------------------------------------
void  mmoBuildHierarchy::Destroy(mafFrame **root)
//----------------------------------------------------------------------------
{
  mafFrame *pCurrent;

  if(*root == NULL)
  {
    cppDEL(m_root);
  }
  else
  {
    if((*root)->GetParent() != NULL)
    {
      if((*root)->GetParent()->GetChild() == (*root))
      {
        (*root)->GetParent()->SetChild(NULL);
      }
      else
      {
        for(pCurrent = (*root)->GetParent()->GetChild(); pCurrent != NULL; pCurrent = pCurrent->GetNext())
        {
          if(pCurrent->GetNext() == (*root))
          {
            //fount it
            pCurrent->SetNext(NULL);
            break;
          }
        }
      }
    }
    cppDEL(*root);
  }
}
//----------------------------------------------------------------------------
void mmoBuildHierarchy::BindToVME(mafVME *pvme, mmoBuildHierarchy::mafFrame *pStart)
//----------------------------------------------------------------------------
{
  //ensure we have root
  mafVME *pVMERoot = pvme;//(mafVME *)pvme->GetRoot();
  mafVME *pFoundVME;
  mafFrame *pCur;

  if (pStart == NULL)
  {
    pStart = m_root;
  }

  if(pStart == NULL)
  {
    //nothing to do
    return;
  }

  pFoundVME = NULL;
  wxString const * pVMENameStr = LookupUserName(pStart->GetName(), m_dictionary);
  if(pVMENameStr != NULL)
  {
    pFoundVME = (mafVME*)pVMERoot->FindInTreeByName(pVMENameStr->GetData());
  }
  //and again ^_^
  if(pFoundVME == NULL)
  {
    pVMENameStr = LookupStdName(pStart->GetName(), m_dictionary);
    if(pVMENameStr != NULL)
      pFoundVME = (mafVME*)pVMERoot->FindInTreeByName(pVMENameStr->GetData());
  }
  //try again in case of failure
  if(pFoundVME == NULL)
  {
    pFoundVME = (mafVME*)pVMERoot->FindInTreeByName(pStart->GetName()->GetData());
  }

  pStart->SetVME(pFoundVME);
  if(pFoundVME != NULL)
    pStart->SetParentVME(pFoundVME->GetParent());
  if(LookupStdName(pStart->GetName(), m_dictionary) != NULL)
  {
    //pStart->SetID(GetBoneIDByName(*LookupStdName(pStart->GetName())));
  }
  else
  {
    //pStart->SetID(GetBoneIDByName(wxString(pStart->GetName()->GetData())));
  }
  //to all children
  if(pStart->GetChild() != NULL)
    BindToVME(pvme, pStart->GetChild());
  for(pCur = pStart->GetNext(); pCur != NULL; pCur = pCur->GetNext())
  {
    BindToVME(pvme, pCur);
  }
}


//----------------------------------------------------------------------------
mmoBuildHierarchy::mafFrame::mafFrame()
//----------------------------------------------------------------------------
{
  //m_landmarks = new LandmarkArray();

  m_next   = NULL;
  m_vme    = NULL;
  m_name   = NULL; 
  m_parent = NULL;
  m_child  = NULL;
}

//----------------------------------------------------------------------------
mmoBuildHierarchy::mafFrame::~mafFrame()
//----------------------------------------------------------------------------
{
  cppDEL(m_next);
  cppDEL(m_name); 
  cppDEL(m_child);
  m_parent = NULL;
  m_vme    = NULL;  
}


//----------------------------------------------------------------------------
mmoBuildHierarchy::mafFrame *mmoBuildHierarchy::FindFrameUsingDictionary(wxString const &str, bool bCreateIfNotFound)    
//----------------------------------------------------------------------------
{
  mafFrame *pRet = NULL;
  wxString const *pStr;

  pRet = FindFrame(str, bCreateIfNotFound);

  if(pRet == NULL)
  {
    pStr = LookupUserName(&str, m_dictionary);
    if(pStr != NULL)
      pRet = FindFrame(*pStr, bCreateIfNotFound);
    if(pRet == NULL)
    {
      pStr = LookupStdName(&str, m_dictionary);
      if(pStr != NULL)
        pRet = FindFrame(*pStr, bCreateIfNotFound);
    }
  }

  return (pRet);
}
//----------------------------------------------------------------------------
mmoBuildHierarchy::mafFrame *mmoBuildHierarchy::FindFrame(wxString const &str, bool bCreateIfNotFound)
//----------------------------------------------------------------------------
{
  mafFrame *pRet = NULL;

  if(m_root != NULL)
  {
    pRet = FindFrame(m_root, str);
  }

  if(bCreateIfNotFound && pRet == NULL)
  {
    pRet = new mafFrame();
    pRet->SetName(&str);
  }

  return (pRet);
}
//----------------------------------------------------------------------------
mmoBuildHierarchy::mafFrame *mmoBuildHierarchy::FindFrame(mmoBuildHierarchy::mafFrame *pRoot, wxString const &str)    
//----------------------------------------------------------------------------
{
  mafFrame *pRet;
  mafFrame *pNext;

  if(pRoot == NULL)
  {
    return (NULL);
  }
  //check this
  if(*pRoot->GetName() == str)
  {
    return (pRoot);
  }

  //look into children
  pRet = FindFrame(pRoot->GetChild(), str);
  if(pRet != NULL)
  {
    return (pRet);
  }

  //look into siblings
  for(pNext = pRoot->GetNext(); pNext != NULL && pRet == NULL; pNext = pNext->GetNext())
  {
    pRet = FindFrame(pNext, str);
  }

  return (pRet);
}
