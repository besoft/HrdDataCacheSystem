/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpBonematCaller.h,v $
  Language:  C++
  Date:      $Date: 2014-10-07 12:33:41 $
  Version:   $Revision: 1.1.1.1.2.2 $
  Authors:   Gianluigi Crimi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpBonematCaller_H__
#define __lhpOpBonematCaller_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "lhpOpBonematCommon.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// lhpOpBonematCaller :
//----------------------------------------------------------------------------
class LHP_OPERATIONS_EXPORT lhpOpBonematCaller: public lhpOpBonematCommon
{
  public:

    lhpOpBonematCaller(wxString label);
    ~lhpOpBonematCaller(); 

    mafOp* Copy();
    void OpRun();
    int Execute();

  protected:
        
    wxString m_bonematCalleePath;

		mafVMEMesh *m_AbsMesh;
		boolean m_NewAbsMeshGenerated;
		mafMatrix m_MeshVolumeAlignMatrix;

		

		void CreateAbsMesh();
		void DeleteAbsMesh();
		void TransformOutput();
		
    int ExecuteBonematCallee();
    int CheckBonematCompatibility();

    void OpenUrl(wxString &url);
};

#endif
