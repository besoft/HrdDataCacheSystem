/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: PKUtils.cpp,v $ 
  Language: C++ 
  Date: $Date: 2011-06-30 10:05:13 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: Petr Kellnhofer
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

#include "PKUtils.h"

//////
/// Hidden contructor of static class.
//////
PKUtils::PKUtils(void)
{
}

//////
/// Hidden destructor of static class.
//////
PKUtils::~PKUtils(void)
{
}

//////
/// Creates new matrix.
/// @param height height of new matrix (input)
/// @param width width of new matrix (input)
/// @return new matrix
//////
PKMatrix* PKUtils::CreateMatrix(int height, int width)
{
	if (height <=0 || width <= 0)
	{
		throw "Impossible size.";
	}

	double** values = new double*[height];
	for (int i = 0; i < height; i++)
	{
		values[i] = new double[width];
		memset(values[i], 0, sizeof(double) * width);
	}

	PKMatrix *matrix = new PKMatrix();
	matrix->values = values;
	matrix->height = height;
	matrix->width = width;

	return matrix;
}

//////
/// Creates deep copy of matrix.
/// @param matrix pointer to original matrix (input)
/// @return deep copy of matrix
//////
PKMatrix* PKUtils::CloneMatrix(PKMatrix *matrix)
{
	int height = matrix->height;
	int width = matrix->width;

	if (height <=0 || width <= 0)
	{
		throw "Impossible size.";
	}

	if (matrix == NULL)
	{
		return NULL;
	}

	double** valCopy = new double*[height];
	for (int i = 0; i < height; i++)
	{
		valCopy[i] = new double[width];
		memcpy(valCopy[i], matrix->values[i], sizeof(double) * width);
	}

	PKMatrix *copy = new PKMatrix();
	copy->values = valCopy;
	copy->height = height;
	copy->width = width;

	return copy;
}

//////
/// Erases content of matrix.
/// @param matrix pointer to original matrix (input and output)
//////
void PKUtils::EraseMatrix(PKMatrix *matrix)
{
	int height = matrix->height;
	int width = matrix->width;

	for (int i = 0; i < height; i++)
	{
		memset(matrix->values[i], 0, sizeof(double) * width);
	}
}

//////
/// Disposes matrix row by row.
/// @param matrix pointer to matrix (input)
//////
void PKUtils::DisposeMatrix(PKMatrix** matrix)
{
	if (matrix == NULL)
	{
		return;
	}

	int height = (*matrix)->height;

	for (int i = 0; i < height; i++)
	{
		delete[] ((*matrix)->values[i]);
	}

	delete[] (*matrix)->values;
	delete (*matrix);
	*matrix = NULL;
}

//////
/// Measures difference of two n-lentgh vertices using MAX norm.
/// @param vertexA first measured vertex (input)
/// @param vertexB second measured vertex (input)
/// @param length length of both vertices (input)
/// @return max norm of vertex difference
//////
double PKUtils::MeasureVertexDifference(const double* vertexA, const double* vertexB, const int length) 
{
	double maxDifference = 0.0;
	double difference;

	for (int i = 0; i < length; i++, vertexA++, vertexB++) 
	{
		difference = abs((*vertexA) - (*vertexB));

		if (difference > maxDifference) 
		{
			maxDifference = difference;
		}
	}
	
	return maxDifference;
}

//////
/// Measures difference of two m x n matrices using MAX norm.
/// @param A first measured matrix (input)
/// @param B second measured matrix (input)
/// @return max norm of matrix difference
//////
double PKUtils::MeasureMatrixDifference(PKMatrix *A, PKMatrix *B)
{
	int width = min(A->width, B->width);

	double **endRow = A->values + min(A->height, B->height);
	double maxDifference = 0.0;
	double **rowA = A->values;
	double **rowB = B->values;

	for ( ; rowA < endRow; rowA++, rowB++)
	{
		double currDiff = PKUtils::MeasureVertexDifference(*rowA, *rowB, width);

		if (currDiff > maxDifference) 
		{
			maxDifference = currDiff;
		}
	}

	return maxDifference;
}

//////
/// Measures max norm of vertex of specified length. Max norm is
/// maximum absolute value of any scalar inside vertex.
/// @param vertex input vertex (input)
/// @param length length of vertex (input)
/// @return max norm of vertex
//////
double PKUtils::MeasureVertexMaxNorm(double *vertex, int length)
{
	double max = 0.0;

	for (int i = 0; i < length; i++, vertex++) 
	{
		if (fabs(*vertex) > max) 
		{
			max = fabs(*vertex);
		}
	}
	
	return max;
}

//////
/// Measures max norm of matrix. Max norm is
/// maximum absolute value of any value in matrix.
/// @param matrix matrix to be measured (input)
/// @return max norm of vertex
//////
double PKUtils::MeasureMatrixMaxNorm(PKMatrix *matrix)
{
	double max = 0.0;

	int width = matrix->width;

	double **endRow = matrix->values + matrix->height;
	double **row = matrix->values;

	for ( ; row < endRow; row++)
	{
		double currMax = PKUtils::MeasureVertexMaxNorm(*row, width);

		if (currMax > max) 
		{
			max = currMax;
		}
	}
	
	return max;
}

//////
/// Merges two matrices by joining their rows, therefore without any copying.
/// Warning: Changing or disposing of merged matrices destroys content of original matrix.
/// @param matrixA first merged matrix (input)
/// @param matrixB second merged matrix (input)
/// @return merged matrix
//////
PKMatrix* PKUtils::MergeMatricesVerticallySoft(PKMatrix *matrixA, PKMatrix *matrixB)
{
	if (matrixA == NULL && matrixB == NULL)
	{
		throw "At least one matrix must be valid.";
	}

	int heightA = matrixA != NULL ? matrixA->height : 0;
	int heightB = matrixB != NULL ? matrixB->height : 0;
	int widthA = matrixA != NULL ? matrixA->width : matrixB->width;
	int widthB = matrixB != NULL ? matrixB->width : matrixA->width;

	double **mergedValues = new double*[heightA + heightB];
	for (int i = 0; i < heightA; i++)
	{
		mergedValues[i] = matrixA->values[i];
	}

	for (int i = 0; i < heightB; i++)
	{
		mergedValues[heightA + i] = matrixB->values[i];
	}

	PKMatrix *result = new PKMatrix();
	result->values = mergedValues;
	result->height = heightA + heightB;
	result->width = min(widthA, widthB);
	return result;
}

//////
/// Negates matrix value per value by multypliyng by -1.
/// @param matrix input matrix (input and output)
//////
void PKUtils::NegateMatrix(PKMatrix *matrix)
{
	int width = matrix->width;
	double **rowPointer = matrix->values;
	double **endRow = rowPointer + matrix->height;
	for ( ; rowPointer < endRow; rowPointer++)
	{
		double* row = *rowPointer;
		double* endColumn = row + width;

		for ( ; row < endColumn; row++)
		{
			*row = -(*row);
		}
	}
}

//////
/// Multiplies matrix A by matrix B. Result = A * B.
/// Width of A must be equal to height of B.
/// Height of res equals to height of A, width to width of B.
/// Uses multithreading provided by OpenMP on level of rows.
/// @param A first operand matrix (input)
/// @param B second operand matrix (input)
/// @param result result of A * B, must be preallocated (output)
//////
void PKUtils::MultiplyMatrices(PKMatrix *A, PKMatrix *B, PKMatrix *result)
{
	if (A->width != B->height)
	{
		throw "Matrix sizes must match.";
	}

	int height = A->height;
	int width = B->width;
	int widthA = A->width;

	if (result->height != height || result->width != width)
	{
		throw "Result matrix has invalid dimensions.";
	}

	if (A == result) {
		throw "Cannot multiply to first operand";
	}

	int i, j;

	PKMatrix *Bt = PKUtils::CreateMatrix(B->width, B->height);
	PKUtils::TransposeMatrix(B, Bt);	

	#pragma omp parallel private(i, j) shared(A, Bt, widthA, width, height)
	{
		#pragma omp for
		for (i = 0; i < height; i++)
		{
			double *rowA = A->values[i];
			double *resultCell = result->values[i];
			double **pointerBt;

			for (j = 0, pointerBt = Bt->values; j < width; j++, pointerBt++, resultCell++)
			{
				(*resultCell) = PKUtils::DotN(rowA, *pointerBt, widthA);
			}
		}
	}

	PKUtils::DisposeMatrix(&Bt);
}

//////
/// Transposes matrix.
/// @param A input matrix (input)
/// @param result preallocated output matrix of reversed dimensions (output)
//////
void PKUtils::TransposeMatrix(PKMatrix *A, PKMatrix *result)
{
	int newHeight = A->width;
	int newWidth = A->height;
	
	for (int i = 0; i < newHeight; i++)
	{
		for (int j = 0; j < newWidth; j++)
		{
			result->values[i][j] = A->values[j][i];
		}
	}
}

//////
/// Adds scalar value to matrix value per value.
/// @param matrix input matrix (input)
/// @param value value to add (input)
/// @param result preallocated output matrix of identical dimensions, matrixA may be used here (output)
//////
void PKUtils::AddScalarToMatrix(PKMatrix *matrix, double value, PKMatrix *result)
{
	double **rowPointer = matrix->values;
	double **endRow = rowPointer + matrix->height;

	double **resRowPointer = result->values;
	int width = matrix->width;

	for ( ; rowPointer < endRow; rowPointer++, resRowPointer++)
	{
		double* row = *rowPointer;
		double* resRow = *resRowPointer;
		double* endColumn = row + width;

		for ( ; row < endColumn; row++, resRow++)
		{
			*resRow = (*row) + value;
		}
	}
}

//////
/// Multiplies matrix by scalar value.
/// @param inputMatrix input matrix (input)
/// @param multiplier multiplier value (input)
/// @param result preallocated output matrix of identical dimensions, inputMatrix may be used here (output)
//////
void PKUtils::MultiplyMatrixByScalar(PKMatrix *inputMatrix, double multiplier, PKMatrix *result)
{
	double **rowPointer = inputMatrix->values;
	double **endRow = rowPointer + inputMatrix->height;

	double **resRowPointer = result->values;
	int width = inputMatrix->width;

	for ( ; rowPointer < endRow; rowPointer++, resRowPointer++)
	{
		double* row = *rowPointer;
		double* resRow = *resRowPointer;
		double* endColumn = row + width;

		for ( ; row < endColumn; row++, resRow++)
		{
			*resRow = (*row) * multiplier;
		}
	}
}

//////
/// Sums all values in matrix.
/// @param matrix input matrix (input)
/// @return sum of matrix values
//////
double PKUtils::SummarizeMatrixValues(PKMatrix *matrix)
{
	if (matrix == NULL)
	{
		throw "Null value exception.";
	}

	double sum = 0;

	double **rowPointer = matrix->values;
	double **endRow = rowPointer + matrix->height;

	for ( ; rowPointer < endRow; rowPointer++)
	{
		double *valuePointer = *rowPointer;
		double *endValue = valuePointer + matrix->width;

		for ( ; valuePointer < endValue; valuePointer++)
		{
			sum += (*valuePointer);
		}
	}

	return sum;
}

//////
/// Linearizes matrix row by row creating new matrix of height 1 and width of oldHeight x oldWidth.
/// @param matrix input matrix (input)
/// @param result preallocated matrix of width 1 for linearization of input matrix (output)
//////
void PKUtils::LinearizeMatrix(PKMatrix *matrix, PKMatrix *result)
{
	if (matrix == NULL || result == NULL)
	{
		throw "Null value exception.";
	}

	int originalWidth = matrix->width;
	double **rowPointer = matrix->values;

	for (int i = 0; i < matrix->height; i++, rowPointer++)
	{
		memcpy(result->values[0] + i * originalWidth, *rowPointer, originalWidth * sizeof(double)); 
	}
}

//////
/// Inverts matrix.
/// @param original input matrix (input)
/// @param inverted output matrix, must be preallocated (output)
/// @return true on success
//////
bool PKUtils::InvertMatrix(PKMatrix *original, PKMatrix *inverted)
{
	PKMatrix *originalClone = PKUtils::CloneMatrix(original);
	int *tmp1Size = new int[originalClone->width];
	double *tmp2Size = new double[originalClone->width];
	
	bool res = vtkMath::InvertMatrix(originalClone->values, inverted->values, originalClone->width, tmp1Size, tmp2Size) != 0;

	delete[] tmp1Size;
	delete[] tmp2Size;
	PKUtils::DisposeMatrix(&originalClone);

	return res;
}

// http://www.opengl.org/sdk/docs/man/xhtml/glRotate.xml
void PKUtils::RotationMatrixGeneral(double axis[3], double angle, RotationMatrix *matrix) {
	PKUtils::NormalizeVertex(axis);

	double x = axis[0];
	double y = axis[1];
	double z = axis[2];

	double c = cos(angle);
	double s = sin(angle);

	matrix->values[0][0] = x * x * (1 - c) + c;
	matrix->values[0][1] = x * y * (1 - c) - z * s;
	matrix->values[0][2] = x * z * (1 - c) + y * s;

	matrix->values[1][0] = y * x * (1 - c) + z * s;
	matrix->values[1][1] = y * y * (1 - c) + c;
	matrix->values[1][2] = y * z * (1 - c) - x * s;

	matrix->values[2][0] = x * z * (1 - c) - y * s;
	matrix->values[2][1] = y * z * (1 - c) + x * s;
	matrix->values[2][2] = z * z * (1 - c) + c;
}


void PKUtils::GetRotation(double dirA[3], double dirB[3], double axis[3], double &angle) {
	vtkMath::Cross(dirA, dirB, axis);

	double sinus = PKUtils::CalculateVertexLength(axis);
	double cosin = PKUtils::Dot(dirA, dirB);
	
	// dirA || dirB => use any perpendicular vertex as axis
	if (abs(sinus) < 0.00001) {
		if (abs(dirA[0]) < 0.8) {
			double temp[] = { 1, 0, 0 };
			vtkMath::Cross(dirA, temp, axis);
			PKUtils::NormalizeVertex(axis);
		} else {
			double temp[] = { 0, 1, 0 };
			vtkMath::Cross(dirA, temp, axis);
			PKUtils::NormalizeVertex(axis);
		}
	} else {
		// normalize axis
		PKUtils::DivideVertex(axis, 1 / sinus);
	}

	// get angle
	double angleSin = asin(sinus);
	double angleCos = acos(cosin);

	if (sinus >= 0) {
		if (cosin >= 0) {
			angle = angleSin;
		} else {
			angle = angleCos;
		}
	} else {
		if (cosin >= 0) {
			angle = angleSin;
		} else {
			angle = -angleCos;
		}
	}
}

double* PKUtils::PKMatrixToArray(PKMatrix* A)
{
	int width =A->width ;
	int height =A->height ;
	double* memarray =(double*) malloc(width*height*sizeof(double)); // malloc
	
	//copy values into column-major order
	//i.e., 
	//1 2 3 
	//4 5 6
	//=> 1 4 2 5 3 6
	for(int j = 0; j < width; j++) 
	{
		for(int i = 0; i< height; i ++) {		
			memarray[i + height*j]= A->values[i][j];
		}
	}

	return memarray;
}

double* PKUtils::PKMatrixTransposedToArray(PKMatrix* A)
{
	int width =A->width ;
	int height =A->height ;
	double* memarray =(double*) malloc(width*height*sizeof(double)); // malloc
	
	//copy values in row-major order
	//i.e., 
	//1 2 3 
	//4 5 6
	//=> 1 2 3 4 5 6
	for(int j = 0; j < height; j++) {
		for(int i = 0; i< width; i ++) {		
			memarray[i + width*j]= A->values[j][i];
		}
	}

	return memarray;
}

PKMatrix* PKUtils::ArrayToPKMatrix(double* memarray, int width, int height)
{
	PKMatrix * matice= CreateMatrix(height,width);	
	//copy values from column-major order
	//i.e., 
	//1 2 3 
	//4 5 6
	//=> 1 4 2 5 3 6
	for(int j = 0; j < width; j++) 
	{
		for(int i = 0; i< height; i ++) {		
			 matice->values[i][j] = memarray[i + height*j];
		}
	}
	
	return matice;
}

void PKUtils::ArrayToPKMatrix(double* memarray, PKMatrix * matice)
{
	int width = matice->width;
	int height = matice->height;

	//copy values
	for(int i = 0; i < height; i ++)
		for(int j = 0; j < width; j++)
			 matice->values[i][j] = memarray[i + height*j];

}


#ifdef _DEBUG_OUTPUT_PKMATRIX
static bool g_DebugOutputFirst = true;
void PKMatrix::DebugOutput(const char* szText)
{
	FILE* fOut = fopen("PKMatrix.log", g_DebugOutputFirst ? "wt" : "at");
	fprintf(fOut, "===================\n");
	fprintf(fOut, szText);
	fprintf(fOut, "\tHeight: %d\tWidth: %d\n", this->height, this->width);
	fprintf(fOut, "-------------------\n");

	for (int i = 0; i < this->height; i++)
	{
		for (int j = 0; j < this->width; j++)
		{
			fprintf(fOut, "%f\t", this->values[i][j]);	
		}

		fprintf(fOut, "\n");
	}
	fprintf(fOut, "===================\n");
	fclose(fOut);
	g_DebugOutputFirst = false;
}

void PKMatrix::DebugOutputMathematica(const char* filename)
{
	FILE* fOut = fopen(filename, "wt");
	fprintf(fOut, "M = {");

	for (int i = 0; i < this->height; i++)
	{
		fprintf(fOut, "{");

		for (int j = 0; j < this->width; j++)
		{
			fprintf(fOut, "%f", this->values[i][j]);	
			
			if (j < this->width - 1) {
				fprintf(fOut, ", ");
			}
		}

		fprintf(fOut, "}");

		if (i < this->height - 1) {
			fprintf(fOut, ",\n");
		}
	}
	fprintf(fOut, "};\n");
	fclose(fOut);
	g_DebugOutputFirst = false;
}
#endif