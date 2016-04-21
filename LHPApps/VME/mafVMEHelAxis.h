/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafVMEHelAxis.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:57 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __mafVMEHelAxis_h
#define __mafVMEHelAxis_h

//-----------------------------------------------------------------------
// Includes:
//-----------------------------------------------------------------------
#include "mafVME.h"
#include "mafVmeOutput.h"
#include "mafVMEGeneric.h"
#include "mafVMERefSys.h"
#include "lhpVMEDefines.h"

//-----------------------------------------------------------------------
// class forwarding
//-----------------------------------------------------------------------
class vtkArrowSource;
class vtkTransformPolyDataFilter;
class vtkTransform;
class vtkAppendPolyData;
class vtkPolyData;

//-----------------------------------------------------------------------
// class mafVMEHelAxis
//-----------------------------------------------------------------------
class LHP_VME_EXPORT mafVMEHelAxis : public mafVME
{
public:
  mafTypeMacro(mafVMEHelAxis,mafVME);

  enum 
  {
    ID_NAME_REF_SYS = Superclass::ID_LAST,
    ID_SCALE_FACTOR,
    ID_PRINT,
    ID_LAST
  };

  /** print a dump of this object */
  virtual void Print(std::ostream& os, const int tabs=0);// const;
  bool     CanReparentTo(mafNode *parent) {return GetParent() == NULL || parent == NULL;}

  void     OnEvent(mafEventBase *maf_event);
  /** Return the suggested pipe-typename for the visualization of this vme */
  virtual mafString GetVisualPipe() {return mafString("mafPipeSurface");};
  /** Return pointer to material attribute. */
  mmaMaterial *GetMaterial();

  /** return an xpm-icon that can be used to represent this node */
  static char ** GetIcon();

  /** Used to change the axes size */
  void SetScaleFactor(double scale);

  /** Return the axes size */
  double GetScaleFactor();

  /**
  Set the Pose matrix of the VME. This function modifies the MatrixVector. You can
  set or get the Pose for a specified time. When setting, if the time does not exist
  the MatrixVector creates a new KeyMatrix on the fly. When getting, the matrix vector
  interpolates on the fly according to the matrix interpolator.*/
  virtual void SetMatrix(const mafMatrix &mat);

  /**
  Return the list of timestamps for this VME. Timestamps list is 
  obtained merging timestamps for matrixes and VME items*/
  virtual void GetLocalTimeStamps(std::vector<mafTimeStamp> &kframes){kframes.clear();}

protected:
  mafVMEHelAxis();
  virtual ~mafVMEHelAxis();

  virtual int InternalStore(mafStorageElement *parent);
  virtual int InternalRestore(mafStorageElement *node);

  mafGUI *CreateGui();
  /** called to prepare the update of the output */
  virtual void InternalPreUpdate();
  /** used to initialize and create the material attribute if not yet present */
  virtual int InternalInitialize();

  /** update the output data structure */
  virtual void InternalUpdate();

  /** update the output data structure */
  void UpdateCS();

  vtkArrowSource             *m_ZArrow;

  vtkTransformPolyDataFilter *m_ZAxis;
  vtkTransform               *m_ZAxisTransform;

  vtkTransformPolyDataFilter *m_ScaleAxis;
  vtkTransform               *m_ScaleAxisTransform;

  vtkAppendPolyData          *m_Axes;

  double                     m_ScaleFactor;
  double                     m_AngleFactor;

  mafTransform               *m_Transform; ///< pose matrix for the slicer plane
private:
  mafVMEHelAxis (const mafVMEHelAxis &); // Not implemented
  void operator=(const mafVMEHelAxis &); // Not implemented
};
#endif
