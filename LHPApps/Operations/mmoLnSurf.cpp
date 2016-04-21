/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoLnSurf.cpp,v $
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

#include "mmoLnSurf.h"

#include "wx/textfile.h"
#include "wx/arrimpl.cpp"
#include <wx/wxprec.h>
#include "wx/busyinfo.h"
#include <math.h>

#include "mafDecl.h"
#include "mafTagArray.h"
#include "mafEvent.h"
#include "mafGUI.h"

#include "mafOpExplodeCollapse.h"

#include "mafSmartPointer.h"
#include "mafVMELandmarkCloud.h"
#include "mafVME.h"
#include "mafVMESurface.h"
#include "mafVMEPolyline.h"
#include "mafVMELandmark.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkPolyData.h"
#include <vectors.h>
#include <forarray.h>
#include <splines.h>

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
mmoLnSurf::mmoLnSurf(const wxString& label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType                = OPTYPE_OP;
  m_Canundo               = true;
  m_Surface               = NULL;
  m_Muscles               = NULL;
  m_Tendons               = NULL;
  m_rhoLine               = 0.0;
  m_rhoSurf               = 0.0;
  m_sgmSurf               = 0.0;
  m_splDim                = 10;
  m_xDim                  = 10;
  m_yDim                  = 10;
  m_parseNames            = 1;
  m_generateLinesSurfaces = 0;
}

//----------------------------------------------------------------------------
mmoLnSurf::~mmoLnSurf()
//----------------------------------------------------------------------------
{
  mafDEL(m_Surface);
  mafDEL(m_Muscles);
  mafDEL(m_Tendons);
}

//----------------------------------------------------------------------------
mafOp* mmoLnSurf::Copy()
//----------------------------------------------------------------------------
{
  return new mmoLnSurf(m_Label);
}

//----------------------------------------------------------------------------
bool mmoLnSurf::Accept(mafNode* vme)
//----------------------------------------------------------------------------
{
  int cloudCounter = 0;
  if(vme == NULL)
    return false;

  for(int i=0; i < vme->GetNumberOfChildren(); i++)
  {
    if(vme->GetChild(i)->IsMAFType(mafVMELandmarkCloud)) 
    {
      if(cloudCounter >= 2 && mafVMELandmarkCloud::SafeDownCast(vme->GetChild(i))->GetNumberOfLandmarks() < 2)
        return false;
      cloudCounter++;
    }
  }

  return cloudCounter >= 4;
}

//----------------------------------------------------------------------------
void mmoLnSurf::OpRun()   
//----------------------------------------------------------------------------
{
  const wxString choices_string[] = {_("Lines and surface"), _("Surface"), _("Lines")}; 
  if(m_Gui == NULL)
  {
    m_Gui = new mafGUI(this);
    m_Gui->FloatSlider(ID_RHO_SPL, "rho spline param",&m_rhoLine, 0.0, 1000.0);
    m_Gui->FloatSlider(ID_RHO_SRF, "rho surface param",&m_rhoSurf, 0.0, 1000.0);
    m_Gui->FloatSlider(ID_SGM_SRF, "sigma surface param",&m_sgmSurf, 0.0, 1000.0);
    m_Gui->Slider(ID_DIM_SPL, "curve split number",&m_splDim, 4, 100);
    m_Gui->Slider(ID_DIMX_SRF, "surface x-split number",&m_xDim, 4, 100);
    m_Gui->Slider(ID_DIMY_SRF, "surface y-split number",&m_yDim, 4, 100);
    m_Gui->Combo(ID_GEN_LIST, _("reg. type"), &m_generateLinesSurfaces, 3, choices_string); 
    m_Gui->Bool(ID_PARSE_NAME, _("parse names"), &m_parseNames);
    m_Gui->SetListener(this);
    m_Gui->Label("");
    m_Gui->OkCancel();
  }
  ShowGui();
}

//----------------------------------------------------------------------------
void mmoLnSurf::OnEvent(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{ 
  switch(maf_event->GetId())
  {
    case wxOK:          
      OpStop(OP_RUN_OK);
    break;
    case wxCANCEL:
      OpStop(OP_RUN_CANCEL);
    break;
    case ID_RHO_SPL:
    case ID_RHO_SRF:
    case ID_SGM_SRF:
    case ID_DIM_SPL:
    case ID_DIMX_SRF:
    case ID_DIMY_SRF:
    case ID_GEN_LIST:
    case ID_PARSE_NAME:
    break;
    default:
      mafEventMacro(*maf_event); 
    break;
  }
}
//----------------------------------------------------------------------------
static void _addSegments(const std::vector<V3d<double> >& coords, unsigned from, unsigned to, vtkPoints *pnts, vtkCellArray  *cells)
//----------------------------------------------------------------------------
{
  vtkIdType pts[2];
  if(to <= from || to > coords.size())
    return;
  for(unsigned i = from; i < to; i++)
    pnts->InsertNextPoint(coords[i].components);
  for(unsigned i = from + 1; i < to; i++)
  {
    pts[0] = pnts->GetNumberOfPoints() - (to - i) - 1;
    pts[1] = pts[0] + 1;
    cells->InsertNextCell(2, pts);
  }
}
//----------------------------------------------------------------------------
void mmoLnSurf::OpDo()
//----------------------------------------------------------------------------
{
  wxInt32       cloudCounter = 0;
  vtkPoints     *newPtsSurf;
  vtkCellArray  *newCellsSurf;
  vtkPoints     *newPtsMsc;
  vtkCellArray  *newCellsMsc;
  vtkPoints     *newPtsTnd;
  vtkCellArray  *newCellsTnd;
  V3d<double>   x;

  std::vector<std::vector<V3d<double> >*> allValues;
  std::vector<V3d<double> >            coords;
  std::vector<V3d<double> >            smoothed;

  mafDEL(m_Surface);
  mafDEL(m_Muscles);
  mafDEL(m_Tendons);

  newPtsSurf = vtkPoints::New();
  newPtsSurf->Allocate(5000,10000);
  newCellsSurf = vtkCellArray::New();
  newCellsSurf->Allocate(10000,20000);
  newPtsMsc = vtkPoints::New();
  newPtsMsc->Allocate(5000,10000);
  newCellsMsc = vtkCellArray::New();
  newCellsMsc->Allocate(10000,20000);
  newPtsTnd = vtkPoints::New();
  newPtsTnd->Allocate(5000,10000);
  newCellsTnd = vtkCellArray::New();
  newCellsTnd->Allocate(10000,20000);

  //modified by Stefano. 18-9-2003
  wxBusyInfo wait("Please wait, working...");

  for(int i=0; i < m_Input->GetNumberOfChildren(); i++)
  {
    if(m_Input->GetChild(i)->IsMAFType(mafVMELandmarkCloud)) 
    {
      mafVMELandmarkCloud *cloud = mafVMELandmarkCloud::SafeDownCast(m_Input->GetChild(i));
      unsigned            from   = 0;
      bool                tendon = true;
      bool                OriIns = (cloudCounter < 2);
      if(m_parseNames)
      {
        OriIns = false;
        int namelen = strlen(cloud->GetName());
        if(namelen >= 4)
        {
          if(strncmp(cloud->GetName(), "Ori_", 4) == 0 || strncmp(cloud->GetName(), "Ins_", 4) == 0)
            OriIns = true;
        }
      }
      if(OriIns || cloud->GetNumberOfLandmarks() >= 4)
      {
        cloudCounter++;
        coords.clear();
        for(wxInt32 nI = 0; nI < cloud->GetNumberOfLandmarks(); nI++)
        {
          cloud->GetLandmark(nI, x.components);
          if(coords.size() == 0 || ((coords[coords.size() - 1] | x) > 4.0) || cloudCounter <= 2)
            coords.push_back(x);
          else
          {
            _addSegments(coords, from, coords.size(), tendon ? newPtsTnd : newPtsMsc, tendon ? newCellsTnd : newCellsMsc);
            from = coords.size() - 1;
            tendon = !tendon;
          }
        }
        if(!OriIns)
        {
          std::vector<V3d<double> > *arr = new std::vector<V3d<double> >(coords);
          allValues.push_back(arr);
        }
        _addSegments(coords, from, coords.size(), tendon ? newPtsTnd : newPtsMsc, tendon ? newCellsTnd : newCellsMsc);
      }
    }
  }
  produceRegularGrid(allValues, m_splDim, m_rhoLine, 2, true, smoothed);
  createSurface(smoothed, allValues.size(), m_splDim, 2, 2, m_xDim, m_yDim, m_rhoSurf, m_sgmSurf, newPtsSurf, newCellsSurf);
  for(unsigned i = 0; i < allValues.size(); i++)
    delete allValues[i];


  vtkPolyData *musc = vtkPolyData::New();
  vtkPolyData *tend = vtkPolyData::New();
  vtkPolyData *surf = vtkPolyData::New();

  musc->SetPoints(newPtsMsc);
  musc->Update();
  tend->SetPoints(newPtsTnd);
  tend->Update();
  surf->SetPoints(newPtsSurf);
  surf->Update();
  musc->SetLines(newCellsMsc);
  tend->SetLines(newCellsTnd);
  surf->SetPolys(newCellsSurf);
  surf->Squeeze();
  musc->Modified();
  musc->Update();
  tend->Modified();
  tend->Update();
  surf->Modified();
  surf->Update();

  newPtsSurf->Delete();
  newCellsSurf->Delete();
  newPtsMsc->Delete();
  newCellsMsc->Delete();
  newPtsTnd->Delete();
  newCellsTnd->Delete();

  if((3 - m_generateLinesSurfaces) & 1)
  {
    mafTimeStamp t;
    wxString     muscnm("MscFbr_");
    wxString     tendnm("TndFbr_");
    t = ((mafVME *)m_Input)->GetTimeStamp();
    mafNEW(m_Muscles);
    mafNEW(m_Tendons);
    muscnm += m_Input->GetName();
    tendnm += m_Input->GetName();
    m_Muscles->SetName(muscnm.c_str());
    m_Muscles->SetData(musc,t);
    m_Tendons->SetName(tendnm.c_str());
    m_Tendons->SetData(tend,t);

    mafTagItem tag_Nature;
    tag_Nature.SetName("VME_NATURE");
    tag_Nature.SetValue("NATURAL");

    m_Muscles->GetTagArray()->SetTag(tag_Nature);
    m_Tendons->GetTagArray()->SetTag(tag_Nature);

    m_Muscles->ReparentTo(m_Input);
    m_Tendons->ReparentTo(m_Input);
    //mafEventMacro(mafEvent(this,VME_ADD,m_Muscles));
    //mafEventMacro(mafEvent(this,VME_ADD,m_Tendons));
  }
  if((3 - m_generateLinesSurfaces) & 2)
  {
    mafTimeStamp t;
    wxString     sfnm("Surf_");
    t = ((mafVME *)m_Input)->GetTimeStamp();
    mafNEW(m_Surface);
    sfnm += m_Input->GetName();
    m_Surface->SetName(sfnm.c_str());
    m_Surface->SetData(surf, t);

    mafTagItem tag_Nature;
    tag_Nature.SetName("VME_NATURE");
    tag_Nature.SetValue("NATURAL");

    m_Surface->GetTagArray()->SetTag(tag_Nature);

    m_Surface->ReparentTo(m_Input);
    //mafEventMacro(mafEvent(this,VME_ADD,m_Surface));
  }
  musc->Delete();
  tend->Delete();
  surf->Delete();
  return;
}



//----------------------------------------------------------------------------
void mmoLnSurf::OpUndo()
//----------------------------------------------------------------------------
{
  mafEventMacro(mafEvent(this,VME_REMOVE,m_Surface));
  mafEventMacro(mafEvent(this,VME_REMOVE,m_Muscles));
  mafEventMacro(mafEvent(this,VME_REMOVE,m_Tendons));
}
