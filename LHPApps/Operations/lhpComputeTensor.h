/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpComputeTensor.h,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Gregor Klajnsek
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/


#ifndef __lhpComputeTensor_H__
#define __lhpComputeTensor_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkIntArray.h>
#include <vtkUnsignedShortArray.h>


//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
// TODO: remove comments when moved to lhpBuilder directory
//class mafNode;

//----------------------------------------------------------------------------
// lhpOpComputeTensors :
//----------------------------------------------------------------------------
/** TODO: Add your own description here

*/

/** Structure representing a tensor
*/
struct Tensor
{
  double e[6]; // vector form
};

bool ComputeTensor(vtkImageData* volume, vtkDataSet* displacementVectors, vtkImageData* tensorVolume, bool bUseGaussPoints = false);
bool ComputeEigenvalues(vtkImageData* tensorVolume);
template <class T> void TensorCalcGauss(T* pVectors, double* pTensors, double J[3][3], int slices, int rows, int cols);

inline void ComputeTensorInVoxel(double xi, double eta, double zeta, const double displacements[24], const double Jprime[3][3], Tensor *tensor);
inline void ComputeJ(double spacing[3], double J[3][3]);

inline void Compute_Bn_x_M(double index, double ksi, double eta, double zeta, double Out[3][24]);
inline void Compute_JTInverted_x_Bn_x_M(const double JTinv[3][3], double BnM[3][24], double Out[3][24]);
inline void Compute_Epsilon_x_Uc(const double Epsilon[6][24], const double Uc[24], double Out[6]);

inline void Compute_Epsilon_optimized(const double JTinv[3][3], double BuM[3][24], double BvM[3][24], double BwM[3][24], double Epsilon[3][24]);
inline void Compute_Epsilon_x_Uc_optimized(const double Epsilon[6][24], const double Uc[24], double Out[6]);


void TransformComponentToScalars(vtkImageData *volume, vtkDataArray *dataArray, int component, int min, int max, int type = 0);
void SetArrayToVolume(vtkImageData *volume, vtkDoubleArray* dataArray, double origMin, double origMax, int destMin, int destMax, int type = 0);

enum SUPERIMPOSITION_TYPE
  {
  SUPERIMPOSITION_ADD,
  SUPERIMPOSITION_MULTIPLY
  };


template <class T> void SetAllElementsOfArrayToZero(T* dataArray);
template <class T1, class T2> void SuperimposeArray(T1* dataArray, T2* superimposeArray, SUPERIMPOSITION_TYPE superimpositionType);
template <class T> void ScaleArray(T* dataArray, double destMin, double destMax);


//------------------------------------------------------------------------------
template <class T>
void SetAllElementsOfArrayToZero(T* dataArray)
//------------------------------------------------------------------------------
  {
  assert(dataArray);
  if (!dataArray)
    return;

  for (vtkIdType id= dataArray->GetMaxId(); id>=0; id--)
    dataArray->SetValue(id, 0);
  }


//------------------------------------------------------------------------------
template <class T1, class T2>
void SuperimposeArray(T1* dataArray, T2* superimposeArray, SUPERIMPOSITION_TYPE superimpositionType)
//------------------------------------------------------------------------------
{
  // sanity check
  assert(dataArray);
  assert(superimposeArray);
  if (!dataArray || !superimposeArray)
    return;

  // if the arrays are not of the same size we will not perform superimposition
  assert(dataArray->GetNumberOfTuples() == superimposeArray->GetNumberOfTuples());
  if (dataArray->GetNumberOfTuples() != superimposeArray->GetNumberOfTuples())
    return;  

  // perform superimposition according to the selected type
  if (superimpositionType == SUPERIMPOSITION_ADD)
    for (int id= dataArray->GetMaxId(); id>=0; id--)
      dataArray->SetValue(id, dataArray->GetValue(id) + superimposeArray->GetValue(id));
  else if (superimpositionType == SUPERIMPOSITION_MULTIPLY)
    for (int id= dataArray->GetMaxId(); id>=0; id--)
      {
      double value1 = dataArray->GetValue(id);
      double value2 = superimposeArray->GetValue(id);
      double value3 = value1 * value2;
      dataArray->SetValue(id, dataArray->GetValue(id) * superimposeArray->GetValue(id));
      double valueResult = dataArray->GetValue(id);
      }
}

//------------------------------------------------------------------------------
template <class T>
void ScaleArray(T* dataArray, double destMin, double destMax)
//------------------------------------------------------------------------------
  {
  if (destMin >= destMax)
    {double tmp = destMin; destMin = destMax; destMax = tmp;}

  double* origRange = dataArray->GetRange();
  if (origRange[1] - origRange[0] == 0.0)
    return;
  double factor = (destMax - destMin)/abs(origRange[1] - origRange[0]);

  //for (int id=dataArray->GetMaxId(); id>=0; id--)
  double value1, value2, value3, value4, value5;
  unsigned short finalValue;
  unsigned short checkValue;
  
  T* pToArray = (T*)dataArray->GetVoidPointer(0);

  for (int id=0; id<dataArray->GetNumberOfTuples(); id++)
    {
    value1 = dataArray->GetValue(id);
    value2 = origRange[0];  
    value3 = value1 - value2;
    value4 = value3 * factor;
    value5 = destMin + value4;
    finalValue = (unsigned short)value5;
    
    //dataArray->SetValue(id, (unsigned short) (destMin + ( dataArray->GetValue(id) - origRange[0]) * factor));
    dataArray->SetValue(id, finalValue);
    checkValue = dataArray->GetValue(id);
    }
  }



#endif