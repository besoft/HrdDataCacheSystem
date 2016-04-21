/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpMultiscaleExplore.cpp,v $
Language:  C++
Date:      $Date: 2011-05-27 07:52:28 $
Version:   $Revision: 1.1.1.1.2.2 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "wx/busyinfo.h"

#include "mafDecl.h"
#include "mafGUI.h"
#include "mafGUIDialog.h"
#include "mafRWIBase.h"
#include "mafRWI.h"
#include "mafDeviceButtonsPadMouse.h"
#include "mafGUIButton.h"
#include "mafGUIFloatSlider.h"
#include "mafGUIValidator.h"
#include "mafEventBase.h"
#include "mafEvent.h"
#include "mafEventInteraction.h"
#include "mafVME.h"
#include "mafVMESurface.h"
#include "mafVMEVolumeGray.h"
#include "mafVMEPolyline.h"
#include "mafVMEOutput.h"

#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSphereSource.h"
#include "vtkProperty.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkMatrix4x4.h"

#include "lhpMultiscaleUtility.h"
#include "lhpMultiscaleActorCoordsUtility.h"
#include "lhpMultiscaleActor.h"
#include "lhpMultiscaleCallbacks.h"
#include "lhpMultiscaleCameraUtility.h"
#include "lhpMultiscaleVectorMath.h"
#include "lhpMultiscaleVisualPipes.h"
#include "lhpOpMultiscaleExplore.h"

#include <fstream>
#include <iostream>
#include <algorithm>


//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpMultiscaleExplore);
//----------------------------------------------------------------------------


namespace lhpMultiscale
{
  //----------------------------------------------------------------------------
  // widget ID's
  //----------------------------------------------------------------------------
  enum MULTISCALE_IDS
  {
    ID_ADDVME = MINID,
    ID_SCALEVALUETXT,
    ID_SCALEUNITSTXT,
    ID_ZOOMOUT,
    ID_CAMERARESET,
    ID_GOBACK,
    ID_OK,
    ID_CANCEL,
    ID_DEBUG,
    ID_START_RENDER,  // vtk start render event
    ID_MOUSE_CLICK,   // vtk mouse click event
    ID_POS_SLIDER,
    ID_POS_VALUE_TXT,
    ID_CHANGE_VIEW,
    ID_OPACITY_VALUE_TXT,
    ID_OPACITY_SLIDER,
    ID_VIEW_COMBO,
    ID_BASE_UNITS
  };
}


//----------------------------------------------------------------------------
// Use multiscale namespace
//----------------------------------------------------------------------------
using namespace lhpMultiscale ;


//----------------------------------------------------------------------------
// Constructor
lhpOpMultiscaleExplore::lhpOpMultiscaleExplore(wxString label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_OP;
  m_Canundo = true ;
  m_Dialog = NULL;
  m_Rwi = NULL;

  m_externalRenderer = NULL ;

  m_MultiscaleUtility = new lhpMultiscaleUtility ;
  m_nextTokenColor = 0 ;

  m_dclickCallback = NULL ;

  m_PosSlider = NULL ;
}


//----------------------------------------------------------------------------
// Destructor
lhpOpMultiscaleExplore::~lhpOpMultiscaleExplore()
//----------------------------------------------------------------------------
{
  int i ;

  delete m_MultiscaleUtility ;

  // delete pipes (each pipe must delete its own vtk objects, including the actor and mapper)
  for (i = 0 ;  i < (int)m_surfacePipes.size() ;  i++)
    delete m_surfacePipes.at(i) ;
  for (i = 0 ;  i < (int)m_tokenPipes.size() ;  i++)
    delete m_tokenPipes.at(i) ; 
  for (i = 0 ;  i < (int)m_volumeSlicePipes.size() ;  i++)
    delete m_volumeSlicePipes.at(i) ; 

  if (m_dclickCallback != NULL)
    delete m_dclickCallback ;
}


//----------------------------------------------------------------------------
// Copy
mafOp* lhpOpMultiscaleExplore::Copy()
//----------------------------------------------------------------------------
{
  /** return a copy of itself, needs to put it into the undo stack */
  return new lhpOpMultiscaleExplore(m_Label);
}


//----------------------------------------------------------------------------
// Accept surface or volume data
bool lhpOpMultiscaleExplore::Accept(mafNode* vme)
//----------------------------------------------------------------------------
{
  return (vme != NULL) ;
  //return (vme != NULL && (vme->IsMAFType(mafVMESurface) || vme->IsMAFType(mafVMEPolyline) || vme->IsMAFType(mafVMEVolumeGray))) ;
}

//----------------------------------------------------------------------------
// Static copy of accept function
bool lhpOpMultiscaleExplore::AcceptStatic(mafNode* vme)
//----------------------------------------------------------------------------
{
  return (vme != NULL) ;
  //return (vme != NULL && (vme->IsMAFType(mafVMESurface) || vme->IsMAFType(mafVMEPolyline) || vme->IsMAFType(mafVMEVolumeGray))) ;
}


//----------------------------------------------------------------------------
// Create gui etc before event loop takes over
void lhpOpMultiscaleExplore::OpRun()
//----------------------------------------------------------------------------
{
  int result = OP_RUN_CANCEL;

  if (m_TestMode == false)
  {
    CreateOpDialog();

    int ret_dlg = m_Dialog->ShowModal();
    if( ret_dlg == wxID_OK )
    {
      result = OP_RUN_OK;
    }
    else 
    {
      result = OP_RUN_CANCEL;
    }

    DeleteOpDialog();

    mafEventMacro(mafEvent(this,result));
  }
}




//----------------------------------------------------------------------------
// Finish executing the op after the gui has finished
void lhpOpMultiscaleExplore::OpDo()
//----------------------------------------------------------------------------
{
  mafEventMacro(mafEvent(this, CAMERA_UPDATE));
}


//----------------------------------------------------------------------------
// Undo the op
void lhpOpMultiscaleExplore::OpUndo()
//----------------------------------------------------------------------------
{
}



//----------------------------------------------------------------------------
// Set up op without dialog
void lhpOpMultiscaleExplore::CreateOpWithoutDialog(vtkRenderer *renderer)
//----------------------------------------------------------------------------
{
  // Set pointer to external renderer (need this when setting up with no dialog)
  m_externalRenderer = renderer ;


  //----------------------------------------------------------------------------
  // get input vme
  //----------------------------------------------------------------------------
  mafVME* vme = mafVME::SafeDownCast(m_Input) ;


  //----------------------------------------------------------------------------
  // setup interface
  //----------------------------------------------------------------------------
  GetRenderWindow()->SetDesiredUpdateRate(0.0001f);
  GetRenderWindow()->SetSize(400,400);


  //----------------------------------------------------------------------------
  // Set up the gui widgets
  //----------------------------------------------------------------------------
  m_BaseUnits = ID_METRES ;

  m_Opacity = 1.0 ;
  double bounds[6] ;
  vme->GetOutput()->GetBounds(bounds) ;
  InitSliceParams(ID_XY, bounds) ;      // set the validator values (nb there are no visual pipes at this stage)


  //----------------------------------------------------------------------------
  // Create visual pipe for selected vme.
  // Additional vme's must be added later from the dialog, because this is an op - not a view.
  //----------------------------------------------------------------------------
  AddVmeToScene(vme) ;
  UpdateCamera() ;


  //----------------------------------------------------------------------------
  // Set up interactor style and callbacks
  //----------------------------------------------------------------------------

  // Set the interactor style to trackball camera
  vtkInteractorStyleTrackballCamera* style = vtkInteractorStyleTrackballCamera::New() ;
  GetInteractor()->SetInteractorStyle(style) ;
  style->Delete() ;

  // add observer to catch vtk start render event
  vtkStartRenderCallback *startRenderCallback = vtkStartRenderCallback::New() ;         // instantiate callback to convert vtk event to maf event
  GetRenderer()->AddObserver(vtkCommand::StartEvent, startRenderCallback) ;             // set callback to get start event from renderer
  startRenderCallback->SetListener(this) ;                                              // set self as listener to callback
  startRenderCallback->SetMafEventId(ID_START_RENDER) ;                                 // set event id to be thrown by callback
  startRenderCallback->Delete() ;

  // add observer to catch vtk mouse click event
  vtkMouseClickCallback *mouseClickCallback = vtkMouseClickCallback::New() ;           // instantiate callback to convert vtk event to maf event
  GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent, mouseClickCallback) ;  // set callback to get mouse event from interactor
  mouseClickCallback->SetListener(this) ;                                              // set self as listener to callback
  mouseClickCallback->SetMafEventId(ID_MOUSE_CLICK) ;                                 // set event id to be thrown by callback
  mouseClickCallback->Delete() ;

  // save the current view so we can go back to it
  GetMultiscaleUtility()->SaveInitialView(GetRenderer()) ;
}



//----------------------------------------------------------------------------
// Create the op dialog
void lhpOpMultiscaleExplore::CreateOpDialog()
//----------------------------------------------------------------------------
{
  wxBusyCursor wait;


  //----------------------------------------------------------------------------
  // Get start vme or vme tree from input
  //----------------------------------------------------------------------------
  enum{
    INIT_VME_SINGLE = 0,
    INIT_VME_TREE
  };
  const int InitVme = INIT_VME_TREE ;

  double bounds[6] = {-1,1,-1,1,-1,1} ;
  std::vector<mafVME*> vmeList ;

  switch(InitVme){
  case INIT_VME_SINGLE:
    if (m_Input != NULL && (m_Input->IsMAFType(mafVMESurface) || m_Input->IsMAFType(mafVMEPolyline) || m_Input->IsMAFType(mafVMEVolumeGray))){
      mafVME* vme = mafVME::SafeDownCast(m_Input) ;
      vmeList.push_back(vme) ;
      vme->GetOutput()->GetBounds(bounds) ;
    }
    break ;
  case INIT_VME_TREE:
    ListValidVmesInTree(m_Input, vmeList, true) ;
    if ((int)vmeList.size() > 0)
      GetBoundsOfVmeList(vmeList, bounds) ;
    break ;
  }





  //----------------------------------------------------------------------------
  // setup interface
  //----------------------------------------------------------------------------
  m_Dialog = new mafGUIDialog("Multiscale Explorer", mafCLOSEWINDOW | mafRESIZABLE);

  m_Rwi = new mafRWI(m_Dialog,ONE_LAYER,false);
  m_Rwi->SetListener(this);
  m_Rwi->CameraSet(CAMERA_PERSPECTIVE);

  m_Rwi->m_RenderWindow->SetDesiredUpdateRate(0.0001f);
  m_Rwi->SetSize(0,0,400,400);
  m_Rwi->Show(true);
  m_Rwi->m_RwiBase->SetMouse(m_Mouse) ;





  //----------------------------------------------------------------------------
  // Set up the gui widgets
  //----------------------------------------------------------------------------
  wxPoint p = wxDefaultPosition;

  // title
  wxStaticText *setupCtrlsStaticTxt  = new wxStaticText(m_Dialog, -1, "Set up view");

  // add vme
  mafGUIButton  *AddVMEButton = new mafGUIButton(m_Dialog, ID_ADDVME, "add vme", p, wxSize(80,20));

  // set base units
  wxString SIUnits[5] = {"m", "cm", "mm", "microns", "nm"} ;
  m_BaseUnits = ID_METRES ;
  wxStaticText *unitsStaticTxt = new wxStaticText(m_Dialog, -1, "base units ") ;
  wxComboBox *unitsCombo = new wxComboBox(m_Dialog, ID_BASE_UNITS, SIUnits[m_BaseUnits], p, wxSize(80,20), 5, SIUnits, wxCB_READONLY) ;

  // multiscale controls
  wxStaticText *mscaleCtrlsStaticTxt = new wxStaticText(m_Dialog, -1, "Multiscale controls");
  mafGUIButton  *ZoomOutButton = new mafGUIButton(m_Dialog, ID_ZOOMOUT, "zoom out x2", p, wxSize(80,20));
  mafGUIButton  *CameraResetButton = new mafGUIButton(m_Dialog, ID_CAMERARESET, "reset camera", p, wxSize(80,20));
  mafGUIButton  *GoBackButton = new mafGUIButton(m_Dialog, ID_GOBACK, "go back", p, wxSize(80,20));
  mafGUIButton  *debug = new mafGUIButton(m_Dialog, ID_DEBUG, "debug", p, wxSize(80,20));

  // display scale
  wxStaticText *scaleStaticTxt = new wxStaticText(m_Dialog, -1, "Current scale: ") ;
  wxTextCtrl *scaleValueTxt = new wxTextCtrl(m_Dialog, ID_SCALEVALUETXT, wxEmptyString, p, wxSize(80,20), wxTE_READONLY | wxTE_RIGHT) ;
  wxTextCtrl *scaleUnitsTxt = new wxTextCtrl(m_Dialog, ID_SCALEUNITSTXT, wxEmptyString, p, wxSize(80,20), wxTE_READONLY | wxTE_RIGHT) ;

  // control of slice position and view direction
  wxString Views[3] = {"XY","XZ","YZ"};
  m_Opacity = 1.0 ;
 
  InitSliceParams(ID_XY, bounds) ;      // set the validator values (nb there are no visual pipes at this stage)
  wxStaticText *sliceCtrlsStaticTxt = new wxStaticText(m_Dialog, -1, "Slice controls");
  wxStaticText *posStaticTxt = new wxStaticText(m_Dialog, -1, "pos ") ;
  wxTextCtrl *posValueTxt = new wxTextCtrl(m_Dialog, ID_POS_VALUE_TXT, wxEmptyString, p, wxSize(80,20), wxTE_RIGHT) ;
  m_PosSlider = new mafGUIFloatSlider(m_Dialog, ID_POS_SLIDER, m_SliderOrigin, bounds[4], bounds[5], p) ;
  wxStaticText *viewStaticTxt = new wxStaticText(m_Dialog, -1, "view ") ;
  wxComboBox *viewCombo = new wxComboBox(m_Dialog, ID_CHANGE_VIEW, Views[m_ViewIndex], p, wxSize(80,20), 3, Views, wxCB_READONLY) ;

  // control of opacity
  //wxStaticText *opacityStaticTxt = new wxStaticText(m_Dialog, -1, "opacity ") ;
  //wxTextCtrl *opacityValueTxt = new wxTextCtrl(m_Dialog, ID_OPACITY_VALUE_TXT, wxEmptyString, p, wxSize(80,20), wxTE_RIGHT) ;
  //mafGUIFloatSlider *opacitySlider = new mafGUIFloatSlider(m_Dialog, ID_OPACITY_SLIDER, 0.5, 0.0, 1.0, p) ;

  // ok and cancel
  mafGUIButton  *ok = new mafGUIButton(m_Dialog, ID_OK, "ok", p, wxSize(80,20));
  mafGUIButton  *cancel = new mafGUIButton(m_Dialog, ID_CANCEL, "cancel", p, wxSize(80,20));


  // set validators
  AddVMEButton->SetValidator(mafGUIValidator(this,ID_ADDVME,AddVMEButton));
  ZoomOutButton->SetValidator(mafGUIValidator(this,ID_ZOOMOUT,ZoomOutButton));
  CameraResetButton->SetValidator(mafGUIValidator(this,ID_CAMERARESET,CameraResetButton));
  GoBackButton->SetValidator(mafGUIValidator(this,ID_GOBACK,GoBackButton));
  debug->SetValidator(mafGUIValidator(this,ID_DEBUG,debug));

  unitsCombo->SetValidator(mafGUIValidator(this, ID_BASE_UNITS, unitsCombo, &m_BaseUnits)) ;

  scaleValueTxt->SetValidator(mafGUIValidator(this,ID_SCALEVALUETXT,ok)) ;
  scaleUnitsTxt->SetValidator(mafGUIValidator(this,ID_SCALEUNITSTXT,ok)) ;

  posValueTxt->SetValidator(mafGUIValidator(this,ID_POS_VALUE_TXT,posValueTxt,&m_SliderOrigin)) ;
  m_PosSlider->SetValidator(mafGUIValidator(this, ID_POS_SLIDER, m_PosSlider, &m_SliderOrigin, posValueTxt));
  viewCombo->SetValidator(mafGUIValidator(this, ID_CHANGE_VIEW, viewCombo, &m_ViewIndex)) ;

  //opacityValueTxt->SetValidator((mafGUIValidator(this, ID_OPACITY_VALUE_TXT, opacityValueTxt, &m_Opacity, 0.0, 1.0))) ;
  //opacitySlider->SetValidator(mafGUIValidator(this, ID_OPACITY_SLIDER, opacitySlider, &m_Opacity, opacityValueTxt));

  ok->SetValidator(mafGUIValidator(this,ID_OK,ok));
  cancel->SetValidator(mafGUIValidator(this,ID_CANCEL,cancel));


  // layout
  wxBoxSizer *h_sizer0 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer0->Add(setupCtrlsStaticTxt, 0, wxLEFT);	

  wxBoxSizer *h_sizer2 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer2->Add(AddVMEButton, 0, wxLEFT);	
  h_sizer2->AddSpacer(20) ;
  h_sizer2->Add(unitsStaticTxt, 0, wxLEFT) ;
  h_sizer2->Add(unitsCombo, 0, wxLEFT) ;

  wxBoxSizer *h_sizer4 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer4->Add(mscaleCtrlsStaticTxt, 0, wxLEFT);	

  wxBoxSizer *h_sizer6 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer6->Add(ZoomOutButton, 0, wxLEFT);	
  h_sizer6->Add(CameraResetButton, 0, wxLEFT);	
  h_sizer6->Add(GoBackButton, 0, wxLEFT);	
  h_sizer6->Add(debug, 0, wxLEFT);	

  wxBoxSizer *h_sizer8 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer8->Add(scaleStaticTxt, 0, wxLEFT) ;
  h_sizer8->Add(scaleValueTxt, 0, wxLEFT) ;
  h_sizer8->Add(scaleUnitsTxt, 0, wxLEFT) ;

  wxBoxSizer *h_sizer10 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer10->Add(sliceCtrlsStaticTxt, 0, wxLEFT);	

  wxBoxSizer *h_sizer12 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer12->Add(posStaticTxt, 0, wxLEFT);
  h_sizer12->Add(posValueTxt, 0, wxLEFT) ;
  h_sizer12->Add(m_PosSlider, 0, wxLEFT);

  wxBoxSizer *h_sizer14 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer14->Add(viewStaticTxt, 0, wxLEFT);
  h_sizer14->Add(viewCombo, 0, wxLEFT) ;

  //wxBoxSizer *h_sizer18 = new wxBoxSizer(wxHORIZONTAL);
  //h_sizer18->Add(opacityStaticTxt, 0, wxLEFT);
  //h_sizer18->Add(opacityValueTxt, 0, wxLEFT) ;
  //h_sizer18->Add(opacitySlider, 0, wxLEFT);

  wxBoxSizer *h_sizer20 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer20->Add(ok, 0, wxLEFT);
  h_sizer20->Add(cancel, 0, wxLEFT);

  wxBoxSizer *v_sizer_renwin = new wxBoxSizer(wxVERTICAL) ;
  v_sizer_renwin->Add(m_Rwi->m_RwiBase, 1, wxEXPAND);

  wxBoxSizer *v_sizer_ctrls =  new wxBoxSizer(wxVERTICAL);
  v_sizer_ctrls->Add(h_sizer0, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer2, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->AddSpacer(20) ;
  v_sizer_ctrls->Add(h_sizer4, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer6, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer8, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->AddSpacer(20) ;
  v_sizer_ctrls->Add(h_sizer10, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer12, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer14, 0, wxEXPAND | wxALL,5);
  //v_sizer_ctrls->Add(h_sizer18, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->AddSpacer(40) ;
  v_sizer_ctrls->Add(h_sizer20, 0, wxEXPAND | wxALL,5);

  wxBoxSizer *h_sizer_all = new wxBoxSizer(wxHORIZONTAL) ;
  h_sizer_all->Add(v_sizer_renwin, 1, wxEXPAND) ;
  h_sizer_all->Add(v_sizer_ctrls, 0, wxEXPAND) ;

  m_Dialog->Add(h_sizer_all, 1, wxEXPAND);


  // set position of dialog
  h_sizer_all->Fit(m_Dialog) ;
  m_Dialog->SetPosition(wxPoint(20,20)) ;



  //----------------------------------------------------------------------------
  // Create visual pipe for selected vme.
  // Additional vme's can be added later from the dialog
  //----------------------------------------------------------------------------
  AddVmeListToScene(vmeList) ;
  UpdateCamera() ;



  //----------------------------------------------------------------------------
  // Set up interactor style and callbacks
  //----------------------------------------------------------------------------

  // Set the interactor style to trackball camera
  vtkInteractorStyleTrackballCamera* style = vtkInteractorStyleTrackballCamera::New() ;
  m_Rwi->m_RwiBase->SetInteractorStyle(style) ;
  style->Delete() ;

  // add observer to catch vtk start render event
  vtkStartRenderCallback *startRenderCallback = vtkStartRenderCallback::New() ;         // instantiate callback to convert vtk event to maf event
  GetRenderer()->AddObserver(vtkCommand::StartEvent, startRenderCallback) ;             // set callback to get start event from renderer
  startRenderCallback->SetListener(this) ;                                              // set self as listener to callback
  startRenderCallback->SetMafEventId(ID_START_RENDER) ;                                 // set event id to be thrown by callback
  startRenderCallback->Delete() ;

  // add observer to catch vtk mouse click event
  vtkMouseClickCallback *mouseClickCallback = vtkMouseClickCallback::New() ;           // instantiate callback to convert vtk event to maf event
  GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent, mouseClickCallback) ;  // set callback to get mouse event from interactor
  mouseClickCallback->SetListener(this) ;                                              // set self as listener to callback
  mouseClickCallback->SetMafEventId(ID_MOUSE_CLICK) ;                                 // set event id to be thrown by callback
  mouseClickCallback->Delete() ;

  // add observer to catch double click event
  //m_dclickCallback = new lhpMultiscaleDoubleClickCallback ;
 // m_dclickCallback->SetListener(this) ;                                              // set self as listener to callback
  //m_dclickCallback->SetMafEventId(ID_MOUSE_CLICK) ;                                 // set event id to be thrown by callback
  //m_Mouse->AddObserver(this, MCH_INPUT) ;

  // save the current view so we can go back to it
  GetMultiscaleUtility()->SaveInitialView(GetRenderer()) ;
}



//----------------------------------------------------------------------------
void lhpOpMultiscaleExplore::DeleteOpDialog()
//----------------------------------------------------------------------------
{

    cppDEL(m_Rwi); 
    cppDEL(m_Dialog);
}



//----------------------------------------------------------------------------
// Create the vtk pipeline (surface pipe)
void lhpOpMultiscaleExplore::CreateSurfacePipeline(mafVME* vme, vtkRenderer *renderer)
//----------------------------------------------------------------------------
{
  // create new pipe and add to list
  lhpMultiscaleSurfacePipeline* pipe = new lhpMultiscaleSurfacePipeline(vme, GetRenderer()) ;

  // add to list of pipes and create multiscale actor
  m_surfacePipes.push_back(pipe) ;
  GetMultiscaleUtility()->AddMultiscaleActor(pipe, MSCALE_DATA_ACTOR) ;
}



//----------------------------------------------------------------------------
// Create the vtk pipeline (token pipe)
void lhpOpMultiscaleExplore::CreateTokenPipeline(vtkRenderer *renderer)
//----------------------------------------------------------------------------
{
  // create new pipe and add to list
  lhpMultiscaleTokenPipeline* pipe = new lhpMultiscaleTokenPipeline(GetRenderer(), m_nextTokenColor++) ;

  // add to list of pipes and create multiscale actor
  m_tokenPipes.push_back(pipe) ;
  GetMultiscaleUtility()->AddMultiscaleActor(pipe, MSCALE_TOKEN) ;
}


//----------------------------------------------------------------------------
// Create the vtk pipeline (volume slice pipe)
void lhpOpMultiscaleExplore::CreateVolumeSlicePipeline(mafVME* vme, vtkRenderer *renderer)
//----------------------------------------------------------------------------
{
  // create new pipe and add to list
  lhpMultiscaleVolumeSlicePipeline* pipe = new lhpMultiscaleVolumeSlicePipeline(vme, m_ViewIndex, m_SliceOrigin, GetRenderer()) ;

  // add to list of pipes and create multiscale actor
  m_volumeSlicePipes.push_back(pipe) ;
  GetMultiscaleUtility()->AddMultiscaleActor(pipe, MSCALE_DATA_ACTOR) ;
}



//----------------------------------------------------------------------------
// Update the camera
// This wraps the dialog-based method in case there is no dialog.
void lhpOpMultiscaleExplore::UpdateCamera()
//----------------------------------------------------------------------------
{
  if (m_Rwi != NULL)
    m_Rwi->CameraUpdate() ;
}


//----------------------------------------------------------------------------
// Initialize the parameters of the slice (origin and view index)
// This only sets the parameters - it does not set or change the visual pipes !
void lhpOpMultiscaleExplore::InitSliceParams(int viewIndex, double *bounds)
//----------------------------------------------------------------------------
{
  // set the view directon validator
  m_ViewIndex = viewIndex ;

  // set the slice position params to the centre of the bounds
  m_SliceOrigin[0] = (bounds[0] + bounds[1]) / 2.0 ;
  m_SliceOrigin[1] = (bounds[2] + bounds[3]) / 2.0 ;
  m_SliceOrigin[2] = (bounds[4] + bounds[5]) / 2.0 ;

  // Set the slider origin validator
  switch(viewIndex){
    case ID_XY:
      m_SliderOrigin = m_SliceOrigin[2] ;
      break ;
    case ID_XZ:
      m_SliderOrigin = m_SliceOrigin[1] ;
      break ;
    case ID_YZ:
      m_SliderOrigin = m_SliceOrigin[0] ;
      break ;
  }

  // save the current values
  m_ViewIndex_old = m_ViewIndex ;
  m_SliderOrigin_old = m_SliderOrigin ;
}



//----------------------------------------------------------------------------
// Move the global slice.
void lhpOpMultiscaleExplore::UpdateSlicePosition()
//----------------------------------------------------------------------------
{
  if (m_SliderOrigin != m_SliderOrigin_old){
    switch(m_ViewIndex){
    case ID_XY:
      m_SliceOrigin[2] = m_SliderOrigin ;
      break ;
    case ID_XZ:
      m_SliceOrigin[1] = m_SliderOrigin ;
      break ;
    case ID_YZ:
      m_SliceOrigin[0] = m_SliderOrigin ;
      break ;
    }

    // Update all the volume slice pipelines
    for (int i = 0 ;  i < (int)m_volumeSlicePipes.size() ;  i++){
      lhpMultiscaleVolumeSlicePipeline *volSlicePipe = m_volumeSlicePipes.at(i) ;
      volSlicePipe->SetSlicePosition(m_SliceOrigin) ;
    }

    // update the saved values
    m_SliderOrigin_old = m_SliderOrigin ;
  }
}


//----------------------------------------------------------------------------
// Update the view axis
void lhpOpMultiscaleExplore::UpdateViewAxis(double *bounds)
//----------------------------------------------------------------------------
{
  double range_min, range_mid, range_max ;

  if (m_ViewIndex_old != m_ViewIndex){
    switch(m_ViewIndex){
    case ID_XY:
      // get the new range for the slider
      range_min = bounds[4] ;
      range_max = bounds[5] ;
      range_mid = (range_min + range_max) / 2.0 ;

      // change the slice position
      m_SliceOrigin[0] = (bounds[0] + bounds[1]) / 2.0 ;
      m_SliceOrigin[1] = (bounds[2] + bounds[3]) / 2.0 ;
      m_SliceOrigin[2] = range_mid ;
      
      break ;

    case ID_XZ:
      // get the new range for the slider
      range_min = bounds[2] ;
      range_max = bounds[3] ;
      range_mid = (range_min + range_max) / 2.0 ;

      // change the slice position
      m_SliceOrigin[0] = (bounds[0] + bounds[1]) / 2.0 ;
      m_SliceOrigin[1] = range_mid ;
      m_SliceOrigin[2] = (bounds[4] + bounds[5]) / 2.0 ;

      break ;

    case ID_YZ:
      // get the new range for the slider
      range_min = bounds[0] ;
      range_max = bounds[1] ;
      range_mid = (range_min + range_max) / 2.0 ;

      // change the slice position
      m_SliceOrigin[0] = range_mid ;
      m_SliceOrigin[1] = (bounds[2] + bounds[3]) / 2.0 ;
      m_SliceOrigin[2] = (bounds[4] + bounds[5]) / 2.0 ;

      break ;
    }

    // update the position slider (if widget is present)
    m_SliderOrigin = range_mid ;
    if (m_PosSlider != NULL){
      m_PosSlider->SetRange(range_min, range_max) ;
      m_Dialog->TransferDataToWindow() ;      // need this to update the associated text control
    }

    // Update all the volume slice pipelines
    for (int i = 0 ;  i < (int)m_volumeSlicePipes.size() ;  i++){
      lhpMultiscaleVolumeSlicePipeline *volSlicePipe = m_volumeSlicePipes.at(i) ;
      volSlicePipe->SetSliceDirection(m_ViewIndex) ;
      volSlicePipe->SetSlicePosition(m_SliceOrigin) ;
    }

    // update the saved values
    m_ViewIndex_old = m_ViewIndex ;
    m_SliderOrigin_old = m_SliderOrigin ;
  }
}


//----------------------------------------------------------------------------
// Set the slider range to fit the given bounds
void lhpOpMultiscaleExplore::SetSliderRange(double *bounds)
//----------------------------------------------------------------------------
{
  // do nothing if slider widget is not present
  if (m_PosSlider == NULL)
    return ;

  switch(m_ViewIndex){
    case ID_XY:
      m_PosSlider->SetRange(bounds[4], bounds[5]) ;
      break ;
    case ID_XZ:
      m_PosSlider->SetRange(bounds[2], bounds[3]) ;
      break ;
    case ID_YZ:
      m_PosSlider->SetRange(bounds[0], bounds[1]) ;
      break ;
  }

  // check that the slider value is in the range, and move it to the centre if not.
  double rmin = m_PosSlider->GetMin() ;
  double rmax = m_PosSlider->GetMax() ;
  if (m_SliderOrigin < rmin || m_SliderOrigin > rmax)
    m_SliderOrigin = (rmin + rmax) / 2.0 ;

  // update the dialog and the slice
  m_Dialog->TransferDataToWindow() ; 
  UpdateSlicePosition() ;
}


//----------------------------------------------------------------------------
// Add new vme to scene.
// Adds vme to list, creates multiscale actors for data and tokens and creates visual pipes.
//----------------------------------------------------------------------------
void lhpOpMultiscaleExplore::AddVmeToScene(mafVME* vme)
{
  // Create pipeline and multiscale actor
  if (vme->IsMAFType(mafVMESurface))
    CreateSurfacePipeline(vme, GetRenderer()) ;
  else if (vme->IsMAFType(mafVMEPolyline))
    CreateSurfacePipeline(vme, GetRenderer()) ;
  else if (vme->IsMAFType(mafVMEVolumeGray))
    CreateVolumeSlicePipeline(vme, GetRenderer()) ;
  else{
    // not a supported vme type - do nothing
    return ;
  }

  int dataActorId = GetMultiscaleUtility()->GetNumberOfActors() - 1 ;

  // create pipeline and multiscale actor for corresponding token
  CreateTokenPipeline(GetRenderer()) ;
  int tokenActorId = GetMultiscaleUtility()->GetNumberOfActors() - 1 ;

  // Put token in same position as actor
  vtkActor* actor_data = GetMultiscaleUtility()->GetMultiscaleActor(dataActorId)->GetActor() ;
  vtkActor* actor_token = GetMultiscaleUtility()->GetMultiscaleActor(tokenActorId)->GetActor() ;
  GetMultiscaleUtility()->GetActorCoordsUtility()->MoveToCenter(actor_token, actor_data) ;

  // Associate data actor and token together as a pair
  GetMultiscaleUtility()->SetActorTokenPair(dataActorId, tokenActorId) ;

  // Reset camera to view all actors currently in scene
  GetMultiscaleUtility()->ResetCameraFitAll(GetRenderer()) ;

  // Set the attention to all the actors
  GetMultiscaleUtility()->SetAttentionAll(true) ;

  // attention has changed, so update the slider range
  double bounds[6] ;
  GetMultiscaleUtility()->GetAttentionBounds(bounds) ;
  SetSliderRange(bounds) ;

  // Must save the camera view again, else the saved view will not contain
  // the required info for the new actors.
  GetMultiscaleUtility()->SaveInitialView(GetRenderer()) ;

  // Render the new scene
  GetRenderer()->GetRenderWindow()->Render() ;
  UpdateCamera() ;
}



//----------------------------------------------------------------------------
// Get bounds of vme list
//----------------------------------------------------------------------------
void lhpOpMultiscaleExplore::GetBoundsOfVmeList(std::vector<mafVME*>& vmeList, double bounds[6])
{
  double b[6] ;
  for (int i = 0 ;  i < (int)vmeList.size() ;  i++){
    vmeList[i]->GetOutput()->GetBounds(b) ;
    if (i == 0){
      for (int j = 0 ;  j < 6 ;  j++)
        bounds[j] = b[j] ;
    }
    else{
      bounds[0] = std::min(bounds[0], b[0]) ;
      bounds[1] = std::max(bounds[1], b[1]) ;
      bounds[2] = std::min(bounds[2], b[2]) ;
      bounds[3] = std::max(bounds[3], b[3]) ;
      bounds[4] = std::min(bounds[4], b[4]) ;
      bounds[5] = std::max(bounds[5], b[5]) ;
    }
  }
}


//----------------------------------------------------------------------------
// Add list of vme's to scene
//----------------------------------------------------------------------------
void lhpOpMultiscaleExplore::AddVmeListToScene(std::vector<mafVME*>& vmeList)
{
  for (int i = 0 ;  i < (int)vmeList.size() ;  i++)
      AddVmeToScene(vmeList[i]) ;
}


//----------------------------------------------------------------------------
// List all valid vme's in tree
//----------------------------------------------------------------------------
void lhpOpMultiscaleExplore::ListValidVmesInTree(mafNode* parentNode, std::vector<mafVME*>& vmeList, bool includeParent)
{
  std::vector<mafNode*> nodeList ;
  ListNodesInTree(parentNode, nodeList, includeParent) ;

  for (int i = 0 ;  i < (int)nodeList.size() ;  i++){
    if (nodeList[i] != NULL && 
      (nodeList[i]->IsMAFType(mafVMESurface) || 
      nodeList[i]->IsMAFType(mafVMEPolyline) || 
      nodeList[i]->IsMAFType(mafVMEVolumeGray))){
      mafVME* vme = mafVME::SafeDownCast(nodeList[i]);
      vmeList.push_back(vme) ;
    }
  }
}


//----------------------------------------------------------------------------
// List all nodes in tree
//----------------------------------------------------------------------------
void lhpOpMultiscaleExplore::ListNodesInTree(mafNode* parentNode, std::vector<mafNode*>& nodeList, bool includeParent)
{
  nodeList.clear() ;
  if (includeParent)
    nodeList.push_back(parentNode) ;

  ListSubTreeOfNode(parentNode, nodeList) ;
}


//----------------------------------------------------------------------------
// Recursively list subtree of parent node
//----------------------------------------------------------------------------
void lhpOpMultiscaleExplore::ListSubTreeOfNode(mafNode* parentNode, std::vector<mafNode*>& nodeList)
{
  for (int i = 0 ;  i < parentNode->GetNumberOfChildren() ;  i++){
    mafNode* child = parentNode->GetChild(i) ;
    nodeList.push_back(child) ;
    ListSubTreeOfNode(child, nodeList) ;
  }
}


//----------------------------------------------------------------------------
// Get renderer
vtkRenderer* lhpOpMultiscaleExplore::GetRenderer()
//----------------------------------------------------------------------------
{
  if (m_Rwi != NULL)
    return m_Rwi->m_RenFront ;
  else if (m_externalRenderer != NULL)
    return m_externalRenderer ;
  else
    return NULL ;
}

//----------------------------------------------------------------------------
// Get render window
vtkRenderWindow* lhpOpMultiscaleExplore::GetRenderWindow()
//----------------------------------------------------------------------------
{
  if (m_Rwi != NULL)
    return m_Rwi->m_RenderWindow ;
  else if (m_externalRenderer != NULL)
    return m_externalRenderer->GetRenderWindow() ;
  else
    return NULL ;
}

//----------------------------------------------------------------------------
// Get interactor
vtkRenderWindowInteractor* lhpOpMultiscaleExplore::GetInteractor()
//----------------------------------------------------------------------------
{
  if (m_Rwi != NULL)
    return vtkRenderWindowInteractor::SafeDownCast(m_Rwi->m_RwiBase) ;
  else if (m_externalRenderer != NULL)
    return m_externalRenderer->GetRenderWindow()->GetInteractor() ;
  else
    return NULL ;
}

//----------------------------------------------------------------------------
// Event Handler
void lhpOpMultiscaleExplore::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {	
    case ID_ADDVME:
      {
        // Create pointer to the Accept() function.
        // We have to use a static version of the function so that it can be cast to long.
        bool (*acceptFunc)(mafNode*) = &lhpOpMultiscaleExplore::AcceptStatic ;

        // raise event to call up select vme dialog
        // The VME_CHOOSE event is handled by mafLogicWithManagers
        mafEvent e(this,VME_CHOOSE);  // create choose event 
        e.SetArg((long)acceptFunc) ;  // pass the accept function to the event.
        mafEventMacro(e);
        mafVME* vme = mafVME::SafeDownCast(e.GetVme());

        // Add vme to scene
        if (vme != NULL)
          AddVmeToScene(vme) ;

        break;
      }

    case ID_ZOOMOUT:
      OnZoomOut(GetRenderer()) ;
      break;

    case ID_CAMERARESET:
      OnCameraReset(GetRenderer()) ;
      break ;

    case ID_GOBACK:
      OnGoBack(GetRenderer()) ;
      break ;

    case ID_DEBUG:
      {
        // Write system state to file
        std::fstream thing ;
        thing.open("C:/Documents and Settings/Nigel.DB6ZB32J/My Documents/Visual Studio Projects/MAF/Multiscale2/thing.txt", thing.out | thing.app) ;
        OnDebug(thing, GetRenderer()) ;
        thing.close() ;
        break ;
      }

    case ID_POS_SLIDER:
      {
        UpdateSlicePosition() ;
        UpdateCamera() ;
        break ;
      }

    case ID_CHANGE_VIEW:
      {
        double bounds[6] ;
        GetMultiscaleUtility()->GetAttentionBounds(bounds) ;
        UpdateViewAxis(bounds) ;
        UpdateCamera() ;
        break ;
      }

    case ID_BASE_UNITS:
      {
        // render in order to update the scale display
        UpdateCamera() ;
        break ;
      }

    case ID_OPACITY_SLIDER:
      {
        m_Dialog->TransferDataToWindow() ;
        break ;
      }

    case ID_OK:
      m_Dialog->EndModal(wxID_OK);
      break;

    case ID_CANCEL:
      m_Dialog->EndModal(wxID_CANCEL);
      break;

    case ID_START_RENDER:
      OnStartRender(GetRenderer()) ;
      break ;

    case ID_MOUSE_CLICK:
      OnMouseClick() ;
      break ;

    default:
      mafEventMacro(*e);
      break; 
    }
  }
  else{
    if (e->GetId() == mafDeviceButtonsPadMouse::GetMouseDClickId())
      mafLogMessage("double click !") ;
  }
}



//------------------------------------------------------------------------------
// Set the token size
void lhpOpMultiscaleExplore::SetTokenSize(vtkRenderer* renderer, int tokenId, int tokenSize)
//------------------------------------------------------------------------------
{
  // get token and corresponding actor
  int actorId = GetMultiscaleUtility()->GetDataActorCorrespondingToToken(tokenId) ;
  vtkActor* actor = GetMultiscaleUtility()->GetMultiscaleActor(actorId)->GetActor() ;
  vtkActor* token = GetMultiscaleUtility()->GetMultiscaleActor(tokenId)->GetActor() ;

  // reset size of token
  GetMultiscaleUtility()->GetActorCoordsUtility()->SetActorDisplaySize(token, renderer, tokenSize) ;

  // reposition the token on the actor because the resizing changes the position slightly
  GetMultiscaleUtility()->GetActorCoordsUtility()->MoveToCenter(token, actor) ;

}


//------------------------------------------------------------------------------
// ZoomOut - Simple zoom out by x2
void lhpOpMultiscaleExplore::OnZoomOut(vtkRenderer* renderer)
//------------------------------------------------------------------------------
{
  vtkCamera *camera = renderer->GetActiveCamera() ;

  // get the current scale and desired new scale
  double scale = GetMultiscaleUtility()->GetCameraUtility()->CalculateScale(camera) ;
  double scaleNew = scale * 2.0 ;

  // calculate the new view angle
  double FL = camera->GetDistance() ;
  double viewAngleRad = 2.0 * atan(0.5*scaleNew/FL) ;
  double viewAngle = 180.0 / 3.14159 * viewAngleRad ;
  camera->SetViewAngle(viewAngle) ;

  // this is a dialog event so we need to manually call a render when we have finished
  renderer->GetRenderWindow()->Render() ;
  UpdateCamera() ;
}



//------------------------------------------------------------------------------
// ZoomReset - reset camera to top level
void lhpOpMultiscaleExplore::OnCameraReset(vtkRenderer* renderer)
//------------------------------------------------------------------------------
{
  // reset the camera to the start view
  GetMultiscaleUtility()->ClearViewSaves() ;      // clears all states except the intial view
  GetMultiscaleUtility()->RestoreView(renderer) ;

  // Set any visible tokens to the default size
  for (int i = 0 ;   i < GetMultiscaleUtility()->GetNumberOfActors() ;  i++){
    lhpMultiscaleActor *ma = GetMultiscaleUtility()->GetMultiscaleActor(i) ;

    if ((ma->GetActorType() == MSCALE_TOKEN) && (ma->GetVisibility() == 1))
      SetTokenSize(renderer, i, TOKENSIZE) ;
  }

  // set the attention on all the actors
  GetMultiscaleUtility()->SetAttentionAll(true) ;

  // attention has changed, so update the slider range
  double bounds[6] ;
  GetMultiscaleUtility()->GetAttentionBounds(bounds) ;
  SetSliderRange(bounds) ;

  // this is a dialog event so we need to manually call a render when we have finished
  renderer->GetRenderWindow()->Render() ;
  UpdateCamera() ;
}



//------------------------------------------------------------------------------
// GoBack - This event requires us to go back up the actor stack
void lhpOpMultiscaleExplore::OnGoBack(vtkRenderer* renderer)
//------------------------------------------------------------------------------
{
  if (GetMultiscaleUtility()->CameraSameSinceLastSave(renderer)){
    // The camera has not moved since the last save so we should go straight back to the previous pick.
    // reset the camera to the last saved view but one
    GetMultiscaleUtility()->RestoreView(renderer) ;
  }
  else{
    // The camera has moved so we should go back to the last pick.
    // Reset the camera to the last saved view without deleting saved view.
    GetMultiscaleUtility()->RestoreViewWithoutDelete(renderer) ;
  }

  // Set any visible tokens to the default size
  for (int i = 0 ;   i < GetMultiscaleUtility()->GetNumberOfActors() ;  i++){
    lhpMultiscaleActor *ma = GetMultiscaleUtility()->GetMultiscaleActor(i) ;

    if ((ma->GetActorType() == MSCALE_TOKEN) && (ma->GetVisibility() == 1))
      SetTokenSize(renderer, i, TOKENSIZE) ;
  }

  // attention has changed, so update the slider range
  double bounds[6] ;
  GetMultiscaleUtility()->GetAttentionBounds(bounds) ;
  SetSliderRange(bounds) ;
  
  // this is a dialog event so we need to manually call a render when we have finished
  renderer->GetRenderWindow()->Render() ;
  UpdateCamera() ;

}



//------------------------------------------------------------------------------
// Start Render handler.
// This is where we look at the scene and decide which actors and tokens should be visible.
// Nowhere else should change the visibility of the actors.
void lhpOpMultiscaleExplore::OnStartRender(vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  int i, j ;


  // Get the current scale in tidy units and display in the dialog
  int iscale ;
  std::ostrstream units ;
  std::ostrstream value ;

  double scale = GetMultiscaleUtility()->GetCameraUtility()->CalculateScale(renderer->GetActiveCamera()) ;
  GetMultiscaleUtility()->ConvertScaleToTidyUnits(scale, m_BaseUnits, &iscale, units) ;
  value << iscale << std::ends ;

  if (m_Dialog != NULL){
    wxTextCtrl* valueTxtCtrl = dynamic_cast<wxTextCtrl*>(m_Dialog->FindWindow(ID_SCALEVALUETXT)) ;
    wxTextCtrl* unitsTxtCtrl = dynamic_cast<wxTextCtrl*>(m_Dialog->FindWindow(ID_SCALEUNITSTXT)) ;

    valueTxtCtrl->Clear() ;
    valueTxtCtrl->AppendText(value.str()) ;
    unitsTxtCtrl->Clear() ;
    unitsTxtCtrl->AppendText(units.str()) ;
  }


  // We check all the data actors (not tokens) to see if they have crossed the size thresholds
  // NB SIZELOWER and SIZEUPPER are the hysteresis bounds of the threshold screen size.
  for (i = 0 ;   i < GetMultiscaleUtility()->GetNumberOfActors() ;  i++){
    lhpMultiscaleActor *ma = GetMultiscaleUtility()->GetMultiscaleActor(i) ;
    vtkActor *actor = ma->GetActor() ;

    // Get the size of the actor
    // Note that we define the actor screen size with the view-independent GetMaxSizeDisplayAnyView()
    // because we don't want the actor to change just because is is being viewed down a long thin axis.
    // To be consistent we should do the same for the tokens, but as they don't have a long axis it doesn't matter.
    double screenSize = GetMultiscaleUtility()->GetActorCoordsUtility()->GetMaxSizeDisplayAnyView(actor, renderer) ;
    double worldSize = GetMultiscaleUtility()->GetActorCoordsUtility()->GetMaxSizeWorld(actor) ;
 
    if (ma->GetActorType() == MSCALE_DATA_ACTOR){
      // check if actor has gone below the size threshold
      if (screenSize < SIZELOWER){
        if (ma->GetVisibility() == 1 || ma->GetScaleStatus() != TOO_SMALL)
          OnActorTooSmall(renderer, i) ;
      }
      // check if actor is in the scale range (actors with attention have no upper size limit)
      else if (ma->GetAttention() && (screenSize > SIZEUPPER)){
        if (ma->GetVisibility() == 0 || ma->GetScaleStatus() != IN_SCALE)
          OnActorInScale(renderer, i) ;
      }
      // check if actor is in the scale range (actors without attention have to also be smaller than current scale)
      else if (!ma->GetAttention() && (screenSize > SIZEUPPER) && (worldSize <= (double)TOO_LARGE_FACTOR*scale)){
        if (ma->GetVisibility() == 0 || ma->GetScaleStatus() != IN_SCALE)
          OnActorInScale(renderer, i) ;
      }
      // check if actor without attention has gone too large
      else if (!ma->GetAttention() && (worldSize > (double)TOO_LARGE_FACTOR*scale)){
        if (ma->GetScaleStatus() != TOO_LARGE)
          OnActorTooLarge(renderer, i) ;
      }
    }
  }

  // Now we check if any of the visible tokens need to be resized
  for (i = 0 ;   i < GetMultiscaleUtility()->GetNumberOfActors() ;  i++){
    lhpMultiscaleActor *ma = GetMultiscaleUtility()->GetMultiscaleActor(i) ;
    vtkActor *actor = ma->GetActor() ;

    // get the screen size of the actor
    double screenSize = GetMultiscaleUtility()->GetActorCoordsUtility()->GetMaxSizeDisplay(actor, renderer) ;

    if ((ma->GetActorType() == MSCALE_TOKEN) && (ma->GetVisibility() == 1)){
      if (ma->GetScreenSizeMode() == FIXED_SIZE){
        // fixed size - reset to default size
        SetTokenSize(renderer, i, TOKENSIZE) ;
      }
      else if (ma->GetScreenSizeMode() == LIMITED_SIZE){
        // limited size - keep size between max or min
        if (screenSize < TOKENSIZEMIN)
          SetTokenSize(renderer, i, TOKENSIZEMIN) ;
        else if (screenSize > TOKENSIZEMAX)
          SetTokenSize(renderer, i, TOKENSIZEMAX) ;
      }
    }
  }

  // Check if any pairs of tokens are touching or not touching
  for (i = 0 ;   i < GetMultiscaleUtility()->GetNumberOfActors() ;  i++){
    for (j = i+1 ;  j < GetMultiscaleUtility()->GetNumberOfActors() ;  j++){
      lhpMultiscaleActor *mai = GetMultiscaleUtility()->GetMultiscaleActor(i) ;
      lhpMultiscaleActor *maj = GetMultiscaleUtility()->GetMultiscaleActor(j) ;
      if ((mai->GetActorType() == MSCALE_TOKEN) && (maj->GetActorType() == MSCALE_TOKEN)){
        vtkActor *actori = mai->GetActor() ;
        vtkActor *actorj = maj->GetActor() ;

        // Check whether both tokens are active, ie their data actors are too small.
        // Tokens should not be declared touching unless they are both active,
        // Otherwise an inactive token could make a visible one disappear.
        int ii = GetMultiscaleUtility()->GetDataActorCorrespondingToToken(i) ;
        int jj = GetMultiscaleUtility()->GetDataActorCorrespondingToToken(j) ;
        lhpMultiscaleActor *maii = GetMultiscaleUtility()->GetMultiscaleActor(ii) ;
        lhpMultiscaleActor *majj = GetMultiscaleUtility()->GetMultiscaleActor(jj) ;
        bool bothActive = ((maii->GetScaleStatus() == TOO_SMALL) && (majj->GetScaleStatus() == TOO_SMALL)) ;

        // get distance between tokens
        double distApart = GetMultiscaleUtility()->DistanceApartPixelUnits(actori, actorj, renderer) ;

        if ((distApart < TOKENSIZEMIN) && bothActive){
          if (!GetMultiscaleUtility()->TokensListedAsTouching(i,j)){
            // tokens were not touching, but now are - call event handler
            OnTokensOverlap(renderer, i, j) ;
          }
        }
        else{
          if (GetMultiscaleUtility()->TokensListedAsTouching(i,j)){
            // tokens were touching, but now are not - call event handler
            OnTokensSeparated(renderer, i, j) ;
          }
        }
      }
    }
  }
}



//------------------------------------------------------------------------------
// Actor gone below threshold size
void lhpOpMultiscaleExplore::OnActorTooSmall(vtkRenderer* renderer, int actorId)
//------------------------------------------------------------------------------
{
  // get actor and token
  int tokenId = GetMultiscaleUtility()->GetTokenCorrespondingToDataActor(actorId) ;
  lhpMultiscaleActor *ma_actor = GetMultiscaleUtility()->GetMultiscaleActor(actorId) ;
  lhpMultiscaleActor *ma_token = GetMultiscaleUtility()->GetMultiscaleActor(tokenId) ;
  vtkActor* actor = ma_actor->GetActor() ;
  vtkActor* token = ma_token->GetActor() ;

  // make token visible instead of actor
  ma_actor->SetVisibility(0) ;
  ma_token->SetVisibility(1) ;

  // set size and position of token
  GetMultiscaleUtility()->GetActorCoordsUtility()->SetActorDisplaySize(token, renderer, TOKENSIZE) ;
  GetMultiscaleUtility()->GetActorCoordsUtility()->MoveToCenter(token, actor) ;

  // set flag to indicate that this actor is now below the scale threshold
  ma_actor->SetScaleStatus(TOO_SMALL) ;
}


//------------------------------------------------------------------------------
// Actor is in visible scale range
void lhpOpMultiscaleExplore::OnActorInScale(vtkRenderer* renderer, int actorId)
//------------------------------------------------------------------------------
{
  // get actor and token
  int tokenId = GetMultiscaleUtility()->GetTokenCorrespondingToDataActor(actorId) ;
  lhpMultiscaleActor *ma_actor = GetMultiscaleUtility()->GetMultiscaleActor(actorId) ;
  lhpMultiscaleActor *ma_token = GetMultiscaleUtility()->GetMultiscaleActor(tokenId) ;
  vtkActor* actor = ma_actor->GetActor() ;
  vtkActor* token = ma_token->GetActor() ;

  // make actor visible and token invisible
  ma_actor->SetVisibility(1) ;
  ma_token->SetVisibility(0) ;

  // set flag to indicate that this actor is now above the scale threshold
  ma_actor->SetScaleStatus(IN_SCALE) ;
}



//------------------------------------------------------------------------------
// Actor gone too large for scale
void lhpOpMultiscaleExplore::OnActorTooLarge(vtkRenderer* renderer, int actorId)
//------------------------------------------------------------------------------
{
  // get actor and token
  int tokenId = GetMultiscaleUtility()->GetTokenCorrespondingToDataActor(actorId) ;
  lhpMultiscaleActor *ma_actor = GetMultiscaleUtility()->GetMultiscaleActor(actorId) ;
  lhpMultiscaleActor *ma_token = GetMultiscaleUtility()->GetMultiscaleActor(tokenId) ;
  vtkActor* actor = ma_actor->GetActor() ;
  vtkActor* token = ma_token->GetActor() ;

  // make both actor and token invisible
  ma_actor->SetVisibility(0) ;
  ma_token->SetVisibility(0) ;

  // set flag to indicate that this actor is now too large
  ma_actor->SetScaleStatus(TOO_LARGE) ;
}



//------------------------------------------------------------------------------
// Tokens overlap
void lhpOpMultiscaleExplore::OnTokensOverlap(vtkRenderer* renderer, int tokenId1, int tokenId2)
//------------------------------------------------------------------------------
{
  // get token actors
  lhpMultiscaleActor *ma_token1 = GetMultiscaleUtility()->GetMultiscaleActor(tokenId1) ;
  lhpMultiscaleActor *ma_token2 = GetMultiscaleUtility()->GetMultiscaleActor(tokenId2) ;
  vtkActor *tokenActor1 = ma_token1->GetActor() ;
  vtkActor *tokenActor2 = ma_token2->GetActor() ;


  // get corresponding data actors and their size
  int actorId1 = GetMultiscaleUtility()->GetDataActorCorrespondingToToken(tokenId1) ;
  int actorId2 = GetMultiscaleUtility()->GetDataActorCorrespondingToToken(tokenId2) ;

  lhpMultiscaleActor *ma_actor1 = GetMultiscaleUtility()->GetMultiscaleActor(actorId1) ;
  lhpMultiscaleActor *ma_actor2 = GetMultiscaleUtility()->GetMultiscaleActor(actorId2) ;
  vtkActor *actor1 = ma_actor1->GetActor() ;
  vtkActor *actor2 = ma_actor2->GetActor() ;

  double siz1 = GetMultiscaleUtility()->GetActorCoordsUtility()->GetMaxSizeWorld(actor1) ;
  double siz2 = GetMultiscaleUtility()->GetActorCoordsUtility()->GetMaxSizeWorld(actor2) ;


  // Make a record of the touching pair and make the one with the smaller data actor invisible.
  // If they are the same size, the data actor with the lower index is considered larger.
  GetMultiscaleUtility()->AddTouchingTokens(tokenId1, tokenId2) ;
  if (siz1 > siz2)
    ma_token2->SetVisibility(0) ;
  else if (siz1 < siz2)
    ma_token1->SetVisibility(0) ;
  else{
    if (actorId1 < actorId2)
      ma_token2->SetVisibility(0) ;
    else
      ma_token1->SetVisibility(0) ;
  }
}


//------------------------------------------------------------------------------
// Tokens separated
void lhpOpMultiscaleExplore::OnTokensSeparated(vtkRenderer* renderer, int tokenId1, int tokenId2)
//------------------------------------------------------------------------------
{
  // get token actors
  lhpMultiscaleActor *ma_token1 = GetMultiscaleUtility()->GetMultiscaleActor(tokenId1) ;
  lhpMultiscaleActor *ma_token2 = GetMultiscaleUtility()->GetMultiscaleActor(tokenId2) ;
  vtkActor *tokenActor1 = ma_token1->GetActor() ;
  vtkActor *tokenActor2 = ma_token2->GetActor() ;


  // get corresponding data actors and their size
  int actorId1 = GetMultiscaleUtility()->GetDataActorCorrespondingToToken(tokenId1) ;
  int actorId2 = GetMultiscaleUtility()->GetDataActorCorrespondingToToken(tokenId2) ;

  lhpMultiscaleActor *ma_actor1 = GetMultiscaleUtility()->GetMultiscaleActor(actorId1) ;
  lhpMultiscaleActor *ma_actor2 = GetMultiscaleUtility()->GetMultiscaleActor(actorId2) ;
  vtkActor *actor1 = ma_actor1->GetActor() ;
  vtkActor *actor2 = ma_actor2->GetActor() ;

  double siz1 = GetMultiscaleUtility()->GetActorCoordsUtility()->GetMaxSizeWorld(actor1) ;
  double siz2 = GetMultiscaleUtility()->GetActorCoordsUtility()->GetMaxSizeWorld(actor2) ;


  // erase the record of the touching pair
  GetMultiscaleUtility()->RemoveTouchingTokens(tokenId1, tokenId2) ;

  // make tokens visible if their data actors are not currently visible and they are not touched by any tokens of larger actors
  if ((ma_actor1->GetVisibility() == 0) && (GetMultiscaleUtility()->GetNumberOfTouchingTokensLarger(tokenId1) == 0))
    ma_token1->SetVisibility(1) ;
  if ((ma_actor2->GetVisibility() == 0) && (GetMultiscaleUtility()->GetNumberOfTouchingTokensLarger(tokenId2) == 0))
    ma_token2->SetVisibility(1) ;
}


//------------------------------------------------------------------------------
// Mouse click handler
void lhpOpMultiscaleExplore::OnMouseClick()
//------------------------------------------------------------------------------
{
  int i, j, xscreen, yscreen ;
  double boundsD[6] ;

  std::vector<ACTORINFO>targetedActors ;

  vtkRenderWindowInteractor *RWI = GetInteractor() ;
  vtkRenderer *renderer = GetRenderer() ;

  // get position of event
  RWI->GetLastEventPosition(xscreen, yscreen) ;

  // list all the visible tokens which are targeted by the click (there may be more than one)
  int nactors = GetMultiscaleUtility()->GetNumberOfActors() ;
  for (i = 0 ;   i < nactors ;  i++){
    lhpMultiscaleActor *ma = GetMultiscaleUtility()->GetMultiscaleActor(i) ;
    vtkActor *actor = ma->GetActor() ;

    if ((ma->GetActorType() == MSCALE_TOKEN) && (ma->GetVisibility() == 1)){
      // get bounds of actor in display coords
      GetMultiscaleUtility()->GetActorCoordsUtility()->GetBoundsDisplay(actor, renderer, boundsD) ;

      // compare bounding box with event coords
      if ((xscreen >= boundsD[0]) && (xscreen <= boundsD[1]) && (yscreen >= boundsD[2]) && (yscreen <= boundsD[3])){
        // get depth of actor in view coords
        double centerV[3] ;
        GetMultiscaleUtility()->GetActorCoordsUtility()->GetCenterView(actor, renderer, centerV) ;

        ACTORINFO info ;
        info.actorId = i ;
        info.depth = centerV[2] ;

        // note index of picked actor
        targetedActors.push_back(info) ;
      }
    }
  }

  // do nothing if no tokens were picked
  if (targetedActors.size() == 0)
    return ;


  // Pick the targeted actor which is closest to the viewer
  int jnearest = 0 ;
  for (j = 0 ;  j < (int)targetedActors.size() ;  j++){
    if (targetedActors[j].depth < targetedActors[jnearest].depth)
      jnearest = j ;
  }
  i = targetedActors[jnearest].actorId ;
  OnPick(renderer, i) ;
}



//------------------------------------------------------------------------------
// Pick token
void lhpOpMultiscaleExplore::OnPick(vtkRenderer* renderer, int tokenId)
//------------------------------------------------------------------------------
{
  lhpMultiscaleUtility *MSU = GetMultiscaleUtility() ;

  // is token currently touching any other tokens
  int ntouching = MSU->GetNumberOfTouchingTokens(tokenId) ;

  // get token and corresponding object
  int actorId = MSU->GetDataActorCorrespondingToToken(tokenId) ;
  vtkActor* actor = MSU->GetMultiscaleActor(actorId)->GetActor() ;
  vtkActor* token = MSU->GetMultiscaleActor(tokenId)->GetActor() ;

  // temporarily allow the token to change screen size so we can see it zoom
  lhpMultiscale::ScreenSizeMode saveMode = MSU->GetMultiscaleActor(tokenId)->GetScreenSizeMode() ;
  MSU->GetMultiscaleActor(tokenId)->SetScreenSizeMode(VARIABLE_SIZE) ;

  if (ntouching == 0){
    // zoom in on actor corresponding to picked token
    MSU->GetCameraUtility()->ZoomCameraOnActor(actor, renderer) ;

    // set the picked actor to have the attention
    MSU->SetAttentionAll(false) ;
    MSU->GetMultiscaleActor(actorId)->SetAttention(true) ;
  }
  else{
    // get collection of touching actors
    vtkActorCollection *AC = vtkActorCollection::New() ;
    MSU->GetTouchingDataActorsAsCollection(tokenId, AC) ;

    // Set the attention to all the actors in the collection,
    // because you don't want any to disappear when you zoom in.
    // Do this before you zoom, while the tokens are still touching !
    std::vector<int> idlist ;
    MSU->GetTouchingDataActorsAsIdList(tokenId, idlist) ;
    MSU->SetAttentionAll(false) ;
    MSU->SetAttentionToList(idlist, true) ;

    // zoom in on the collection
    MSU->GetCameraUtility()->ZoomCameraOnActorCollection(AC, renderer) ;

    // delete actor collection
    AC->Delete() ;
  }

  // set the token's screen size back to original behaviour
  MSU->GetMultiscaleActor(tokenId)->SetScreenSizeMode(saveMode) ;

  // save the camera params and attention flags
  MSU->SaveView(renderer) ;


  // attention has changed, so update the slider range
  double bounds[6] ;
  MSU->GetAttentionBounds(bounds) ;
  SetSliderRange(bounds) ;

}


//------------------------------------------------------------------------------
// Debug handler
void lhpOpMultiscaleExplore::OnDebug(std::ostream& os, vtkRenderer *renderer)
//------------------------------------------------------------------------------
{
  // print state of system as held by multiscale utility
  GetMultiscaleUtility()->PrintSelf(os, 0) ;

  // print positions and sizes of actors
  os << "size and postion of actors" << std::endl ;
  double sizW[4], sizD[3], posW[4], posD[3], posV[3] ;
  for (int i = 0 ;  i < GetMultiscaleUtility()->GetNumberOfActors() ;  i++){
    os << i << std::endl ;
    vtkActor* actor = GetMultiscaleUtility()->GetMultiscaleActor(i)->GetActor() ;
    this->GetMultiscaleUtility()->GetActorCoordsUtility()->GetCenterWorld(actor, posW) ;
    this->GetMultiscaleUtility()->GetActorCoordsUtility()->GetCenterView(actor, renderer, posV) ;
    this->GetMultiscaleUtility()->GetActorCoordsUtility()->GetCenterDisplay(actor, renderer, posD) ;
    this->GetMultiscaleUtility()->GetActorCoordsUtility()->GetSizeWorld(actor, sizW) ;
    this->GetMultiscaleUtility()->GetActorCoordsUtility()->GetSizeDisplay(actor, renderer, sizD) ;
    os << "center (world)   " ; lhpMultiscaleVectorMath::PrintVector(os, posW) ;
    os << "center (view)    " ; lhpMultiscaleVectorMath::PrintVector(os, posV) ;
    os << "center (display) " ; lhpMultiscaleVectorMath::PrintVector(os, posD) ;
    os << "size (world)   " ; lhpMultiscaleVectorMath::PrintVector(os, sizW) ;
    os << "size (display) " ; lhpMultiscaleVectorMath::PrintVector(os, sizD) ;
    os << std::endl ;
  }
  os << std::endl ;
}
