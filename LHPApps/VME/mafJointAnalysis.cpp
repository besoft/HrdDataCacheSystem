/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafJointAnalysis.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:52 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "mafJointAnalysis.h"


mafVMEAFRefSys *GetRefSys(mafVME *vme)
{
  mafVMEAFRefSys *afs = NULL;
  mafVMELandmarkCloud *lmc = mafVMELandmarkCloud::SafeDownCast(vme);
  if(lmc == NULL)
    return NULL;
  for(int i = 0; i < lmc->GetNumberOfChildren(); i++)
  {
    mafNode *child = vme->GetChild(i);
    if(child->IsA("mafVMEAFRefSys"))
    {
      mafVMEAFRefSys *rs = mafVMEAFRefSys::SafeDownCast(child);
      if(rs->GetActive())
      {
        afs = rs;
        break;
      }
    }
  }
  return afs;
}

//----------------------------------------------------------------------------
void GetGlobalMatrix(mafVME *vme, mafTimeStamp ts, DiMatrix *pMat)
//----------------------------------------------------------------------------
{
  DiMatrixIdentity(pMat);
  if(vme == NULL)
    return;

  mafMatrix      matrix;
  mafVMEAFRefSys *afs = GetRefSys(vme);
  if(afs == NULL)
    vme->GetOutput()->GetAbsMatrix(matrix, ts);
  else
    afs->CalculateMatrix(matrix, ts);
  mflMatrixToDi(matrix.GetVTKMatrix(), pMat);
}

//----------------------------------------------------------------------------
void GetLocalMatrix(mafVME *vme, mafTimeStamp ts, DiMatrix *pMat)
//----------------------------------------------------------------------------
{
  DiMatrix pmatrix;
  DiMatrix cmatrix;
  DiMatrix pInv;

  DiMatrixIdentity(pMat);
  if(vme == NULL)
    return;

  GetGlobalMatrix(vme->GetParent(), ts, &pmatrix);//in case of GetParent == NULL Global matrix is filled as identity
  GetGlobalMatrix(vme,              ts, &cmatrix);

  DiMatrixInvert(&pmatrix, &pInv);
  DiMatrixMultiply(&cmatrix, &pInv, pMat);
}


int FindParentID(int id)
{
  switch(id)
  {
  case mafVMEAFRefSys::ID_AFS_LTHIGH:
  case mafVMEAFRefSys::ID_AFS_RTHIGH:
    return mafVMEAFRefSys::ID_AFS_PELVIS;
  case mafVMEAFRefSys::ID_AFS_LSHANK:
    return mafVMEAFRefSys::ID_AFS_LTHIGH;
  case mafVMEAFRefSys::ID_AFS_RSHANK:
    return mafVMEAFRefSys::ID_AFS_RTHIGH;
  case mafVMEAFRefSys::ID_AFS_LFOOT:
    return mafVMEAFRefSys::ID_AFS_LSHANK;
  case mafVMEAFRefSys::ID_AFS_RFOOT:
    return mafVMEAFRefSys::ID_AFS_RSHANK;
  default:
    return mafVMEAFRefSys::ID_AFS_NOTDEFINED;
  }
}
void OVP_GES(mafVME *vme, mafTimeStamp ts, mafTimeStamp tsRef, DiV4d *vOVPPos, DiV4d *vOVPRot, DiV4d *vGESPos, DiV4d *vGESRot)
{
  DiV4d vOVPPosInt;
  DiV4d vOVPRotInt;
  DiV4d vGESPosInt;
  DiV4d vGESRotInt;

  DiV4d *vOVPPosOut = (vOVPPos == NULL) ? &vOVPPosInt : vOVPPos;
  DiV4d *vOVPRotOut = (vOVPRot == NULL) ? &vOVPRotInt : vOVPRot;
  DiV4d *vGESPosOut = (vGESPos == NULL) ? &vGESPosInt : vGESPos;
  DiV4d *vGESRotOut = (vGESRot == NULL) ? &vGESRotInt : vGESRot;
  vOVPPosOut->x = 0.0;vOVPPosOut->y = 0.0;vOVPPosOut->z = 0.0;vOVPPosOut->w = 0.0;
  vOVPRotOut->x = 0.0;vOVPRotOut->y = 0.0;vOVPRotOut->z = 0.0;vOVPRotOut->w = 0.0;
  vGESPosOut->x = 0.0;vGESPosOut->y = 0.0;vGESPosOut->z = 0.0;vGESPosOut->w = 0.0;
  vGESRotOut->x = 0.0;vGESRotOut->y = 0.0;vGESRotOut->z = 0.0;vGESRotOut->w = 0.0;
  if(vme == NULL)
    return;

  DiMatrix mLTM;
  DiMatrix mRefLTM;
  DiV4d    vTm;
  DiV4d    vOVPRefPos;
  GetLocalMatrix(vme, ts,    &mLTM);
  GetLocalMatrix(vme, tsRef, &mRefLTM);
  mafTransfInverseTransformUpright(&mLTM, &vTm, vOVPRotOut);

  mafVMEAFRefSys *vmeSys = GetRefSys(vme);
  if(vmeSys == NULL || vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_NOTDEFINED)
    return;
  if(vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_PELVIS)
  {
    DiV4dCopy(&vTm, vOVPPosOut);
    DiV4dCopy(vOVPRotOut, vGESRotOut);
    DiV4dCopy(vOVPPosOut, vGESPosOut);
    return;
  }
  mafVME *parent = vme->GetParent();
  if(parent == NULL)
    return;
  mafVMEAFRefSys *parentSys = GetRefSys(parent);
  if(parentSys == NULL || parentSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_NOTDEFINED || parentSys->GetBoneID() != FindParentID(vmeSys->GetBoneID()))
    return;

  mafMatrix vmeMatr;
  mafMatrix vmeMatrRef;
  mafMatrix parentMatr;
  mafMatrix parentMatrRef;
  vmeSys->CalculateMatrix(vmeMatr, ts);
  vmeSys->CalculateMatrix(vmeMatrRef, tsRef);
  parentSys->CalculateMatrix(parentMatr, ts);
  parentSys->CalculateMatrix(parentMatrRef, tsRef);
  DiMatrix vmeMatrix;
  DiMatrix vmeMatrixRef;
  DiMatrix parentMatrix;
  DiMatrix parentMatrixRef;
  mflMatrixToDi(vmeMatr.GetVTKMatrix(), &vmeMatrix);
  mflMatrixToDi(vmeMatrRef.GetVTKMatrix(), &vmeMatrixRef);
  mflMatrixToDi(parentMatr.GetVTKMatrix(), &parentMatrix);
  mflMatrixToDi(parentMatrRef.GetVTKMatrix(), &parentMatrixRef);
  if(vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_LTHIGH || vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_RTHIGH)
  {
    V3d<double> pelvisPnt;
    V3d<double> thighPnt;
    if(vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_RTHIGH)
    {
      if(!parentSys->GetVector("RIAC", tsRef, pelvisPnt) || !vmeSys->GetVector("RFCH", tsRef, thighPnt))
        return;
    }
    else
    {
      if(!parentSys->GetVector("LIAC", tsRef, pelvisPnt) || !vmeSys->GetVector("LFCH", tsRef, thighPnt))
        return;
    }

    DiV4d vPlv, vThg, vTmp, vDist, vProx;
    vPlv.x = -pelvisPnt.x;vPlv.y =  pelvisPnt.y;vPlv.z =  pelvisPnt.z;vPlv.w =  1.0;
    vThg.x = -thighPnt.x; vThg.y =  thighPnt.y; vThg.z =  thighPnt.z; vThg.w =  1.0;
    DiV4dInverseTransformPointTo(&vThg, &vmeMatrixRef, &vTmp);
    DiV4dTransformPointTo(&vTmp, &vmeMatrix, &vDist);

    DiV4dInverseTransformPointTo(&vPlv, &parentMatrixRef, &vTmp);
    DiV4dTransformPointTo(&vTmp, &parentMatrix, &vProx);

    DiV4dSub(&vDist, &vProx, &vTmp);
    DiV4dInverseTransformVectorTo(&vTmp, &parentMatrix, vOVPPosOut);

    DiV4dSub(&vThg, &vPlv, &vTmp);
    DiV4dInverseTransformVectorTo(&vTmp, &parentMatrix, &vOVPRefPos);
  }
  else
  {
    V3d<double> middle;

    middle.x = parentMatrRef.GetVTKMatrix()->GetElement(0, 3);
    middle.y = parentMatrRef.GetVTKMatrix()->GetElement(1, 3);
    middle.z = parentMatrRef.GetVTKMatrix()->GetElement(2, 3);
    
    //if(!parentSys->GetVector("MIDDLE", tsRef, middle))
    //  return;
    DiV4d vMdl, vTmp, vDist, vProx;
    vMdl.x = -middle.x;vMdl.y =  middle.y;vMdl.z =  middle.z;vMdl.w =  1.0;

    DiV4dInverseTransformPointTo(&vMdl, &vmeMatrixRef, &vTmp);
    DiV4dTransformPointTo(&vTmp, &vmeMatrix, &vDist);

    DiV4dInverseTransformPointTo(&vMdl, &parentMatrixRef, &vTmp);
    DiV4dTransformPointTo(&vTmp, &parentMatrix, &vProx);

    DiV4dSub(&vDist, &vProx, &vTmp);
    DiV4dInverseTransformVectorTo(&vTmp, &parentMatrix, vOVPPosOut);

    vOVPRefPos.x = 0.0;vOVPRefPos.y = 0.0;vOVPRefPos.z = 0.0;vOVPRefPos.w = 1.0;
  }
  vOVPRefPos.x  = -vOVPRefPos.x;
  vOVPPosOut->x = -vOVPPosOut->x;

  //GES
  {
    DiMatrix mPGTM;
    DiMatrix mDLTM;
    DiMatrix mDURLTM;
    DiMatrix mDURLTMInv;
    DiMatrix mDGTM;
    DiMatrix mDGTMInv;
    DiMatrix mGSBasic;
    DiMatrix mInitRotX;
    DiMatrix mInitRotY;
    DiMatrix mInitRot;
    DiV4d    vGS, vGSPos;
    DiFloat  rAngleX = 0.f  * diPI / 180.f;
    DiFloat  rAngleY = 11.f * diPI / 180.f;

    DiMatrixIdentity(&mInitRotX);
    DiMatrixIdentity(&mInitRotY);
    DiMatrixRotateRight(&mInitRotX, rAngleX, diREPLACE);
    DiMatrixRotateUp(&mInitRotY, rAngleY, diREPLACE);
    DiMatrixMultiply(&mInitRotX, &mInitRotY, &mInitRot);

    DiMatrixIdentity(&mPGTM);
    DiMatrixMultiply(&mLTM, &mInitRot, &mDLTM);
    DiMatrixCopy(&mRefLTM, &mDURLTM);
    DiMatrixTransposeRotationalSubmatrix(&mDURLTM, &mDURLTMInv);
    DiMatrixMultiply(&mDURLTMInv, &mDLTM, &mDGTMInv);
    mafTransfRightLeftConv(&mDGTMInv, &mDGTM);

    mafTransfMatrixToGES(&mPGTM, &mDGTM, &mGSBasic, &vGS);

    if(vmeSys->GetBoneID() != mafVMEAFRefSys::ID_AFS_NOTDEFINED && vmeSys->GetBoneID() != mafVMEAFRefSys::ID_AFS_PELVIS)//applying additional 
    {
      DiV4dSub(vOVPPosOut, &vOVPRefPos, &vGSPos);
      DiV4dTransformVectorTo(&vGSPos, &mGSBasic, &vGSPos);
      DiV4dCopy(&vGSPos, vGESPosOut);
    }
    DiV4dCopy(&vGS, vGESRotOut);
    if(vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_LTHIGH ||
      vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_LSHANK ||
      vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_LFOOT  )
    {
      vGESRotOut->y = -vGESRotOut->y;
      vGESRotOut->x = -vGESRotOut->x;
    }
    if(vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_RSHANK ||
      vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_LSHANK )
    {
      vGESRotOut->z = -vGESRotOut->z;
    }
  }
}


void SetOVP(mafVME *vme, mafTimeStamp ts, mafTimeStamp tsRef, DiV4d *vOVPPos, DiV4d *vOVPRot)
{
  if(vme == NULL)
    return;

  mafMatrix vmeNodeLMatr;
  mafMatrix vmeNodeGMatr;
  vme->GetOutput()->GetMatrix(vmeNodeLMatr, ts);
  vme->GetOutput()->GetAbsMatrix(vmeNodeGMatr, ts);

  DiMatrix vmeNodeLMatrix;
  DiMatrix vmeNodeGMatrix;
  mflMatrixToDi(vmeNodeLMatr.GetVTKMatrix(), &vmeNodeLMatrix);
  mflMatrixToDi(vmeNodeGMatr.GetVTKMatrix(), &vmeNodeGMatrix);

  DiMatrix mLTM;
  DiMatrix mLTMTrg;
  DiMatrix mRefLTM;
  DiMatrix mGTM;

  DiMatrix mAFCL;
  DiMatrix mAFCLInv;
  DiMatrix mTmp;

  GetLocalMatrix(vme,  ts,    &mLTM);
  GetGlobalMatrix(vme, ts,    &mGTM);
  GetLocalMatrix(vme,  tsRef, &mRefLTM);

  //mafTransfInverseTransformUpright(&mLTM, &vTm, vOVPRotOut);
  DiV4d    vPosComp, vRotComp;
  //vPosComp.x = 0.0;vPosComp.y = 0.0;vPosComp.z = 0.0;vPosComp.w = 1.0;
  vPosComp.x = vOVPPos->x;vPosComp.y = vOVPPos->y;vPosComp.z = vOVPPos->z;vPosComp.w = 1.0;
  vRotComp.x = vOVPRot->x;vRotComp.y = vOVPRot->y;vRotComp.z = vOVPRot->z;vRotComp.w = 1.0;
  mafTransfTransformUpright(&vPosComp, &vRotComp, &mLTMTrg);
  
  DiMatrix mOVPMatrix;
  DiMatrix mOVPMatrixTrg;
  DiMatrix mDelta;

  DiMatrixInvert(&vmeNodeGMatrix, &mTmp);
  DiMatrixMultiply(&mGTM, &mTmp, &mAFCL);
  DiMatrixInvert(&mAFCL, &mAFCLInv);

  DiMatrixMultiply(&mAFCLInv, &mLTM,    &mOVPMatrix);
  DiMatrixMultiply(&mAFCLInv, &mLTMTrg, &mOVPMatrixTrg);
  DiMatrixInvert(&mOVPMatrix, &mTmp);
  DiMatrixMultiply(&mOVPMatrixTrg, &mTmp, &mDelta);

  DiMatrixCopy(&vmeNodeLMatrix, &mTmp);
  DiMatrixMultiply(&mDelta, &mTmp, &vmeNodeLMatrix);
  DiMatrixToVTK(&vmeNodeLMatrix, vmeNodeLMatr.GetVTKMatrix());
  vme->SetPose(vmeNodeLMatr, ts);

  mafVMEAFRefSys *vmeSys = GetRefSys(vme);
  if(vmeSys == NULL || vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_NOTDEFINED)
    return;
  if(vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_PELVIS)
  {
    return;
  }
  mafVME *parent = vme->GetParent();
  if(parent == NULL)
    return;
  mafVMEAFRefSys *parentSys = GetRefSys(parent);
  if(parentSys == NULL || parentSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_NOTDEFINED || parentSys->GetBoneID() != FindParentID(vmeSys->GetBoneID()))
    return;

  mafMatrix parNodeLMatr;
  mafMatrix parNodeGMatr;

  vme->GetOutput()->GetMatrix(vmeNodeLMatr, ts);
  vme->GetOutput()->GetAbsMatrix(vmeNodeGMatr, ts);
  parent->GetOutput()->GetMatrix(parNodeLMatr, ts);
  parent->GetOutput()->GetAbsMatrix(parNodeGMatr, ts);
  mafMatrix vmeMatr;
  mafMatrix vmeMatrRef;
  mafMatrix parentMatr;
  mafMatrix parentMatrRef;
  vmeSys->CalculateMatrix(vmeMatr, ts);
  vmeSys->CalculateMatrix(vmeMatrRef, tsRef);
  parentSys->CalculateMatrix(parentMatr, ts);
  parentSys->CalculateMatrix(parentMatrRef, tsRef);
  DiMatrix vmeMatrix;
  DiMatrix vmeMatrixRef;
  DiMatrix parentMatrix;
  DiMatrix parentMatrixRef;
  DiMatrix parNodeGMatrix;
  mflMatrixToDi(vmeMatr.GetVTKMatrix(), &vmeMatrix);
  mflMatrixToDi(vmeMatrRef.GetVTKMatrix(), &vmeMatrixRef);
  mflMatrixToDi(parentMatr.GetVTKMatrix(), &parentMatrix);
  mflMatrixToDi(parentMatrRef.GetVTKMatrix(), &parentMatrixRef);
  mflMatrixToDi(vmeNodeLMatr.GetVTKMatrix(), &vmeNodeLMatrix);
  mflMatrixToDi(vmeNodeGMatr.GetVTKMatrix(), &vmeNodeGMatrix);
  mflMatrixToDi(parNodeGMatr.GetVTKMatrix(), &parNodeGMatrix);

  DiV4d vOVPPosTrg;
  vOVPPosTrg.x = -vOVPPos->x;vOVPPosTrg.y = vOVPPos->y;vOVPPosTrg.z = vOVPPos->z;vOVPPosTrg.w = 1.0;

  if(vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_LTHIGH || vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_RTHIGH)
  {
    V3d<double> pelvisPnt;
    V3d<double> thighPnt;
    if(vmeSys->GetBoneID() == mafVMEAFRefSys::ID_AFS_RTHIGH)
    {
      if(!parentSys->GetVector("RIAC", tsRef, pelvisPnt) || !vmeSys->GetVector("RFCH", tsRef, thighPnt))
        return;
    }
    else
    {
      if(!parentSys->GetVector("LIAC", tsRef, pelvisPnt) || !vmeSys->GetVector("LFCH", tsRef, thighPnt))
        return;
    }

    DiV4d vPlv, vThg, vTmp, vDist, vDistCur, vDelta, vProx;
    vPlv.x = -pelvisPnt.x;vPlv.y =  pelvisPnt.y;vPlv.z =  pelvisPnt.z;vPlv.w =  1.0;
    vThg.x = -thighPnt.x; vThg.y =  thighPnt.y; vThg.z =  thighPnt.z; vThg.w =  1.0;
    DiV4dInverseTransformPointTo(&vThg, &vmeMatrixRef, &vTmp);
    DiV4dTransformPointTo(&vTmp, &vmeMatrix, &vDistCur);

    DiV4dInverseTransformPointTo(&vPlv, &parentMatrixRef, &vTmp);
    DiV4dTransformPointTo(&vTmp, &parentMatrix, &vProx);



    DiV4dTransformVectorTo(&vOVPPosTrg, &parentMatrix, &vTmp);
    DiV4dAdd(&vTmp, &vProx, &vDist);
    DiV4dSub(&vDist, &vDistCur, &vTmp);
    DiV4dInverseTransformVectorTo(&vTmp, &parNodeGMatrix, &vDelta);
    DiV4dCopy(&vmeNodeLMatrix.vPos, &vTmp);
    DiV4dAdd(&vTmp, &vDelta, &vmeNodeLMatrix.vPos);
    DiMatrixToVTK(&vmeNodeLMatrix, vmeNodeLMatr.GetVTKMatrix());
    vme->SetPose(vmeNodeLMatr, ts);
  }
  else
  {
    V3d<double> middle;

    middle.x = parentMatrRef.GetVTKMatrix()->GetElement(0, 3);
    middle.y = parentMatrRef.GetVTKMatrix()->GetElement(1, 3);
    middle.z = parentMatrRef.GetVTKMatrix()->GetElement(2, 3);

    //if(!parentSys->GetVector("MIDDLE", tsRef, middle))
    //  return;
    DiV4d vMdl, vTmp, vDist, vDistCur, vDelta, vProx;
    vMdl.x = -middle.x;vMdl.y =  middle.y;vMdl.z =  middle.z;vMdl.w =  1.0;

    DiV4dInverseTransformPointTo(&vMdl, &vmeMatrixRef, &vTmp);
    DiV4dTransformPointTo(&vTmp, &vmeMatrix, &vDistCur);

    DiV4dInverseTransformPointTo(&vMdl, &parentMatrixRef, &vTmp);
    DiV4dTransformPointTo(&vTmp, &parentMatrix, &vProx);

    DiV4dTransformVectorTo(&vOVPPosTrg, &parentMatrix, &vTmp);
    DiV4dAdd(&vTmp, &vProx, &vDist);
    DiV4dSub(&vDist, &vDistCur, &vTmp);
    DiV4dInverseTransformVectorTo(&vTmp, &parNodeGMatrix, &vDelta);
    DiV4dCopy(&vmeNodeLMatrix.vPos, &vTmp);
    DiV4dAdd(&vTmp, &vDelta, &vmeNodeLMatrix.vPos);
    DiMatrixToVTK(&vmeNodeLMatrix, vmeNodeLMatr.GetVTKMatrix());
    vme->SetPose(vmeNodeLMatr, ts);
  }
}