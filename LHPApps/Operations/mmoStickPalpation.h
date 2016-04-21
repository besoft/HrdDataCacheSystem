/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoStickPalpation.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __mmoStickPalpation_H__
#define __mmoStickPalpation_H__

//----------------------------------------------------------------------------
// Includes:
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class mafGui;
class mafEvent;
class mafVMELandmarkCloud;
class vtkPoints;
class vtkMatrix4x4;

//----------------------------------------------------------------------------
// mmoRefSys :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT mmoStickPalpation: public mafOp
{
public:
  mmoStickPalpation(const wxString& label = "StickPalpation");
 ~mmoStickPalpation(); 

  virtual void OnEvent(mafEventBase *maf_event);
  mafOp* Copy();

  bool Accept(mafNode* vme);   
  void OpRun();
  void OpDo();
  void OpUndo();
  void CreateGui();

protected: 
  void   ProcessSingleLM();

  typedef mafVME *(mmoStickPalpation::*MatchName)(mafVME  *pVME, const char *name) const;

  int    ExtractMatchingPoints(mafVMELandmarkCloud *src, mafVMELandmarkCloud *trg, mafTimeStamp srctime, mafTimeStamp trgtime);
  double RegisterPoints(vtkMatrix4x4 *res_matrix);


  void OpStop(int result);

  //----------------------------------------------------------------------------
  // widget id's
  //----------------------------------------------------------------------------
  enum 
  {
    ID_DEFAULT = MINID,
    ID_STICK_CALIBRATION,
    ID_STICK_CALIBRATION_NAME,
    ID_STICK_DEFINITION,
    ID_STICK_DEFINITION_NAME,
    ID_LIMB_CALIBRATION,
    ID_LIMB_CALIBRATION_NAME,
    ID_LIMB_CLOUD,
    ID_TRG_MOT,
    ID_LIMB_CLOUD_NAME,
    ID_LOAD_DICTIONARY,
    ID_LOAD_SCRIPT,
    ID_CLEAN_SCRIPT,
    ID_REG_TYPE,
    ID_LAST,
    ID_FORCED_DWORD = 0x7fffffff
  };


  ///stick calibration cloud: animated
  mafVMELandmarkCloud                          *m_StickCalibration;
  mafString                                    m_StickCalibrationName;
  ///stick definition cloud: unanimated 
  mafVMELandmarkCloud                          *m_StickDefinition;
  mafString                                    m_StickDefinitionName;
  ///libm cloud from calibration: animated
  mafVMELandmarkCloud                          *m_LimbCalibration;
  mafString                                    m_LimbCalibrationName;
  /// limb cloud from motion: animated, we will insert stick tip here
  mafVMELandmarkCloud                          *m_LimbCloud;
  mafString                                    m_LimbCloudName;
  //target motion, node that contains clouds as children
  mafVME                                       *m_TrgMotion;
  mafString                                    m_TrgMotionName;

  std::vector<std::pair<wxString, wxString> >  m_LMDict;
  mafString                                    m_ScriptFName;

  mafVMELandmarkCloud*                         m_Registered;
  double*                                      m_weight;       
  int                                          m_RegistrationMode; 

  //arrays for registration transform calculation
  vtkPoints                                    *m_pointsSource;
  vtkPoints                                    *m_pointsTarget;
  int                                          m_numPoints;
  ///see name
  mafString                                    m_DictionaryFName;

  void    GetLandmark(mafVMELandmarkCloud  *pStickCloud, int nIDx, double vpPoint[4], mafTimeStamp t = 0) const;
  /** get right landmarks from palpation stick with regards of dictionary */
  wxInt32 GetStickLandmarks(mafVMELandmarkCloud  *pStickCloud, double vpPoints[], mafTimeStamp t = 0) const;
  wxInt32 GetCloudLandmarks(mafVMELandmarkCloud  *pStickCloud, double vpPoints[], mafTimeStamp t = 0) const;

  /** attempt to match stick definition cloud with some sense */
  mafVME *MatchStickDefinition(mafVME *pVME, const char *name) const;
  /** attempt to match stick cloud with some sense */
  mafVME *MatchStick(mafVME *pVME, const char *name) const;
  /** attempt to match cloud with some sense to name*/
  mafVME *MatchWithName(mafVME *pVME, const char *name) const;
  /** Just traverse VME:) */
  mafVME *MatchCriterion(mafVME *pRoot, MatchName pCritFunc, const char *name) const;

  /**Just a misc. stuff*/
  void SetNodeName(mafVME *pVME, mafString *pName);

  ///work variables
  double                        m_LocalTip[4];
  wxString                      m_NewLandmarkName;

private:
  mutable  bool                                          m_WarnNamesNotMatched;
  /// index of new landmark for undo opration 
  wxInt32                                                m_NewIndex;
  std::vector<std::pair<mafVMELandmarkCloud*, wxInt32> > m_UndoList;
};
#endif
