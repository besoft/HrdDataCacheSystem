/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpComputeTensor.cpp,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Gregor Klajnsek
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h"  
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpComputeTensor.h"


//---------------------------------------------------------------------------------------------------------------------
// T E N S O R   C A L C U L U S
//---------------------------------------------------------------------------------------------------------------------

/**
*/

/** Function computes nodal tensors in the nodes of the volume
@volume vtkImageData dataset that has to include additional array of vectors
*/

//------------------------------------------------------------------------------
bool ComputeTensor(vtkImageData* volume, vtkDataSet* displacementVectors, vtkImageData* tensorVolume, bool bUseGaussPoints)
//------------------------------------------------------------------------------
{
  double origin[3];
  double spacing[3];
  int dimensions[3];
  int sliceSize;
  volume->GetOrigin(origin);
  volume->GetSpacing(spacing);
  volume->GetDimensions(dimensions);
  int cols = dimensions[0];  
  int rows = dimensions[1];  
  int slices = dimensions[2];
  sliceSize = rows * cols;       // number of voxels in one slice
  int nodesPerSlice = dimensions[0] * dimensions[1];


  // From the spacing we can calculate matrix J. As the input is image data all the voxels are of the same size and the cells are axis aligned.
  // The matrix J transforms global coordinates into local coordinates. 
  double J[3][3];
  ComputeJ(spacing, J);


  // Check if the array of displacement vectors is present and store the pointer to it into variable for faster access.
  vtkDataArray* vectors;
  vectors = displacementVectors->GetPointData()->GetVectors();  // get the list of displacement vectors
  if (!vectors)                                    // if there are no vectors present 
    return false;                                        // terminate the process


  // Create a tensor array which we will fill with data.
  int numPoints = volume->GetNumberOfPoints();
  vtkDoubleArray* tensors = vtkDoubleArray::New();
  tensors->SetNumberOfComponents(9);
  tensors->SetNumberOfTuples(numPoints);

  void* pVectors = vectors->GetVoidPointer(0);
  void* pTensors = tensors->GetVoidPointer(0);


  // call the routine that performs calculation1
  if (vectors->GetDataType() == VTK_FLOAT)
    TensorCalcGauss((float*)pVectors, (double*)pTensors, J, slices, rows, cols);
  else if (vectors->GetDataType() == VTK_DOUBLE)
    TensorCalcGauss((double*)pVectors, (double*)pTensors, J, slices, rows, cols);

  // set the tensors to the Volume dataset
  tensorVolume->Update();
  tensorVolume->GetPointData()->SetTensors(tensors);
  tensors->Delete();
  return true;
}


//------------------------------------------------------------------------------
bool ComputeEigenvalues(vtkImageData* tensorVolume)
//------------------------------------------------------------------------------
{
  vtkDoubleArray* tensorArray =  vtkDoubleArray::SafeDownCast(tensorVolume->GetPointData()->GetTensors());
  if (!tensorArray)
    return false;
  int numTensors = tensorArray->GetNumberOfTuples();

  vtkDoubleArray* eigenvalueArray = vtkDoubleArray::New();
  eigenvalueArray->SetNumberOfComponents(3);
  eigenvalueArray->SetNumberOfTuples(numTensors);

  double Tensor[3][3];  // matrix representing the current tensor
  double Evalues[3];        // array of eigenvalues
  double Evectors[3][3];     // matrix representing the eigenvectors

  for (int i=0; i<numTensors; i++)
    {
    tensorArray->GetTuple(i, (double*)Tensor);
    // calculate eigenvalues
    vtkMath::Diagonalize3x3(Tensor, Evalues, Evectors);
    eigenvalueArray->SetTuple(i, Evalues);
    }
  
  // set the tensors to the Volume dataset
  tensorVolume->GetPointData()->SetVectors(eigenvalueArray);
  tensorVolume->Update();
  eigenvalueArray->Delete();
  return true;
}




/**
*/
//------------------------------------------------------------------------------
template <class T> 
void TensorCalcGauss(T* pVectors, double* pTensors, double J[3][3], int slices, int rows, int cols)
//------------------------------------------------------------------------------
 {  

  DWORD timeA_START = GetTickCount();

  // Transformation matrices that we need
  double TriLinearInterpolationGaussCoef[8][8] = 
  { { 2.549038098871852, -0.6830126984083893,   0.1830127005138035,  -0.6830126984083892,  -0.6830126984083891,   0.1830127005138034,  -0.04903810518809487,  0.1830127005138034},
    {-3.23205079728024,   3.232050797280241,   -0.8660253989221927,   0.8660253989221926,   0.8660253989221926,  -0.8660253989221926,   0.2320508057018984,  -0.2320508057018984},
    {-3.232050797280241,  0.8660253989221928,  -0.8660253989221927,   3.232050797280241,    0.8660253989221927,  -0.2320508057018985,   0.2320508057018985,  -0.8660253989221927},
    {-3.232050797280241,  0.8660253989221922,  -0.2320508057018981,   0.8660253989221923,   3.232050797280241,   -0.8660253989221923,   0.2320508057018982,  -0.8660253989221923},
    { 4.098076196202434, -4.098076196202434,    4.098076196202434,   -4.098076196202434,   -1.098076204624092,    1.098076204624092,   -1.098076204624092,    1.098076204624092},
    { 4.098076196202435, -4.098076196202435,    1.098076204624091,   -1.098076204624091,   -4.098076196202434,    4.098076196202434,   -1.098076204624091,    1.098076204624091},
    { 4.098076196202434, -1.098076204624092,    1.098076204624092,   -4.098076196202434,   -4.098076196202434,    1.098076204624091,   -1.098076204624091,    4.098076196202434},
    {-5.196152400826526,  5.196152400826526,   -5.196152400826526,    5.196152400826526,    5.196152400826526,   -5.196152400826526,    5.196152400826526,   -5.196152400826526}};

  double TransformationCoefficientsArray[8][8] = 
  { {1, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 0, 1, 0, 0, 0},
    {1, 0, 1, 0, 0, 0, 0, 0},
    {1, 0, 0, 1, 0, 0, 0, 0},
    {1, 1, 0, 1, 0, 1, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 1, 0, 0, 1, 0} };




  int nodesPerSlice = cols * rows;
  int nodesPerRow = cols;

  // small speedup
  int r3 = 1 * 3;
  int c3 = nodesPerRow * 3;
  int nps3 = nodesPerSlice * 3;


  // reserve memory temporary arrays. This temporary arrays will store the values of gauss tensors
  // which will be used for calculating nodal tensors
  Tensor gaussTensors[8];
  int nodeIndex[8];
  

  int numNodes = nodesPerSlice * slices;
  int *numTensorsPerNode = new int[numNodes];
  for (int i=0; i<numNodes; i++)
    numTensorsPerNode[i] = 0;
  memset(pTensors, 0, sizeof(double) * numNodes * 9);


  // fill the matrix of Gauss Points
  double GP[8][3];
  for (int GaussIndex = 0; GaussIndex < 8; GaussIndex++) 
    GetGaussPoint(GaussIndex, GP[GaussIndex]);

  DWORD timeB_START = GetTickCount();
  
  // loop through all slices
  for (int z=0; z<(slices-1); z++)   // slices
    for (int y=0; y<(rows-1); y++)     // rows 
      for (int x=0; x<(cols-1); x++)     // columns
        {
          nodeIndex[0] = x + y * nodesPerRow + z * nodesPerSlice;
          nodeIndex[1] = (x + 1) + y * nodesPerRow + z * nodesPerSlice;
          nodeIndex[2] = (x + 1) + (y  + 1)* nodesPerRow + z * nodesPerSlice;
          nodeIndex[3] = x + (y  + 1)* nodesPerRow + z * nodesPerSlice;
          nodeIndex[4] = x + y * nodesPerRow + (z + 1) * nodesPerSlice;
          nodeIndex[5] = (x + 1) + y * nodesPerRow + (z + 1) * nodesPerSlice;
          nodeIndex[6] = (x + 1) + (y  + 1) * nodesPerRow + (z + 1) * nodesPerSlice;
          nodeIndex[7] = x + (y + 1) * nodesPerRow + (z + 1) * nodesPerSlice;
    
          for (int i=0; i<8; i++)
            numTensorsPerNode[nodeIndex[i]]++;

          double UC[24];
          int offsetOfFirstNode = (z * nodesPerSlice + y * nodesPerRow + x)*3;
          int voxelIndex;
          voxelIndex = offsetOfFirstNode;
          UC[0] = pVectors[voxelIndex++ ];
          UC[1] = pVectors[voxelIndex++ ];
          UC[2] = pVectors[voxelIndex ];

          voxelIndex = offsetOfFirstNode + r3;
          UC[3] = pVectors[voxelIndex++ ];
          UC[4] = pVectors[voxelIndex++ ];
          UC[5] = pVectors[voxelIndex ];

          voxelIndex = offsetOfFirstNode + r3 + c3;
          UC[6] = pVectors[voxelIndex++ ];
          UC[7] = pVectors[voxelIndex++ ];
          UC[8] = pVectors[voxelIndex ];

          voxelIndex = offsetOfFirstNode + c3;
          UC[9] = pVectors[voxelIndex++ ];
          UC[10] = pVectors[voxelIndex++ ];
          UC[11] = pVectors[voxelIndex ];

          voxelIndex = offsetOfFirstNode + nps3;
          UC[12] = pVectors[voxelIndex++ ];
          UC[13] = pVectors[voxelIndex++ ];
          UC[14] = pVectors[voxelIndex ];

          voxelIndex = offsetOfFirstNode + nps3+ r3;
          UC[15] = pVectors[voxelIndex++ ];
          UC[16] = pVectors[voxelIndex++ ];
          UC[17] = pVectors[voxelIndex ];

          voxelIndex = offsetOfFirstNode + nps3 + r3 + c3 ;
          UC[18] = pVectors[voxelIndex++ ];
          UC[19] = pVectors[voxelIndex++ ];
          UC[20] = pVectors[voxelIndex ];

          voxelIndex = offsetOfFirstNode + nps3 + c3;
          UC[21] = pVectors[voxelIndex++ ];
          UC[22] = pVectors[voxelIndex++ ];
          UC[23] = pVectors[voxelIndex ];

          for (int GaussIndex = 0; GaussIndex < 8; GaussIndex++) 
            ComputeTensorInVoxel(GP[GaussIndex][0], GP[GaussIndex][1], GP[GaussIndex][2], UC, J, &gaussTensors[GaussIndex]); 

          // calculate nodal value by applying inverse of trilinear interpolation
          double p[6][8];
          for (int px = 0; px<6; px++)
            {
            for (int position = 0; position<8; position++)
              {
              p[px][position] = 0.0;
              for (int matrixElement = 0; matrixElement<8; matrixElement++)
                p[px][position] += TriLinearInterpolationGaussCoef[position][matrixElement] * gaussTensors[matrixElement].e[px];
              }
            }


        // create matrix q
        double q[8][9];
        for (int i = 0; i<9; i++)
          {
          int component = 0;
          if (i ==1 || i == 3)
            component = 3;
          else if (i==2 || i ==6)
            component = 5;
          else if (i == 5 || i == 7)
            component = 4;
          else if (i == 4)
            component = 1;
          else if (i == 8)
            component = 2;

          for (int j=0; j<8; j++)
             q[j][i] = p[component][j];        
          }

        
        // calculate tensors
        //{ {1, 0, 0, 0, 0, 0, 0, 0},
        //{1, 1, 0, 0, 0, 0, 0, 0},
        //{1, 1, 1, 0, 1, 0, 0, 0},
        //{1, 0, 1, 0, 0, 0, 0, 0},
        //{1, 0, 0, 1, 0, 0, 0, 0},
        //{1, 1, 0, 1, 0, 1, 0, 0},
        //{1, 1, 1, 1, 1, 1, 1, 1},
        //{1, 0, 1, 1, 0, 0, 1, 0} };

        double values[9];
        for (int node = 0; node<8; node++)
          {
          // calculate values          
          for (int i=0; i<9; i++)
            {
            values[i] = 0;
            for (int j=0; j<8; j++)
                values[i] += TransformationCoefficientsArray[node][j] * q[j][i];
            }
          
          // add values to nodes
          for (int i=0; i<9; i++)
            pTensors[nodeIndex[node]*9+i] += values[i];
          }
        }

  DWORD timeB_END = GetTickCount();
  DWORD timeC_START = GetTickCount();

  
  for (int i=0; i<numNodes; i++)
    {

    double divisor = numTensorsPerNode[i];
    for(int j = 0; j<9; j++)
      pTensors[i * 9 + j] /= divisor;
    }

  DWORD timeC_END = GetTickCount();


  // free the memory used for temporary arrays
  delete[] numTensorsPerNode;
  DWORD timeA_END = GetTickCount();

}




/**  Function calculates the matrix J from the size of the voxel.
@param spacing		Spacing of the voxel in three directions. 
@param J			The matrix that represents a transformation from global coordinates to local coordinates inside voxel.
*/

//------------------------------------------------------------------------------
inline void ComputeJ(double spacing[3], double J[3][3])
//------------------------------------------------------------------------------
{
  J[0][0] = spacing[0];  J[1][0] = 0;           J[2][0] = 0;
  J[0][1] = 0;           J[1][1] = spacing[1];  J[2][1] = 0;
  J[0][2] = 0;           J[1][2] = 0;           J[2][2] = spacing[2];
  vtkMath::Transpose3x3(J,J);
  vtkMath::Invert3x3(J, J);

}


/**  Function calculates the matrix J from the coordinates of the voxel
This function is useful if the size of the voxels changes (ie. if we are working with a rectilinear grid or if voxels are not axis aligned.
@param h	Coordinates of node H of the voxel.
@param i	Coordinates of node I of the voxel.
@param k	Coordinates of node K of the voxel.
@param l	Coordinates of node L of the voxel.
*/
//------------------------------------------------------------------------------
inline void ComputeJ(double h[3], double i[3], double k[3], double l[3], double J[3][3])
//------------------------------------------------------------------------------
{  
  J[0][0] = i[0] - h[0]; J[1][0] = k[0] - h[0]; J[2][0] = l[0] - h[0];
  J[0][1] = i[1] - h[1]; J[1][1] = k[1] - h[1]; J[2][1] = l[1] - h[1];
  J[0][2] = i[2] - h[2]; J[1][2] = k[2] - h[2]; J[2][2] = l[2] - h[2];
  vtkMath::Transpose3x3(J,J);
  vtkMath::Invert3x3(J, J);
}



/** Computes the strain tensor in a point P that lies inside a voxel. The parameters xi, eta, zeta represent the local 
coordinates of the point P.
@param xi		Local x coordinate of point P.
@param eta		Local y coordinate of point P.
@param zeta		Local z coordinate of Point P.
@param displacements	Vector consisting of 24 displacements values obtained from displacement vectors associated with nodes. 
@param JPrime	Precomputed matrix JPrime. 	
@param tensor	Tensor structure that is filled with data after in the function.
*/
//------------------------------------------------------------------------------
void ComputeTensorInVoxel(double ksi, double eta, double zeta, const double displacements[24], const double Jprime[3][3], Tensor *tensor)
//------------------------------------------------------------------------------
{  
  // Step 1 - calculate the matrix Jprime.
  // As all the voxels are of the same size we precompute matrix J and send it into the function as a parameter

  double BuM[3][24], BvM[3][24], BwM[3][24];
  Compute_Bn_x_M(1, ksi, eta, zeta, BuM);
  Compute_Bn_x_M(2, ksi, eta, zeta, BvM);
  Compute_Bn_x_M(3, ksi, eta, zeta, BwM);

  /* OLD UNOPTIMIZED APPROACH THAT PERFORMS FULL MATRIX MULTIPLICATION
  //double epsilon1[3][24], epsilon2[3][24], epsilon3[3][24];
  //Compute_JTInverted_x_Bn_x_M(Jprime, BuM, epsilon1);
  //Compute_JTInverted_x_Bn_x_M(Jprime, BvM, epsilon2);
  //Compute_JTInverted_x_Bn_x_M(Jprime, BwM, epsilon3);

  double epsilon[6][24];
  Compute_JTInverted_x_Bn_x_M_optimized(Jprime, BuM, epsilon1,1);
  Compute_JTInverted_x_Bn_x_M_optimized(Jprime, BvM, epsilon2,2);
  Compute_JTInverted_x_Bn_x_M_optimized(Jprime, BwM, epsilon3,3);
 
  for (int i=0; i<24; i++)
    {
    epsilon[0][i] = epsilon1[0][i];
    epsilon[1][i] = epsilon2[1][i];
    epsilon[2][i] = epsilon3[2][i];
    epsilon[3][i] = ( epsilon1[1][i] + epsilon2[0][i]);
    epsilon[4][i] = ( epsilon2[2][i] + epsilon3[1][i]);
    epsilon[5][i] = ( epsilon3[0][i] + epsilon1[2][i]);
    }
  */
  // NEW OPTIMIZED APPROACH - CALCULATES ONLY RELEVANT ENTRIES (ignores all multiplications where one element is 0)
  double epsilon[6][24] = {0.0};
  Compute_Epsilon_optimized(Jprime, BuM, BvM, BwM, epsilon);
 
  Compute_Epsilon_x_Uc_optimized(epsilon, displacements, tensor->e); 
}


//------------------------------------------------------------------------------
// index 1 = bu, 2 = bv, 3 = bw
void Compute_Bn_x_M(double index, double ksi, double eta, double zeta, double Out[3][24])
//------------------------------------------------------------------------------
  {
  for (int i=0; i<3; i++)
    for (int j=0; j<24; j++)
      Out[i][j] = 0;
  double etaZeta = eta*zeta;
  double ksiEta = ksi * eta;
  double ksiZeta = ksi * zeta; 
  int off = index-1;
  Out[0][0+off] = -1 + eta + zeta - etaZeta;  
  Out[0][3+off] =  1 - eta - zeta + etaZeta;
  Out[0][6+off] = eta - etaZeta;
  Out[0][9+off] = -eta + etaZeta;
  Out[0][12+off] = -zeta + etaZeta;
  Out[0][15+off] = zeta - etaZeta;
  Out[0][18+off] = etaZeta;
  Out[0][21+off] = -etaZeta;

  Out[1][0+off] = -1 + ksi + zeta - ksiZeta;
  Out[1][3+off] = -ksi + ksiZeta;
  Out[1][6+off] = ksi - ksiZeta;
  Out[1][9+off] = 1 - ksi - zeta +ksiZeta;
  Out[1][12+off] = - zeta + ksiZeta;
  Out[1][15+off] = - ksiZeta;
  Out[1][18+off] = ksiZeta;
  Out[1][21+off] = zeta - ksiZeta;

  Out[2][0+off] = -1 + ksi + eta - ksiEta;
  Out[2][3+off] = - ksi + ksiEta;
  Out[2][6+off] = - ksiEta;
  Out[2][9+off] = - eta + ksiEta;
  Out[2][12+off] = 1 - ksi - eta + ksiEta;
  Out[2][15+off] = ksi - ksiEta;
  Out[2][18+off] = ksiEta; 
  Out[2][21+off] = eta - ksiEta;                     
  }

//------------------------------------------------------------------------------
void Compute_JTInverted_x_Bn_x_M(const double JTinv[3][3], double BnM[3][24], double Out[3][24])
//------------------------------------------------------------------------------
  {
  // TODO: add possible optimizations
  for (int i=0; i<3; i++)
    for (int j=0; j<24; j++)
      {
      double value = 0;
      for (int k=0; k<3; k++)
        value += JTinv[i][k]* BnM[k][j];
      Out[i][j] = value;
      }
  }
//------------------------------------------------------------------------------
void Compute_Epsilon_x_Uc(const double epsilon[6][24], const double Uc[24], double Out[6])
//------------------------------------------------------------------------------
  {
  for (int j = 0; j<6; j++)
    {
    double value = 0;
    for (int k=0; k<24; k++)
      value += epsilon[j][k]*Uc[k];
    Out[j] = value;
    }
  }
//------------------------------------------------------------------------------
inline void Compute_Epsilon_x_Uc_optimized(const double Epsilon[6][24], const double Uc[24], double Out[6])
//------------------------------------------------------------------------------
  {
  Out[0] = Epsilon[0][0] * Uc[0]  +  Epsilon[0][3] * Uc[3]  +  Epsilon[0][6] * Uc[6]  +  Epsilon[0][9] * Uc[9]  +  Epsilon[0][12] * Uc[12]  +  Epsilon[0][15] * Uc[15]  +  Epsilon[0][18] * Uc[18]  +  Epsilon[0][21] * Uc[21];
  Out[1] = Epsilon[1][1] * Uc[1]  +  Epsilon[1][4] * Uc[4]  +  Epsilon[1][7] * Uc[7]  +  Epsilon[1][10] * Uc[10]  +  Epsilon[1][13] * Uc[13]  +  Epsilon[1][16] * Uc[16]  +  Epsilon[1][19] * Uc[19]  +  Epsilon[1][22] * Uc[22];
  Out[2] = Epsilon[2][2] * Uc[2]  +  Epsilon[2][5] * Uc[5]  +  Epsilon[2][8] * Uc[8]  +  Epsilon[2][11] * Uc[11]  +  Epsilon[2][14] * Uc[14]  +  Epsilon[2][17] * Uc[17]  +  Epsilon[2][20] * Uc[20]  +  Epsilon[2][23] * Uc[23];
  Out[3] = Epsilon[3][0] * Uc[0]  +  Epsilon[3][1] * Uc[1]  +  Epsilon[3][3] * Uc[3]  +  Epsilon[3][4] * Uc[4]  +  Epsilon[3][6] * Uc[6]  +  Epsilon[3][7] * Uc[7]  +  Epsilon[3][9] * Uc[9]  +  Epsilon[3][10] * Uc[10]  +  Epsilon[3][12] * Uc[12]  +  Epsilon[3][13] * Uc[13]  +  Epsilon[3][15] * Uc[15]  +  Epsilon[3][16] * Uc[16]  +  Epsilon[3][18] * Uc[18]  +  Epsilon[3][19] * Uc[19]  +  Epsilon[3][21] * Uc[21]  +  Epsilon[3][22] * Uc[22];
  Out[4] = Epsilon[4][1] * Uc[1]  +  Epsilon[4][2] * Uc[2]  +  Epsilon[4][4] * Uc[4]  +  Epsilon[4][5] * Uc[5]  +  Epsilon[4][7] * Uc[7]  +  Epsilon[4][8] * Uc[8]  +  Epsilon[4][10] * Uc[10]  +  Epsilon[4][11] * Uc[11]  +  Epsilon[4][13] * Uc[13]  +  Epsilon[4][14] * Uc[14]  +  Epsilon[4][16] * Uc[16]  +  Epsilon[4][17] * Uc[17]  +  Epsilon[4][19] * Uc[19]  +  Epsilon[4][20] * Uc[20]  +  Epsilon[4][22] * Uc[22]  +  Epsilon[4][23] * Uc[23];
  Out[5] = Epsilon[5][0] * Uc[0]  +  Epsilon[5][2] * Uc[2]  +  Epsilon[5][3] * Uc[3]  +  Epsilon[5][5] * Uc[5]  +  Epsilon[5][6] * Uc[6]  +  Epsilon[5][8] * Uc[8]  +  Epsilon[5][9] * Uc[9]  +  Epsilon[5][11] * Uc[11]  +  Epsilon[5][12] * Uc[12]  +  Epsilon[5][14] * Uc[14]  +  Epsilon[5][15] * Uc[15]  +  Epsilon[5][17] * Uc[17]  +  Epsilon[5][18] * Uc[18]  +  Epsilon[5][20] * Uc[20]  +  Epsilon[5][21] * Uc[21]  +  Epsilon[5][23] * Uc[23];
  }

//------------------------------------------------------------------------------
inline void Compute_Epsilon_optimized(const double JTinv[3][3], double BuM[3][24], double BvM[3][24], double BwM[3][24], double Epsilon[3][24])
//------------------------------------------------------------------------------
  {
    Epsilon[0][0] = JTinv[0][0] * BuM[0][0]  +  JTinv[0][1] * BuM[1][0]  +  JTinv[0][2] * BuM[2][0];
    Epsilon[0][3] = JTinv[0][0] * BuM[0][3]  +  JTinv[0][1] * BuM[1][3]  +  JTinv[0][2] * BuM[2][3];
    Epsilon[0][6] = JTinv[0][0] * BuM[0][6]  +  JTinv[0][1] * BuM[1][6]  +  JTinv[0][2] * BuM[2][6];
    Epsilon[0][9] = JTinv[0][0] * BuM[0][9]  +  JTinv[0][1] * BuM[1][9]  +  JTinv[0][2] * BuM[2][9];
    Epsilon[0][12] = JTinv[0][0] * BuM[0][12]  +  JTinv[0][1] * BuM[1][12]  +  JTinv[0][2] * BuM[2][12];
    Epsilon[0][15] = JTinv[0][0] * BuM[0][15]  +  JTinv[0][1] * BuM[1][15]  +  JTinv[0][2] * BuM[2][15];
    Epsilon[0][18] = JTinv[0][0] * BuM[0][18]  +  JTinv[0][1] * BuM[1][18]  +  JTinv[0][2] * BuM[2][18];
    Epsilon[0][21] = JTinv[0][0] * BuM[0][21]  +  JTinv[0][1] * BuM[1][21]  +  JTinv[0][2] * BuM[2][21];
    Epsilon[1][1] = JTinv[1][0] * BvM[0][1]  +  JTinv[1][1] * BvM[1][1]  +  JTinv[1][2] * BvM[2][1];
    Epsilon[1][4] = JTinv[1][0] * BvM[0][4]  +  JTinv[1][1] * BvM[1][4]  +  JTinv[1][2] * BvM[2][4];
    Epsilon[1][7] = JTinv[1][0] * BvM[0][7]  +  JTinv[1][1] * BvM[1][7]  +  JTinv[1][2] * BvM[2][7];
    Epsilon[1][10] = JTinv[1][0] * BvM[0][10]  +  JTinv[1][1] * BvM[1][10]  +  JTinv[1][2] * BvM[2][10];
    Epsilon[1][13] = JTinv[1][0] * BvM[0][13]  +  JTinv[1][1] * BvM[1][13]  +  JTinv[1][2] * BvM[2][13];
    Epsilon[1][16] = JTinv[1][0] * BvM[0][16]  +  JTinv[1][1] * BvM[1][16]  +  JTinv[1][2] * BvM[2][16];
    Epsilon[1][19] = JTinv[1][0] * BvM[0][19]  +  JTinv[1][1] * BvM[1][19]  +  JTinv[1][2] * BvM[2][19];
    Epsilon[1][22] = JTinv[1][0] * BvM[0][22]  +  JTinv[1][1] * BvM[1][22]  +  JTinv[1][2] * BvM[2][22];
    Epsilon[2][2] = JTinv[2][0] * BwM[0][2]  +  JTinv[2][1] * BwM[1][2]  +  JTinv[2][2] * BwM[2][2];
    Epsilon[2][5] = JTinv[2][0] * BwM[0][5]  +  JTinv[2][1] * BwM[1][5]  +  JTinv[2][2] * BwM[2][5];
    Epsilon[2][8] = JTinv[2][0] * BwM[0][8]  +  JTinv[2][1] * BwM[1][8]  +  JTinv[2][2] * BwM[2][8];
    Epsilon[2][11] = JTinv[2][0] * BwM[0][11]  +  JTinv[2][1] * BwM[1][11]  +  JTinv[2][2] * BwM[2][11];
    Epsilon[2][14] = JTinv[2][0] * BwM[0][14]  +  JTinv[2][1] * BwM[1][14]  +  JTinv[2][2] * BwM[2][14];
    Epsilon[2][17] = JTinv[2][0] * BwM[0][17]  +  JTinv[2][1] * BwM[1][17]  +  JTinv[2][2] * BwM[2][17];
    Epsilon[2][20] = JTinv[2][0] * BwM[0][20]  +  JTinv[2][1] * BwM[1][20]  +  JTinv[2][2] * BwM[2][20];
    Epsilon[2][23] = JTinv[2][0] * BwM[0][23]  +  JTinv[2][1] * BwM[1][23]  +  JTinv[2][2] * BwM[2][23];
    Epsilon[3][0] = JTinv[1][0] * BuM[0][0]  +  JTinv[1][1] * BuM[1][0]  +  JTinv[1][2] * BuM[2][0];
    Epsilon[3][1] = JTinv[0][0] * BvM[0][1]  +  JTinv[0][1] * BvM[1][1]  +  JTinv[0][2] * BvM[2][1];
    Epsilon[3][3] = JTinv[1][0] * BuM[0][3]  +  JTinv[1][1] * BuM[1][3]  +  JTinv[1][2] * BuM[2][3];
    Epsilon[3][4] = JTinv[0][0] * BvM[0][4]  +  JTinv[0][1] * BvM[1][4]  +  JTinv[0][2] * BvM[2][4];
    Epsilon[3][6] = JTinv[1][0] * BuM[0][6]  +  JTinv[1][1] * BuM[1][6]  +  JTinv[1][2] * BuM[2][6];
    Epsilon[3][7] = JTinv[0][0] * BvM[0][7]  +  JTinv[0][1] * BvM[1][7]  +  JTinv[0][2] * BvM[2][7];
    Epsilon[3][9] = JTinv[1][0] * BuM[0][9]  +  JTinv[1][1] * BuM[1][9]  +  JTinv[1][2] * BuM[2][9];
    Epsilon[3][10] = JTinv[0][0] * BvM[0][10]  +  JTinv[0][1] * BvM[1][10]  +  JTinv[0][2] * BvM[2][10];
    Epsilon[3][12] = JTinv[1][0] * BuM[0][12]  +  JTinv[1][1] * BuM[1][12]  +  JTinv[1][2] * BuM[2][12];
    Epsilon[3][13] = JTinv[0][0] * BvM[0][13]  +  JTinv[0][1] * BvM[1][13]  +  JTinv[0][2] * BvM[2][13];
    Epsilon[3][15] = JTinv[1][0] * BuM[0][15]  +  JTinv[1][1] * BuM[1][15]  +  JTinv[1][2] * BuM[2][15];
    Epsilon[3][16] = JTinv[0][0] * BvM[0][16]  +  JTinv[0][1] * BvM[1][16]  +  JTinv[0][2] * BvM[2][16];
    Epsilon[3][18] = JTinv[1][0] * BuM[0][18]  +  JTinv[1][1] * BuM[1][18]  +  JTinv[1][2] * BuM[2][18];
    Epsilon[3][19] = JTinv[0][0] * BvM[0][19]  +  JTinv[0][1] * BvM[1][19]  +  JTinv[0][2] * BvM[2][19];
    Epsilon[3][21] = JTinv[1][0] * BuM[0][21]  +  JTinv[1][1] * BuM[1][21]  +  JTinv[1][2] * BuM[2][21];
    Epsilon[3][22] = JTinv[0][0] * BvM[0][22]  +  JTinv[0][1] * BvM[1][22]  +  JTinv[0][2] * BvM[2][22];
    Epsilon[4][1] = JTinv[2][0] * BvM[0][1]  +  JTinv[2][1] * BvM[1][1]  +  JTinv[2][2] * BvM[2][1];
    Epsilon[4][2] = JTinv[1][0] * BwM[0][2]  +  JTinv[1][1] * BwM[1][2]  +  JTinv[1][2] * BwM[2][2];
    Epsilon[4][4] = JTinv[2][0] * BvM[0][4]  +  JTinv[2][1] * BvM[1][4]  +  JTinv[2][2] * BvM[2][4];
    Epsilon[4][5] = JTinv[1][0] * BwM[0][5]  +  JTinv[1][1] * BwM[1][5]  +  JTinv[1][2] * BwM[2][5];
    Epsilon[4][7] = JTinv[2][0] * BvM[0][7]  +  JTinv[2][1] * BvM[1][7]  +  JTinv[2][2] * BvM[2][7];
    Epsilon[4][8] = JTinv[1][0] * BwM[0][8]  +  JTinv[1][1] * BwM[1][8]  +  JTinv[1][2] * BwM[2][8];
    Epsilon[4][10] = JTinv[2][0] * BvM[0][10]  +  JTinv[2][1] * BvM[1][10]  +  JTinv[2][2] * BvM[2][10];
    Epsilon[4][11] = JTinv[1][0] * BwM[0][11]  +  JTinv[1][1] * BwM[1][11]  +  JTinv[1][2] * BwM[2][11];
    Epsilon[4][13] = JTinv[2][0] * BvM[0][13]  +  JTinv[2][1] * BvM[1][13]  +  JTinv[2][2] * BvM[2][13];
    Epsilon[4][14] = JTinv[1][0] * BwM[0][14]  +  JTinv[1][1] * BwM[1][14]  +  JTinv[1][2] * BwM[2][14];
    Epsilon[4][16] = JTinv[2][0] * BvM[0][16]  +  JTinv[2][1] * BvM[1][16]  +  JTinv[2][2] * BvM[2][16];
    Epsilon[4][17] = JTinv[1][0] * BwM[0][17]  +  JTinv[1][1] * BwM[1][17]  +  JTinv[1][2] * BwM[2][17];
    Epsilon[4][19] = JTinv[2][0] * BvM[0][19]  +  JTinv[2][1] * BvM[1][19]  +  JTinv[2][2] * BvM[2][19];
    Epsilon[4][20] = JTinv[1][0] * BwM[0][20]  +  JTinv[1][1] * BwM[1][20]  +  JTinv[1][2] * BwM[2][20];
    Epsilon[4][22] = JTinv[2][0] * BvM[0][22]  +  JTinv[2][1] * BvM[1][22]  +  JTinv[2][2] * BvM[2][22];
    Epsilon[4][23] = JTinv[1][0] * BwM[0][23]  +  JTinv[1][1] * BwM[1][23]  +  JTinv[1][2] * BwM[2][23];
    Epsilon[5][0] = JTinv[2][0] * BuM[0][0]  +  JTinv[2][1] * BuM[1][0]  +  JTinv[2][2] * BuM[2][0];
    Epsilon[5][2] = JTinv[0][0] * BwM[0][2]  +  JTinv[0][1] * BwM[1][2]  +  JTinv[0][2] * BwM[2][2];
    Epsilon[5][3] = JTinv[2][0] * BuM[0][3]  +  JTinv[2][1] * BuM[1][3]  +  JTinv[2][2] * BuM[2][3];
    Epsilon[5][5] = JTinv[0][0] * BwM[0][5]  +  JTinv[0][1] * BwM[1][5]  +  JTinv[0][2] * BwM[2][5];
    Epsilon[5][6] = JTinv[2][0] * BuM[0][6]  +  JTinv[2][1] * BuM[1][6]  +  JTinv[2][2] * BuM[2][6];
    Epsilon[5][8] = JTinv[0][0] * BwM[0][8]  +  JTinv[0][1] * BwM[1][8]  +  JTinv[0][2] * BwM[2][8];
    Epsilon[5][9] = JTinv[2][0] * BuM[0][9]  +  JTinv[2][1] * BuM[1][9]  +  JTinv[2][2] * BuM[2][9];
    Epsilon[5][11] = JTinv[0][0] * BwM[0][11]  +  JTinv[0][1] * BwM[1][11]  +  JTinv[0][2] * BwM[2][11];
    Epsilon[5][12] = JTinv[2][0] * BuM[0][12]  +  JTinv[2][1] * BuM[1][12]  +  JTinv[2][2] * BuM[2][12];
    Epsilon[5][14] = JTinv[0][0] * BwM[0][14]  +  JTinv[0][1] * BwM[1][14]  +  JTinv[0][2] * BwM[2][14];
    Epsilon[5][15] = JTinv[2][0] * BuM[0][15]  +  JTinv[2][1] * BuM[1][15]  +  JTinv[2][2] * BuM[2][15];
    Epsilon[5][17] = JTinv[0][0] * BwM[0][17]  +  JTinv[0][1] * BwM[1][17]  +  JTinv[0][2] * BwM[2][17];
    Epsilon[5][18] = JTinv[2][0] * BuM[0][18]  +  JTinv[2][1] * BuM[1][18]  +  JTinv[2][2] * BuM[2][18];
    Epsilon[5][20] = JTinv[0][0] * BwM[0][20]  +  JTinv[0][1] * BwM[1][20]  +  JTinv[0][2] * BwM[2][20];
    Epsilon[5][21] = JTinv[2][0] * BuM[0][21]  +  JTinv[2][1] * BuM[1][21]  +  JTinv[2][2] * BuM[2][21];
    Epsilon[5][23] = JTinv[0][0] * BwM[0][23]  +  JTinv[0][1] * BwM[1][23]  +  JTinv[0][2] * BwM[2][23];
  }




/** TODO: Add comment
*/
//------------------------------------------------------------------------------
void GetGaussPoint(int i, double coordinates[3])
//------------------------------------------------------------------------------
{
  const double a = 0.57735027;
  const double a1 = (1 - a) / 2.0;
  const double a2 = (1 + a) / 2.0;
  double points[8][3] = { {a1, a1, a1}, {a2, a1, a1}, {a2, a2, a1}, { a1, a2, a1},
  {a1, a1, a2}, {a2, a1, a2}, {a2, a2, a2}, {a1, a2, a2}};

  coordinates[0] = points[i][0]; 
  coordinates[1] = points[i][1]; 
  coordinates[2] = points[i][2]; 
}


/** TODO: Add comment
*/
//------------------------------------------------------------------------------
void CreateGaussPointMatrix(double matrix[8][8])
//------------------------------------------------------------------------------
{
  for (int i=0; i<8; i++)
  {
    double GP[3];
    GetGaussPoint(i, GP);
    matrix[i][0] = 1;
    matrix[i][1] = GP[0];
    matrix[i][2] = GP[1];
    matrix[i][3] = GP[3];
    matrix[i][4] = GP[0] * GP[1];
    matrix[i][5] = GP[1] * GP[2];
    matrix[i][6] = GP[2] * GP[0];
    matrix[i][7] = GP[0] * GP[1] * GP[2];
  }

}




/** TODO: Add comment
*/
//------------------------------------------------------------------------------
void TransformComponentToScalars(vtkImageData *volume, vtkDataArray* dataArray, int component, int min, int max, int type)
//------------------------------------------------------------------------------
{
  // sanity checks
  assert(volume);
  if (!volume)
    return;
  
  assert(dataArray);
  if (!dataArray)
    {
    volume->GetPointData()->GetScalars()->Reset(); // clear the scalar data in the volume 
    return;
    }


  // perform sanity checks
  int numTuples = dataArray->GetNumberOfTuples();
  if (numTuples <= 0)
    return; 
  int numComponents = dataArray->GetNumberOfComponents();
  if (numComponents <= component)     // invalid component index
    return;
  if (min > max) // swap
    { unsigned int tmp = min; min = max; max = tmp; }


  // run through the volume and get min and max values and the range
  double value[9];              // tensors have 9 components
  double origMin, origMax;
  dataArray->GetTuple(0, value);
  origMin = origMax = value[component];

  double tensorRange[2];
  dataArray->GetRange(tensorRange);
  origMin = tensorRange[0];
  origMax = tensorRange[1];

  double origRange = origMax-origMin;
  double factor  = (max-min)/origRange;


  // create new array of scalars that represent the volume - choose the appropriate data type according to the parameter type
  // fast solution, but not best looking
  vtkDataArray* createdArray = NULL;
  if (type == 0)
    {
      vtkUnsignedShortArray *newScalars = vtkUnsignedShortArray::New();
      for (int i=0; i<numTuples; i++)
      {
        dataArray->GetTuple(i, value);
        int cvalue = min + (int)(factor * value[component] + 0.5);
        newScalars->InsertValue(i, cvalue);
      }
      createdArray = newScalars;
    } 
  else 
    {
    vtkIntArray *newScalars = vtkIntArray::New();
    for (int i=0; i<numTuples; i++)
    {
      dataArray->GetTuple(i, value);
      int cvalue = min + (int)(factor * value[component] + 0.5);
      newScalars->InsertValue(i, cvalue);
    }
    createdArray = newScalars;
  } 

  // replace the old array
  volume->GetPointData()->SetScalars(createdArray);
  volume->Update();
  createdArray->Delete();
}


//------------------------------------------------------------------------------
void SetArrayToVolume(vtkImageData *volume, vtkDoubleArray* dataArray, double origMin, double origMax, int destMin, int destMax, int type)
//------------------------------------------------------------------------------
{
  // sanity checks
  assert(volume);
  if (!volume)
    return;

  assert(dataArray);
  if (!dataArray)
  {
    volume->GetPointData()->GetScalars()->Reset(); // clear the scalar data in the volume 
    return;
  }

  int numTuples = dataArray->GetNumberOfTuples();
  assert (numTuples >0 && volume->GetPointData()->GetScalars()->GetNumberOfTuples() == numTuples);
  if (numTuples <= 0 || volume->GetPointData()->GetScalars()->GetNumberOfTuples() != numTuples)
    return; 

  // arrange min and max values in right order
  if (origMin > origMax) 
   { unsigned int tmp = origMin; origMin = origMax; origMax = tmp; }

  if (destMin > destMax) // swap
    { unsigned int tmp = destMin; destMin = destMax; destMax = tmp; }


  if (origMax - origMin == 0.0)  // this can happen if the dataset is 'empty'
    return;
  double factor  = abs(destMax-destMin)/abs(origMax - origMin);

  double* range = dataArray->GetRange();

  vtkDataArray* newScalars = NULL;
  if (type == 0)
    {
    vtkUnsignedShortArray* newScalarsS = vtkUnsignedShortArray::New();
    newScalarsS->SetNumberOfComponents(1);
    newScalarsS->SetNumberOfTuples(numTuples);
    for (int id=0; id<numTuples; id++)
      {
      double value = dataArray->GetValue(id);
      double result = factor * value;
      double finalResult = destMin + (int(factor * value + 0.5));
      newScalarsS->SetValue(id, (unsigned short)(destMin + (int)(factor * dataArray->GetValue(id) + 0.5)));
      }
    newScalars = newScalarsS;
    }
  else if (type == 1)
    {
    vtkIntArray* newScalarsI = vtkIntArray::New();
    newScalarsI->SetNumberOfComponents(1);
    newScalarsI->SetNumberOfTuples(numTuples);
      for (int id=0; id<numTuples; id++)
      {
      double value = dataArray->GetValue(id);
      double result = factor * value;
      double finalResult = destMin + (int(factor * value + 0.5));
      newScalarsI->SetValue(id, destMin + (int)(factor * dataArray->GetValue(id) + 0.5));
      }
    newScalars = newScalarsI;
    }
  // replace the old array
  volume->GetPointData()->SetScalars(newScalars);
  volume->Update();
  newScalars->Delete();
}






