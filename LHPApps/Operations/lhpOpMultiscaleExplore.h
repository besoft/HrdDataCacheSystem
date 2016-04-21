/*========================================================================= 
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpMultiscaleExplore.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpMultiscaleExplore_H__
#define __lhpOpMultiscaleExplore_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include "mafRWI.h"
#include "mafEventBase.h"
#include "mafVME.h"
#include "mafGUIDialog.h"
#include "mafGUIFloatSlider.h"

#include "vtkPolyData.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkInteractorStyleTrackballCamera.h"

#include "lhpMultiscaleUtility.h"
#include "lhpMultiscaleVisualPipes.h"
#include "lhpMultiscaleCallbacks.h"

#include <vector>
#include <iostream>


//------------------------------------------------------------------------------
// lhpOpMultiscaleExplore:
// A multiscale "view", implemented as a modal operation.
// This is an op for viewing surface data where the objects are on different scales.

// The user selects an intial vme, on which to run the op.
// Only one input vme can be selected because this is an op, not a view,
// but additonal vme's can be added from the op dialog.

// The op displays a surface view.
// Vme's which are too small to see are replaced by tokens.  
// Clicking the mouse on a token zooms in on the object.
// Vme's which become too large for the view are made invisible, unless they were
// the target of a zoom, in which case they have current "attention", and must remain 
// visible.

// When tokens become too close to resolve, they merge into one token, representing 
// the group of actors.  Clicking on such a token zooms in on the group.

// Operation and associated classes:
// lhpOpMultiscaleExplore -     Main op, Responsible for gui, events and visual pipes.
// lhpMultiscaleActor -         Container for a vtk actor and its pipe - can be data or token.
// mafMultiscaleCoordsUtility - Methods for moving and sizing actors in world, view and display coords.
// lhpMultiscaleCallbacks -     Callbacks which convert vtk events to maf events.
// lhpMultiscaleCameraParams -  Class for saving camera parameters.
// lhpMultiscaleCameraUtility - Methods for moving camera.
// lhpMultiscaleUtility -       Blackboard and most of the methods for multiscale.
// lhpMultiscaleVectorMath -    Methods for vector arithmetic.
// lhpMultiscaleVisualPipes -   Visual pipes for data and tokens.  Create and delete vtk objects.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Dependency diagram:
//
//           ------------------------------------------------>
//         /                                                   \
//        /                                                     \                                     
//       /                      ----> CameraParams ------------->
//      /                     /                                   \       
// lhpOp --> MultiscaleUtility ------> CameraUtility --------------> VectorMath
//      \                     \                     \             /
//       \                     \                     \           /
//        \                      -----------> ActorCoordsUtility 
//         \                     \
//          \                     \
//            ------------------> MultiscaleActor ---------------> Visual Pipes
//            \                                                 /
//             \                                               /
//               ---------------------------------------------
//               \
//                 -------------------------------------------------> Callbacks
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/// Class lhpOpMultiscaleExplore. \n
/// Operation which implements a click-and zoom multiscale view \n
/// for volume or surface data.
//------------------------------------------------------------------------------
class LHP_OPERATIONS_EXPORT lhpOpMultiscaleExplore : public mafOp
{
  friend class lhpOpMultiscaleExploreTest ;   ///< test class is a friend

public:
  lhpOpMultiscaleExplore(wxString label = "Explore Multiscale");
  ~lhpOpMultiscaleExplore(); 

  mafTypeMacro(lhpOpMultiscaleExplore, mafOp);

  mafOp* Copy();

  void OnEvent(mafEventBase *maf_event);

  /// Return true for the acceptable vme type.
  bool Accept(mafNode* vme);

  /// Static copy of Accept(), required so that we can pass the function \n
  /// pointer to the VME_CHOOSE event
  static bool AcceptStatic(mafNode* vme);

  /// Builds operation's interface by calling CreateOpDialog() method.
  void OpRun();

  /// Execute the operation.
  void OpDo();

  /// Makes the undo for the operation.
  void OpUndo();

protected:
  //----------------------------------------------------------------------------
  // methods for operation's workflow
  //----------------------------------------------------------------------------

  /// Builds operation's interface and visualization pipeline. 
  void CreateOpDialog();

  /// Remove operation's interface. 
  void DeleteOpDialog();

  /// Builds operation without dialog for testing 
  void CreateOpWithoutDialog(vtkRenderer* renderer) ;

  /// Get bounds of vme list
  void GetBoundsOfVmeList(std::vector<mafVME*>& vmeList, double bounds[6]) ;

  /// Add new vme to scene.
  /// Creates multiscale actors for data and tokens and creates visual pipes.  
  void AddVmeToScene(mafVME* vme) ;

  /// Add list of vme's to scene
  void AddVmeListToScene(std::vector<mafVME*>& vmeList) ;

  /// List all valid vme's in tree
  void ListValidVmesInTree(mafNode* parentNode, std::vector<mafVME*>& vmeList, bool includeParent) ;

  /// List all nodes in tree
  void ListNodesInTree(mafNode* parentNode, std::vector<mafNode*>& nodeList, bool includeParent) ;

  /// Recursively list subtree of parent node
  void ListSubTreeOfNode(mafNode* parentNode, std::vector<mafNode*>& nodeList) ;

  /// Visual pipe for surface data. \n
  /// Joins vme to renderer 
  void CreateSurfacePipeline(mafVME* vme, vtkRenderer *renderer);

  /// Visual pipe for polydata token 
  void CreateTokenPipeline(vtkRenderer *renderer);

  /// Visual pipe for volume data. \n
  /// Joins vme to renderer 
  void CreateVolumeSlicePipeline(mafVME* vme, vtkRenderer *renderer);

  /// Update the camera 
  void UpdateCamera() ;



  //----------------------------------------------------------------------------
  // methods for controlling the slice
  //----------------------------------------------------------------------------

  /// Initialize the parameters of the slice (origin and view index). \n
  /// This only sets the parameters - it does not set or change the visual pipes ! 
  void InitSliceParams(int viewIndex, double *bounds) ;

  /// Move the global slice. \n
  void UpdateSlicePosition() ;

  /// Update the view direction. \n
  /// This also recalculates the slice position 
  void UpdateViewAxis(double *bounds) ;

  /// Set the slider range to fit the given bounds 
  void SetSliderRange(double *bounds) ;


  //----------------------------------------------------------------------------
  // methods for handling callbacks
  //----------------------------------------------------------------------------

  // Constants for size thresholds
  // SIZELOWER and SIZEUPPER are different to provide hysteresis, \n
  // so the actor doesn't flicker between states.
  // They should be different enough such that the state should not 
  // change under a complete camera rotation.
  static const int SIZEUPPER = 60 ;         ///< upper size threshold for actor, ie screen size when it changes from token to visible
  static const int SIZELOWER = 40 ;         ///< lower size threshold for actor, ie screen size when it becomes a token
  static const int TOKENSIZE = 10 ;         ///< standard size of token
  static const int TOKENSIZEMAX = 15 ;      ///< maximum size of visible token
  static const int TOKENSIZEMIN = 5 ;       ///< minimum size of visible token
  static const int TOO_LARGE_FACTOR = 10 ;   ///< how many x larger than scale before an actor is deemed too large

  /// structure for actor and depth info used in pick callback 
  typedef struct {
    int actorId ;
    double depth ;
  } ACTORINFO ;

  /// set token size 
  void SetTokenSize(vtkRenderer* renderer, int actorId, int tokenSize) ;          

  /// Get renderer 
  vtkRenderer* GetRenderer() ;

  /// Get render window 
  vtkRenderWindow* GetRenderWindow() ;

  /// Get interactor 
  vtkRenderWindowInteractor* GetInteractor() ;

  /// Get Multiscale utility 
  lhpMultiscaleUtility* GetMultiscaleUtility() {return m_MultiscaleUtility ;}

  /// Zoom out handler 
  void OnZoomOut(vtkRenderer *renderer) ;

  /// Camera reset handler 
  void OnCameraReset(vtkRenderer *renderer) ;

  /// Go back handler 
  void OnGoBack(vtkRenderer *renderer) ;

  // Start Render handler.
  // This is where we look at the scene and decide which actors and tokens should be visible.
  // Nowhere else should change the visibility of the actors. 
  void OnStartRender(vtkRenderer *renderer) ;                              ///< start render handler
  void OnActorTooSmall(vtkRenderer* renderer, int actorId) ;               ///< on actor becoming too small
  void OnActorInScale(vtkRenderer* renderer, int actorId) ;                ///< on actor coming back into scale
  void OnActorTooLarge(vtkRenderer* renderer, int actorId) ;               ///< on actor becoming too large
  void OnTokensOverlap(vtkRenderer* renderer, int token1, int token2) ;    ///< on tokens overlapping
  void OnTokensSeparated(vtkRenderer* renderer, int token1, int token2) ;  ///< on tokens separating

  /// Mouse click handler 
  void OnMouseClick() ;
  void OnPick(vtkRenderer* renderer, int tokenId) ;        ///< on token picked by mouse click

  /// Debug handler.  This prints diagnostic info 
  void OnDebug(std::ostream& os, vtkRenderer *renderer) ;



  //----------------------------------------------------------------------------
  // member variables
  //----------------------------------------------------------------------------

  lhpMultiscaleUtility *m_MultiscaleUtility ;                   ///< multiscale utility 

  std::vector <lhpMultiscaleSurfacePipeline*> m_surfacePipes ;          ///< list of surface pipes
  std::vector <lhpMultiscaleTokenPipeline*> m_tokenPipes ;              ///< list of token pipes
  std::vector <lhpMultiscaleVolumeSlicePipeline*> m_volumeSlicePipes ;  ///< list of slice pipes

  int m_nextTokenColor ;                                        ///< color id of next token

  /// pointer to external renderer, which should be defined if op created with no dialog 
  vtkRenderer* m_externalRenderer ;

  lhpMultiscaleDoubleClickCallback *m_dclickCallback ;

  mafGUIDialog		*m_Dialog;        // dialog and interactor
  mafRWI      *m_Rwi;
  mafGUIFloatSlider *m_PosSlider ;       // slice position slider (destroyed with dialog - don't delete in deconstructor)
  double m_SliceOrigin[3] ;           // position of slice
  int m_BaseUnits ;                   // base units validator
  int m_ViewIndex ;                   // view direction validator (don't assume that 0,1,2 = x,y,z !!)
  int m_ViewIndex_old ;
  double m_SliderOrigin ;             // position slider validator
  double m_SliderOrigin_old ;
  double m_Opacity ;                  // opacity slider validator
};



#endif
