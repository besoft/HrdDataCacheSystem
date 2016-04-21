/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpOpScaleMMA.h,v $
Language:  C++
Date:      $Date: 2011-10-21 16:16:06 $
Version:   $Revision: 1.1.2.2 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2011
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lhpOpExtractLandmarkEOS_H__
#define __lhpOpExtractLandmarkEOS_H__

#include "mafop.h"
#include "lhpOperationsDefines.h"
#include "mafInteractorCompositorMouse.h"
#include "mafInteractorGenericMouse.h"
#include "mafVMEImage.h"
#include "mafMatrix.h"
#include "mafVMELandmarkCloud.h"
#include "mafVMELandmark.h"


class vtkTextMapper;
class vtkTextActor;
class vtkActor2D;
class vtkRenderWindowInteractor;

class mafGUI;
class mafGUIDialog;
class mafRWI;
class mafViewImage;
class mafGUIButton;
class mafAction;
//class medDeviceButtonsPadMouseDialog;
class mafDeviceButtonsPadMouse;
class mafDeviceManager;
class mafInteractor;
class mafInteractorSER;
class mafInteractorPER;
class lhpInteractorExtractLandmarkEOS;
class mafGUITransformMouse;
class mafInteractorPicker;

class medViewVTKCompound;
class mafViewVTK;
class mafVME;

class medDeviceButtonsPadMouseDialog;
class mafGUICheckListBox ;

#include <mafSmartPointer.h>
#include <vtkSmartPointer.h>
#include <vtkMAFSmartPointer.h>

template class LHP_OPERATIONS_EXPORT mafSmartPointer<mafInteractorCompositorMouse>;
template class LHP_OPERATIONS_EXPORT mafSmartPointer<mafVMEImage>;
template class LHP_OPERATIONS_EXPORT mafSmartPointer<mafMatrix>;
template class LHP_OPERATIONS_EXPORT mafSmartPointer<mafVMELandmarkCloud>;
template class LHP_OPERATIONS_EXPORT mafSmartPointer<mafVMELandmark>;

class LHP_OPERATIONS_EXPORT lhpOpExtractLandmarkEOS : public mafOp
{

	friend class lhpInteractorExtractLandmarkEOS;

public: 
	enum VIEW_ID 
	{
		CORONAL,
		SAGGITAL,
		VIEW_NUM,

	};

	enum LANDMARK_ID 
	{
		LEFT_FEMUR,
		
		LEFT_KNEE,
		LEFT_ANKLE,
		RIGHT_FEMUR,
		RIGHT_KNEE,
		RIGHT_ANKLE,

		LANDMARK_NUM,

		
	};

	mafTypeMacro(lhpOpExtractLandmarkEOS, mafOp);

	explicit lhpOpExtractLandmarkEOS(wxString label = "ExtractLandmarkEOS");
	virtual ~lhpOpExtractLandmarkEOS();


	mafOp* Copy();

	/** Class for handle mafEvent*/
	void OnEvent(mafEventBase *maf_event);

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode* vme);

	static bool AcceptImage(mafNode * vme);

	/** 
	Static copy of Accept(), required so that we can pass the function
	pointer to the VME_CHOOSE event */
	static bool AcceptStatic(mafNode* vme);

	/** return number of images in a tree rooted nodeStart */
	static int CountImagesInTree(mafNode * nodeStart);


		/** Builds operation's GUI by calling CreateOpDialog() method. */
	void OpRun();

	/** Execute the operation. */
	//virtual void OpDo();

	/** Makes the undo for the operation. */
	//void OpUndo();

	mafView * GetView(vtkRenderWindowInteractor * );
	void UpdateViews();

protected:
		/** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
	void OpStop(int result);

	/** create op GUI panel */
	void CreateGUI();

	void CreateOpDialog();

	void DeleteOpDialog();

	void InitializeInteractors();

	void StopInteractors();

	void AddLandmarks();
	void AddLandmark(mafSmartPointer<mafVMELandmark> , mafString & name, const double pos[3]);

	void SetPickedVME(mafVME * vme);
	void CreateISA(int idx);

	void ShowLandmark(int id, bool show);
	void FlipImage(int imageIdx, int flipdir, bool bFlip);

	virtual void OnEventGuiTransformMouse(mafEventBase *);
	virtual void PostMultiplyEventMatrix(mafEventBase *maf_event);

	void OnChooseEosImage(int view);
	void AddVme(mafView * view, mafVME * vme);

	mafSmartPointer<mafInteractorCompositorMouse> m_IsaCompositor[LANDMARK_NUM];

	mafSmartPointer<mafInteractorGenericMouse> m_IsaTranslate[LANDMARK_NUM];

 
	mafGUI * m_Gui;
	mafGUIDialog * m_OpDialog;
	mafRWI *m_Rwi ;
	mafGUICheckListBox  * m_pLdCheckList ;
	mafGUICheckListBox  * m_pFlipImageCheckList[2];

	mafInteractor *m_OldBehavior;             //<Old volume behavior
	//mafNode *m_OldVolumeParent;               //<Old volume parent

	//mafGUITransformMouse        *m_GuiTransformMouse;
	mafVME * m_InputVME;
	mafVME * m_CurrentVME;

	medDeviceButtonsPadMouseDialog* m_DialogMouse;
	mafDeviceManager *m_DeviceManager;        //<Device manager
	mafInteractorSER *m_SER;      //<Static event router                        

	mafInteractorPicker *m_Picker;
	lhpInteractorExtractLandmarkEOS * m_PER;



	static const wxString m_ViewNames[VIEW_NUM] ;
	mafString m_ImageNames[VIEW_NUM];
	mafSmartPointer<mafVMEImage> m_Images[VIEW_NUM];
	mafVME * m_ImageVme[VIEW_NUM];
	mafSmartPointer<mafMatrix>  m_matImage[VIEW_NUM];

	//mafViewImage * m_View[VIEW_NUM];
	mafViewVTK * m_View[VIEW_NUM];


	vtkMAFSmartPointer<vtkTextMapper>  m_TextMapper[VIEW_NUM];
	vtkMAFSmartPointer<vtkActor2D> m_TextActor[VIEW_NUM];

	mafSmartPointer<mafVMELandmarkCloud>  m_Cloud;
	mafSmartPointer<mafVMELandmark>  m_Landmark[LANDMARK_NUM];

	//mafSmartPointer<mafMatrix>  m_oldFrontalMatrix;
	
	/** event ids */
	enum 
	{
		ID_CHOOSE_CORONAL = MINID,
		ID_CHOOSE_SAGGITAL,
		ID_LD_CHECKLIST,
		ID_CORONAL_CHECKLIST,
		ID_SAGGITAL_CHECKLIST,
		//ID_OK = MINID,	/* choose gadolinium left inner ventricle */
		//ID_CANCEL,// temp
	};



};

#endif 