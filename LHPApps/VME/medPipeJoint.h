/*========================================================================= 
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: medPipeJoint.h,v $ 
  Language: C++ 
  Date: $Date: 2011-08-05 12:10:07 $ 
  Version: $Revision: 1.1.2.2 $ 
  Authors: Josef Kohout
  ========================================================================== 
  Copyright (c) 2011 University of West Bohemia
  See the COPYINGS file for license details 
  =========================================================================
*/
#ifndef __medPipeJoint_H__
#define __medPipeJoint_H__

#include "mafPipe.h"
#include "lhpVMEDefines.h"
//----------------------------------------------------------------------------
// forward refs :
//----------------------------------------------------------------------------
class vtkSphereSource;
class vtkActor;

/**
    class name: medPipeJoint
    Pipe which represent visually medVMEJoint.
*/
class LHP_VME_EXPORT medPipeJoint : public mafPipe
{
public:
  /** RTTI Macro */
  mafTypeMacro(medPipeJoint, mafPipe);

protected:
  /** IDs for the GUI */
  enum PIPE_JOINT_WIDGET_ID
  {
    ID_JOINT_DISPLAY = Superclass::ID_LAST,    
		ID_JOINT_RADIUS,
		ID_MAC_DISPLAY,
		ID_SAC_DISPLAY,
		ID_MASAC_RADIUS,
		ID_MABOX_DISPLAY,
		ID_SABOX_DISPLAY,
		ID_MASABOX_RADIUS,
		ID_AXIS_DISPLAY,
		ID_AXIS_LENGTH,

    ID_LAST,
  };   
		
	vtkActor* m_JointPositionActor;
	vtkActor* m_JointAxisActor[3];
	vtkActor* m_MainActorCentreActor;
	vtkActor* m_SecondActorCentreActor;
	
	vtkActor* m_MainActorBBoxActor;
	vtkActor* m_SecondActorBBoxActor;


	bool m_bSizesInitialized;	///<true, if sizes has been already initialized and therefore, there is no need to update them
	double m_JointRadius;		///<for all kind of joints		
	double m_CentroidRadius;	///<radius of centroid ball representation
	double m_BBoxTubeRadius;	///<radius of boxes
	double m_AxisLength;			///<length of the axis
	

	int m_EnableJoint;		///<1, if joint is to be displayed
	int m_EnableMAC;			///<1, if the mass centre of main actor is to be displayed
	int m_EnableSAC;			///<1, if the mass centre of second actor is to be displayed
	int m_EnableMABBox;		///<1, if the bounding box of main actor is to be displayed
	int m_EnableSABBox;		///<1, if the bounding box of second actor is to be displayed
	int m_EnableAxis;			///<1, if the rotation axis of the joint should be displayed

	
public:
  /** constructor */
	medPipeJoint();

  /** destructor */
	virtual ~medPipeJoint ();

public:
  /** Creates the VTK rendering pipeline */
  /*virtual*/ void Create(mafSceneNode *n);

	/** Processes events coming from GUI */
	/*virtual*/ void OnEvent(mafEventBase *maf_event);

protected:
  /** creation of the gui */
  /*virtual*/ mafGUI  *CreateGui();  

	/** Creates visual pipe for visualization of bounding box of actorVME */
	virtual vtkActor* CreateBBoxPipe(mafVME* actorVME, double tuberadius);
	
	/** Creates visual pipe for visualization of sphere with the given local centre (relative to actorVME) */
	virtual vtkActor* CreateSpherePipe(mafVME* actorVME, const double* centre, double radius);

	/** Creates visual pipe for visualization of arrow started at the centre position directing in the given direction dir and having the given length.
	Centre is relative to the actorVME.*/
	virtual vtkActor* CreateArrowPipe(mafVME* actorVME, const double* centre, const double* dir, double length);

	/** Creates all visual pipes to reflect the changes in VME */
	virtual void CreatePipe();

	/** Destroys already existing visual pipes*/
	virtual void DestroyPipe();	

	/** Initializes sizes for various parts of the pipe using the sizes of joint actors.
	If joint actors are unknown, this routine sets the sizes to the default values and
	m_bSizesInitialized is left unchanged, otherwise they are set to be appropriate to 
	the sizes of joint actors and the method sets m_bSizesInitialized to true.
	N.B. The method terminates without any change, if m_bSizesInitialized is already true. */
	virtual void InitSizes();

	/** Updates the radius of vtkSphereSource at the beginning of the pipe ending with the given actor. */
	virtual void UpdateSphereRadius(vtkActor* actor, double radius);

	/** Updates the radius of vtkTubeFilter in the pipe ending with the given actor. */
	virtual void UpdateTubeRadius(vtkActor* actor, double radius);
};  
#endif // __medPipeJoint_H__
