/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafPlotMath.h,v $
  Language:  C++
  Date:      $Date: 2012-03-24 16:48:03 $
  Version:   $Revision: 1.1.1.1.2.3 $
  Authors:   Vladik Aranov
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __mafPlotMath_H__
#define __mafPlotMath_H__

#include "lhpCommonDefines.h"

#ifdef __GNUG__
    #pragma interface "mafIntGraphHyer.cpp"
#endif

#ifndef WX_PRECOMP
  #include "wx/wx.h"
#endif

#include "vtkMatrix4x4.h"

#define FLT_EPSILON     1.192092896e-07F        /* smallest such that 1.0+FLT_EPSILON != 1.0 */

#define _X_IDX                       0
#define _Y_IDX                       1
#define _Z_IDX                       2
#define _W_IDX                       3
#define _MATR_EL(xyzw,rua)           (*((&mpMat->vRight.x) + (sizeof(mpMat->vRight)/sizeof(DiFloat)) * (rua) + (xyzw)))

#define EulFrmS      0
#define EulFrmR      1
#define EulParEven   0
#define EulParOdd    1
#define EulSafe      "\000\001\002\000"
#define EulNext      "\001\002\000\001"
#define EulRepNo     0
#define EulRepYes    1
#define EulGetOrd(ord,i,j,k,h,n,s,f) {DiUInt32 o=DiF2L(ord);f=o&1;o>>=1;s=o&1;o>>=1;\
    n=o&1;o>>=1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}

// EulOrd creates an order value between 0 and 23 from 4-tuple choices.
#define EulOrd(i,p,r,f)    (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
#define _QUAT_COMP_ASSIGN(idx,val)   *((DiFloat *)qpQuat+(idx)) = (val)
#define EPSILON                     0.0004f

#define FPEqualTo(a,b)          (fabs((a)-(b)) < 1e-4)  
#define FPNotEqualTo(a,b)       (fabs((a)-(b)) > 1e-4)  


#define Sb(a)  (*((DiSplitBits *)(&(a))))

// Static axes 
#define EulOrdXYZs    EulOrd(_X_IDX,EulParEven,EulRepNo,EulFrmS)
#define EulOrdXYXs    EulOrd(_X_IDX,EulParEven,EulRepYes,EulFrmS)
#define EulOrdXZYs    EulOrd(_X_IDX,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdXZXs    EulOrd(_X_IDX,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdYZXs    EulOrd(_Y_IDX,EulParEven,EulRepNo,EulFrmS)
#define EulOrdYZYs    EulOrd(_Y_IDX,EulParEven,EulRepYes,EulFrmS)
#define EulOrdYXZs    EulOrd(_Y_IDX,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdYXYs    EulOrd(_Y_IDX,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdZXYs    EulOrd(_Z_IDX,EulParEven,EulRepNo,EulFrmS)
#define EulOrdZXZs    EulOrd(_Z_IDX,EulParEven,EulRepYes,EulFrmS)
#define EulOrdZYXs    EulOrd(_Z_IDX,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdZYZs    EulOrd(_Z_IDX,EulParOdd,EulRepYes,EulFrmS)

// Rotating axes 
#define EulOrdZYXr    EulOrd(_X_IDX,EulParEven,EulRepNo,EulFrmR)
#define EulOrdXYXr    EulOrd(_X_IDX,EulParEven,EulRepYes,EulFrmR)
#define EulOrdYZXr    EulOrd(_X_IDX,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdXZXr    EulOrd(_X_IDX,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdXZYr    EulOrd(_Y_IDX,EulParEven,EulRepNo,EulFrmR)
#define EulOrdYZYr    EulOrd(_Y_IDX,EulParEven,EulRepYes,EulFrmR)
#define EulOrdZXYr    EulOrd(_Y_IDX,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdYXYr    EulOrd(_Y_IDX,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdYXZr    EulOrd(_Z_IDX,EulParEven,EulRepNo,EulFrmR)
#define EulOrdZXZr    EulOrd(_Z_IDX,EulParEven,EulRepYes,EulFrmR)
#define EulOrdXYZr    EulOrd(_Z_IDX,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdZYZr    EulOrd(_Z_IDX,EulParOdd,EulRepYes,EulFrmR)

#define FLT_GARB         888.0f  
#define DiMax(a, b)      (((a) > (b))?(a):(b))
#define DiMin(a, b)      (((a) < (b))?(a):(b))
#define DiFAbs(a)        fabs(a)
#define DiSign(Value)    (((Value) < 0.f)? -1.0f:(((Value) > 0.f)?1.0f:0.0f))
#define DiRound(val)     (((val) - floor(val) > ceil(val) - (val)) ? ceil(val) : floor(val))
#define DiSqr(val)       ((val) * (val))
#define diPI             3.14159265358979323846f

#ifndef DIM
  #define DIM(a)  (sizeof((a)) / sizeof(*(a)))
#endif
#define DiF2L(val)       ((DiInt32)(val))
#define DiL2F(val)       ((float)(val))

#ifdef __cplusplus
template <bool test> struct Compiled_Time_Assertion_Failed;

template <> struct Compiled_Time_Assertion_Failed<true> 
{ 
  enum 
  { 
    value = 1 
  }; 
};

template <int x> struct DiCompileAssertTest
{
};

#define DICOMPILEASSERT(expression) \
  {                                       \
    typedef DiCompileAssertTest<sizeof(Compiled_Time_Assertion_Failed<(bool)(expression)>)> DiCompileAssertType; \
  }
#else
#define DICOMPILEASSERT(expression) ((void)0)
#endif

enum DiOpCombainType
{
  diREPLACE,
  diPRECONCAT,
  diPOSTCONCAT,
  di_OP_FORCEDWORD = 0x7FFFFFFF
};


//----------------------------------------------------------------------------
// Inline functions
//----------------------------------------------------------------------------

/**
Fits variable into diapason type regardless
*/
#define DiFFitIn(val, from, to)   DiMax(from, DiMin(val, to))

//----------------------------------------------------------------------------
// type definitions
//----------------------------------------------------------------------------

typedef void          DiVoid;
typedef bool          DiBool;
typedef unsigned int  DiUInt32;
typedef float         DiFloat;
typedef double        DiDouble;
typedef int           DiInt32;
typedef char          DiTChar;

//----------------------------------------------------------------------------
// types definitions
//----------------------------------------------------------------------------
struct DiV4d
{
  DiFloat x;
  DiFloat y;
  DiFloat z;
  DiFloat w;
};

struct DiV2d
{
  DiFloat x;
  DiFloat z;
};

struct DiMatrix
{
  DiV4d     vRight;
  DiV4d     vUp;
  DiV4d     vAt;
  DiV4d     vPos;
} ;

typedef DiV4d DiQuaternion;


typedef union TagDiSplitBits
{
  DiFloat           rFloat;
  volatile DiInt32  nInt;
  volatile DiUInt32 nUInt;
} DiSplitBits;



/**
 *This macro initalizes matrix.
 *@memo     Initalize matrix
 *@param    m      matrix
 *@param    t      init type
 */
#define DiMatrixInitialize(m)       \
{                                   \
  memset(m, 0, sizeof(DiMatrix)); \
}


//----------------------------------------------------------------------------
// external variables
//----------------------------------------------------------------------------
extern DiV4d const *vp4GZero;

//----------------------------------------------------------------------------
// forward references
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Functions references
//----------------------------------------------------------------------------
DiBool     mafTransfMirrorMatrixYOX(DiMatrix *mpIn, DiMatrix *mpOut);
LHP_COMMON_EXPORT DiBool mafTransfMatrixToEuler(DiMatrix const *mpR, DiV4d *vpR);
DiBool     mafTransfEulerToMatrix(DiV4d *vpRot, DiMatrix *mpMat);
DiBool     mafTransfEulerToQuaternion(DiV4d *vpRot, DiQuaternion *qpQuat);
DiVoid     mafTransfOrtoNormalizeMatrix(DiMatrix *mpMat, DiV4d const*vpRot, DiBool bLeftRight);
LHP_COMMON_EXPORT DiVoid mafTransfRightLeftConv(DiMatrix *mpSource, DiMatrix *mpDest);
LHP_COMMON_EXPORT DiVoid mafTransfMatrixToGES(DiMatrix *mpParent, DiMatrix *mpChild, DiMatrix *mpBasic, DiV4d *vpR);
void       mafAttVecToVTKMat(double ThetaIn[3], vtkMatrix4x4 *pMat);
LHP_COMMON_EXPORT DiBool mafTransfTransformUpright(DiV4d const*vpPos, DiV4d const*vpRot, DiMatrix *mpOut);
LHP_COMMON_EXPORT DiBool mafTransfInverseTransformUpright(DiMatrix const *mpIn, DiV4d *vpPos, DiV4d *vpRot);
DiBool     mafTransfAFCoords(DiV4d const *vpP1, DiV4d const *vpP2, DiV4d const *vpP3, DiV4d const *vpP4, DiV4d const *vpP5, DiV4d *vpX,  DiV4d *vpY,  DiV4d *vpZ);
LHP_COMMON_EXPORT DiVoid mflMatrixToDi(vtkMatrix4x4 const *pMatrix, DiMatrix *mp);
LHP_COMMON_EXPORT DiVoid mflMatrixToDi2(vtkMatrix4x4 const *pMatrix, DiMatrix *mp);
LHP_COMMON_EXPORT DiVoid DiMatrixToVTK(DiMatrix const *mp, vtkMatrix4x4 *pMatrix);
LHP_COMMON_EXPORT DiVoid DiMatrixTransposeRotationalSubmatrix(const DiMatrix *mpMatSrc, DiMatrix *mpMatDst);
LHP_COMMON_EXPORT DiVoid DiMatrixOrtoNormalizeVectSys(DiV4d  *vpMain, DiV4d  *vpSub1, DiV4d  *vpSub2, DiBool bLeftRight);
LHP_COMMON_EXPORT DiVoid DiV4dTransformVectorTo(DiV4d const *vpIn, DiMatrix const *mpMat, DiV4d *vpOut);
LHP_COMMON_EXPORT DiVoid DiV4dInverseTransformVectorTo(DiV4d const *vpIn, DiMatrix const *mpMat, DiV4d *vpOut);
LHP_COMMON_EXPORT DiVoid DiV4dTransformPointTo(DiV4d const *vpIn, DiMatrix const *mpMat, DiV4d *vpOut);
LHP_COMMON_EXPORT DiVoid DiV4dInverseTransformPointTo(DiV4d const *vpIn, DiMatrix const *mpMat, DiV4d *vpOut);
LHP_COMMON_EXPORT DiBool DiMatrixInvert(const DiMatrix *mpIn, DiMatrix *mpOut);
DiBool     DiMatrixTransform(DiMatrix *mpDst, DiMatrix *mpSrc, DiInt32 op);
DiVoid     DiV4dCrossProduct(const DiV4d *vpVect1, const DiV4d *vpVect2, DiV4d *vpOut);
LHP_COMMON_EXPORT DiVoid DiMatrixMultiply(const DiMatrix *mpA, const DiMatrix *mpB, DiMatrix *mpOut);
DiVoid     DiMatrixTestIntegrity(const DiMatrix *mpMatrix);
LHP_COMMON_EXPORT bool DiMatrixTestIdentity(const DiMatrix *mpMatrix);
bool       DiMatrixTestRotIdentity(const DiMatrix *mpMatrix);
LHP_COMMON_EXPORT DiVoid DiMatrixIdentity(DiMatrix *mpM);
DiVoid     DiQuatBuildFromMatrix(DiMatrix const *mpMatIn, DiQuaternion *qpQuat);
DiVoid     DiQuatBuildMatrix(DiQuaternion *qpQuat, DiMatrix *mpMatrix);
DiBool     mafTransfDecomposeMatrix(DiMatrix *mpIn, DiV4d *vpRot, DiV4d *vpPos);
DiBool     mafTransfDecomposeMatrixStright(DiMatrix *mpIn, DiV4d *vpRot, DiV4d *vpPos);
LHP_COMMON_EXPORT DiBool mafTransfDecomposeMatrixEulOrdXYZr(DiMatrix *mpIn, DiV4d *vpRot, DiV4d *vpPos);
DiBool     mafTransfComposeMatrix(DiMatrix *mpIn, DiV4d *vpRot, DiV4d *vpPos);
LHP_COMMON_EXPORT DiBool mafTransfComposeMatrixStright(DiMatrix *mpIn, DiV4d const *vpRot, DiV4d const *vpPos);
DiBool     mafTransfIsMatrixOrtoNormalized(DiMatrix *mpMat);
DiVoid     DiMatrixScale(DiMatrix *mpMat, DiV4d *vpScale, DiOpCombainType nOperation);
DiVoid     DiMatrixTranslate(DiMatrix *mpMat, DiV4d *vpTran, DiOpCombainType nOperation);
DiVoid     DiMatrixRotateAtSinCosLeft(DiMatrix *mpMat, DiFloat rSin, DiFloat rCos, DiOpCombainType nOperation);
DiVoid     DiMatrixRotateAt(DiMatrix *mpMat, DiFloat rAngle, DiOpCombainType nOperation);
DiVoid     DiMatrixRotateRightSinCos(DiMatrix *mpMat, DiFloat rSin, DiFloat  rCos, DiOpCombainType nOperation);
LHP_COMMON_EXPORT DiVoid DiMatrixRotateRight(DiMatrix *mpMat, DiFloat rAngle, DiOpCombainType nOperation);
DiVoid     DiMatrixRotateUpSinCosLeft(DiMatrix *mpMat, DiFloat rSin, DiFloat  rCos, DiOpCombainType nOperation);
LHP_COMMON_EXPORT DiVoid DiMatrixRotateUp(DiMatrix *mpMat, DiFloat rAngle, DiOpCombainType nOperation);
DiVoid     BuildConfluencePoint(DiV4d const *axes, DiV4d const *points, DiInt32 numAxes, DiV4d &vConf);
DiBool     mafOptimFindBestRotPoint(DiMatrix const *mpMatrs, DiInt32 nMatrNum, DiMatrix const *mpReference, DiV4d const *vpStartPoint, DiFloat rEpsilon, DiV4d *vpPos);
DiVoid     BuildInstantaneousRotationCenter(DiMatrix const *mpFirst, DiMatrix const *mpSecond, DiV4d &vConf, DiV4d const &vPrev);

struct  DiMatrixInterpolationData
{
  // Matrices
  DiQuaternion  qtCurQuat;
  DiQuaternion  qtPrevQuat;
  // Linear
  DiV4d         vPosDelta;
  DiV4d         vPrevPos;
};
DiVoid DiMatrixBuildInterpolated(DiMatrixInterpolationData *midpData, DiFloat rT, DiMatrix *mpNewMatrix);
DiVoid DiMatrixBuildInterpolationData(DiMatrix const *mpCurMatrix, DiMatrix const *mpPrevMatrix, DiMatrixInterpolationData *midpData);

#define            DiMatrixRotateUpSinCosRight(a, b, c, d)  DiMatrixRotateUpSinCosLeft((a), -(b), (c), (d))
#define            DiMatrixRotateUpSinCos(a, b, c, d)  DiMatrixRotateUpSinCosLeft((a), (b), (c), (d))
#define            DiMatrixRotateAtSinCosRight(a, b, c, d)  DiMatrixRotateAtSinCosLeft((a), -(b), (c), (d))
#define            DiMatrixRotateAtSinCos(a, b, c, d)  DiMatrixRotateAtSinCosLeft((a), (b), (c), (d))

DiFloat Determinant(DiMatrix const * inmat, DiInt32 n);
DiFloat BuildFourPointSphere(DiV4d &c, DiV4d *p);
DiFloat BuildFourPointSphere(DiV4d *c, DiV4d const *p1, DiV4d const *p2, DiV4d const *p3, DiV4d const *p4);


inline     DiVoid DiMatrixCopy(const DiMatrix *mpSrc, DiMatrix *mpDst)
{
  wxASSERT(mpSrc); wxASSERT(mpDst);  *mpDst = *mpSrc;
}
inline     DiVoid DiV4dNegate(const DiV4d *vpIn, DiV4d *vpOut)
{
  vpOut->x = -vpIn->x;  vpOut->y = -vpIn->y;  vpOut->z = -vpIn->z;  vpOut->w = 1.0f;
}
inline     DiVoid DiV4dCopy(const DiV4d *vpIn, DiV4d *vpOut)
{
  vpOut->x = vpIn->x;  vpOut->y = vpIn->y;  vpOut->z = vpIn->z;  vpOut->w = vpIn->w;
}
inline     DiFloat DiV4dPointDistance2(const DiV4d *vpP0, const DiV4d *vpP1)
{
  DiV4d vV; DiFloat rDist;
  vV.x = vpP0->x - vpP1->x;  vV.y = vpP0->y - vpP1->y;  vV.z = vpP0->z - vpP1->z;
  rDist = vV.x * vV.x + vV.y * vV.y + vV.z * vV.z;
  return (rDist);
} 
inline     DiFloat DiV4dDotProduct(const DiV4d *vpVect1, const DiV4d *vpVect2)
{
  return(vpVect1->x * vpVect2->x + vpVect1->y * vpVect2->y + vpVect1->z * vpVect2->z);
} 
inline     DiVoid DiV4dMakeUnit(DiV4d *vpVect)
{
  DiFloat   rLen, rRecLen;
  rLen = DiV4dDotProduct(vpVect, vpVect);
  wxASSERT(rLen > 0.f);

  rRecLen = 1.0 / sqrt(rLen);
  vpVect->x = vpVect->x * rRecLen;
  vpVect->y = vpVect->y * rRecLen;
  vpVect->z = vpVect->z * rRecLen;
  vpVect->w = 0.0f;
}
inline     DiFloat DiV4dNormalize(const DiV4d *vpIn, DiV4d *vpOut)
{
  DiFloat   rLen, rRecLen;
  rLen = DiV4dDotProduct(vpIn, vpIn);
  rLen = sqrt(rLen);
  wxASSERT(rLen > 0.f);
  rRecLen = 1.0f / rLen;  
  vpOut->x = vpIn->x * rRecLen;
  vpOut->y = vpIn->y * rRecLen;
  vpOut->z = vpIn->z * rRecLen;
  vpOut->w = 0.0f;
  return rLen;
} 
#define diSHIFT_COMB(rA, rB, rC)      ((rA)*(rB)+(rC))
inline     DiVoid DiV4dShiftComb(const DiV4d *vpVect1, const DiV4d *vpVect2, DiFloat rCoef, DiV4d *vpOut)
{
  vpOut->x = diSHIFT_COMB(vpVect2->x, rCoef, vpVect1->x);
  vpOut->y = diSHIFT_COMB(vpVect2->y, rCoef, vpVect1->y);
  vpOut->z = diSHIFT_COMB(vpVect2->z, rCoef, vpVect1->z);
  vpOut->w = 1.0f;
} 
inline     DiVoid DiV4dScale(const DiV4d *vpIn, DiFloat rScale, DiV4d *vpOut)
{
  vpOut->x = vpIn->x * rScale;  vpOut->y = vpIn->y * rScale;  vpOut->z = vpIn->z * rScale;  vpOut->w = 1.0f;
}

inline     DiVoid DiV2dSub(const DiV2d *vpA, const DiV2d *vpB, DiV2d *vpC)
{
  vpC->x = vpA->x - vpB->x;  vpC->z = vpA->z - vpB->z;  
} 

inline     DiFloat DiV2dCrossProductY(const DiV2d *vpVect1, const DiV2d *vpVect2)
{
  DiFloat rCross;

  rCross = -vpVect1->z * vpVect2->x + vpVect1->x * vpVect2->z;

  return rCross;
} // end of DiV2dCrossProductY

inline     DiVoid DiV4dSub(const DiV4d *vpA, const DiV4d *vpB, DiV4d *vpC)
{
  vpC->x = vpA->x - vpB->x;  vpC->y = vpA->y - vpB->y;  vpC->z = vpA->z - vpB->z;  vpC->w = 1.0f;
} 

inline     DiVoid DiV4dAdd(const DiV4d *vpA, const DiV4d *vpB, DiV4d *vpC)
{
  vpC->x = vpA->x + vpB->x;  vpC->y = vpA->y + vpB->y;  vpC->z = vpA->z + vpB->z;  vpC->w = 1.0f;
} 

inline     DiVoid DiV4dLineComb(const DiV4d *vpVect1, DiFloat rCoef1, const DiV4d *vpVect2, DiFloat rCoef2, DiV4d *vpOut)
{
  vpOut->x = diSHIFT_COMB(vpVect1->x, rCoef1, vpVect2->x * rCoef2);
  vpOut->y = diSHIFT_COMB(vpVect1->y, rCoef1, vpVect2->y * rCoef2);
  vpOut->z = diSHIFT_COMB(vpVect1->z, rCoef1, vpVect2->z * rCoef2);
  vpOut->w = 1.0f;
} // end of DiV4dLineComb

inline     DiVoid DiQuatLineComb(const DiQuaternion *vpVect1, DiFloat rCoef1, const DiQuaternion *vpVect2, DiFloat rCoef2, DiQuaternion *vpOut)
{
  vpOut->x = diSHIFT_COMB(vpVect1->x, rCoef1, vpVect2->x * rCoef2);
  vpOut->y = diSHIFT_COMB(vpVect1->y, rCoef1, vpVect2->y * rCoef2);
  vpOut->z = diSHIFT_COMB(vpVect1->z, rCoef1, vpVect2->z * rCoef2);
  vpOut->w = diSHIFT_COMB(vpVect1->w, rCoef1, vpVect2->w * rCoef2);
} // end of DiV4dLineComb

#endif
