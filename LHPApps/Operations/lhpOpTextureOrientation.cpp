/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpTextureOrientation.cpp,v $
Language:  C++
Date:      $Date: 2010-12-06 14:53:07 $
Version:   $Revision: 1.1.1.1.2.5 $
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
#include "mafVMEOutput.h"

#include "lhpOpTextureOrientation.h"
#include "lhpHistogramEqualizationFilter.h"
#include "lhpTextureOrientationFilter.h"
#include "lhpTextureOrientationSlicePipe.h"

#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkStructuredPoints.h"
#include "vtkPolyData.h"
#include "vtkActor.h"
#include "vtkOutlineCornerFilter.h"
#include "vtkPlaneSource.h"
#include "vtkProbeFilter.h"
#include "vtkSphereSource.h"
#include "vtkLookupTable.h"
#include "vtkPointData.h"
#include "vtkCamera.h"
#include "vtkGlyph3D.h"
#include "vtkPolyDataNormals.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkProperty.h"

#include <fstream>
#include <ostream>
#include <algorithm>

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpTextureOrientation);
//----------------------------------------------------------------------------

namespace lhpTextureOrientation
{
  //----------------------------------------------------------------------------
  // widget ID's
  //----------------------------------------------------------------------------
  enum TextureOrientationIds
  {
    ID_OK = MINID,
    ID_CANCEL,
    ID_POS_SLIDER,
    ID_POS_VALUE_TXT,
    ID_CHANGE_VIEW,
    ID_VIEW_COMBO,
    ID_RANGE_LO_SLIDER,
    ID_RANGE_LO_VALUE_TXT,
    ID_RANGE_HI_SLIDER,
    ID_RANGE_HI_VALUE_TXT,
    ID_TEXWINSIZE_VALUE_TXT,
    ID_TEXWINSTEP_VALUE_TXT,
    ID_OUTPUT_FORMAT_COMBO,
    ID_FILENAME_TXT,
    ID_PROGRESS_GAUGE,
    ID_PROGRESS_EVENT,
    ID_PRINT_BUTTON,
    ID_UPDATE
  };
}


//----------------------------------------------------------------------------
// Use texture orientation namespace
//----------------------------------------------------------------------------
using namespace lhpTextureOrientation ;



//----------------------------------------------------------------------------
// Constructor
lhpOpTextureOrientation::lhpOpTextureOrientation(wxString label) :
mafOp(label), m_Dialog(NULL), m_Rwi(NULL), m_polydata(NULL), m_polydataVME(NULL),
m_externalRenderer(NULL), m_slicePipe(NULL), m_vectorGlyphPipe(NULL), m_tensorGlyphPipe(NULL),
m_printButton(NULL), m_progressGauge(NULL), m_progressCallback(NULL),
m_vectorPipeUpdated(false), m_tensorPipeUpdated(false),
m_ProbeSizeSceneUnits(5), m_SampleSpacingSceneUnits(5), m_outputFormat(VectorFormat)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_OP;
  m_Canundo = true ;
  m_Dialog = NULL;
  m_Rwi = NULL;
}


//----------------------------------------------------------------------------
// Destructor
lhpOpTextureOrientation::~lhpOpTextureOrientation()
//----------------------------------------------------------------------------
{
  if (m_polydata != NULL)
    m_polydata->Delete() ;

  if (m_slicePipe != NULL)
    delete m_slicePipe ;

  if (m_vectorGlyphPipe != NULL)
    delete m_vectorGlyphPipe ;

  if (m_tensorGlyphPipe != NULL)
    delete m_tensorGlyphPipe ;

  if (m_progressCallback != NULL)
    delete m_progressCallback ;
}




//----------------------------------------------------------------------------
// Copy
mafOp* lhpOpTextureOrientation::Copy()
//----------------------------------------------------------------------------
{
  /** return a copy of itself, needs to put it into the undo stack */
  return new lhpOpTextureOrientation(m_Label);
}


//----------------------------------------------------------------------------
// Accept volume data
bool lhpOpTextureOrientation::Accept(mafNode* vme)
//----------------------------------------------------------------------------
{
  return (vme != NULL && (vme->IsMAFType(mafVMEVolumeGray))) ;
}



//----------------------------------------------------------------------------
// Create gui etc before event loop takes over
void lhpOpTextureOrientation::OpRun()
//----------------------------------------------------------------------------
{
  int result = OP_RUN_CANCEL;

  if (m_TestMode == false)
  {
    CreateOpDialog();

    //----------------------------------------------------------------------------
    // Create visual pipe and render
    //----------------------------------------------------------------------------
    CreateVisualPipe() ;
    //GetRenderer()->GetRenderWindow()->Render() ;
    //UpdateCamera() ;

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
void lhpOpTextureOrientation::OpDo()
//----------------------------------------------------------------------------
{
}


//----------------------------------------------------------------------------
// Undo the op
void lhpOpTextureOrientation::OpUndo()
//----------------------------------------------------------------------------
{
}





//----------------------------------------------------------------------------
// Create the op dialog
void lhpOpTextureOrientation::CreateOpDialog()
//----------------------------------------------------------------------------
{
  wxBusyCursor wait;


  //----------------------------------------------------------------------------
  // get input vme
  //----------------------------------------------------------------------------
  mafVME* vme = mafVME::SafeDownCast(m_Input) ;



  //----------------------------------------------------------------------------
  // setup interface
  //----------------------------------------------------------------------------
  m_Dialog = new mafGUIDialog("Texture Orientation", mafCLOSEWINDOW | mafRESIZABLE);

  m_Rwi = new mafRWI(m_Dialog,ONE_LAYER,false);
  m_Rwi->SetListener(this);
  m_Rwi->CameraSet(CAMERA_PERSPECTIVE);

  m_Rwi->m_RenderWindow->SetDesiredUpdateRate(0.0001f);
  m_Rwi->SetSize(0,0,600,600);
  m_Rwi->Show(true);
  m_Rwi->m_RwiBase->SetMouse(m_Mouse) ;




  //----------------------------------------------------------------------------
  // Set up the gui widgets
  //----------------------------------------------------------------------------
  wxPoint p = wxDefaultPosition;

  // slice pipeline controls
  wxString Views[3] = {"XY","XZ","YZ"};
  double bounds[6] ;
  vme->GetOutput()->GetBounds(bounds) ;
  InitSliceParams(ID_XY, bounds) ;      // set the validator values (nb there are no visual pipes at this stage)
  wxStaticText *sliceCtrlsStaticTxt = new wxStaticText(m_Dialog, -1, "Slice Controls");
  wxStaticText *posStaticTxt = new wxStaticText(m_Dialog, -1, "pos ") ;
  wxTextCtrl *posValueTxt = new wxTextCtrl(m_Dialog, ID_POS_VALUE_TXT, wxEmptyString, p, wxSize(80,20), wxTE_RIGHT) ;
  m_PosSlider = new mafGUIFloatSlider(m_Dialog, ID_POS_SLIDER, m_SliderOrigin, bounds[4], bounds[5], p) ;
  wxStaticText *viewStaticTxt = new wxStaticText(m_Dialog, -1, "view ") ;
  wxComboBox *viewCombo = new wxComboBox(m_Dialog, ID_CHANGE_VIEW, Views[m_ViewIndex], p, wxSize(80,20), 3, Views, wxCB_READONLY) ;

  // texture pipeline controls
  wxString OutputFormats[2] = {"Vector", "Tensor"};
  wxStaticText *textureCtrlsStaticTxt  = new wxStaticText(m_Dialog, -1, "Texture Controls");
  wxStaticText *texWinSizeStaticTxt = new wxStaticText(m_Dialog, -1, "probe size (scene units) ") ;
  wxStaticText *texWinStepStaticTxt = new wxStaticText(m_Dialog, -1, "sample spacing (scene units) ") ;
  wxStaticText *outputFormatStaticTxt = new wxStaticText(m_Dialog, -1, "output format ") ;
  wxTextCtrl *texWinSizeValueTxt = new wxTextCtrl(m_Dialog, ID_TEXWINSIZE_VALUE_TXT, wxEmptyString, p, wxSize(80,20), wxTE_RIGHT) ;
  wxTextCtrl *texWinStepValueTxt = new wxTextCtrl(m_Dialog, ID_TEXWINSTEP_VALUE_TXT, wxEmptyString, p, wxSize(80,20), wxTE_RIGHT) ;
  wxComboBox *outputFormatCombo = new wxComboBox(m_Dialog, ID_OUTPUT_FORMAT_COMBO, OutputFormats[0], p, wxSize(80,20), 2, OutputFormats, wxCB_READONLY) ;
  wxStaticText *progressStaticTxt = new wxStaticText(m_Dialog, -1, "progress ") ;
  m_progressGauge = new wxGauge(m_Dialog, ID_PROGRESS_GAUGE, 100, p, wxSize(80,20), wxGA_HORIZONTAL) ;
  wxStaticText *filenameStaticTxt = new wxStaticText(m_Dialog, -1, "filename ") ;
  wxTextCtrl *filenameValueTxt = new wxTextCtrl(m_Dialog, ID_FILENAME_TXT, wxEmptyString, p, wxSize(80,20), wxTE_RIGHT) ;
  m_printButton = new mafGUIButton(m_Dialog, ID_PRINT_BUTTON, "Print", p, wxSize(80,20));
  m_printButton->Enable(false) ;
  mafGUIButton *updateButton = new mafGUIButton(m_Dialog, ID_UPDATE, "Update", p, wxSize(80,20));

  // ok and cancel
  m_okButton = new mafGUIButton(m_Dialog, ID_OK, "ok", p, wxSize(80,20));
  m_okButton->Enable(false) ; // can't use ok until pipeline has been updated
  mafGUIButton  *cancel = new mafGUIButton(m_Dialog, ID_CANCEL, "cancel", p, wxSize(80,20));


  // Set validators
  // nb no validator exists for wxGauge - you have to use a callback
  posValueTxt->SetValidator(mafGUIValidator(this,ID_POS_VALUE_TXT,posValueTxt,&m_SliderOrigin)) ;
  m_PosSlider->SetValidator(mafGUIValidator(this, ID_POS_SLIDER, m_PosSlider, &m_SliderOrigin, posValueTxt));
  viewCombo->SetValidator(mafGUIValidator(this, ID_CHANGE_VIEW, viewCombo, &m_ViewIndex)) ;
  texWinSizeValueTxt->SetValidator(mafGUIValidator(this, ID_TEXWINSIZE_VALUE_TXT, texWinSizeValueTxt, &m_ProbeSizeSceneUnits)) ;
  texWinStepValueTxt->SetValidator(mafGUIValidator(this, ID_TEXWINSTEP_VALUE_TXT, texWinStepValueTxt, &m_SampleSpacingSceneUnits)) ;
  outputFormatCombo->SetValidator(mafGUIValidator(this, ID_OUTPUT_FORMAT_COMBO, outputFormatCombo, &m_outputFormat)) ;
  filenameValueTxt->SetValidator(mafGUIValidator(this, ID_FILENAME_TXT, filenameValueTxt, &m_outputFilename)) ;
  m_printButton->SetValidator(mafGUIValidator(this,ID_PRINT_BUTTON,m_printButton));
  updateButton->SetValidator(mafGUIValidator(this,ID_UPDATE,updateButton));
  m_okButton->SetValidator(mafGUIValidator(this,ID_OK, m_okButton));
  cancel->SetValidator(mafGUIValidator(this,ID_CANCEL,cancel));




  // layout
  wxBoxSizer *h_sizer10 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer10->Add(sliceCtrlsStaticTxt, 0, wxLEFT);	

  wxBoxSizer *h_sizer12 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer12->Add(posStaticTxt, 0, wxLEFT);
  h_sizer12->Add(posValueTxt, 0, wxLEFT) ;
  h_sizer12->Add(m_PosSlider, 0, wxLEFT);

  wxBoxSizer *h_sizer14 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer14->Add(viewStaticTxt, 0, wxLEFT);
  h_sizer14->Add(viewCombo, 0, wxLEFT) ;

  wxBoxSizer *h_sizer16 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer16->Add(textureCtrlsStaticTxt, 0, wxLEFT);	

  wxBoxSizer *h_sizer18 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer18->Add(texWinSizeStaticTxt, 0, wxLEFT);
  h_sizer18->Add(texWinSizeValueTxt, 0, wxLEFT) ;

  wxBoxSizer *h_sizer20 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer20->Add(texWinStepStaticTxt, 0, wxLEFT);
  h_sizer20->Add(texWinStepValueTxt, 0, wxLEFT) ;

  wxBoxSizer *h_sizer22 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer22->Add(outputFormatStaticTxt, 0, wxLEFT);
  h_sizer22->Add(outputFormatCombo, 0, wxLEFT) ;

  wxBoxSizer *h_sizer23 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer23->Add(progressStaticTxt, 0, wxLEFT);
  h_sizer23->Add(m_progressGauge, 0, wxLEFT);

  wxBoxSizer *h_sizer24 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer24->Add(filenameStaticTxt, 0, wxLEFT);
  h_sizer24->Add(filenameValueTxt, 0, wxLEFT) ;

  wxBoxSizer *h_sizer26 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer26->Add(m_printButton, 0, wxLEFT);

  wxBoxSizer *h_sizer28 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer28->Add(updateButton, 0, wxLEFT);

  wxBoxSizer *h_sizer30 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer30->Add(m_okButton, 0, wxLEFT);
  h_sizer30->Add(cancel, 0, wxLEFT);

  wxBoxSizer *v_sizer_renwin = new wxBoxSizer(wxVERTICAL) ;
  v_sizer_renwin->Add(m_Rwi->m_RwiBase, 1, wxEXPAND);

  wxBoxSizer *v_sizer_ctrls =  new wxBoxSizer(wxVERTICAL);
  v_sizer_ctrls->Add(h_sizer10, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer12, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer14, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer16, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer18, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer20, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer22, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer23, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer24, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer26, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer28, 0, wxEXPAND | wxALL,5);
  v_sizer_ctrls->Add(h_sizer30, 0, wxEXPAND | wxALL,5);

  wxBoxSizer *h_sizer_all = new wxBoxSizer(wxHORIZONTAL) ;
  h_sizer_all->Add(v_sizer_renwin, 1, wxEXPAND) ;
  h_sizer_all->Add(v_sizer_ctrls, 0, wxEXPAND) ;

  m_Dialog->Add(h_sizer_all, 1, wxEXPAND);

  // set position of dialog
  h_sizer_all->Fit(m_Dialog) ;
  m_Dialog->SetPosition(wxPoint(20,20)) ;



  //----------------------------------------------------------------------------
  // Set up interactor style and callbacks
  //----------------------------------------------------------------------------

  // Set the interactor style to trackball camera
  vtkInteractorStyleTrackballCamera* style = vtkInteractorStyleTrackballCamera::New() ;
  m_Rwi->m_RwiBase->SetInteractorStyle(style) ;
  style->Delete() ;


  // Add observer to catch progress event
  // Don't forget to add the callback to the invoking class with ->AddObserver(vtkEventId, callback)
  m_progressCallback = lhpTextureOrientationProgressCallback::New() ;
  m_progressCallback->SetListener(this) ;                 // set self as listener to callback
  m_progressCallback->SetMafEventId(ID_PROGRESS_EVENT) ;  // set maf event id to be thrown by callback
}



//----------------------------------------------------------------------------
void lhpOpTextureOrientation::DeleteOpDialog()
//----------------------------------------------------------------------------
{

  cppDEL(m_Rwi); 
  cppDEL(m_Dialog);
}


//----------------------------------------------------------------------------
// Update the camera
// This wraps the dialog-based method in case there is no dialog.
void lhpOpTextureOrientation::UpdateCamera()
//----------------------------------------------------------------------------
{
  if (m_Rwi != NULL)
    m_Rwi->CameraUpdate() ;
}



//----------------------------------------------------------------------------
// Event Handler
void lhpOpTextureOrientation::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {	
    case ID_POS_SLIDER:
      // Change the position of the slice
      UpdateSlicePosition() ;
      UpdateCamera() ;
      break ;

    case ID_CHANGE_VIEW:
      {
        // change the axes of the slice
        double bounds[6] ;
        mafVME* vme = mafVME::SafeDownCast(m_Input) ;
        vme->GetOutput()->GetBounds(bounds) ;
        UpdateViewAxis(bounds) ;
        UpdateCamera() ;
        break ;
      }

    case ID_TEXWINSIZE_VALUE_TXT:
      // change the size of the texture sampling window
      m_vectorGlyphPipe->SetProbeSizeSceneUnits(m_ProbeSizeSceneUnits) ;
      m_tensorGlyphPipe->SetProbeSizeSceneUnits(m_ProbeSizeSceneUnits) ;

      // inputs have changed so unset updated flags and disable print and ok buttons
      m_vectorPipeUpdated = false ;
      m_tensorPipeUpdated = false ;
      m_printButton->Enable(false) ;
      m_okButton->Enable(false) ;

      break ;

    case ID_TEXWINSTEP_VALUE_TXT:
      // change the sampling interval
      m_vectorGlyphPipe->SetSampleSpacingSceneUnits(m_SampleSpacingSceneUnits) ;
      m_tensorGlyphPipe->SetSampleSpacingSceneUnits(m_SampleSpacingSceneUnits) ;

      // inputs have changed so unset updated flags and disable print and ok buttons
      m_vectorPipeUpdated = false ;
      m_tensorPipeUpdated = false ;
      m_printButton->Enable(false) ;
      m_okButton->Enable(false) ;

      break ;

    case ID_OUTPUT_FORMAT_COMBO:
      // output format changed

      // check whether the ok button should be enabled or disabled for the current pipe
      if (m_outputFormat == VectorFormat){
        m_okButton->Enable(m_vectorPipeUpdated) ;
      }
      else if (m_outputFormat == TensorFormat){
        m_okButton->Enable(m_tensorPipeUpdated) ;
      }

      // enable print button if ok is enabled and a filename is present, else disable  
      m_printButton->Enable(m_okButton->IsEnabled() && !m_outputFilename.IsEmpty()) ; 

      break ;

    case ID_FILENAME_TXT:
      // output filename changed
      // enable print button if ok is enabled and a filename is present, else disable  
      m_printButton->Enable(m_okButton->IsEnabled() && !m_outputFilename.IsEmpty()) ;
      break ;

    case ID_PRINT_BUTTON:
      OnUpdate() ;                // first make sure the pipeline is updated
      OnPrint() ;                 // print to file
      break ;

    case ID_UPDATE:   
      // We don't want to update a slow pipeline to update every time a widget is changed
      // So we provide a general update button instead.
      OnUpdate() ;
      break ;

    case ID_PROGRESS_EVENT:
      // Update the progress bar
      {
        double progress = *((double*)e->GetData()) ;
        m_progressGauge->SetValue(progress) ;
        break ;
      }

    case ID_OK:
      // Copy result to output polydata
      if (m_polydata == NULL)
        m_polydata = vtkPolyData::New() ;

      if (m_outputFormat == VectorFormat)
        m_polydata->DeepCopy(m_vectorGlyphPipe->GetPolydata()) ;
      else if (m_outputFormat == TensorFormat)
        m_polydata->DeepCopy(m_tensorGlyphPipe->GetPolydata()) ;

      // Create output VME and close dialog
      CreateOutputVME() ;
      m_Dialog->EndModal(wxID_OK);
      break;

    case ID_CANCEL:
      m_Dialog->EndModal(wxID_CANCEL);
      break;

    default:
      mafEventMacro(*e);
      break; 
    }
  }
}



//----------------------------------------------------------------------------
// On update event
void lhpOpTextureOrientation::OnUpdate()
//----------------------------------------------------------------------------
{
  if (m_outputFormat == VectorFormat){
    m_vectorGlyphPipe->SetVisibility(1) ;
    m_tensorGlyphPipe->SetVisibility(0) ;
    m_vectorPipeUpdated = true ;
  }
  else if (m_outputFormat == TensorFormat){
    m_vectorGlyphPipe->SetVisibility(0) ;
    m_tensorGlyphPipe->SetVisibility(1) ;
    m_tensorPipeUpdated = true ;
  }
  else{
    // unknown format
    assert(false) ;
  }

  UpdateCamera() ;  // update the pipeline

  // enable the ok and print buttons
  m_okButton->Enable(true) ;
  m_printButton->Enable(!m_outputFilename.IsEmpty()) ;
}



//----------------------------------------------------------------------------
// On print event
void lhpOpTextureOrientation::OnPrint()
//----------------------------------------------------------------------------
{
  // print the results to file
  fstream outputStream ;
#if defined(_MSC_VER) && _MSC_VER >= 1600
	outputStream.open((char*)m_outputFilename.GetData(), fstream::out | fstream::app) ;
#else
  outputStream.open((char*)m_outputFilename.GetData(), fstream.out | fstream.app) ;
#endif

  if (m_outputFormat == VectorFormat){
    //m_vectorGlyphPipe->PrintSelf(outputStream, 0) ; // useful for debugging
    m_vectorGlyphPipe->PrintResults(outputStream) ;
  }
  else if (m_outputFormat == TensorFormat){
    //m_tensorGlyphPipe->PrintSelf(outputStream, 0) ; // useful for debugging
    m_tensorGlyphPipe->PrintResults(outputStream) ;
  }
  else{
    // unknown format
    assert(false) ;
  }

  outputStream.close() ;
}




//----------------------------------------------------------------------------
// Create the output vme and reparent
void lhpOpTextureOrientation::CreateOutputVME()
//----------------------------------------------------------------------------
{
  mafNEW(m_polydataVME) ;
  m_polydataVME->SetName("orientation") ;
  m_polydataVME->SetData(m_polydata, 0) ;
  m_Output = m_polydataVME ;
  m_Output->ReparentTo(m_Input) ;
}



//----------------------------------------------------------------------------
// Get renderer
vtkRenderer* lhpOpTextureOrientation::GetRenderer()
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
// Create the visual pipes
// This should not execute the pipes, except those parts necessary for initialization.
void lhpOpTextureOrientation::CreateVisualPipe()
//----------------------------------------------------------------------------
{
  vtkImageData *inputData = (vtkImageData*)(((mafVMEVolumeGray*)m_Input)->GetVTKOutput()->GetVTKData()) ;
  inputData->Update() ;

  double bounds[6] ;
  inputData->GetBounds(bounds) ;


  //--------------------------------------------------------------
  // construct the slice pipe
  //--------------------------------------------------------------
  if (m_slicePipe == NULL){
    double slicepos[3] ;
    slicepos[0] = (bounds[1] + bounds[0]) / 2.0 ;
    slicepos[1] = (bounds[3] + bounds[2]) / 2.0 ;
    slicepos[2] = (bounds[5] + bounds[4]) / 2.0 ;
    m_slicePipe = new lhpTextureOrientationSlicePipe((mafVME*)m_Input, lhpTextureOrientation::ID_XY, slicepos, GetRenderer()) ;
  }


  //--------------------------------------------------------------
  // Construct the glyph pipes
  // The pipes are very slow, so we use SetVisibility(0) to stop them updating before the dialog is ready.
  //--------------------------------------------------------------

  double size[3], minSize ;
  size[0] = (bounds[1] - bounds[0]) ;
  size[1] = (bounds[3] - bounds[2]) ;
  size[2] = (bounds[5] - bounds[4]) ;
  minSize = std::min(size[0], size[1]) ;
  minSize = std::min(minSize, size[2]) ;
  m_ProbeSizeSceneUnits = minSize / 10.0 ;
  m_SampleSpacingSceneUnits = minSize / 2.0 ;

  // vector glyph pipe
  if (m_vectorGlyphPipe == NULL){
    m_vectorGlyphPipe = new lhpTextureOrientationVectorGlyphPipe((mafVME*)m_Input, GetRenderer()) ;
    m_vectorGlyphPipe->SetProbeSizeSceneUnits(m_ProbeSizeSceneUnits) ;
    m_vectorGlyphPipe->SetSampleSpacingSceneUnits(m_SampleSpacingSceneUnits) ;
    m_vectorGlyphPipe->AddProgressObserver(ProgressEventId, m_progressCallback) ;
    m_vectorGlyphPipe->SetVisibility(0) ;
  }

  // tensor glyph pipe
  if (m_tensorGlyphPipe == NULL){
    m_tensorGlyphPipe = new lhpTextureOrientationTensorGlyphPipe((mafVME*)m_Input, GetRenderer()) ;
    m_tensorGlyphPipe->SetProbeSizeSceneUnits(m_ProbeSizeSceneUnits) ;
    m_tensorGlyphPipe->SetSampleSpacingSceneUnits(m_SampleSpacingSceneUnits) ;
    m_tensorGlyphPipe->AddProgressObserver(ProgressEventId, m_progressCallback) ;
    m_tensorGlyphPipe->SetVisibility(0) ;
  }
}




//----------------------------------------------------------------------------
// Initialize the parameters of the slice (origin and view index)
// This only sets the parameters - it does not set or change the visual pipes !
void lhpOpTextureOrientation::InitSliceParams(int viewIndex, double *bounds)
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
void lhpOpTextureOrientation::UpdateSlicePosition()
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

    // Update slice pipe
    m_slicePipe->SetSlicePosition(m_SliceOrigin) ;

    // update the saved values
    m_SliderOrigin_old = m_SliderOrigin ;
  }
}



//----------------------------------------------------------------------------
// Update the view axis
void lhpOpTextureOrientation::UpdateViewAxis(double *bounds)
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

    // Update slice pipeline
    m_slicePipe->SetSliceDirection(m_ViewIndex) ;
    m_slicePipe->SetSlicePosition(m_SliceOrigin) ;

    // update the saved values
    m_ViewIndex_old = m_ViewIndex ;
    m_SliderOrigin_old = m_SliderOrigin ;

  }
}


//----------------------------------------------------------------------------
// Set the slider range to fit the given bounds
void lhpOpTextureOrientation::SetSliderRange(double *bounds)
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


