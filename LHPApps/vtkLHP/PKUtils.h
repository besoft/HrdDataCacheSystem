/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: PKUtils.h,v $ 
  Language: C++ 
  Date: $Date: 2012-02-08 14:27:41 $ 
  Version: $Revision: 1.1.2.8 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#ifndef PKUtils_h__
#define PKUtils_h__

#pragma once

//#define _DEBUG_OUTPUT_PKMATRIX

// std
#include <cmath>
#if defined(_MSC_VER) && _MSC_VER >= 1500
#pragma message("Make sure that the project is built with /openmp")
#include <omp.h> // compile with: /openmp 
#endif

// VTK
#include "vtkMath.h"
#include "vtkPolyData.h"

//////
/// PI value used in calculations.
//////
#define PI 3.14159265358979323846264338327950288419716939937510

//////
/// Returns smaller of two values.
//////
#ifndef min
#define min(a, b) ((a) <= (b) ? (a) : (b))
#endif

//////
/// Returns greater of two values.
//////
#ifndef max
#define max(a, b) ((a) >= (b) ? (a) : (b))
#endif

#ifndef sign
#define sign(a) ((a) > 0 ? (1) : ((a) < 0 ? (-1) : (0)))
#endif

#ifndef mySwap
#define mySwap(a, b, type) \
	{\
		type tempGq3w5dyaUNIQUE = a;\
		a = b;\
		b = tempGq3w5dyaUNIQUE;\
	}

#endif

typedef struct Vector3_ {
	Vector3_() {
		memset(this, 0, sizeof(Vector3_));
	}

	Vector3_(double src[3]) {
#if _MSC_VER >= 1400 // grater than vs 2005
	memcpy_s(this, sizeof(Vector3_), src, sizeof(double) * 3);
#else
	memcpy(this,src,sizeof(double) * 3);
#endif
	}

	union {
		struct {
			double x;
			double y;
			double z;
		};
		double data[3];
	};

} Vector3;


//////
/// Matrix structure for algebraic functions.
//////
class PKMatrix
{
public:
//	double* linear_values;	//matrix values in a linear order 

	double **values; // matrix values as array of arrays
	int height;		 // height = length of **values
	int width;		 // width = length of each *values
	
#ifdef _DEBUG_OUTPUT_PKMATRIX
public:
	void DebugOutput(const char* szText);
	void DebugOutputMathematica(const char* filename);
#endif
};

class RotationMatrix: public PKMatrix 
{
public:
	RotationMatrix() {
		this->height = 3;
		this->width = 3;

		this->values = new double*[3];
		this->values[0] = new double[3];
		this->values[1] = new double[3];
		this->values[2] = new double[3];

		memset(this->values[0], 0, 3 * sizeof(double));
		this->values[0][0] = 1;
		memset(this->values[1], 0, 3 * sizeof(double));
		this->values[1][1] = 1;
		memset(this->values[2], 0, 3 * sizeof(double));
		this->values[2][2] = 1;
	}

	~RotationMatrix() {
		delete this->values[0];
		delete this->values[1];
		delete this->values[2];
		delete this->values;
	}

};

//////
/// Static class with math utilites from linear algebra.
/// Mainly consits of vertex and matrix operations.
//////
class PKUtils
{
public:
	//////
	/// Measures difference of two n-lentgh vertices using MAX norm.
	/// @param vertexA first measured vertex (input)
	/// @param vertexB second measured vertex (input)
	/// @param length length of both vertices (input)
	/// @return max norm of vertex difference
	//////
	static double MeasureVertexDifference(const double* vertexA, const double* vertexB, const int length);

	//////
	/// Measures difference of two m x n matrices using MAX norm.
	/// @param A first measured matrix (input)
	/// @param B second measured matrix (input)
	/// @return max norm of matrix difference
	//////
	static double MeasureMatrixDifference(PKMatrix *A, PKMatrix *B);

	//////
	/// Measures max norm of vertex of specified length. Max norm is
	/// maximum absolute value of any scalar inside vertex.
	/// @param vertex input vertex (input)
	/// @param length length of vertex (input)
	/// @return max norm of vertex
	//////
	static double MeasureVertexMaxNorm(double *vertex, int length);

	//////
	/// Measures max norm of matrix. Max norm is
	/// maximum absolute value of any value in matrix.
	/// @param matrix matrix to be measured (input)
	/// @return max norm of vertex
	//////
	static double MeasureMatrixMaxNorm(PKMatrix *matrix);

	//////
	/// Merges two matrices by joining their rows, therefore without any copying.
	/// Warning: Changing or disposing of merged matrices destroys content of original matrix.
	/// @param matrixA first merged matrix (input)
	/// @param matrixB second merged matrix (input)
	/// @return merged matrix
	//////
	static PKMatrix* MergeMatricesVerticallySoft(PKMatrix *matrixA, PKMatrix *matrixB);

	//////
	/// Multiplies matrix A by matrix B. Result = A * B.
	/// Width of A must be equal to height of B.
	/// Height of res equals to height of A, width to width of B.
	/// Uses multithreading provided by OpenMP on level of rows.
	/// @param A first operand matrix (input)
	/// @param B second operand matrix (input)
	/// @param result result of A * B, must be preallocated (output)
	//////
	static void MultiplyMatrices(PKMatrix *A, PKMatrix *B, PKMatrix *result);

	//////
	/// Negates matrix value per value by multypliyng by -1.
	/// @param matrix input matrix (input and output)
	//////
	static void NegateMatrix(PKMatrix *matrix);

	//////
	/// Inverts matrix.
	/// @param original input matrix (input)
	/// @param inverted output matrix, must be preallocated (output)
	/// @return true on success
	//////
	static bool InvertMatrix(PKMatrix *original, PKMatrix *inverted);

	//////
	/// Adds scalar value to matrix value per value.
	/// @param matrix input matrix (input)
	/// @param value value to add (input)
	/// @param result preallocated output matrix of identical dimensions, matrixA may be used here (output)
	//////
	static void AddScalarToMatrix(PKMatrix *matrix, double value, PKMatrix *result);

	//////
	/// Multiplies matrix by scalar value.
	/// @param inputMatrix input matrix (input)
	/// @param multiplier multiplier value (input)
	/// @param result preallocated output matrix of identical dimensions, inputMatrix may be used here (output)
	//////
	static void MultiplyMatrixByScalar(PKMatrix *inputMatrix, double multiplier, PKMatrix *result);

	//////
	/// Sums all values in matrix.
	/// @param matrix input matrix (input)
	/// @return sum of matrix values
	//////
	static double SummarizeMatrixValues(PKMatrix *matrix);

	//////
	/// Linearizes matrix row by row creating new matrix of height 1 and width of oldHeight x oldWidth.
	/// @param matrix input matrix (input)
	/// @param result preallocated matrix of width 1 for linearization of input matrix (output)
	//////
	static void LinearizeMatrix(PKMatrix *matrix, PKMatrix *result);

	//////
	/// Transposes matrix.
	/// @param A input matrix (input)
	/// @param result preallocated output matrix of reversed dimensions (output)
	//////
	static void TransposeMatrix(PKMatrix *A, PKMatrix *result);
	
	//////
	/// Creates new matrix.
	/// @param height height of new matrix (input)
	/// @param width width of new matrix (input)
	/// @return new matrix
	//////
	static PKMatrix* CreateMatrix(int height, int width);

	//////
	/// Erases content of matrix.
	/// @param matrix pointer to original matrix (input and output)
	//////
	static void EraseMatrix(PKMatrix *matrix);

	//////
	/// Disposes matrix row by row.
	/// @param matrix pointer to matrix (input)
	//////
	static void DisposeMatrix(PKMatrix** matrix);

	//////
	/// Creates deep copy of matrix.
	/// @param matrix pointer to original matrix (input)
	/// @return deep copy of matrix
	//////
	static PKMatrix* CloneMatrix(PKMatrix *matrix);

	static void GetRotation(double dirA[3], double dirB[3], double axis[3], double &angle);

	//////
	/// Disposes matrix container (structure) only, leaves content (rows) allocated and intact.
	/// Usually used to safely dispose remains of matrix soft merge.
	/// @param matrixContainer double pointer to matrix whose container should be destroyed (input)
	//////
	static void DisposeMatrixContainer(PKMatrix **matrixContainer)
	{
		if ((*matrixContainer) != NULL)
		{
			if ((*matrixContainer)->values != NULL)
			{
				delete (*matrixContainer)->values;
			}
			delete (*matrixContainer);
		}
		
		(*matrixContainer) = NULL;
	}

	static void RotateMatrixX(double angle, RotationMatrix *matrix) {
		memset(matrix->values, 0, 9 * sizeof(double));

		double c = cos(angle);
		double s = sin(angle);

		matrix->values[0][0] = 1;
		matrix->values[1][1] = c;
		matrix->values[1][2] = -s;
		matrix->values[2][1] = s;
		matrix->values[2][2] = c;
	}

	static void RotateMatrixY(double angle, RotationMatrix *matrix) {
		memset(matrix->values, 0, 9 * sizeof(double));

		double c = cos(angle);
		double s = sin(angle);

		matrix->values[0][0] = c;
		matrix->values[0][2] = -s;
		matrix->values[1][1] = 1;
		matrix->values[2][0] = s;
		matrix->values[2][2] = c;
	}

	static void RotateMatrixZ(double angle, RotationMatrix *matrix) {
		memset(matrix->values, 0, 9 * sizeof(double));

		double c = cos(angle);
		double s = sin(angle);

		matrix->values[0][0] = c;
		matrix->values[0][1] = -s;
		matrix->values[1][0] = s;
		matrix->values[1][1] = c;
		matrix->values[2][2] = 1;
	}

	static void RotationMatrixGeneral(double axis[3], double angle, RotationMatrix *matrix);

	static inline void CopyVertex2(const double src[2], double dst[2])
	{
		dst[0] = src[0];
		dst[1] = src[1];
	}

	//////
	/// Copies 3D vertex values to another one.
	/// @param src source 3D vertex (input)
	/// @param dst preallocated destination 3D vertex (output)
	//////
	static inline void CopyVertex(const double src[3], double dst[3])
	{
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
	}

	static inline void CopyVertex4(const double src[4], double dst[4])
	{
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = src[3];
	}

	static inline void AddVertex2(const double v1[2], const double v2[2], double result[2])
	{
		result[0] = v1[0] + v2[0];
		result[1] = v1[1] + v2[1];
	}

	//////
	/// Adds 3D vertex v1 to v2 and stores result to result. (result = v1 + v2)
	/// @param v1 first operand 3D vertex (input)
	/// @param v2 second operand 3D vertex (input)
	/// @param result preallocated result 3D vertex (output)
	//////
	static inline void AddVertex(const double v1[3], const double v2[3], double result[3])
	{
		result[0] = v1[0] + v2[0];
		result[1] = v1[1] + v2[1];
		result[2] = v1[2] + v2[2];
	}

	static inline void SubtractVertex2(const double v1[2], const double v2[2], double result[2])
	{
		result[0] = v1[0] - v2[0];
		result[1] = v1[1] - v2[1];
	}

	//////
	/// Subtracts 3D vertex v2 from v1 and stores result to result. (result = v1 - v2)
	/// @param v1 first operand 3D vertex (input)
	/// @param v2 second operand 3D vertex (input)
	/// @param result preallocated result 3D vertex (output)
	//////
	static inline void SubtractVertex(const double v1[3], const double v2[3], double result[3])
	{
		result[0] = v1[0] - v2[0];
		result[1] = v1[1] - v2[1];
		result[2] = v1[2] - v2[2];
	}

	//////
	/// Multiplies 3D vertex by factor. (vertex = vertex * factor)
	/// @param vertex 3D vertex (input and output)
	/// @param factor multiplicator (input)
	//////
	static inline void MultiplyVertex(double vertex[3], const double factor)
	{
		vertex[0] *= factor;
		vertex[1] *= factor;
		vertex[2] *= factor;
	}

	//////
	/// Multiplies 3D vertex by factor. (vertex = vertex * factor)
	/// @param vertex 3D vertex (input and output)
	/// @param factor multiplicator (input)
	//////
	static inline void MultiplyVertex4(double vertex[4], const double factor)
	{
		vertex[0] *= factor;
		vertex[1] *= factor;
		vertex[2] *= factor;
		vertex[3] *= factor;
	}

	//////
	/// Divides 3D vertex by factor. (vertex = vertex / divider)
	/// @param vertex 3D vertex (input and output)
	/// @param factor divider (input)
	//////
	static inline void DivideVertex(double vertex[3], const double divider)
	{
		vertex[0] /= divider;
		vertex[1] /= divider;
		vertex[2] /= divider;
	}

	static inline double CalculateVertex2LengthSq(const double vertex[2])
	{
		return vertex[0] * vertex[0] + vertex[1] * vertex[1];
	}

	static inline double CalculateVertex2Length(const double vertex[2])
	{
		return sqrt(vertex[0] * vertex[0] + vertex[1] * vertex[1]);
	}

	//////
	/// Calculates squared length of 3D vertex using Euclid norm. (returns v dot v)
	/// Faster than normal length.
	/// @param vertex 3D vertex (input)
	/// @return squared Euclid norm of vertex
	//////
	static inline double CalculateVertexLengthSq(const double vertex[3])
	{
		return vertex[0] * vertex[0] + vertex[1] * vertex[1] + vertex[2] * vertex[2];
	}
	
	//////
	/// Calculates length of 3D vertex using Euclid norm. (returns sqrt(v dot v))
	/// @param vertex 3D vertex (input)
	/// @return Euclid norm of vertex
	//////
	static inline double CalculateVertexLength(const double vertex[3])
	{
		return sqrt(vertex[0] * vertex[0] + vertex[1] * vertex[1] + vertex[2] * vertex[2]);
	}

	static inline void NormalizeVertex(double vertex[3]) {
		PKUtils::DivideVertex(vertex, PKUtils::CalculateVertexLength(vertex));
	}

	static inline double CalculateVertex4LengthSq(const double vertex[4])
	{
		return vertex[0] * vertex[0] + vertex[1] * vertex[1] + vertex[2] * vertex[2] + vertex[3] * vertex[3];
	}

	static inline double CalculateVertex4Length(const double vertex[4])
	{
		return sqrt(vertex[0] * vertex[0] + vertex[1] * vertex[1] + vertex[2] * vertex[2] + vertex[3] * vertex[3]);
	}

	static inline void MultiplyMatrixVertex(PKMatrix *matrix, double *vertex, double *result) {
		for (int i = 0; i < matrix->height; i++) {
			result[i] = PKUtils::DotN(matrix->values[i], vertex, matrix->width);
		}
	}

	//////
	/// Calculates cotangens of angle between two vertices. (returns cot(v1, v2))
	/// @param v1 first 3D vertex (input)
	/// @param v2 second 3D vertex (input)
	/// @return cotan of angle defined by v1 and v2
	//////
	static inline double CalculateVertexCotan(const double v1[3], const double v2[3]) 
	{
		double length1 = PKUtils::CalculateVertexLength(v1);
		double length2 = PKUtils::CalculateVertexLength(v2);

		double cosValue = vtkMath::Dot(v1, v2) / (length1 * length2);
		double sinValue = sqrt(1 - cosValue * cosValue);

		return cosValue / sinValue;
	}

	//////
	/// Calculates signum of argument x. (returns sign(x))
	/// @param x argument (input)
	/// @return signum of x (-1/0/1)
	//////
	static inline double Sign(const double x)
	{
		if (x < 0)
		{
			return -1.0;
		}
		else if (x > 0)
		{
			return 1.0;
		}
		else 
		{
			return 0.0;	
		}
	}

	static inline double Dot2(const double v1[2], const double v2[2]) {
		return v1[0] * v2[0] + v1[1] * v2[1];
	}

	static inline double Dot(const double v1[3], const double v2[3]) {
		return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	}

	static inline double Dot4(const double v1[4], const double v2[4]) {
		return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] + v1[3] * v2[3];
	}

	//////
	/// Calculates dot product of two general vertices. (returns v1 * v2)
	/// @param v1 first n-length vertex (input)
	/// @param v2 second n-length vertex (input)
	/// @param length length of both vertices (input)
	/// @return dot product of vertices v1 adn v2
	//////
	static inline double DotN(const double *v1, const double *v2, const int length)
	{
		double product = 0;
		
		const double *v1End = v1 + length;

		for ( ; v1 < v1End; v1++, v2++)
		{
			product += (*v1) * (*v2);
		}		

		return product;
	}

	//////
	/// Adds one general vertex to another and stores result to target. (target = v1 + v2)
	/// @param v1 first argument n-length vertex (input)
	/// @param v2 second argument n-length vertex (input)
	/// @param target preallocated target n-length vertex (output)
	/// @param length length of both vertices (input)
	//////
	static inline void AddVerticesN(const double *v1, const double *v2, double *target, const int length)
	{
		double *targetEnd = target + length;
		for( ; target < targetEnd; v1++, v2++, target++)
		{
			*target = (*v1) + (*v2);
		}
	}

	//////
	/// Subtracts general vertex v2 from v1 and stores result to target. (target = v1 - v2)
	/// @param v1 first argument n-length vertex (input)
	/// @param v2 second argument n-length vertex (input)
	/// @param target preallocated target n-length vertex (output)
	/// @param length length of both vertices (input)
	//////
	static inline void SubtractVerticesN(const double *v1, const double *v2, double *target, const int length)
	{
		double *targetEnd = target + length;
		for( ; target < targetEnd; v1++, v2++, target++)
		{
			*target = (*v1) - (*v2);
		}
	}

	//////
	/// Adds one matrix to another and stores result to result. (result = A + B)
	/// @param A first argument m x n matrix (input)
	/// @param B second argument m x n matrix (input)
	/// @param target preallocated target m x n matrix (output)	
	//////
	static inline void AddMatrices(PKMatrix *A, PKMatrix *B, PKMatrix *result)
	{
		double **rowA = A->values;
		double **rowB = B->values;
		double **rowRes = result->values;
		double **endRow = rowRes + A->height;

		int width = A->width;
		
		for ( ; rowRes < endRow; rowRes++, rowA++, rowB++)
		{
			AddVerticesN(*rowA, *rowB, *rowRes, width);
		}
	}

	//////
	/// Subtracts matrix B from A and stores result to result. (result = A + B)
	/// @param A first argument m x n matrix (input)
	/// @param B second argument m x n matrix (input)
	/// @param target preallocated target m x n matrix (output)	
	//////
	static inline void SubtractMatrices(PKMatrix *A, PKMatrix *B, PKMatrix *result)
	{
		double **rowA = A->values;
		double **rowB = B->values;
		double **rowRes = result->values;
		double **endRow = rowRes + A->height;

		int width = A->width;
		
		for ( ; rowRes < endRow; rowRes++, rowA++, rowB++)
		{
			SubtractVerticesN(*rowA, *rowB, *rowRes, width);
		}
	}

	//////
	/// Calculates 4 dimensional cross product of 3 vertices. For usage with 3D homogenous coordinates.
	/// @param a input vertex of length 4 (input)
	/// @param b input vertex of length 4 (input)
	/// @param c input vertex of length 4 (input)
	/// @param product preallocated memory for cross product - vertex of length 4 (input)
	//////
	static inline void Cross4(const double a[4], const double b[4], const double c[4], double product[4])
	{
		product[0] = a[3] * b[2] * c[1] - a[2] * b[3] * c[1] - a[3] * b[1] * c[2] + a[1] * b[3] * c[2] + a[2] * b[1] * c[3] - a[1] * b[2] * c[3];
		product[1] = -a[3] * b[2] * c[0] + a[2] * b[3] * c[0] + a[3] * b[0] * c[2] - a[0] * b[3] * c[2] - a[2] * b[0] * c[3] + a[0] * b[2] * c[3];
		product[2] = a[3] * b[1] * c[0] - a[1] * b[3] * c[0] - a[3] * b[0] * c[1] + a[0] * b[3] * c[1] + a[1] * b[0] * c[3] - a[0] * b[1] * c[3];
		product[3] = -a[2] * b[1] * c[0] + a[1] * b[2] * c[0] + a[2] * b[0] * c[1] - a[0] * b[2] * c[1] - a[1] * b[0] * c[2] + a[0] * b[1] * c[2];
	}

	static inline double GetRandomDouble() {
		return rand() / (double)RAND_MAX;
	}

	static inline double GetRandomDouble(double max) {
		return PKUtils::GetRandomDouble() * max;
	}

	static inline bool IsSubMatrix(PKMatrix *matrix, PKMatrix *sub) {
		double bigRow[3];
		double subRow[3];


		for (int i = 0; i < matrix->height; i++) {
			PKUtils::CopyVertex(matrix->values[i], bigRow);
			PKUtils::CopyVertex(sub->values[i], subRow);

			double sizeBig = PKUtils::CalculateVertexLengthSq(bigRow);
			double sizeSub = PKUtils::CalculateVertexLengthSq(subRow);

			if (sizeSub < 0.00001) {
				continue;
			}

			// check size
			if (sizeSub > sizeBig + 0.00001) {
				return false;
			}

			double dot = PKUtils::Dot(bigRow, subRow) / (sqrt(sizeBig) * sqrt(sizeSub));

			double error = dot - (-1);

			if (dot > 0.00001) {
				return false;
			}
		}

		return true;
	}

	//////
	/// Binary search
	//////
	static inline int FindNearestElement(const double &sample, double* values, const int length) {
		int a = 0;
		int b = length - 1;

		while (a < b) {
			int pivot = (a + b) / 2;
			if (values[pivot] > sample) {
				b = pivot;
			} else {
				a = pivot;
			}
		}

		return a;
	}

private:
	//////
	/// Hidden contructor of static class.
	//////
	PKUtils(void);

	//////
	/// Hidden destructor of static class.
	//////
	~PKUtils(void);

public:
	//////
	/// Matrix memory serialization
	//////
	static double* PKMatrixToArray(PKMatrix* A);
	static double* PKMatrixTransposedToArray(PKMatrix* A);

	//////
	/// Array memory deserialization to matrix
	//////
	static PKMatrix* ArrayToPKMatrix(double* A, int width, int height);

	//////
	/// Array memory deserialization to preallocated matrix
	//////
	static void ArrayToPKMatrix(double* A, PKMatrix * matice);
};


#ifndef myGetPoint
inline double* myGetPoint(vtkPolyData *mesh, PKMatrix *points, vtkIdType i) {
	if (points == NULL) {
		return mesh->GetPoint(i);
	}
	else {
		return points->values[(int)i];
	}
}
#endif

#ifndef myGetPointDeep
inline void myGetPointDeep(vtkPolyData *mesh, PKMatrix *points, vtkIdType i, double point[3]) {
	if (points == NULL) {
		mesh->GetPoint(i, point);
	}
	else {
		PKUtils::CopyVertex(points->values[(int)i], point);
	}
}
#endif

#endif
