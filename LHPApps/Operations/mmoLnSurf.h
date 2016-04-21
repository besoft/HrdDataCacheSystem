/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoLnSurf.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __mmoLnSurf_H__
#define __mmoLnSurf_H__

//----------------------------------------------------------------------------
// Includes:
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVMESurface;
class mafVMEPolyline;

//----------------------------------------------------------------------------
// mmoRefSys :
//----------------------------------------------------------------------------
/** */
class LHP_OPERATIONS_EXPORT mmoLnSurf: public mafOp
{

public:
  mmoLnSurf(const wxString& label);
 ~mmoLnSurf(); 

  virtual void OnEvent(mafEventBase *maf_event);
  mafOp* Copy();

  bool Accept(mafNode* vme);   
  void OpRun();
  void OpDo();
  void OpUndo();

protected: 
  enum 
  {
    ID_RHO_SPL = MINID,
    ID_RHO_SRF,
    ID_SGM_SRF,
    ID_DIM_SPL,
    ID_DIMX_SRF,
    ID_DIMY_SRF,
    ID_GEN_LIST,
    ID_PARSE_NAME,
    ID_LAST
  };

private:
  double          m_rhoLine;
  double          m_rhoSurf;
  double          m_sgmSurf;
  int             m_splDim;
  int             m_xDim;
  int             m_yDim;
  int             m_generateLinesSurfaces;
  int             m_parseNames;
  mafVMESurface   *m_Surface;
  mafVMEPolyline  *m_Muscles;
  mafVMEPolyline  *m_Tendons;
};
#endif
