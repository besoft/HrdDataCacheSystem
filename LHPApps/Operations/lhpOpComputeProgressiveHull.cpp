/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: lhpOpComputeProgressiveHull.cpp,v $ 
  Language: C++ 
  Date: $Date: 2011-09-07 05:42:37 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: Josef Kohout
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpComputeProgressiveHull.h"
#include "lhpOpSetLowerRes.h"
#include "wx/busyinfo.h"

#include "mafGUIDialog.h"
#include "mafRWIBase.h"
#include "mafRWI.h"
#include "mafDeviceButtonsPadMouse.h"
#include "mafEvent.h"

#include "mafGUIButton.h"
#include "mafGUIValidator.h"
#include "mafVMESurface.h"
#include "mafDecl.h"
#include "mafEvent.h"
#include "mafGUI.h"
#include "mafNode.h"
#include "mafVME.h"

#include "vtkRenderWindow.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkPolyData.h"
#include "vtkProgressiveHull.h"
#include "vtkMath.h"

#include "mafMemDbg.h"
#include "mafDbg.h"


//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpComputeProgressiveHull);
//----------------------------------------------------------------------------

#define DEFAULT_DESIRED_REDUCTION		0.95
#define DEFAULT_DESIRED_QUALITY_MIN 	-100
#define DEFAULT_DESIRED_QUALITY_MAX	 	 100
#define DEFAULT_DESIRED_QUALITY			 90
#define VTK_DESIRED_QUALITY(x)		((x) / 100.0f)	//converts quality to vtk filter format

//----------------------------------------------------------------------------
lhpOpComputeProgressiveHull::lhpOpComputeProgressiveHull(const wxString &label) : mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;
  m_Canundo	= true;  
  m_InputPreserving = true;	
  
  for (int i = 0; i < 2; i++)
  {
    m_Meshes[i] = NULL;
    m_MeshesVisibility[i] = 1;    
  }
  
	m_OpSetHull = new lhpOpSetLowerRes();
	m_DesiredReduction = DEFAULT_DESIRED_REDUCTION;	
	m_DesiredQuality = DEFAULT_DESIRED_QUALITY;
}
//----------------------------------------------------------------------------
lhpOpComputeProgressiveHull::~lhpOpComputeProgressiveHull()
//----------------------------------------------------------------------------
{    
	cppDEL(m_OpSetHull);
  mafDEL(m_Output);
}
//----------------------------------------------------------------------------
bool lhpOpComputeProgressiveHull::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return (node && node->IsA("mafVMESurface"));
}
//----------------------------------------------------------------------------
mafOp *lhpOpComputeProgressiveHull::Copy()   
//----------------------------------------------------------------------------
{
  return (new lhpOpComputeProgressiveHull(m_Label));
}

//----------------------------------------------------------------------------
void lhpOpComputeProgressiveHull::OpRun()   
//----------------------------------------------------------------------------
{ 
  //create internal structures for the visualization
  if (!CreateInternalStructures()) // Erkil
  {
    mafEventMacro(mafEvent(this, OP_RUN_CANCEL)); 
    return;
  }
 
  // interface:	
  CreateOpDialog();	

  int result = m_Dialog->ShowModal() == wxID_OK ? OP_RUN_OK : OP_RUN_CANCEL;
  DeleteOpDialog();

  DeleteInternalStructures();
  mafEventMacro(mafEvent(this,result));
}

//----------------------------------------------------------------------------
void lhpOpComputeProgressiveHull::OpDo()
//----------------------------------------------------------------------------
{
  if (m_Output != NULL)
	{
    m_Output->ReparentTo(m_Input); 
		
		m_OpSetHull->SetInput(m_Input);
		m_OpSetHull->SetNewHullVME(mafVME::SafeDownCast(m_Output));
		m_OpSetHull->OpDo();
	}
}

//----------------------------------------------------------------------------
void lhpOpComputeProgressiveHull::OpUndo()
//----------------------------------------------------------------------------
{
  if (m_Output != NULL)
  {
    m_Output->ReparentTo(NULL);
    mafDEL(m_Output);

		m_OpSetHull->OpUndo();
  }
}

//----------------------------------------------------------------------------
void lhpOpComputeProgressiveHull::OpStop(int result)
//----------------------------------------------------------------------------
{
  mafEventMacro(mafEvent(this,result));        
}

//----------------------------------------------------------------------------
//Destroys GUI
void lhpOpComputeProgressiveHull::CreateOpDialog()
//----------------------------------------------------------------------------
{
  wxBusyCursor wait;

  //===== setup interface ====
  m_Dialog = new mafGUIDialog("Progressive Hull Construction", mafCLOSEWINDOW | mafRESIZABLE);  
  m_Dialog->SetWindowStyle(m_Dialog->GetWindowStyle() | wxMAXIMIZE_BOX);

  //rendering window
  m_Rwi = new mafRWI(m_Dialog,ONE_LAYER,false);
  m_Rwi->SetListener(this);
  m_Rwi->CameraSet(CAMERA_PERSPECTIVE);
  m_Rwi->m_RenderWindow->SetDesiredUpdateRate(0.0001f);
  m_Rwi->SetSize(0,0,400,400);
  m_Rwi->Show(true);
  m_Rwi->m_RwiBase->SetMouse(m_Mouse);


  //The following code was originally generated using wxFormBuilder
  //and modified here to work with MAF
#pragma region //wxFormBuilder Component Construction
  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer( wxHORIZONTAL );

  //Left panel - RWI
  wxBoxSizer* bSizer18;
  bSizer18 = new wxBoxSizer( wxVERTICAL );
  bSizer18->Add( m_Rwi->m_RwiBase, 1, wxEXPAND | wxALL, 5 );  
  bSizer1->Add( bSizer18, 1, wxEXPAND, 5 );  

  //Right panel
  wxBoxSizer* bSizer3 = new wxBoxSizer( wxVERTICAL );

  wxStaticBoxSizer* sbSizer2;
  sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( m_Dialog, wxID_ANY, 
    wxT("Computation Options") ), wxVERTICAL );

	wxBoxSizer* bSizer11 = new wxBoxSizer( wxHORIZONTAL );
	bSizer11->Add( new wxStaticText( m_Dialog, wxID_ANY, wxT("Decimation:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT ), 1, wxALL, 5 );
	
	wxTextCtrl* editPoints = new wxTextCtrl( m_Dialog, ID_REQ_POINTS );
	editPoints->SetToolTip( wxT("Specifiy the desired reduction coefficient. Larger values mean coarser hulls.") );		
	bSizer11->Add(editPoints, 1, wxALL, 0 );	
	bSizer3->Add( bSizer11, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer111 = new wxBoxSizer( wxHORIZONTAL );	
	bSizer111->Add( new wxStaticText( m_Dialog, wxID_ANY, wxT("Quality:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT ), 1, wxALL, 5 );
	
	wxTextCtrl* editQuality = new wxTextCtrl( m_Dialog, ID_QUALITY);
	editQuality->SetToolTip( wxT("Specify the desired quality of produced hull. Smaller values tend to lead into smoother meshes "
															 "but they may not preserve round shapes of the original mesh, whilst with larger meshes it is "
															 "usually true that the desired number of polygons is achieved but some the produced mesh may "
															 "contain spikes (especially, if the input mesh is irregular).") );	
	bSizer111->Add( editQuality, 1, wxALL, 0 );	

	wxBoxSizer* bSizer112 = new wxBoxSizer( wxHORIZONTAL );	
	wxCheckBox* chckGPU = new wxCheckBox( m_Dialog, ID_USE_GPU, 
		wxT("Use the GPU version"), wxDefaultPosition, wxDefaultSize, 0 );
	bool bCUDAAvailable = vtkProgressiveHull::IsCudaAvailable(); // check if its possible to use the cuda version - this call sets the vtkProgressiveHull::useCuda variable
	chckGPU->SetValue(bCUDAAvailable);
	if (!bCUDAAvailable)	// if cuda is not available, the checkbox will be disabled
		chckGPU->Disable();
	chckGPU->SetToolTip( wxT("There are two versions of the algorithm available. One uses graphics card for the computation (requires an nVidia GPU GeForce 4xx or newer) ") );	

	bSizer112->Add( chckGPU, 1, wxALL, 5 );

	sbSizer2->Add( bSizer111, 1, wxEXPAND, 5 );
	sbSizer2->Add( bSizer112, 1, wxEXPAND, 5 );
  bSizer3->Add( sbSizer2, 0, wxEXPAND|wxALIGN_RIGHT, 5 );

	//Left panel - Buttons   
  wxButton* bttnPreview = new wxButton( m_Dialog, ID_PREVIEW, wxT("Preview"), wxDefaultPosition, wxDefaultSize, 0 );
  bttnPreview->SetToolTip( wxT("Compute the progressive hull mesh according to the current configuration") );  
  bSizer3->Add( bttnPreview, 0, wxALL, 5 );

	m_StatusHelp = new wxTextCtrl( m_Dialog, ID_STATUS_HELP, wxEmptyString, 
    wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_WORDWRAP );
  m_StatusHelp->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVEBORDER ) );  
  bSizer3->Add( m_StatusHelp, 0, wxALL|wxEXPAND, 5 );  	

  //Right panel - Visual Options
  wxStaticBoxSizer* sbSizer4 = new wxStaticBoxSizer( 
		new wxStaticBox( m_Dialog, wxID_ANY, wxT("Visual Options") ), wxVERTICAL );

  wxBoxSizer* bSizer141;
  bSizer141 = new wxBoxSizer( wxHORIZONTAL );

  wxCheckBox* chckShowOM = new wxCheckBox( m_Dialog, ID_SHOW_ORIGINAL, 
    wxT("Show Original Mesh"), wxDefaultPosition, wxDefaultSize, 0 );
  chckShowOM->SetValue(true);

  bSizer141->Add( chckShowOM, 0, wxALL, 5 );

  wxCheckBox* chckShowDM = new wxCheckBox( m_Dialog, ID_SHOW_HULL, 
    wxT("Show Hull Mesh"), wxDefaultPosition, wxDefaultSize, 0 );
  chckShowDM->SetValue(true);

  bSizer141->Add( chckShowDM, 0, wxALL, 5 );
  sbSizer4->Add( bSizer141, 1, wxEXPAND, 5 );
  bSizer3->Add( sbSizer4, 0, wxEXPAND, 5 );

  //Right panel - Buttons         
  wxBoxSizer* bSizer16;
  bSizer16 = new wxBoxSizer( wxHORIZONTAL );
  
  wxButton* bttnOK = new wxButton( m_Dialog, ID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
  bttnOK->SetToolTip( wxT("Computes the hull mesh according to the current configuration  "
    "and creates new VMEs for the constructed hull.") );    
  bSizer16->Add( bttnOK, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

  wxButton* bttnCancel = new wxButton( m_Dialog, ID_CANCEL, wxT("Cancel"), 
    wxDefaultPosition, wxDefaultSize, 0 );
  bSizer16->Add( bttnCancel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
  bSizer3->Add( bSizer16, 0, wxEXPAND|wxALIGN_RIGHT, 5 );  
  bSizer1->Add( bSizer3, 0, wxEXPAND, 5 );
#pragma endregion //wxFormBuilder

  //validators 
  bttnPreview->SetValidator(mafGUIValidator(this, ID_PREVIEW, bttnPreview));  
  chckGPU->SetValidator(mafGUIValidator(this, ID_USE_GPU, chckGPU, &(vtkProgressiveHull::useCuda)));  
    
  //validators for visual options
  chckShowOM->SetValidator(mafGUIValidator(this, ID_SHOW_ORIGINAL, chckShowOM, &m_MeshesVisibility[0]));
  chckShowDM->SetValidator(mafGUIValidator(this, ID_SHOW_HULL, chckShowDM, &m_MeshesVisibility[1]));  

	editPoints->SetValidator(mafGUIValidator(this, ID_REQ_POINTS, editPoints, 
		&m_DesiredReduction, 0.0, 1.0)); 
	editQuality->SetValidator(mafGUIValidator(this, ID_QUALITY, editQuality, &m_DesiredQuality, 
		DEFAULT_DESIRED_QUALITY_MIN, DEFAULT_DESIRED_QUALITY_MAX));  
  
  bttnOK->SetValidator(mafGUIValidator(this, ID_OK, bttnOK));
  bttnCancel->SetValidator(mafGUIValidator(this, ID_CANCEL, bttnCancel));

  m_Dialog->Add(bSizer1, 1, wxEXPAND);

  int x_pos,y_pos,w,h;
  mafGetFrame()->GetPosition(&x_pos,&y_pos);
  m_Dialog->GetSize(&w,&h);
  m_Dialog->SetSize(x_pos+5,y_pos+5,w,h);
  

  //and initialize the renderer window
  double bounds[6];	
  m_Meshes[0]->pPoly->GetBounds(bounds);

  m_Rwi->m_RenFront->AddActor(m_Meshes[0]->pActor);	
  m_Rwi->m_RenFront->ResetCamera(bounds);

	UpdateStatusInfo();
  UpdateVisibility();  
}

//----------------------------------------------------------------------------
//Creates GUI including renderer window
void lhpOpComputeProgressiveHull::DeleteOpDialog()
//----------------------------------------------------------------------------
{  
  //remove all actors
  RemoveAllActors();    

  cppDEL(m_Rwi); 
  cppDEL(m_Dialog);
}

//----------------------------------------------------------------------------
void lhpOpComputeProgressiveHull::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
		case ID_SHOW_HULL:
		case ID_SHOW_ORIGINAL:
      //visibility options has changed
      UpdateVisibility();
      break;

    case ID_PREVIEW:
      OnPreview();
      break;
      
    case ID_OK:
      OnOk();
      m_Dialog->EndModal(wxID_OK);
      break;

    case ID_CANCEL:
      m_Dialog->EndModal(wxID_CANCEL);
      break;
    }
  }
}

#pragma region GUI Handlers
//------------------------------------------------------------------------
//Performs the deformation using the current settings.
/*virtual*/ void lhpOpComputeProgressiveHull::OnPreview()
//------------------------------------------------------------------------
{  
  ComputeHullMesh();
  
	UpdateStatusInfo();
  this->m_Rwi->CameraUpdate(); 
}

//------------------------------------------------------------------------
//Performs the deformation and creates outputs
/*virtual*/ void lhpOpComputeProgressiveHull::OnOk()
//------------------------------------------------------------------------
{
  ComputeHullMesh();

  //create VME  
  mafVMESurface* surface;

  mafNEW(surface);
  surface->SetName(wxString::Format(wxT("%s_HULL"), m_Input->GetName()));
  surface->SetData(m_Meshes[1]->pPoly, 0, mafVMEGeneric::MAF_VME_REFERENCE_DATA);

  mafDEL(m_Output);
  m_Output = surface; 
}

#pragma endregion //GUI Handlers

//------------------------------------------------------------------------
//Computes the hull mesh for the input mesh
void lhpOpComputeProgressiveHull::ComputeHullMesh()
//------------------------------------------------------------------------
{
	vtkProgressiveHull* hull = vtkProgressiveHull::New();
	hull->SetTargetReduction(m_DesiredReduction);
	hull->SetMeshRoughness(VTK_DESIRED_QUALITY(m_DesiredQuality));	
	//hull->SetPriorityTreshold(m_PriorityTreshold);

	hull->SetInput(m_Meshes[0]->pPoly);
	hull->SetOutput(m_Meshes[1]->pPoly);
	hull->Update();

	hull->SetOutput(NULL);	//detach output from the filter
	hull->Delete();
}

//------------------------------------------------------------------------
//Creates internal data structures used in the editor.
//Returns false, if an error occurs (e.g. unsupported input)
/*virtual*/ bool lhpOpComputeProgressiveHull::CreateInternalStructures()
//------------------------------------------------------------------------
{
  mafVMESurface* surface = mafVMESurface::SafeDownCast(m_Input); 
  _VERIFY_RETVAL(surface != NULL, false); 

  vtkPolyData* pPoly = vtkPolyData::SafeDownCast(surface->GetOutput()->GetVTKData());
  _VERIFY_CMD_RPT(pPoly != NULL, false);
  pPoly->Register(NULL);  //this prevents its destruction
    
  m_Meshes[0] = CreateMesh(pPoly);                //original mesh
  m_Meshes[1] = CreateMesh(vtkPolyData::New());   //deformed mesh, it is empty now
  m_Meshes[1]->pActor->GetProperty()->SetRepresentationToWireframe();
  m_Meshes[1]->pActor->GetProperty()->SetColor(1, 0, 0); //red 
  return true;
}

//------------------------------------------------------------------------
//Destroys internal data structures created by CreateInternalStructures
/*virtual*/ void lhpOpComputeProgressiveHull::DeleteInternalStructures()
//------------------------------------------------------------------------
{
  //and destroy meshes
  for (int i = 0; i < 2; i++)
  {
    vtkDEL(m_Meshes[i]->pActor);
    vtkDEL(m_Meshes[i]->pMapper);
    vtkDEL(m_Meshes[i]->pPoly);
    cppDEL(m_Meshes[i]);
  }
}


//------------------------------------------------------------------------
//This method creates the internal mesh structure.
//It constructs also the VTK pipeline.
lhpOpComputeProgressiveHull::MESH* lhpOpComputeProgressiveHull::CreateMesh(vtkPolyData* pMesh)
//------------------------------------------------------------------------
{
  MESH* pRet = new MESH;
  memset(pRet, 0, sizeof(MESH));

  pRet->pPoly = pMesh;
  pRet->pMapper = vtkPolyDataMapper::New();
  pRet->pActor = vtkActor::New();
  pRet->pActor->SetMapper(pRet->pMapper);
  pRet->pActor->PickableOff();

  UpdateMesh(pRet);
  return pRet;
}

//------------------------------------------------------------------------
//Updates the VTK pipeline for the given curve.
//N.B. It does not connects actors into the renderer.
void lhpOpComputeProgressiveHull::UpdateMesh(MESH* pMesh)
//------------------------------------------------------------------------
{
  if (pMesh->pPoly == NULL)
    pMesh->pActor->SetVisibility(0);
  else
  {
    pMesh->pMapper->SetInput(pMesh->pPoly);
    pMesh->pMapper->Update();

    pMesh->pActor->SetVisibility(1);
  }  
}

//------------------------------------------------------------------------
//Updates the visibility of meshes 
//It adds/removes actors from the renderer according to the
//status of their associated data and the visual options
//specified by the user in the GUI.
//This method is typically calls after UpdateMesh or 
//UpdateControlCurve is finished
void lhpOpComputeProgressiveHull::UpdateVisibility()
//------------------------------------------------------------------------
{
  //remove all actors and 
  RemoveAllActors();

  //readd actors according to the visual options
  for (int i = 0; i < 2; i++)
  {
    if (m_MeshesVisibility[i] != 0)
      m_Rwi->m_RenFront->AddActor(m_Meshes[i]->pActor);
  }
    
  this->m_Rwi->CameraUpdate();  
}


//------------------------------------------------------------------------
//Removes all actors from the renderer.
void lhpOpComputeProgressiveHull::RemoveAllActors()
//------------------------------------------------------------------------
{  
  m_Rwi->m_RenFront->RemoveActor(m_Meshes[0]->pActor);
  m_Rwi->m_RenFront->RemoveActor(m_Meshes[1]->pActor);
}

//------------------------------------------------------------------------
//Updates status info 
void lhpOpComputeProgressiveHull::UpdateStatusInfo()
//------------------------------------------------------------------------
{
	wxString szStr = wxString::Format("Original Mesh: %d vertices, %d polygons\r\n"
		"Hull Mesh: %d vertices, %d polygons\n",
		m_Meshes[0]->pPoly->GetNumberOfPoints(),
		m_Meshes[0]->pPoly->GetNumberOfCells(),
		m_Meshes[1]->pPoly->GetNumberOfPoints(),
		m_Meshes[1]->pPoly->GetNumberOfCells());

	m_StatusHelp->SetLabel(szStr);
}