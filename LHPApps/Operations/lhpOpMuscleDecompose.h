/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: lhpOpMuscleDecompose.h,v $ 
  Language: C++ 
  Date: $Date: 2011-09-07 05:42:37 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: David Cholt, Josef Kohout
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef __lhpOpMuscleDecompose_H__
#define __lhpOpMuscleDecompose_H__

#include "mafOp.h"
#include "lhpBuilderDecl.h"
#include "lhpOperationsDefines.h"

//#include "vtkstd/vector"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
//class lhpOpSetLowerRes;
class mafVME;
//class mafGUIButton;
class mafGUIDialog;
class mafRWI;
class vtkPolyDataMapper;
class vtkActor;
class vtkPolyData;

//----------------------------------------------------------------------------
// lhpOpMuscleDecompose :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT lhpOpMuscleDecompose: public mafOp
{
protected:
  //----------------------------------------------------------------------------
  // Constants:
  //----------------------------------------------------------------------------
  enum GUI_IDS
  {    
	  ID_UNASSIGNED_DATA = MINID ,
       
	   ID_ADD_TENDON  ,
       ID_REM_TENDON  ,
       ID_TENDONS 	  ,
       ID_UP_TENDON   ,
	   ID_DOWN_TENDON ,

	   ID_ADD_FIBER	  ,
       ID_REM_FIBER	  ,
       ID_FIBERS	  ,
       ID_UP_FIBER    ,
	   ID_DOWN_FIBER  ,

	   ID_FIBRE_SUB   ,
       ID_SURFACE_SUB ,

       ID_PREVIEW 	  ,
       ID_CANCEL 	  ,
       ID_OK 		  ,
	   ID_SHOW_MESH   ,
	   ID_SHOW_DATA   ,
	   ID_SHOW_RESULT ,
	   // selection dialog
	   ID_DATA_SEL    ,
	   ID_SEL_OK      ,
	   ID_SEL_CANCEL
  };

#define MUSCLE_COLOR 0.7,0.7,0.7
#define UNASSIGNED_COLOR 0,1,1
#define TENDON_COLOR 0.3, 0.3, 0.3
#define FIBER_COLOR 1, 0.5, 0.5
#define SELECTION_COLOR 1, 1, 0

#pragma region //Help structures

  typedef struct MESH
  {
    vtkPolyData* pPoly;                 //<polydata of the mesh
    vtkPolyDataMapper* pMapper;         //<mapper
    vtkActor* pActor;                   //<and its actor
  } MESH;

#pragma endregion //Help structures

public:
	lhpOpMuscleDecompose(const wxString &label = "Decompose Muscle");
	~lhpOpMuscleDecompose(); 

	/*virtual*/ void OnEvent(mafEventBase *maf_event);

	mafTypeMacro(lhpOpMuscleDecompose, mafOp);

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

  /** Converts VTKPoints to VTKPolyLine*/
  void ConvertToPolyline(vtkPolyData* pointData);

  /** Destroys internal data structures created by CreateInternalStructures */
  virtual void DeleteInternalStructures();

  /** Creates GUI including renderer window */
	void CreateOpDialog();
	
	/** Populates the unassigned data list */
	void PopulateList();

  /** Destroys GUI */
	void DeleteOpDialog();

	void CreateDataSelectionDialog(mafNode* muscleGeometryNode);

  /** This method creates the internal mesh structure.
  It constructs also the VTK pipeline. */
  MESH* CreateMesh(vtkPolyData* pMesh);

  void DecomposeMuscle(bool useTuber);

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
  
  void UpdateSelected(MESH* mesh);
  void UpdateColors();
	/** Updates status info */
  void UpdateStatusInfo();

  /** Removes all actors from the renderer. */
  void RemoveAllActors();  

  //GUI handlers
protected:      
  /** Performs the deformation using the current settings. */
  virtual void OnPreview();

  /** Performs the deformation and creates outputs */
  virtual void OnOk();

  virtual void OnAddTendon();

  virtual void OnRemTendon();

  virtual void OnAddFiber();

  virtual void OnRemFiber();

  void OnMoveData(wxListBox* lb, int dir);


  //void OnListBox(wxEvent& e);
  void OnListBoxSel(wxEvent& event);

protected:

protected:  
	//lhpOpSetLowerRes* m_OpSetHull;		     	///<governs OpDo and OpUndo of Hull settings
	double InterpolationSubdivision;				///<desired fibre subdivision
	double SurfaceSubdivision;				    ///<desired surface subdivision
	int ShowData;
	int ShowMesh;
	int ShowResult;

	mafNode* dataNode;

#pragma region //Visualization stuff
  mafGUIDialog	*m_Dialog;						//<dialog - GUI
  mafGUIDialog	*m_DataSelectionDialog;	

  mafRWI *m_Rwi;								//<rendering windows
  
  MESH* m_MuscleMesh;							//< muscle data
  MESH* m_OutputMesh;							//< output data

  wxString* m_Labels;							//< fiber and tendon data labels
  MESH** m_Data;						        //< fiber and tendon data data
  int dataCount;								//< only for destruction

  MESH* m_Selected;

//  vtkstd::vector<int> m_Visibility;				//< visibility of various data.

#pragma region //GUI Controls      
    // Main dialog
  wxListBox* lbUnassigned;
	wxListBox* lbTendons;	
	wxListBox* lbFibers;
	// Data selection dialog
	wxListBox* lbDataSel;

#pragma endregion //GUI Controls

#pragma endregion //Visualization stuff
	
};
#endif
