/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: lhpOpComputeProgressiveHull.h,v $ 
  Language: C++ 
  Date: $Date: 2011-09-07 05:42:37 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: Josef Kohout
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef __lhpOpComputeProgressiveHull_H__
#define __lhpOpComputeProgressiveHull_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class lhpOpSetLowerRes;
class mafVME;
class mafGUIButton;
class mafGUIDialog;
class mafRWI;
class vtkPolyDataMapper;
class vtkActor;
class vtkPolyData;

//----------------------------------------------------------------------------
// lhpOpComputeProgressiveHull :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT lhpOpComputeProgressiveHull: public mafOp
{
protected:
  //----------------------------------------------------------------------------
  // Constants:
  //----------------------------------------------------------------------------
  enum GUI_IDS
  {    
    ID_PREVIEW = MINID,    
   
    ID_REQ_POINTS,
		ID_QUALITY,
    ID_SHOW_ORIGINAL,
    ID_SHOW_HULL,
		ID_STATUS_HELP,
    ID_USE_GPU,
    ID_OK,
    ID_CANCEL,
  };


#pragma region //Help structures

  typedef struct MESH
  {
    vtkPolyData* pPoly;                 //<polydata of the mesh
    vtkPolyDataMapper* pMapper;         //<mapper
    vtkActor* pActor;                   //<and its actor
  } MESH;

#pragma endregion //Help structures

public:
	lhpOpComputeProgressiveHull(const wxString &label = "Compute Progressive Hull");
	~lhpOpComputeProgressiveHull(); 

	/*virtual*/ void OnEvent(mafEventBase *maf_event);

	mafTypeMacro(lhpOpComputeProgressiveHull, mafOp);

	/*virtual*/ mafOp* Copy();

	/** Return true for the acceptable vme type. */
	/*virtual*/ bool Accept(mafNode *node);

	/** Builds operation's interface. */
	/*virtual*/ void OpRun();

	/** Execute the operation. */
	/*virtual*/ void OpDo();

	/** Makes the undo for the operation. */
	/*virtual*/ void OpUndo();	

protected:
	/** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
	/*virtual*/ void OpStop(int result);

  /** Creates internal data structures used in the editor.
  Returns false, if an error occurs (e.g. unsupported input) */
  virtual bool CreateInternalStructures();

  /** Destroys internal data structures created by CreateInternalStructures */
  virtual void DeleteInternalStructures();

  /** Creates GUI including renderer window */
	void CreateOpDialog();

  /** Destroys GUI */
	void DeleteOpDialog();

  /** This method creates the internal mesh structure.
  It constructs also the VTK pipeline. */
  MESH* CreateMesh(vtkPolyData* pMesh);

  /** Updates the VTK pipeline for the given curve.
  N.B. It does not connects actors into the renderer. */
  void UpdateMesh(MESH* pMesh);

  /** Updates the visibility of meshes.
  It adds/removes actors from the renderer according to the
  status of their associated data and the visual options
  specified by the user in the GUI.
  This method is typically calls after UpdateMesh or 
  UpdateControlCurve is finished */
  void UpdateVisibility();  
  
	/** Updates status info */
	void UpdateStatusInfo();

  /** Removes all actors from the renderer. */
  void RemoveAllActors();  

  /** Computes the hull mesh for the input mesh */
  void ComputeHullMesh();

  //GUI handlers
protected:      
  /** Performs the deformation using the current settings. */
  virtual void OnPreview();

  /** Performs the deformation and creates outputs */
  virtual void OnOk();

  
protected:  
	lhpOpSetLowerRes* m_OpSetHull;						///<governs OpDo and OpUndo of Hull settings
	double m_DesiredReduction;								///<desired reduction coefficient
	int m_DesiredQuality;											///<desired quality the hull

#pragma region //Visualization stuff
  mafGUIDialog						*m_Dialog;			 //<dialog - GUI
  mafRWI									*m_Rwi;					 //<rendering windows
  
  MESH* m_Meshes[2];                       //<original and hull mesh      
  int m_MeshesVisibility[2];							 //<original and deformed mesh visibility    

#pragma region //GUI Controls      
  wxTextCtrl* m_StatusHelp;
#pragma endregion //GUI Controls
#pragma endregion //Visualization stuff
};

#endif
