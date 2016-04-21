/*=========================================================================
Module:    $RCSfile: vtkMuscleDecomposer.h $
Language:  C++
Date:      $Date: 2011-08-07 21:10 $
Version:   $Revision: 0.1.0.0 $
Author:    David Cholt
Notes:
=========================================================================*/

#ifndef __vtkMuscleDecomposer_h
#define __vtkMuscleDecomposer_h

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "vtkstd/vector"
#include "vtkstd/algorithm"
#pragma warning(push)
#pragma warning(disable: 4996)
#include "vtkPolyDataToPolyDataFilter.h"
#include "vtkMath.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkOBBTree.h"
#include "vtkTubeFilter.h"
#pragma warning(pop)

#include <float.h>
#include <time.h> // for testing
#include <fstream>
#include <iomanip>
#include "vtkLHPConfigure.h"

//----------------------------------------------------------------------------
// Define:
//----------------------------------------------------------------------------
#define CONNECT_CLOSEST_TENDON_POINT 0
#define CONNECT_INTENDED_TENDON_POINT 1

#define RAY_POINT_DISTANCE 10000
#define MIN_THICKNESS 2

class VTK_vtkLHP_EXPORT vtkMuscleDecomposer : public vtkPolyDataSource
{
	///
	///  Filter instance setup methods.
	///
public:
#pragma region Nested data structure classes

	class Fiber
	{
	public:
		vtkstd::vector<int>  PointIDs;
		int Id;
		bool BBoundary;
		Fiber(void);
		~Fiber(void);
	};

	/**
	class name: Point
	Nested Point class
	Represents one point in dataset.*/
	class Point
	{
	public:
		double	DCoord[3];
		double  DNormal[3];
		bool	interpolated;

		int		Id;
		int		TendonConnectionID; // for border Fiber PointIDs, if BBoundary, this ID corresponds to tendon
		bool	BMuscle;	// indicates, if point is part of muscle or tendon
		bool    BBoundary;
		static int	count;

		// for interpolation purposes:
		double	a[3], b[3], c[3], d[3]; // (3 axes)
		double	distToNext;
		double  t;

		double thickness;

		/** constructor */
		Point(double *pCoord);
		/** destructor */
		~Point();

		void ComputeInterpolatedCurveCoord(double t, double * result, bool substract);
		// Assigns normal to this Point
		void SetNormal(double* normal);
	};

#pragma endregion

#pragma region Nested interpolator

#define SPLINE_NATURAL 0
#define SPLINE_PARABOLIC_RUNOUT 1
#define SPLINE_CUBIC_RUNOUT 2
#define SPLINE_CONSTRAINED 3

	class SplineInterpolator
	{
	public:
		SplineInterpolator(int splineType);
		~SplineInterpolator();

		void Interpolate(Fiber* fiber, vtkstd::vector<Point*> & points);

	private:
		int splineType;

		Fiber* fiber;   // Pointer to fiber data
		vtkstd::vector<Point*> pointCloud; // Point data
		int n; // number of points

		vtkstd::vector<double> subdiagonal;			// Diagonal under main diagonal (for all axes)
		vtkstd::vector<double> superdiagonal;		// Diagonal above main diagonal (for all axes)
		vtkstd::vector<double> diagonal[3];			// Main diagonal (for 3 axes - modified by calculations)
		vtkstd::vector<double> result[3];			// Resulting vector, containing S_i..S_n (for 3 axes)
		vtkstd::vector<double> rightHand[3];		// Right hand vector (for 3 axes)

		vtkstd::vector<double> derivations[3];		// Derivations for Constrained Splines

		void BuildParameter();						// Builds independent variable (parameter) t
		void BuildEquation();						// Creates sub, sup and diagonal vectors and righthand vectors
		void SolveEquation();						// Solves equation and associates weights to points
		void ComputeWeights();						// Computes weights for all points using eq. result

		void ComputeFibreDerivations();
		void ComputeConstrainedWeights();
	public:
		int GetInterpolationType();
	};

#pragma endregion

#pragma region MuscleDecompositor
	// TODO: TEMP, move to protected later
	// TODO: Clear everything from memory
	vtkPolyData *MuscleMesh;
	vtkPolyData *NormalMesh;

	static vtkMuscleDecomposer* New();
	vtkTypeRevisionMacro(vtkMuscleDecomposer, vtkPolyDataSource);

	void AddFiberData(vtkPolyData *Fibers);
	void AddTendonData(vtkPolyData *Tendons);
	void AddMeshData(vtkPolyData *Mesh);

	/** Get tendon-muscle snapping metod **/
	vtkGetMacro(TendonConnectionMethod, int);
	/** Set tendon-muscle snapping metod **/
	vtkSetMacro(TendonConnectionMethod, int);

	/** Get tendon-muscle snapping metod **/
	vtkGetMacro(InterpolationSubdivision, int);
	/** Set tendon-muscle snapping metod **/
	vtkSetMacro(InterpolationSubdivision, int);

	/** Get tendon-muscle snapping metod **/
	vtkGetMacro(SurfaceSubdivision, int);
	/** Set tendon-muscle snapping metod **/
	vtkSetMacro(SurfaceSubdivision, int);

	/** Get tendon-muscle snapping metod **/
	vtkGetMacro(ArtifactEpsilon, double);
	/** Set tendon-muscle snapping metod **/
	vtkSetMacro(ArtifactEpsilon, double);

	/** Get tendon-muscle snapping metod **/
	vtkGetMacro(InterpolationMethod, int);
	/** Set tendon-muscle snapping metod **/
	vtkSetMacro(InterpolationMethod, int);



	///
	///  Used variables and structures, support methods
	///
protected:
	/** constructor */
	vtkMuscleDecomposer();
	/** destructor */
	~vtkMuscleDecomposer();

	int fiberCount;
	int tendonCount;
	vtkstd::vector<Fiber*>			 Fibers;  // list of fibers of the muscle
	vtkstd::vector<Fiber*>			 Tendons; // list of tendons of the muscle
	vtkstd::vector<Fiber*>			 OutputFibers; // list of output fibers
	vtkstd::vector<Fiber*>			 BottomFibers;  // list of bottom fibers of the muscle
	vtkstd::vector<Fiber*>			 LeftFibers;	// list of left fibers
	vtkstd::vector<Fiber*>			 RightFibers;	// list of right fibers

	vtkstd::vector<Point*>			 PointCloud; // cloud of fiber points of the muscle

	// PROPERTIES
	int TendonConnectionMethod;
	int InterpolationMethod;
	int InterpolationSubdivision;
	int SurfaceSubdivision;
	double ArtifactEpsilon;


	/** Adds data to correct list */
	void AddGenericData( vtkstd::vector<Fiber*> & Where, vtkPolyData * Data );

	// Moves the muscle mesh so its centered in the fibre-defined muscle hull
	void AlignMesh();
	// Computes normals for all fiber points
	void ComputeNormals(void);

	/** Splits polydata to polylines in internal structures */
	void LoadFiber(vtkPolyData *data, vtkstd::vector<Fiber*> & Polyline);

	/** Computes distance between two PointIDs */
	double ComputePointDistance(int ID1, int ID2);
	/** Connects fibers to tendons based on proximity */
	void ConnectClosestTendons();
	void ConnectSortedTendons();

	// Computes muscle thinckness for fiber points
	void ComputeThickness(void);
	// Builds bottom layer of fibers from normals and thickness
	void BuildBottomFibers(float t);
	// Builds side layers after bottom layer has been built
	void BuildSideLayers(int subdivision, int interpolationType);

	/** Builds OutputMesh **/
	void BuildAndOutputConnectedData(vtkstd::vector<Fiber*> & Data, char* what);

	void RemoveArtifacts(double epsilon, vtkstd::vector<Fiber*> & Data);

	void InterpolateSurface(int subdivision, int interpolationType, vtkstd::vector<Fiber*> & Data);

	void InterpolateFibers(int subdivision, int interpolationType, vtkstd::vector<Fiber*> & Data, bool tendons);
	void InterpolateFiber(Fiber* fiber, int subdivision, SplineInterpolator *interpolator, int skipOnEnds, bool insertSingularity);

	void ComputeWeights( Fiber * Fiber );
	void Interpolate( Fiber * Fiber);

	/** Outputs vector of fibers */
	void OutputFibersData( vtkstd::vector<Fiber*> & Data, vtkPoints * PointIDs, vtkCellArray * lines );
	void OutputNormals();
	///
	///  Filter execution methods
	///
protected:
	vtkPolyData *OutputMesh;

	/** Build internal data structure */
	bool InitData();
	/** Filter execution */
	void Execute();
	/** Build internal mesh relationships */
	void BuildMesh();
	/** Hand over finished mesh */
	void DoneMesh();
	/** Clears the mesh from memory */
	void ClearMesh();

	///
	/// Console logging methods
	///
private:
	static void Log(char* what)
	{
		printf(what);
		printf("\n");
	}
	static void Log(char* what, int parameter)
	{
		printf(what);
		printf("%d", parameter);
		printf("\n");
	}

	static void Log(char* what, double parameter)
	{
		printf(what);
		printf("%f", parameter);
		printf("\n");
	}

	static void LogFormated(char* what, int parameter)
	{
		printf(what, parameter);
		printf("\n");
	}

	static void LogFormated(char* what, char* parameter)
	{
		printf(what, parameter);
		printf("\n");
	}

	static char* GetSplineType(int SplineType)
	{
		switch (SplineType)
		{
		case SPLINE_NATURAL:
			return "natural runout";
			break;
		case SPLINE_PARABOLIC_RUNOUT:
			return "parabolic runout";
			break;
		case SPLINE_CUBIC_RUNOUT:
			return "cubic runout";
			break;
		case SPLINE_CONSTRAINED:
			return "constrained spline";
			break;
		default:
			return "<INVALID>";
		}
	}
	/*
	///
	/// File logging methods
	///
	public:
	/** Use this method to create the log file with input filename parameter "what" from outside of the module!
	static void FLog(const char* what, bool endline)
	{
	ifstream t;
	ofstream out;
	t.open("Log.txt", ios::in);
	out.open("Log.txt", ios::out | ios::app);
	if (t.is_open())
	{
	t.close();
	}
	else
	{
	out << setw(40) << "File;" << setw(6) << "R;" << setw(8) << "OldPts;" << setw(6) << "Rgh;" << setw(6) << "Q;" << setw(8) << "PriTime;" << setw(8) << "DecTime;"  << setw(8) << "DecPts;" << endl;
	}
	out << setw(40) << what<<";";
	if (endline)
	out << endl;
	out.close();
	}

	private:
	static void FLog(double what, bool endline)
	{
	ofstream out;
	out.open("Log.txt", ios::out | ios::app);
	out << setw(6) << what<<";";
	if (endline)
	out << endl;
	out.close();
	}

	static void FLog(int what, bool endline)
	{
	ofstream out;
	out.open("Log.txt", ios::out | ios::app);
	out << setw(8) << what<<";";
	if (endline)
	out << endl;
	out.close();
	}
	*/
#pragma endregion

#pragma region Utils
	int PointIsWithinBounds(double point[3], double bounds[6], double delta[3]);


	// Computes Line-Line distance between lines defined by (p11, p12) and (p21, p22)
	double LineLineDistance(double* p11, double* p12, double* p21, double* p22);

	// Substracts point1 from point2 to result
	void Substract(double* point1, double* point2, double* result);

	void Add(double* point1, double* point2, double* result, double multiplication);

	double PointPointDistance(double* point1, double* point2);

	// Computes normal of from 3 points (point1 being the normal origin)
	void ComputeNormal(double* point1, double* point2, double* point3, double* normal);

#pragma endregion

};
#endif