/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoBuildHierarchy.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev
==========================================================================
  Copyright (c) 2002/2005
  ULB - Free University of Brussels (www.ulb.ac.be)
=========================================================================*/
#ifndef __mmoBuildHierarchy_H__
#define __mmoBuildHierarchy_H__

//----------------------------------------------------------------------------
// Includes:
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "mafVME.h"
#include "lhpOperationsDefines.h"
#include <vector>

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafGui;
class mafEvent;
class mafIntGraphHyer;
class mafVMELandmarkCloud;
class vtkPoints;

//----------------------------------------------------------------------------
// mmoRefSys :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT mmoBuildHierarchy: public mafOp
{
public:
  class mafFrame
  {
  protected:
    /// actual VME it points to
    mafVME   *m_vme;
    mafVME   *m_parentVME;
    /// name of frame in hierarchy file
    wxString *m_name; 
    //identification mark
    //mafBonesList m_IDname; 
    /// relations
    mafFrame *m_parent;
    /// relations
    mafFrame *m_child;
    /// relations: one the same level with same parents
    mafFrame *m_next;
  public:
    mafFrame();
    ~mafFrame();
    mafVME *GetVME() {return m_vme;}
    void SetVME(mafVME *cloud) {m_vme = cloud;}
    mafVME *GetParentVME() {return m_parentVME;}
    void SetParentVME(mafVME *cloud) {m_parentVME = cloud;}
    mafFrame *GetParent() {return m_parent;}
    void SetParent(mafFrame *parent) {m_parent = parent;}
    mafFrame *GetChild() {return m_child;}
    void SetChild(mafFrame *child) {m_child = child;}
    mafFrame *GetNext() {return m_next;}
    void SetNext(mafFrame *next) {m_next = next;}
    wxString *GetName(void) {return m_name;}
    void      SetName(wxString const *str) {delete m_name; m_name = new wxString(*str);}
  };

  mmoBuildHierarchy(wxString label = "BuildHierarchy");
 ~mmoBuildHierarchy(); 

  virtual void OnEvent(mafEventBase *maf_event);
  mafOp* Copy();

  bool Accept(mafNode* vme);   
  void OpRun();
  void OpDo();
  void OpUndo();
  void CreateGui();

  /** Load links from file. */
  bool  ReadFromFile(const wxString &sFName);  

protected: 

  //void OpStop(int result);

  /// limb cloud from motion: animated, we will insert stick tip here
  mafVMELandmarkCloud  *m_LimbCloud;
  mafString            m_DictionaryFName;
  mafString            m_HierarchyFName;

  mafFrame             *m_root;
  std::vector<std::pair<wxString, wxString> > m_dictionary;

private:
  /** Destroy entire tree */
  void  Destroy(mafFrame **root);
  /** Destroy entire tree */
  void  BindToVME(mafVME *pvme, mafFrame *pStart = NULL);
  /** just see name */
  //static void  ExtractTwoWordsFromString(wxString &str, wxString &one, wxString &two); 
  /** Find frame with given name */
  mafFrame *FindFrame(wxString const &str, bool bCreateIfNotFound = FALSE);
  mafFrame *FindFrame(mafFrame *pRoot, wxString const &str);
  mafFrame *FindFrameUsingDictionary(wxString const &str, bool bCreateIfNotFound = FALSE);
};
#endif
