/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpScaleMMA.cpp,v $
Language:  C++
Date:      $Date: 2011-10-21 16:16:06 $
Version:   $Revision: 1.1.2.2 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2001/2005
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "lhpDefines.h"
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
#include "lhpOpExtractlandmarkEOS.h"

#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkInteractorStyleTrackballCamera.h"

#include <vtkTextProperty.h>
#include <vtkProperty2D.h>
#include <vtkTextMapper.h>
#include <vtkTextActor.h>
#include <vtkTransform.h>

#include "mafNode.h"
#include "mafAction.h"
#include "mafDeviceManager.h"
#include "mafInteractor.h"
#include "mafInteractionFactory.h"
#include "mafInteractorPicker.h"
#include "mafInteractorSER.h"
#include "mafInteractorPER.h"
#include "mafInteractorGenericMouse.h"
#include "mafGUITransformMouse.h"
#include "mafVME.h"
#include "mafGUI.h"
#include "mafGUIDialog.h"
#include "mafRWI.h"
#include "mafGUIButton.h"
#include "mafViewImage.h"
#include "mafVMEImage.h"

#include "medDeviceButtonsPadMouseDialog.h"
//#include "mafDeviceButtonsPadMouse.h"
#include "lhpInteractorExtractLandmarkEOS.h"
#include "mafInteractorCompositorMouse.h"
#include "mafInteractorGenericMouse.h"
#include "mafGUICheckListBox.h"

#include "medViewVTKCompound.h"
#include "mafViewVTK.h"

#include "mafVMELandmarkCloud.h"
#include "mafVMELandmark.h"
#include "vtkDataSet.h"

#include "mafRefSys.h"

#include "vtkImageData.h"

mafCxxTypeMacro(lhpOpExtractLandmarkEOS);

const wxString lhpOpExtractLandmarkEOS::m_ViewNames[lhpOpExtractLandmarkEOS::VIEW_NUM] = {_("Coronal"), _("Sagittal") };

lhpOpExtractLandmarkEOS::lhpOpExtractLandmarkEOS(wxString label):mafOp(label)
{
	m_Gui = NULL;
	m_Rwi = NULL;
	m_PER = NULL;
	m_Picker = NULL;
	//m_GuiTransformMouse  = NULL;
	m_InputVME = NULL;
	//m_Cloud = NULL;
	m_CurrentVME = NULL;

	//m_oldFrontalMatrix = NULL;
	m_pLdCheckList = NULL;

	for (int i = 0; i < VIEW_NUM; i++)
	{
		m_View[i] = NULL;
		// initialise the images to NULL for there are default pointers from mafSmartPointer
		m_Images[i] = NULL;
		m_ImageNames[i] = mafString("none");
		m_pFlipImageCheckList[i] = NULL;
		//m_TextMapper[i] = NULL;
		//m_TextActor[i] = NULL;
	}
}

lhpOpExtractLandmarkEOS::~lhpOpExtractLandmarkEOS()
{
	//cppDEL(m_GuiTransformMouse);
	//mafDEL(m_oldFrontalMatrix);

	for (int i = 0; i < LANDMARK_NUM; i++)
	{
		//mafDEL(m_Landmark[i]);
		//mafDEL(m_IsaCompositor);
	}
}

bool lhpOpExtractLandmarkEOS::Accept(mafNode* vme)
{
	return (CountImagesInTree(vme) >=2 );
}

bool lhpOpExtractLandmarkEOS::AcceptStatic(mafNode* vme)
{
	return (CountImagesInTree(vme) >=2 );
}

bool lhpOpExtractLandmarkEOS::AcceptImage(mafNode * node)
{
	return (node != NULL && (node->IsMAFType(mafVMEImage) ));
}

int lhpOpExtractLandmarkEOS::CountImagesInTree(mafNode * nodeStart)
{
	if (!nodeStart)
		return 0;

	int nImages = 0;

	if (AcceptImage(nodeStart))
		nImages += 1;

	for (mafID i=0;i<nodeStart->GetNumberOfChildren();i++)
	{
		mafNode * node = nodeStart->GetChild(i);
		nImages += CountImagesInTree(node);
	}

	return nImages;
}

mafOp * lhpOpExtractLandmarkEOS::Copy()
{
	return new lhpOpExtractLandmarkEOS();
}

void lhpOpExtractLandmarkEOS::CreateGUI()
{
	m_Gui = new mafGUI(this);

	m_Gui->SetListener(this);
}

void lhpOpExtractLandmarkEOS::CreateOpDialog()
{
	m_OpDialog = new mafGUIDialog("EOS Landmark Selection", mafCLOSEWINDOW | mafRESIZABLE);
	m_OpDialog->SetWindowStyle(m_OpDialog->GetWindowStyle() | wxMAXIMIZE_BOX);

	m_Gui = new mafGUI(this);
	m_Gui->Reparent(m_OpDialog);
	m_Gui->SetMinSize(wxSize(200,650));

	m_Gui->Label("Operation Panel");

	m_Gui->Label(_("Select coronal EOS image:"),true);
	m_Gui->Label(&m_ImageNames[CORONAL]);
	m_Gui->Button(ID_CHOOSE_CORONAL,_("Select coronal EOS image"));

	m_Gui->Label(_("Select sagittal EOS image:"),true);
	m_Gui->Label(&m_ImageNames[SAGGITAL]);
	m_Gui->Button(ID_CHOOSE_SAGGITAL,_("Select sagittal EOS image"));

	//for (int i = 0; i < LANDMARK_NUM * 2; i++)
	//	m_LdIds.push_back(ID_LD_START + i);

	 //m_LdLabelsColumns.push_back("Left");
	 //m_LdLabelsColumns.push_back("Rightt");

	m_Gui->Label(_("Show/hide landmarks:"),true);
	//m_Gui->BoolGrid3,2, m_LdIds, m_LdLabelsRows,m_LdLabelsColumns, m_LdVar,);
	m_pLdCheckList = m_Gui->CheckList(ID_LD_CHECKLIST,"Show/hide landmarks", 100);
	m_pLdCheckList->AddItem(0, "LeftFemur", true);
	m_pLdCheckList->AddItem(1, "LeftKnee", true);
	m_pLdCheckList->AddItem(2, "LeftAnkle", true);
	m_pLdCheckList->AddItem(3, "RightFemur", true);
	m_pLdCheckList->AddItem(4, "RightKnee", true);
	m_pLdCheckList->AddItem(5, "RightAnkle", true);
	m_pLdCheckList->Enable(false);

	m_Gui->Label(_("Coronal Image Flip:"),true);
	m_pFlipImageCheckList[0] = m_Gui->CheckList(ID_CORONAL_CHECKLIST,"Coronal flip", 30);
	m_pFlipImageCheckList[0]->AddItem(0, "Vertical flip", false);
	m_pFlipImageCheckList[0]->AddItem(1, "Horizontal flip", false);
	m_pFlipImageCheckList[0]->Enable(false);

	m_Gui->Label(_("Sagittal Image Flip:"),true);
	m_pFlipImageCheckList[1] = m_Gui->CheckList(ID_SAGGITAL_CHECKLIST,"Saggital flip", 30);
	m_pFlipImageCheckList[1]->AddItem(0, "Vertical flip", false);
	m_pFlipImageCheckList[1]->AddItem(1, "Horizontal flip", false);
	m_pFlipImageCheckList[1]->Enable(false);

	m_Gui->Divider(2);
	m_Gui->OkCancel();
	m_Gui->Enable(wxOK, false);

	//m_GuiTransformMouse = new mafGUITransformMouse(m_Cloud, this);
	//m_GuiTransformMouse->EnableWidgets(true);

	// add transform gui to operation
	//m_Gui->AddGui(m_GuiTransformMouse->GetGui());
	//m_Gui->Update();

		  //Change default frame to our dialog
	  wxWindow* oldFrame = mafGetFrame();
	  mafSetFrame(m_OpDialog);

	  //Create rendering view
	//m_View[0] = new mafViewImage(m_ViewNames[0] + _(" Slice"),CAMERA_RIGHT,true,false,false,0);
	//m_View[1] = new mafViewImage(m_ViewNames[1] + _(" Slice"),CAMERA_TOP,true,false,false,0);
	  m_View[0] = new mafViewVTK(m_ViewNames[0] + _(" Slice"), CAMERA_RXFEM_YPOS);
	  m_View[1] = new mafViewVTK(m_ViewNames[1] + _(" Slice"), CAMERA_RX_LEFT);
	  for (int i = 0 ; i < VIEW_NUM; i++)
	  {
 			m_View[i] ->Create();
			//m_View[i] ->GetGui(); // create camera, for mafViewImage only

			 // vtkNEW(m_TextMapper[i]);
			//m_TextMapper[i] = vtkTextMapper::New();
			  m_TextMapper[i]->SetInput(m_ViewNames[i]);
			  m_TextMapper[i]->GetTextProperty()->SetColor(1.0,0.0,0.0);

			  m_TextMapper[i]->GetTextProperty()->AntiAliasingOff();

			  //vtkNEW(m_TextActor[i]);
			  //m_TextActor[i] = vtkTextActor::New();
			  m_TextActor[i]->SetMapper(m_TextMapper[i].GetPointer());
			  m_TextActor[i]->SetPosition(3,5);
			 m_TextActor[i]->GetProperty()->SetColor(1.0,0.0,0.0);

			 m_View[i]->GetFrontRenderer()->AddActor(m_TextActor[i].GetPointer());
			 m_View[i]->m_Rwi->SetSize(0,0,300,600);
			m_View[i]->m_Rwi->Show(true);
	  }

	  mafSetFrame(oldFrame);

		// Box sizer for render window column and widgets column
    wxBoxSizer *totalHBoxSizer = new wxBoxSizer(wxHORIZONTAL) ;
	wxBoxSizer *viewHBoxSizer = new wxBoxSizer(wxHORIZONTAL) ;
	viewHBoxSizer->Add(m_View[0]->m_Rwi->m_RwiBase, 1, wxEXPAND | wxALL, 5) ;
	viewHBoxSizer->Add(m_View[1]->m_Rwi->m_RwiBase, 1, wxEXPAND | wxALL, 5) ;

	 wxBoxSizer *renWinVBoxSizer = new wxBoxSizer(wxVERTICAL) ;
	renWinVBoxSizer->Add(viewHBoxSizer, 1, wxEXPAND | wxALL, 5) ;

	// add render window vbox to parent sizer
		totalHBoxSizer->Add(renWinVBoxSizer, 0, wxEXPAND | wxALL, 5) ;

		totalHBoxSizer->Add(m_Gui,0,wxRIGHT);

	 for (int i = 0 ; i < VIEW_NUM; i++)
	 {
		m_View[i]->VmeShow(m_Input, true);

		m_View[i]->VmeShow(m_Cloud, true);
		for (int j = 0; j < LANDMARK_NUM; j++)
		{
			m_View[i]->VmeShow(m_Landmark[j], true);
		}
		//m_View[0]->VmeShow(m_Landmark, true);
	 }

	      // add total box sizer to dialog
    m_OpDialog->Add(totalHBoxSizer, 1, wxEXPAND);
}

mafView * lhpOpExtractLandmarkEOS::GetView(vtkRenderWindowInteractor * rwi)
{
	for (int i = 0 ; i < VIEW_NUM; i++)
	 {
		 if (m_View[i]->GetRWI() == rwi)
			 return m_View[i];
	 }

	return NULL;
}

void lhpOpExtractLandmarkEOS::DeleteOpDialog()
{
	m_OpDialog->Show(false);

	for (int view = 0; view < VIEW_NUM; view++)
	{
		for (int j = 0; j < LANDMARK_NUM; j++)
		{
			m_View[view]->VmeRemove(m_Landmark[j]);
		}

		if (m_Images[view])
		{
			m_View[view]->VmeShow(m_Images[view], false);
			m_View[view]->VmeRemove(m_Images[view]);
		}
	}

	cppDEL(m_View[0]);
	cppDEL(m_View[1]);
	cppDEL(m_Gui);
	cppDEL(m_Rwi);
	cppDEL(m_OpDialog);
}

void lhpOpExtractLandmarkEOS::InitializeInteractors()
{
  //Create the device manager
  mafNEW(m_DeviceManager);
  m_DeviceManager->SetListener(this);
  m_DeviceManager->SetName("DialogDeviceManager");
  m_DeviceManager->Initialize();

  //Create a Mouse device
  //mafPlugDevice<medDeviceButtonsPadMouseDialog>("Mouse");
  mafPlugDevice<medDeviceButtonsPadMouseDialog>("Mouse");

  m_DialogMouse = (medDeviceButtonsPadMouseDialog *)m_DeviceManager->AddDevice("medDeviceButtonsPadMouseDialog",false); // add as persistent device
  assert(m_DialogMouse);
  m_DialogMouse->SetName("DialogMouse");

    //Create the static event router and connect it
  mafNEW(m_SER);
  m_SER->SetName("StaticEventRouter");
  m_SER->SetListener(this);

 mafNEW(m_PER);
 m_PER->SetOperation(this);
  m_PER->SetName("Landmark PER");
  m_PER->SetListener(this);
  m_PER->SetRenderer(m_View[0]->GetFrontRenderer());
  //m_PER->AddObserver(m_GuiTransformMouse);
 /*
  mafNEW(m_Picker);
  m_Picker->SetRenderer(m_View[0]->GetFrontRenderer());
  m_Picker->SetListener(this);
  m_Picker->AddObserver(this);

  mafVME * vme = mafVME::SafeDownCast(m_Input);
  vme->SetBehavior(m_Picker);
  */

  //Define the action for pointing and manipulating
	mafAction *pntAction = m_SER->AddAction("pntAction",-10);
	pntAction->BindDevice(m_DialogMouse);
	pntAction->BindInteractor(m_PER);

  	m_View[0]->SetMouse(m_DialogMouse);
	m_View[1]->SetMouse(m_DialogMouse);
	m_View[0]->GetRWI()->SetMouse(m_DialogMouse);
	m_View[1]->GetRWI()->SetMouse(m_DialogMouse);

	//todo
	//m_DialogMouse->SetView(m_View[1]);

	//mafEventMacro(mafEvent(this,VME_SELECTED,vme));
}

void lhpOpExtractLandmarkEOS::StopInteractors()
{
	m_DeviceManager->Shutdown();

	mafDEL(m_DeviceManager);
	mafDEL(m_PER);
	mafDEL(m_SER);
	mafDEL(m_Picker);
}

void lhpOpExtractLandmarkEOS::OnEvent(mafEventBase *event)
{
	if (mafEvent *e = mafEvent::SafeDownCast(event))
	{
		int id = e->GetId();

		switch(id)
		{
		case 		ID_CHOOSE_CORONAL:
			OnChooseEosImage(CORONAL);
			break;
		case ID_CHOOSE_SAGGITAL:
			OnChooseEosImage(SAGGITAL);
			break;
		case ID_TRANSFORM: // from mafGUITransformMouse
				PostMultiplyEventMatrix(event);
				break;
		case ID_LD_CHECKLIST:
			ShowLandmark(e->GetArg(),e->GetBool());
			break;
		case ID_CORONAL_CHECKLIST:
			FlipImage(0, e->GetArg() ,e->GetBool());
			break;
		case ID_SAGGITAL_CHECKLIST:
			FlipImage(1, e->GetArg(),e->GetBool());
			break;
		case wxOK:
			m_OpDialog->EndModal(wxID_OK);

			//OpDo();
			//OpStop(OP_RUN_OK);
			break;
		case wxCANCEL:
			m_OpDialog->EndModal(wxID_CANCEL);
			//OpStop(OP_RUN_CANCEL);
			break;
		default:
			mafEventMacro(*e);
			break;
		}
	}
}

void lhpOpExtractLandmarkEOS::ShowLandmark(int idx, bool show)
{
		//m_View[0]->VmeAdd(m_Landmark[j]);
		m_View[0]->VmeShow(m_Landmark[idx], show);
		//m_View[1]->VmeAdd(m_Landmark[j]);
		m_View[1]->VmeShow(m_Landmark[idx], show);
		UpdateViews();
}

void lhpOpExtractLandmarkEOS::FlipImage(int imageIdx, int flipdir, bool bFlip)
{
	//mafSmartPointer<mafVMEImage> pImage = m_Images[imageIdx];;

	vtkMAFSmartPointer<vtkTransform> trV;
	vtkMAFSmartPointer<vtkTransform> trH;
	trV->PostMultiply();
	trH->PostMultiply();

	vtkImageData * pImage= vtkImageData::SafeDownCast(m_Images[imageIdx]->GetOutput()->GetVTKData());
	double bound[6];
	double spacing[3];
	pImage->GetBounds(bound);
	pImage->GetSpacing(spacing);
	//vtkTransform * trans = vtkTransform::New();
	if (imageIdx == 0)
	{
		trV->RotateX(180.0);
		trV->Translate(0.0, 0.0, bound[3] + bound[2]);
		trH->RotateZ(180.0);
		trH->Translate( bound[1]+ bound[0], 0.0, 0.0);
	}
	else
	{
		trV->RotateY(180.0);
		trV->Translate(0.0, 0.0, bound[3]+ bound[2]);
		trH->RotateZ(180.0);
		trH->Translate( 0.0, bound[1] + bound[0], 0.0);
	}

	trV->Update();
	trH->Update();

	mafMatrix matV;
	matV.DeepCopy(trV->GetMatrix());
	mafMatrix matH;
	matH.DeepCopy(trH->GetMatrix());
	//tr->Delete();

	// ?? time stamp
	mafTimeStamp m_CurrentTime =m_Images[imageIdx]->GetTimeStamp();
	matV.SetTimeStamp(m_CurrentTime);
	matH.SetTimeStamp(m_CurrentTime);
	//m_Images[imageIdx]->GetAbsMatrix();
	m_Images[imageIdx]->SetAbsMatrix(* m_matImage[imageIdx]);
	if (m_pFlipImageCheckList[imageIdx]->IsItemChecked(0))
	{
		m_Images[imageIdx]->ApplyMatrix(matV,m_CurrentTime);
		//m_ImageVme[imageIdx]->ApplyMatrix(matV,0);
	}
	if (m_pFlipImageCheckList[imageIdx]->IsItemChecked(1))
	{
		m_Images[imageIdx]->ApplyMatrix(matH,m_CurrentTime);
	}

	 m_View[imageIdx]->CameraUpdate();
}

void lhpOpExtractLandmarkEOS::OnChooseEosImage(int view)
{
	if (view > VIEW_NUM ||	view < 0)
		return;

	mafEvent e(this,VME_CHOOSE);
	mafEventMacro(e);
	mafNode *vme = e.GetVme();

	if(!vme) return; // the user choosed cancel - keep previous target

	if(!AcceptImage(vme)) // the user choosed ok     - check if it is a valid vme
	{
		wxString msg = _("Must be an EOS image\n please choose another \n");
		wxMessageBox(msg,_("incorrect vme type"),wxOK|wxICON_ERROR);
		//m_Images[view] = NULL;
		//m_ImageNames[view] = _("none");
		m_Gui->Enable(wxOK,false);
		m_Gui->Update();
		return;
	}

	((mafVME*)vme)->GetOutput()->Update();

	// TODO: multiple loading
	m_Images[view] = mafVMEImage::SafeDownCast(vme);
	m_ImageVme[view] = (mafVME*)vme;
	m_ImageNames[view] = m_Images[view]->GetName();
	m_matImage[view] = m_Images[view]->GetOutput()->GetAbsMatrix();

	AddVme(m_View[view], m_Images[view]);

	m_View[view]->VmeShow(m_Images[view], true);

	if (m_Images[CORONAL] && m_Images[SAGGITAL])
	{
		// should be done only once
		//if (NULL == m_oldFrontalMatrix)
		//	mafNEW(m_oldFrontalMatrix);

		/*
		vtkTransform *tr = vtkTransform::New();
		tr->PostMultiply();

		m_oldFrontalMatrix->DeepCopy(m_Images[CORONAL]->GetOutput()->GetAbsMatrix()->GetVTKMatrix());

		tr->SetMatrix(m_oldFrontalMatrix->GetVTKMatrix());

		vtkImageData * pImage= vtkImageData::SafeDownCast(m_Images[SAGGITAL]->GetOutput()->GetVTKData());
		double bound[6];
		double spacing[3];
		pImage->GetBounds(bound);
		pImage->GetSpacing(spacing);
		//vtkTransform * trans = vtkTransform::New();
		tr->Translate(0, bound[1], 0);
		//tr->Concatenate(e->GetMatrix()->GetVTKMatrix());
		tr->Update();

		mafMatrix absPose;
		absPose.DeepCopy(tr->GetMatrix());
		mafTimeStamp m_CurrentTime =m_Images[CORONAL]->GetTimeStamp();
		absPose.SetTimeStamp(m_CurrentTime);
		m_Images[CORONAL]->SetAbsMatrix(absPose);
		tr->Delete();
		*/

		AddLandmarks();

		m_View[0]->VmeAdd(m_Cloud);
		m_View[1]->VmeAdd(m_Cloud);
		for (int j = 0; j < LANDMARK_NUM; j++)
		{
			m_View[0]->VmeAdd(m_Landmark[j]);
			//m_View[0]->VmeShow(m_Landmark[j], true);

			m_View[1]->VmeAdd(m_Landmark[j]);
			//m_View[1]->VmeShow(m_Landmark[j], true);
		}

		for (int j = 0; j < LANDMARK_NUM; j++)
		{
			ShowLandmark(j, true);
		}

		UpdateViews();

		m_pLdCheckList->Enable(true);
		m_pFlipImageCheckList[0]->Enable(true);
		m_pFlipImageCheckList[1]->Enable(true);
		m_Gui->Enable(wxOK,true);
	}

	m_Gui->Update();
}

void lhpOpExtractLandmarkEOS::AddVme(mafView * view, mafVME * vme)
{
	std::list<mafNode *> nodeList;
	std::list<mafNode *>::iterator iter;

	mafNode * root = vme->GetRoot();
	mafNode * parent = vme->GetParent();

	nodeList.push_front(vme);
	for (mafNode * parent = vme->GetParent(); parent != root; parent = parent->GetParent())
	{
		nodeList.push_front(parent);
	}
	nodeList.push_front(root);

	for (iter = nodeList.begin(); iter!=nodeList.end(); iter++)
		view->VmeAdd(* iter);
}
void lhpOpExtractLandmarkEOS::OpRun()
{
	m_InputVME = mafVME::SafeDownCast(m_Input);

	CreateOpDialog();
	InitializeInteractors();

	m_OpDialog->ShowModal() ;
	int res = (m_OpDialog->GetReturnCode() == wxID_OK) ? OP_RUN_OK : OP_RUN_CANCEL;

	OpStop(res) ;
}

//----------------------------------------------------------------------------
void lhpOpExtractLandmarkEOS::OpStop(int result)
//----------------------------------------------------------------------------
{
	/*
	if (m_Gui)
		HideGui();
	*/
	StopInteractors();
	DeleteOpDialog() ;

	if (OP_RUN_OK == result)
	{
		for (int i = 0; i < LANDMARK_NUM; i++)
			m_Landmark[i]->SetBehavior(NULL);
	}
	else
	{
		if (m_Cloud->GetParent())
		{
			m_Cloud->GetParent()->RemoveChild(m_Cloud);
		}
	}

	//mafDEL(m_Cloud);

	/*
	if (m_oldFrontalMatrix.GetPointer())
	{
		if (m_Images[CORONAL])
			m_Images[CORONAL]->SetAbsMatrix(* m_oldFrontalMatrix);
	}
	*/

    //m_GuiTransformMouse->DetachInteractorFromVme();

  mafEventMacro(mafEvent(this,CAMERA_UPDATE));

	mafEventMacro(mafEvent(this,result));
}

//----------------------------------------------------------------------------
void lhpOpExtractLandmarkEOS::AddLandmarks()
//----------------------------------------------------------------------------
{
	if  (NULL == m_Images[0])
		return;

	mafNode * parent = m_Images[0]->GetParent();
	//mafNEW(m_Cloud);
	m_Cloud->ReparentTo(parent);
	m_Cloud->Open();
	m_Cloud->SetName(_("landmark cloud"));
	//m_Cloud->SetRadius(m_InputVME->GetOutput()->GetVTKData()->GetLength()/60.0);
	m_Cloud->SetRadius(10);

	bool cloud_was_open = m_Cloud->IsOpen();
	if (!cloud_was_open)
	{
	m_Cloud->Open();
	}

	const double pos[3] = {150, 100, 200};
	AddLandmark(m_Landmark[0], mafString("LeftFemur"), pos);
	CreateISA(0);

	const double pos1[3] = {150, 100, 100};
	AddLandmark(m_Landmark[1], mafString("LeftKnee"), pos1);
	CreateISA(1);

	const double pos2[3] = {150, 100, 50};
	AddLandmark(m_Landmark[2], mafString("LeftAnkle"), pos2);
	CreateISA(2);

	const double pos3[3] = {50,100, 200};
	AddLandmark(m_Landmark[3], mafString("RightFemur"), pos3);
	CreateISA(3);

	const double pos4[3] = {50, 100, 100};
	AddLandmark(m_Landmark[4], mafString("RightKnee"), pos4);
	CreateISA(4);

	const double pos5[3] = {50,100, 50};
	AddLandmark(m_Landmark[5], mafString("RightAnkle"), pos5);
	CreateISA(5);

	if (cloud_was_open)
	{
	// m_Cloud->Close();
	}

	mafEventMacro(mafEvent(this,VME_SHOW,m_Cloud,true));
	mafEventMacro(mafEvent(this,CAMERA_UPDATE));
}

void lhpOpExtractLandmarkEOS::AddLandmark( mafSmartPointer<mafVMELandmark> pLandmark, mafString & name, const double pos[3])
{
  //mafSmartPointer<mafVMELandmark> landmark;
	//mafNEW(pLandmark);
	pLandmark->SetName(name.GetCStr());
	pLandmark->ReparentTo(m_Cloud);

	if(NULL != m_InputVME)
		pLandmark->SetTimeStamp(m_InputVME->GetTimeStamp());

	pLandmark->Update();

	pLandmark->SetAbsPose(pos[0],pos[1],pos[2],0,0,0);
}

//----------------------------------------------------------------------------
void lhpOpExtractLandmarkEOS::OnEventGuiTransformMouse(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  switch(maf_event->GetId())
	{
    case ID_TRANSFORM: // from mafGUITransformMouse
    {
      PostMultiplyEventMatrix(maf_event);
    }
    break;
    default:
    {
      mafEventMacro(*maf_event);
    }
  }
}

void lhpOpExtractLandmarkEOS::SetPickedVME(mafVME * vme)
{
	//m_GuiTransformMouse->SetRefSys(vme);
	m_CurrentVME = vme;
}

void lhpOpExtractLandmarkEOS::CreateISA(int idx)
{
	mafMatrix *absMatrix;
	absMatrix = m_Landmark[idx]->GetOutput()->GetAbsMatrix();

	//m_IsaCompositor[idx] = mafInteractorCompositorMouse::New();

	m_IsaTranslate[idx] = m_IsaCompositor[idx]->CreateBehavior(MOUSE_LEFT);

	m_IsaTranslate[idx]->SetListener(this);
	m_IsaTranslate[idx]->SetVME(m_Landmark[idx]);
	m_IsaTranslate[idx]->GetTranslationConstraint()->GetRefSys()->SetTypeToView();
	// set the pivot point
	m_IsaTranslate[idx]->GetTranslationConstraint()->GetRefSys()->SetMatrix(absMatrix);
	m_IsaTranslate[idx]->GetPivotRefSys()->SetTypeToCustom(absMatrix);

	m_IsaTranslate[idx]->GetTranslationConstraint()->SetConstraintModality(mafInteractorConstraint::FREE, mafInteractorConstraint::FREE, mafInteractorConstraint::LOCK);
	m_IsaTranslate[idx]->EnableTranslation(true);

	//mafInteractor * interactor = m_Landmark[idx]->GetBehavior();

	m_Landmark[idx]->SetBehavior(m_IsaCompositor[idx]);
}

//----------------------------------------------------------------------------
void lhpOpExtractLandmarkEOS::PostMultiplyEventMatrix(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
	if (! m_CurrentVME)
		return;

  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    long arg = e->GetArg();

    // handle incoming transform events
	/*
    vtkTransform *tr = vtkTransform::New();
    tr->PostMultiply();
    tr->SetMatrix(m_CurrentVME->GetOutput()->GetAbsMatrix()->GetVTKMatrix());
    tr->Concatenate(e->GetMatrix()->GetVTKMatrix());
    tr->Update();

    mafMatrix absPose;
    absPose.DeepCopy(tr->GetMatrix());
	mafTimeStamp m_CurrentTime =m_CurrentVME->GetTimeStamp();
    absPose.SetTimeStamp(m_CurrentTime);\
	*/

    if (arg == mafInteractorGenericMouse::MOUSE_MOVE)
    {
      // move vme
		mafVMELandmark * pLandmark = mafVMELandmark::SafeDownCast(m_CurrentVME);
		if (!pLandmark)
			return;

		vtkMatrix4x4 * pMatrix = e->GetMatrix()->GetVTKMatrix();

		double inPos[4], outPos[4];
		pLandmark->GetPoint(inPos);
		inPos[3] = 1;
		pMatrix->MultiplyPoint(inPos, outPos);
      //m_CurrentVME->SetAbsMatrix(absPose);
		pLandmark->SetPoint(outPos);
      // update matrix for OpDo()
      // todo
	  //m_NewAbsMatrix = absPose;
    }
    //mafEventMacro(mafEvent(this, CAMERA_UPDATE));

    // clean up
    //tr->Delete();
	UpdateViews();
  }
}

void lhpOpExtractLandmarkEOS::UpdateViews()
{
	 for (int i = 0 ; i < VIEW_NUM; i++)
	 {
		 m_View[i]->CameraUpdate();
	 }
}