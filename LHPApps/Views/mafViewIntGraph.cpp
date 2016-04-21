/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafViewIntGraph.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:56 $
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

#include "mafViewIntGraphWindow.h"
#include <wx/dc.h>

#ifdef IMPORTED

#include "wx/image.h"
#include "mafDecl.h"
#include "mafGUIHolder.h"
#include "mafGUI.h"
#include "mafSceneNode.h"
#include "mafSceneGraph.h"
#include "mafVMEPointSet.h"
#include "mafVMESurface.h"

#include "mafTagArray.h"
#include "mafTransform.h"
#include "mafSmartPointer.h"
#include "mafAgent.h"

#ifdef _MSC_FULL_VER
#pragma warning (disable: 4786)
#endif

#endif

#include "mafViewIntGraph.h"
#include "mafPipeIntGraph.h"

#include "mafIndent.h"

#include "mafPlotMath.h"
#include "mafPipe.h"
#include "mafPipeFactory.h"

#include "mafStringSet.h"
#include "mafTagArray.h"
#include "mafVME.h"
#include "mafVMELandmarkCloud.h"
#include "mafVMESurface.h"
#include "mafVMELandmark.h"
#include "mafVMERoot.h"

#include "vtkMAFSmartPointer.h"
#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(mafViewIntGraph);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void mafViewIntGraph::setWindowGraph()
//----------------------------------------------------------------------------
{
  mafViewIntSetGraph setGraph;
  setGraph.pidDesc  = m_Descriptions;
  setGraph.pmgGraph = m_Graph;
  if(m_RenderWindow != NULL)
  {
    m_RenderWindow->SetGraphData(&setGraph);
  }
}

//----------------------------------------------------------------------------
mafViewIntGraph::mafViewIntGraph(const wxString &label)
:mafView(label)
//----------------------------------------------------------------------------
{
  m_RenderWindow       = NULL;
  m_IsFrozen           = 0;
  m_ReferenceFrame     = 0;
  m_PlotFrameStart     = 0;
  m_PlotFrameStop      = 0;
  m_Graph              = NULL;
  m_Descriptions       = NULL;
  m_LoadMode           = false;
  m_ModifyMode         = false;
  m_VMEs               = NULL;
  m_Pipes              = NULL;
  m_Sg                 = NULL;
  m_Rwi                = NULL;
}

//----------------------------------------------------------------------------
mafViewIntGraph::~mafViewIntGraph() 
//----------------------------------------------------------------------------
{
  //if(m_Gui != NULL){;} //HideGui();
  cppDEL(m_Sg);  
  cppDEL(m_Rwi);
  cppDEL(m_VMEs);
  cppDEL(m_Graph);
  cppDEL(m_Pipes);
  cppDEL(m_Descriptions);
  setWindowGraph();
}
//----------------------------------------------------------------------------
mafView *mafViewIntGraph::Copy(mafObserver *Listener)
//----------------------------------------------------------------------------
{
  mafViewIntGraph *v = new mafViewIntGraph(m_Label);
  v->m_Listener = Listener;
  v->m_Id = m_Id;

  v->m_RenderWindow   = m_RenderWindow;
  v->m_IsFrozen       = m_IsFrozen;
  v->m_ReferenceFrame = m_ReferenceFrame;
  v->m_PlotFrameStart = m_PlotFrameStart;
  v->m_PlotFrameStop  = m_PlotFrameStop;
  v->m_Graph          = m_Graph;
  v->m_Descriptions   = m_Descriptions;
  v->m_LoadMode       = m_LoadMode;
  v->m_ModifyMode     = m_ModifyMode;
  v->m_VMEs           = m_VMEs;
  v->m_Pipes          = m_Pipes;

  v->Create();
  return v;
}

//----------------------------------------------------------------------------
void mafViewIntGraph::Create()
//----------------------------------------------------------------------------
{
  m_RenderWindow = new mafViewIntGraphWindow(m_Label);
  m_Win          = m_RenderWindow;

  m_RenderWindow->SetNotifiedView(this);


  m_Rwi = new mafRWI(m_Win,ONE_LAYER);
  m_Rwi->SetListener(this);//SIL. 16-6-2004: 
  m_Sg  = new mafSceneGraph(this,m_Rwi->m_RenFront,m_Rwi->m_RenBack);
  m_Sg->SetListener(this);
  m_Rwi->m_Sg = m_Sg;


  //m_Sg  = new mafSceneGraph(this,NULL,NULL);
  //m_Sg->SetListener(this);

  m_VMEs  = new VMEArray();
  m_Graph = new mafMemoryGraph(FLT_GARB, 1, 1000);

  m_Pipes = new PipeArray();
  int nIndex = 0;

  IDType setID;
  setID.zero();
  m_Graph->SetID(0,setID);
  //m_Graph->SetID(1,nIndex + 1);
  //m_Graph->SetGraphID(nIndex+1);

  m_Graph->SetXDim(1);
  m_Graph->SetDim(1);
  m_Graph->SetYDim(0/*m_VMEs->Count()*/);
  m_Graph->SetXIndex(0,0);
  //m_Graph->SetYIndex(0,1);

  m_Descriptions = new mafGraphIDDesc(m_Pipes);

  setWindowGraph();

  //set reference frame as first sequence frame
  m_ReferenceFrame   = 0;

  m_PlotFrameStart = 0;
  m_PlotFrameStop  = MAXINT;
  m_LoadMode       = false;
  m_ModifyMode     = false;
}

//----------------------------------------------------------------------------
void mafViewIntGraph::VmeAdd(mafNode *vme)
//----------------------------------------------------------------------------
{
  std::vector<mafTimeStamp> mpStamps;

  m_Sg->VmeAdd(vme);
  m_VMEs->Add(mafVME::SafeDownCast(vme));
  if(mafVME::SafeDownCast(vme)->GetNumberOfTimeStamps() > 0)
  {
    mafVME::SafeDownCast(vme)->GetTimeStamps(mpStamps);
    m_PlotFrameStop = max(m_PlotFrameStop, mpStamps[mpStamps.size() - 1]);
  }
  else
  {
    m_PlotFrameStop  = MAXINT;
  }
  loadPlotInfo(mafVME::SafeDownCast(vme));

  //load from first VME
  if(m_VMEs->GetCount() == 1)
  {
    loadPlotGen();
  }
}
//----------------------------------------------------------------------------
void mafViewIntGraph::VmeShow(mafNode *vme, bool show)
//----------------------------------------------------------------------------
{
  assert(m_Sg); 
  m_Sg->VmeShow(vme,show);
}

//----------------------------------------------------------------------------
void mafViewIntGraph::VmeUpdateProperty(mafNode *vme, bool fromTag)
//----------------------------------------------------------------------------
{
  assert(m_Sg); 
  m_Sg->VmeUpdateProperty(vme,fromTag);
}
//----------------------------------------------------------------------------
int mafViewIntGraph::GetNodeStatus(mafNode *vme)
//----------------------------------------------------------------------------
{
  return m_Sg ? m_Sg->GetNodeStatus(vme) : NODE_NON_VISIBLE;
}

//----------------------------------------------------------------------------
void mafViewIntGraph::VmeRemove(mafNode *vme)
//----------------------------------------------------------------------------
{
  int                        nVMEIndex;
  std::vector<mafTimeStamp>  mpStamps;

  m_Sg->VmeRemove(vme);
  nVMEIndex = m_VMEs->Index(mafVME::SafeDownCast(vme));
  if(nVMEIndex != wxNOT_FOUND)
  {
    m_VMEs->RemoveAt(nVMEIndex);
  }

  //support range
  if(m_VMEs->Count() > 0)
  {
    if(m_VMEs->Item(0)->GetNumberOfTimeStamps() > 0)
    {
      m_VMEs->Item(0)->GetTimeStamps(mpStamps);
      m_PlotFrameStop = mpStamps[m_VMEs->Item(0)->GetNumberOfTimeStamps() - 1];
    }
    else
    {
      m_PlotFrameStop = MAXINT;
    }
  }
  else
  {
    m_PlotFrameStop = MAXINT;
  }
}
//----------------------------------------------------------------------------
void mafViewIntGraph::VmeSelect(mafNode *vme, bool select)
//----------------------------------------------------------------------------
{
  assert(m_Sg); 
  m_Sg->VmeSelect(vme,select);
}
//----------------------------------------------------------------------------
void mafViewIntGraph::CameraUpdate() 
//----------------------------------------------------------------------------
{
  wxInt32 nI, nJ;
  wxInt32 nMaxFrame;
  std::vector<mafTimeStamp> mpStamps;

  if(m_IsFrozen)
  {
    if(m_Pipes->Count() > 0)
    {
      m_Pipes->Item(0)->m_Vme->GetTimeStamps(mpStamps);
      nMaxFrame = mpStamps[m_Pipes->Item(0)->m_Vme->GetNumberOfTimeStamps() - 1];
      for(nJ = m_PlotFrameStart; nJ < min(m_PlotFrameStop, nMaxFrame); nJ++)
      {
        for(nI = 0; nI < m_Pipes->Count(); nI++)
        {
          m_Pipes->Item(nI)->GrabData(nI, nJ);
        }
      }
    }
  }
  else
  {
    for(nI = 0; nI < m_Pipes->Count(); nI++)
    {
      m_Pipes->Item(nI)->GrabData(nI);
    }
  }

  //grab data from all pipes
  //just pass update here
  if(m_RenderWindow != NULL)
    m_RenderWindow->Update();
}
//----------------------------------------------------------------------------
mafPipe* mafViewIntGraph::GetNodePipe(mafNode *vme)
//----------------------------------------------------------------------------
{
  assert(m_Sg);
  mafSceneNode *n = m_Sg->Vme2Node(vme);
  if(!n) return NULL;
  return n->m_Pipe;
}
//----------------------------------------------------------------------------
void mafViewIntGraph::GetVisualPipeName(mafNode *node, mafString &pipe_name)
//----------------------------------------------------------------------------
{
  assert(node->IsA("mafVME"));
  mafVME *v = ((mafVME*)node);
  pipe_name = "mafPipeIntGraph";
}
//----------------------------------------------------------------------------
void mafViewIntGraph::VmeCreatePipe(mafNode *vme)
//----------------------------------------------------------------------------
{
  //mafString pipe_name = "";
  //GetVisualPipeName(vme, pipe_name);

  mafPipeIntGraph *pNewPipe;

  mafSceneNode *n = m_Sg->Vme2Node(vme);
  assert(n && !n->m_Pipe);

  pNewPipe = new mafPipeIntGraph();
  pNewPipe->Create(n);
  pNewPipe->SetView(this);
  n->m_Pipe = pNewPipe;
  m_Pipes->Add(pNewPipe);
  //m_Graph->Clean();
  //m_Graph->SetDim(m_Pipes->Count() + 1);
  //m_Graph->SetYDim(m_Pipes->Count());
  if(!m_LoadMode)
  {
    IDType setID;
    setID[0] = m_Pipes->Index(pNewPipe);
    setID[1] = 1;
    m_Graph->AddYVar(setID);
  }
  return;
  //mafErrorMessage(_("Cannot create visual pipe object of type \"%s\"!"),pipe_name.GetCStr());
}

//----------------------------------------------------------------------------
void mafViewIntGraph::VmeDeletePipe(mafNode *vme)
//----------------------------------------------------------------------------
{
  m_NumberOfVisibleVme--;
  mafSceneNode *n = m_Sg->Vme2Node(vme);
  int nPipeIndex;
  int nI;

  assert(n && n->m_Pipe);
  nPipeIndex = m_Pipes->Index((mafPipeIntGraph *)n->m_Pipe);
  if(nPipeIndex != wxNOT_FOUND)
  {
    if(m_Graph->GetXID(0)[0] == nPipeIndex && !m_Graph->GetXID(0).isZero())
    {
      IDType zero;
      zero.zero();
      m_Graph->SetXID(0, zero);
    }
    for(nI = m_Graph->GetYDim() - 1; nI >= 0; nI--)
    {
      if(nPipeIndex == m_Graph->GetYID(nI)[0] && !m_Graph->GetYID(nI).isZero())
      {
        m_Graph->RemYVar(m_Graph->GetYID(nI));
      }
    }

    for(nI = 0; nI < m_Graph->GetDim(); nI++)
    {
      if(nPipeIndex == m_Graph->GetID(nI)[0] && !m_Graph->GetID(nI).isZero())
      {
        wxASSERT(false);
      }
      else if(m_Graph->GetID(nI)[0] > nPipeIndex)
      {
        IDType setID;
        setID = m_Graph->GetID(nI);
        setID[0]--;
        m_Graph->SetID(nI, setID);
      }
    }

    m_Pipes->RemoveAt(nPipeIndex);
  }
  cppDEL(n->m_Pipe);
}
//-------------------------------------------------------------------------
mafGUI *mafViewIntGraph::CreateGui()
//-------------------------------------------------------------------------
{
  assert(m_Gui == NULL);
  m_Gui = mafView::CreateGui();
  int nMaxFrame = MAXINT;
  std::vector<mafTimeStamp> mpStamps;

  if(m_VMEs->Count() > 0)
  {
    if(m_VMEs->Item(0)->GetNumberOfTimeStamps() > 0)
    {
      m_VMEs->Item(0)->GetTimeStamps(mpStamps);
      nMaxFrame = mpStamps[mpStamps.size() - 1];
      nMaxFrame = max(nMaxFrame, 0);
    }
    else
    {
      nMaxFrame = MAXINT;
    }
  }

  //////////////////////////////////////////Plot gui
  //m_Gui = new mafGUI(this);
  m_Gui->SetListener(this);

  m_Gui->Label("General Features",true);
  //m_Gui->Integer(ID_REFERENCE_FRAME, "Reference frame", &(m_ReferenceFrame), 0, nMaxFrame, "This frame will be treated as upright(reference) for all representations that require it!");
  //m_Gui->Button(ID_FIND_REFERENCE, "Autofind reference", "", "Find best reference frames for all joints (hierarchially based or not) ");
  //m_Gui->VectorN(ID_ZOOM_START_STOP, "Frame limit", m_FreezeFrames, DIM(m_FreezeFrames), 0, nMaxFrame, "This frame will be treated as upright(reference) for all representations that require it!");

  m_Gui->Bool(ID_FREEZE_GRAPH,"Freeze graph", &(m_IsFrozen),0);

  m_Gui->Divider(2);

  m_Gui->Button(ID_SAVE_PLOT, "Save plot", "", "Save plot to VME tree");
  m_Gui->Button(ID_LOAD_PLOT, "Load plot", "", "Restore plot from VME tree");

  m_Gui->Divider(2);
  m_Gui->RollOut(ID_ROLLOUT_RENDER, "Plot appearance", m_RenderWindow->GetGUI(NULL, this, ID_SHOW_LAST), false);

  /////////////////////////////////////////DisplayList GUI
  m_Gui->Divider(2);
  //m_Gui->AddGui(m_sg->GetGui());

  //ShowGui();
  m_Gui->Update();

  return m_Gui;
}
//----------------------------------------------------------------------------
void mafViewIntGraph::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    //give it non listener child
    m_RenderWindow->OnEvent(*e);

    switch(e->GetId())
    {
    case VIEW_DELETE:
      {
        if(m_RenderWindow) 
          m_RenderWindow->Destroy();
        m_RenderWindow = NULL;
        mafEventMacro(*e);
        break;
      }
    case ID_ROLLOUT_RENDER:
      break;
    case ID_ZOOM_START_STOP:
    case ID_REFERENCE_FRAME:
      {
        m_Graph->Clean();
        if(m_IsFrozen)
        {
          CameraUpdate();
        }
        break;
      }
    case ID_FREEZE_GRAPH:
      {
        //force it for new condition
        CameraUpdate();      
        break;
      }    
    case ID_LOAD_PLOT:
      {
        loadPlot();
        break;
      }
    case ID_SAVE_PLOT:
      {
        savePlot();
        break;
      }
    default:
      Superclass::OnEvent(e);
      break;
    }

  }
  else
  {
    mafEventMacro(*maf_event);

  }

}
//----------------------------------------------------------------------------
void mafViewIntGraph::SetWindowSize(int w, int h)
//----------------------------------------------------------------------------
{
  GetRenderWindow()->SetSize(w,h);
}
//----------------------------------------------------------------------------
void mafViewIntGraph::Print(wxDC *dc, wxRect margins)
//----------------------------------------------------------------------------
{
  wxBitmap image;
  GetImage(image/*, 2*/);
  PrintBitmap(dc, margins, &image);
}
//----------------------------------------------------------------------------
void mafViewIntGraph::GetImage(wxBitmap &bmp, int magnification)
//----------------------------------------------------------------------------
{
  bmp = m_RenderWindow->GetBitmap();
}
//----------------------------------------------------------------------------
void mafViewIntGraph::OptionsUpdate()
//----------------------------------------------------------------------------
{
}

//-------------------------------------------------------------------------
void mafViewIntGraph::Print(std::ostream& os, const int tabs)// const
//-------------------------------------------------------------------------
{
  Superclass::Print(os,tabs);
  mafIndent indent(tabs);

  os << indent << "mafViewIntGraph " << '\t' << this << "\n";
  os << indent << "Name: " << '\t' << m_Label << "\n";
  os << indent << "View ID: " << '\t' << m_Id << "\n";
  os << indent << "View Mult: " << '\t' << m_Mult << "\n";
  os << indent << "Visible VME counter: " << '\t' << m_NumberOfVisibleVme << "\n";

  m_Sg->Print(os, 1);
  os << std::endl;
}

//----------------------------------------------------------------------------
void mafViewIntGraph::VmeSelect(const IDType& nGraphID, bool select)
//----------------------------------------------------------------------------
{
  mafVME *vme;
  wxInt32 nPipeIndex;
  //just scan all active pipes and find where 
  nPipeIndex = nGraphID[0];
  if(m_Pipes->GetCount() > nPipeIndex)
  {
    vme = m_Pipes->Item(nPipeIndex)->m_Vme;
    mafEventMacro(mafEvent(this, VME_SELECTED, vme));
  }
}
//----------------------------------------------------------------------------
void mafViewIntGraph::UpdateGui() 
//----------------------------------------------------------------------------
{ 
  if(m_Gui != NULL)
    m_Gui->Update();
}

//----------------------------------------------------------------------------
void mafViewIntGraph::savePlot(void)
//----------------------------------------------------------------------------
{
  wxInt32 nI;

  savePlotGen();

  for(nI = 0; nI < m_VMEs->GetCount(); nI++)
  {
    savePlotInfo(m_VMEs->Item(nI));
  }
}

//----------------------------------------------------------------------------
void mafViewIntGraph::savePlotGen(void)
//----------------------------------------------------------------------------
{
  mafStringSet *pSave = m_RenderWindow->SaveSettings();

  //save general settings

  for(int nI = 0; nI < m_VMEs->GetCount(); nI++)
  {
    m_ModifyMode = true;
    m_VMEs->Item(nI)->GetTagArray()->SetTag(mafTagItem(mafINTGG_SAVEINFO_TAG, const_cast<const char **>(pSave->GetData()), pSave->GetStringNumber()));
    mafEventMacro(mafEvent(this,VME_MODIFIED, m_VMEs->Item(0)));
    m_ModifyMode = false;
  }
  cppDEL(pSave);

}
//----------------------------------------------------------------------------
void mafViewIntGraph::loadPlotGen(void)
//----------------------------------------------------------------------------
{
  mafTagItem        Tag;
  //load general settings
  for(int nI = 0; nI < m_VMEs->GetCount(); nI++)
  {
    if(m_VMEs->Item(nI)->GetTagArray()->IsTagPresent(mafINTGG_SAVEINFO_TAG))
    {
      if(m_VMEs->Item(nI)->GetTagArray()->GetTag(mafINTGG_SAVEINFO_TAG, Tag))
      {
        m_RenderWindow->LoadSettings(&mafStringSet(Tag.GetNumberOfComponents(), Tag.GetComponents()));
        UpdateGui();
        break;
      }
    }
  }
}

//----------------------------------------------------------------------------
void mafViewIntGraph::loadPlot(void)
//----------------------------------------------------------------------------
{
  wxInt32 nI;

  loadPlotGen();

  //clean all old plots
  for(nI = m_Pipes->GetCount() - 1; nI >= 0; nI--)
  {
    mafEventMacro(mafEvent(this, VME_SHOW, m_Pipes->Item(nI)->m_Vme, false));
  }

  for(nI = 0; nI < m_VMEs->GetCount(); nI++)
  {
    loadPlotInfo(m_VMEs->Item(nI));
  }
}

//----------------------------------------------------------------------------
void mafViewIntGraph::loadPlotInfo(mafVME *vme)
//----------------------------------------------------------------------------
{
  mafTagItem        Tag;
  wxInt32           nI;
  wxChar const      *cpTagValue = NULL ;
  wxInt32           nValue;

  if(!vme->GetTagArray()->IsTagPresent(mafINTG_SAVEINFO_TAG))
  {
    return;
  }
  // X Value have double value + GDT_LAST;
  //read a values one by one and add them to plot
  for(nI = 0; nI< Tag.GetNumberOfComponents(); nI++)
  {
    cpTagValue = Tag.GetValue(nI);
    sscanf(cpTagValue, "%d", &nValue);
    if(m_Descriptions == NULL)
    {
      return;
    }
    IDType setID;
    setID[1] = mafGraphDescType(nValue % GDT_LAST);
    {
      m_LoadMode = true;
      mafEventMacro(mafEvent(this, VME_SHOW, vme, true));
      mafEventMacro(mafEvent(this, CAMERA_UPDATE));

      if(nValue >  GDT_LAST)
      {
        setID[0] = m_Pipes->Count() - 1;
        m_Graph->SetXVar(0, setID);
      }
      else// if(nValue <= GDT_LAST)
      {
        setID[0] = m_Pipes->Count() - 1;
        m_Graph->AddYVar(setID);
      }
      m_LoadMode = false;
    }
  }
}
//----------------------------------------------------------------------------
void mafViewIntGraph::savePlotInfo(mafVME *vme)
//----------------------------------------------------------------------------
{
  mafTagItem        *pTag = NULL;
  wxInt32           nI;
  wxChar const      *cpTagValue = NULL ;
  wxInt32           nPipeIndex = -1;
  wxInt32           nNumComp= 0, nCount = 0;
  wxString          sString("");
  wxString          sNumString("");
  wxChar     const  **pEntries = NULL; 

  //for all pipes
  for(nI = 0; nI< m_Pipes->GetCount(); nI++)
  {
    if(m_Pipes->Item(nI)->m_Vme == vme)
    {
      nPipeIndex = nI;
      break;
    }
  }

  if(nPipeIndex != -1)
  {
    nNumComp = 0;
    for(nI = m_Graph->GetYDim() - 1; nI >= 0; nI--)
    {
      if(nPipeIndex == m_Graph->GetYID(nI)[0] && m_Graph->GetYID(nI)[1] != 0)
      {
        //save Y var
        nNumComp++;
      }
    }
    for(nI = m_Graph->GetXDim() - 1; nI >= 0; nI--)
    {
      if(nPipeIndex == m_Graph->GetXID(nI)[0] && m_Graph->GetYID(nI)[1] != 0)
      {
        //save X var
        nNumComp++;
      }
    }
    //actually save
    nCount = 0;
    pEntries = (wxChar const **)malloc(sizeof(wxChar *) * nNumComp);
    for(nI = m_Graph->GetYDim() - 1; nI >= 0; nI--)
    {
      if(nPipeIndex == m_Graph->GetYID(nI)[0] && m_Graph->GetYID(nI)[1] != 0)
      {
        //save Y var
        sNumString.Printf("%d", m_Graph->GetYID(nI)[1]);
        pEntries[nCount] = strdup(sNumString.GetData());
        nCount ++;
      }
    }
    for(nI = m_Graph->GetXDim() - 1; nI >= 0; nI--)
    {
      if(nPipeIndex == m_Graph->GetXID(nI)[0] && m_Graph->GetYID(nI)[1] != 0)
      {
        //save X var
        sNumString.Printf("%d", m_Graph->GetYID(nI)[1] + GDT_LAST);
        pEntries[nCount] = strdup(sNumString.GetData());
        nCount ++;
      }
    }
  }

  wxASSERT(nNumComp == nCount);
  pTag = new mafTagItem(mafINTG_SAVEINFO_TAG, pEntries, nNumComp);
  m_ModifyMode = true;
  vme->GetTagArray()->SetTag(*pTag);
  mafEventMacro(mafEvent(this,VME_MODIFIED,vme));
  m_ModifyMode = false;
  cppDEL(pTag);
  for(nI = nNumComp - 1; nI >= 0; nI--)
  {
    free(const_cast<char *>(pEntries[nI]));
  }
  free(pEntries);
}
