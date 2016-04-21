/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafVMEAFRefSys.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:57 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/
 
#ifndef __mfAFRefSys_h
#define __mfAFRefSys_h

//----------------------------------------------------------------------------
// Includes:
//----------------------------------------------------------------------------
#include "mafVME.h"
#include "mafVmeOutput.h"
#include "mafVMEGeneric.h"
#include "mafVMERefSys.h"
#include "VecProc.h"
#include "lhpVMEDefines.h"

//----------------------------------------------------------------------------
// class fowarding:
//----------------------------------------------------------------------------
class vtkArrowSource;
class vtkTransformPolyDataFilter;
class vtkTransform;
class vtkAppendPolyData;
class vtkPolyData;
class mafVMELandmarkCloud;

//----------------------------------------------------------------------------
// class mafVMEAFRefSys
//----------------------------------------------------------------------------
class LHP_VME_EXPORT mafVMEAFRefSys : public mafVME
{
public:
  mafTypeMacro(mafVMEAFRefSys,mafVME);

  /** print a dump of this object */
  virtual void Print(std::ostream& os, const int tabs=0);

  enum AFREFSYS_WIDGET_ID
  {
    ID_NAME_REF_SYS = Superclass::ID_LAST,
    ID_SCALE_FACTOR,
    ID_ACTIVE,
    ID_PRINT,
    ID_X_OFFSET,
    ID_Y_OFFSET,
    ID_Z_OFFSET,
    ID_X_ROTATE,
    ID_Y_ROTATE,
    ID_Z_ROTATE,
    ID_SELECT_BONEID,
    ID_FIRSTDYN,
    ID_LAST = ID_FIRSTDYN + 100
  };
  enum AFREFSYS_BONEID
  {
    ID_AFS_NOTDEFINED,
    ID_AFS_FIRST,
    ID_AFS_PELVIS = ID_AFS_FIRST,
    ID_AFS_RTHIGH,
    ID_AFS_LTHIGH,
    ID_AFS_RSHANK,
    ID_AFS_LSHANK,
    ID_AFS_RFOOT,
    ID_AFS_LFOOT,
    ID_AFS_LAST
  };

  static bool LandmarkAccept(mafNode *node) {return(node != NULL && node->IsMAFType(mafVMELandmark));};

  /** Precess events coming from other objects */ 
  virtual void OnEvent(mafEventBase *maf_event);

  /** Copy the contents of another VME-RefSys into this one. */
  virtual int DeepCopy(mafNode *a);

  /** Compare with another VME-RefSys. */
  virtual bool Equals(mafVME *vme);

  /** Used to change the axes size */
  void SetScaleFactor(double scale);

  /** Return the axes size */
  double GetScaleFactor();

  /** return the right type of output */  
  mafVMEOutputSurface *GetSurfaceOutput();

  /**
  Set the Pose matrix of the VME. This function modifies the MatrixVector. You can
  set or get the Pose for a specified time. When setting, if the time does not exist
  the MatrixVector creates a new KeyMatrix on the fly. When getting, the matrix vector
  interpolates on the fly according to the matrix interpolator.*/
  virtual void SetMatrix(const mafMatrix &mat);

  /**
  Return the list of timestamps for this VME. Timestamps list is 
  obtained merging timestamps for matrixes and VME items*/
  virtual void GetLocalTimeStamps(std::vector<mafTimeStamp> &kframes);

  /** return always false since (currently) the slicer is not an animated VME (position 
  is the same for all timestamps). */
  virtual bool IsAnimated();

  /** Return pointer to material attribute. */
  mmaMaterial *GetMaterial();

  /** return an xpm-icon that can be used to represent this node */
  static char ** GetIcon();

  /** Return the suggested pipe-typename for the visualization of this vme */
  virtual mafString GetVisualPipe() {return mafString("mafPipeSurface");};

  /** 
  Set links for the ref-sys*/
  void SetRefSysLink(const char *link_name, mafNode *n);

  /** 
  Set Vector virtual machine*/
  void SetVM(VecManVM<double> *vm){m_vm = vm;}

  /** 
  Calculate global matrix for given timestamp*/
  void CalculateMatrix(mafMatrix& mat, mafTimeStamp ts);

  /** 
  Load vector virtual machine script from file*/
  void LoadScriptFromFile(const mafString& filename);

  /** 
  Set vector virtual machine script*/
  void SetScriptText(const std::vector<mafString>& script);

  /** 
  Return current activity state*/
  int  GetActive()const {return m_Active;}

  /** 
  Set current activity state*/
  void SetActive(int active);

  /** 
  Return current bone ID*/
  int  GetBoneID()const {return m_BoneID;}

  /** 
  Set current bone ID*/
  void SetBoneID(int ID);

  /** 
  Return vector for given name and timestamp from vector virtual machine*/
  bool GetVector(const char *name, mafTimeStamp ts, V3d<double>& output);

  /** 
  Return scalar for given name and timestamp from vector virtual machine*/
  bool GetScalar(const char *name, mafTimeStamp ts,     double&  output);

protected:

  /** 
  Update vector virtual machine with given timestamp*/
  bool UpdateVM(mafTimeStamp ts);

  /** 
  Rebuild vector virtual machine from text*/
  bool ConvertTextToVM(bool buildMapping);

  /** 
  Selection control function for dialog*/
  static bool AcceptLandmark(mafNode *node);

  mafVMEAFRefSys();
  virtual ~mafVMEAFRefSys();

  virtual int InternalStore(mafStorageElement *parent);
  virtual int InternalRestore(mafStorageElement *node);

  /** called to prepare the update of the output */
  virtual void InternalPreUpdate();

  /** update the output data structure */
  virtual void InternalUpdate();

  /** used to initialize and create the material attribute if not yet present */
  virtual int InternalInitialize();

  void     SetTransf(double x, double y, double z, double xr, double yr, double zr);
  void     GetTransf(double &x, double &y, double &z, double &xr, double &yr, double &zr);


  /** Internally used to create a new instance of the GUI.*/
  virtual mafGUI *CreateGui();

  vtkArrowSource                                      *m_XArrow;
  vtkArrowSource                                      *m_YArrow;
  vtkArrowSource                                      *m_ZArrow;

  vtkTransformPolyDataFilter                          *m_XAxis;
  vtkTransform                                        *m_XAxisTransform;

  vtkTransformPolyDataFilter                          *m_YAxis;
  vtkTransform                                        *m_YAxisTransform;

  vtkTransformPolyDataFilter                          *m_ZAxis;
  vtkTransform                                        *m_ZAxisTransform;

  vtkTransformPolyDataFilter                          *m_ScaleAxis;
  vtkTransform                                        *m_ScaleAxisTransform;

  vtkAppendPolyData                                   *m_Axes;

  double                                              m_ScaleFactor;

  mafTransform                                        *m_Transform; ///< pose matrix for the slicer plane

  double                                              m_XOffset, m_YOffset, m_ZOffset;
  double                                              m_XRotate, m_YRotate, m_ZRotate;
  VecManVM<double>                                    *m_vm;
  std::vector<mafString>                              m_scriptText;
  int                                                 m_BoneID;

  int                                                 m_Active;
  std::map<mafString, mafString>                      m_lmMapping;
  std::map<int, mafString>                            m_buttonMapping;
  int                                                 m_textSize;
  bool                                                m_VMValid;
  mafTimeStamp                                        m_VMTime;

private:
  mafVMEAFRefSys(const mafVMERefSys&); // Not implemented
  void operator=(const mafVMERefSys&); // Not implemented
};

#endif
