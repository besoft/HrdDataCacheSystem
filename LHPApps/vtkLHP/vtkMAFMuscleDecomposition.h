/*========================================================================= 
Program: Multimod Application Framework RELOADED 
Module: $RCSfile: vtkMAFMuscleDecomposition.h,v $ 
Language: C++ 
Date: $Date: 2012-04-04 16:02:03 $ 
Version: $Revision: 1.1.1.1.2.2 $ 
Authors: Josef Kohout (Josef.Kohout *AT* beds.ac.uk)
========================================================================== 
Copyright (c) 2008 University of Bedfordshire (www.beds.ac.uk)
See the COPYINGS file for license details 
=========================================================================
vtkMAFMuscleDecomposition decomposes the muscle volume represented by
the input surface into fibers represented by a set of polylines 
(in vtkPolyData). Various fiber geometries are supported (e.g.,
parallel, pennate, curven, fanned, etc.). The decomposition was
written according to following papers:

SILVIA S. BLEMKER and SCOTT L. DELP: Three-Dimensional Representation of
Complex Muscle Architectures and Geometries. Annals of Biomedical Engineering, 
Vol. 33, No. 5, May 2005, pp. 661–673
http://www.mae.virginia.edu/muscle/pdf/Blemker2005b.pdf

Silvia S. Blemker, Scott L. Delp: Rectus femoris and vastus intermedius fiber excursions predicted
by three-dimensional muscle models. Journal of Biomechanics 39 (2006): 1383–1391
http://www.mae.virginia.edu/muscle/pdf/Blemker2006.pdf
*/
#ifndef vtkMAFMuscleDecomposition_h__
#define vtkMAFMuscleDecomposition_h__

#pragma once

#pragma warning(push)
#pragma warning(disable:4996)
#include "vtkPolyDataToPolyDataFilter.h"
#pragma warning(pop)

#include "vtkMAFMuscleFibers.h"
#include "vtkPolyData.h"

#include "vtkLHPConfigure.h"

#define ADV_SLICING_ORTHOPLANES
#define ADV_KUKACKA	//The idea by Martin Kukacka, Ricardo Prague, s.r.o.

//#define PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
//#define PUBLICATION_TEST_GENERATED_DATA_OVERRIDE

//#define ADV_KUKACKA_TEST
#ifdef ADV_KUKACKA_TEST
#define __PROFILING__
#include <atlbase.h>
#include "BSGenLib.h"	//To get the library, please contact besoft@kiv.zcu.cz
#else
#define PROFILE_TIMER_START(var)	
#define PROFILE_TIMER_STOP(var)		
#define PROFILE_FUNCTION(var)		
#define PROFILE_THIS_FUNCTION()	

#define PROFILE_COUNTER_INC_S(var)
#define PROFILE_COUNTER_DEC_S(var)
#define PROFILE_TIMER_START_S(var)
#define PROFILE_TIMER_STOP_S(var)
#define PROFILE_FUNCTION_S(var)
#endif

#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
#include "optimization.h"
#endif

class vtkPoints;
class vtkCellLocator;
class vtkCellArray;
class vtkMAFVisualDebugger;
class vtkParametricFunction;

class VTK_vtkLHP_EXPORT vtkMAFMuscleDecomposition : public vtkPolyDataToPolyDataFilter
{
public:
	static vtkMAFMuscleDecomposition *New();

	vtkTypeRevisionMacro(vtkMAFMuscleDecomposition, vtkPolyDataToPolyDataFilter);  

protected:
	vtkMAFMuscleDecomposition();           
	virtual ~vtkMAFMuscleDecomposition();

public:
	//structures used by routines
	typedef double VCoord[3];
protected:
	typedef struct LOCAL_FRAME
	{    
		VCoord O;       //<origin
		VCoord uvw[3];  //<axis (may not be of unit size)    
	} LOCAL_FRAME;  
	
	typedef struct CONTOUR_MAPPING_PARAMETERIZATION
	{
		int nPoints;											///<number of points both in BoxContour and TemplateMuscleContour		
		VCoord* BoxContour;								///<global coordinates of rectangular contour constructed by from slicing fibres template
		VCoord* TemplateMuscleContour;		///<global or parameterized coordinates of muscle contour constructed by from slicing muscle template
		int* TemplateMuscleContourTriIds;	///<if NULL, TemplateMuscleContour stores global coordinates, 
																			///otherwise it stores barycentric coordinates relative to the triangle with the id specified in this array at the same index

		int iStartPos, iEndPos;		///<values to be used for mapping fibres for this contour
	} CONTOUR_MAPPING_PARAMETERIZATION;

protected:
	vtkMAFMuscleFibers* FibersTemplate;    //<instance of fiber type to be used
	vtkPoints* OriginArea;                 //<origin area points for the input muscle
	vtkPoints* InsertionArea;              //<insertion area points for the input muscle

	int NumberOfFibres;   //<number of fibres to be generated; default = 50
	int Resolution;       //<number of segments per fibre; default = 9
	int SmoothFibers;		  //<non-zero, if fibres are smoothed (default)
	int SmoothSteps;      //<number of smoothing iterations
	double SmoothFactor;  //<smoothing weight (lower values mean more smoothed, higher less), default is 4
	int DebugMode;				//<masked debug mode - see below
	int AdvancedSlicing;		///<non-zero, if AdvancedSlicing mode is enabled
#ifdef ADV_KUKACKA
	int AdvancedKukacka;		///<non-zero, if Kukacka mode is enabled
#endif
  int UniformSampling;    ///<non-zero, if using uniform sampling for the particle system	

	vtkPolyData* InputTemplate;			///<surface mesh compatible with Input: it has the same topology, the same number of vertices but vertices may have different coordinates

public:
	enum DebugModeFlags
	{
		dbgNone = 0,                  //no extra things
		dbgVisualizeFitting = 1,      //visualizes in an external renderer the cube fitting process
		dbgVisualizeFittingResult = 2,//visualizes in an external renderer the result of cube fitting process
		dbgDoNotProjectFibres = 4,    //does not project fibres 
		dbgVisualizeSlicing = 8,      //visualizes in an external renderer the cube slicing process
		dbgVisualizeSlicingResult = 16,//visualizes in an external renderer the result of cube slicing process
		dbgVisualizeAttachmentConstruction = 32, //visualizes in an external renderer how attachment areas are processed
		dbgVisualizeHarmonicField = 64,	//visualizes in an external renderer the harmonic field
		dbgVisualizeFibresPostprocessing = 128, //visualizes the produced fibres as they are being trimmed and extrapolated
		dbgVisualizeFibresPostprocessingResult = 256, //visualizes the produced fibres
	};


public:
	/** Gets the input template mesh. 
	This mesh is typically rest-pose muscle and it is used to make a plan for decomposing Input mesh in order to
	diminish inconsistency in decomposition of time-variant mesh when results from one time frame are often very different from
	the results from the consequent time frame, though both results are realistic.
	N.B. when specified, both insertion and origin areas must be related to this template mesh instead of the input mesh.
	N.B. the mesh must be compatible with Input so that it has the same topology, the same number of vertices but may have different vertex coordinates */
	vtkGetObjectMacro(InputTemplate, vtkPolyData);

	/** Sets the input template mesh. 
	This mesh is typically rest-pose muscle and it is used to make a plan for decomposing Input mesh in order to
	diminish inconsistency in decomposition of time-variant mesh when results from one time frame are often very different from
	the results from the consequent time frame, though both results are realistic.
	N.B. when specified, both insertion and origin areas must be related to this template mesh instead of the input mesh.
	N.B. the mesh must be compatible with Input so that it has the same topology, the same number of vertices but may have different vertex coordinates */
	vtkSetObjectMacro(InputTemplate, vtkPolyData);

	/** Gets the number of muscle fibres to be generated */
	vtkGetMacro(NumberOfFibres, int);

	/** Sets the number of muscle fibres to be generated */
	vtkSetMacro(NumberOfFibres, int);

	/** Gets the number of vertices per one fiber */
	vtkGetMacro(Resolution, int);

	/** Sets the number of vertices per one fiber */
	vtkSetMacro(Resolution, int);

	/** Sets new template for muscle fibers */
	virtual void SetFibersTemplate(vtkMAFMuscleFibers* pTemplate);

	/** Gets the currently associated template for muscle fibers */
	vtkGetMacro(FibersTemplate, vtkMAFMuscleFibers*);

	/** Sets new origin area points for the input muscle */
	virtual void SetOriginArea(vtkPoints* pPoints);

	/** Gets the currently associated origin area points for the input muscle */
	vtkGetMacro(OriginArea, vtkPoints*);

	/** Sets new insertion area points for the input muscle */
	virtual void SetInsertionArea(vtkPoints* pPoints);

	/** Gets the currently associated insertion area points for the input muscle */
	vtkGetMacro(InsertionArea, vtkPoints*);
	
	/** Gets non-zero, if  the generated fibres should be smoothed */
	vtkGetMacro(SmoothFibers, int);

	/** Defines whether the generated fibres should be smoothed (non-zero); by default they are smoothed */
	vtkSetMacro(SmoothFibers, int);

	/** Defines whether the generated fibres should be smoothed (non-zero) */
	vtkBooleanMacro(SmoothFibers, int);

	/** Gets the number of smoothing iteration steps (default is 5) */
	vtkGetMacro(SmoothSteps, int);

	/** Sets the number of smoothing iteration steps (default is 5)*/
	vtkSetMacro(SmoothSteps, int);

	/** Gets the smoothing weight; lower values mean more smoothed fibers, default is 4 */
	vtkGetMacro(SmoothFactor, double);

	/** Sets the smoothing weight; lower values mean more smoothed fibers, default is 4 */
	vtkSetMacro(SmoothFactor, double);


	/** Gets debug mode (see Dbg enums)*/
	vtkGetMacro(DebugMode, int);

	/** Sets debug mode (see Dbg enums) */
	vtkSetMacro(DebugMode, int);

	/** Returns non-zero, if advanced slicing mode is enabled, 0 otherwise*/
	vtkGetMacro(AdvancedSlicing, int);

	/** Sets, if advanced slicing mode is enabled (non-zero), or disabled (0) */
	vtkSetMacro(AdvancedSlicing, int);

	/** Returns non-zero, if advanced mode using Kukacka method is enabled, 0 otherwise*/
	vtkGetMacro(AdvancedKukacka, int);

	/** Sets, if advanced mode using Kukacka method is enabled (non-zero), or disabled (0) */
	vtkSetMacro(AdvancedKukacka, int);

	/** Returns non-zero, if using uniform sampling for the particle system, 0 otherwise */
	vtkGetMacro(UniformSampling, int);

	/** Sets, if using uniform sampling for the particle system (non-zero), or not (0)*/
	vtkSetMacro(UniformSampling, int);

	
	/** Smooth the fiber defined by the given points and smoothing parametres - is used as "static", i.e. does not use the parametres set in this vtkMADMuscleDecomposition. */
	void SmoothFiber(VCoord* pPoints, int nPoints, double smoothFactor, int smoothSteps);
protected:
	/** 
	By default, UpdateInformation calls this method to copy information
	unmodified from the input to the output.*/
	/*virtual*/void ExecuteInformation();

	/**
	This method is the one that should be used by subclasses, right now the 
	default implementation is to call the backwards compatibility method */
	/*virtual*/void ExecuteData(vtkDataObject *output);


	/** Samples E2 space <0..1>x<0..1> quasy randomly storing samples into points buffer.
	N samples are created. Points buffer must be capable enough to hold
	2*N doubles (format is x1,y1,x2,y2...).
	The routine is based on the code by Frances Y. Kuo <f.kuo@unsw.edu.au>*/
	void CreateSobolPoints(int N, double* points);

	
	/** Samples E2 space <0..1>x<0..1> uniformly storing samples into points buffer.
	N samples are created. Points buffer must be capable enough to hold
	2*N doubles (format is x1,y1,x2,y2...).
	N.B: N must be k*k, where k is an integer. */
	void CreateUniformPoints(int N, double* points);

	/**
	Computes the minimal oriented box that fits the input data so that all
	points are inside of this box (or on its boundary) and the total squared
	distance of template origin points from the input mesh origin points and
	the total squared distance of template insertion points from the input
	mesh origin points are minimized */
	void ComputeFittingOB(vtkPoints* points, LOCAL_FRAME& out_lf);

#ifdef ADV_SLICING_ORTHOPLANES
	/** Projects all points onto the iPlane axis of the local frame lf and returns the extreme distances from the origin of lf.
	Extremes are returned in a fraction of  iPlane vector length, i.e., 0.0 = origin, 1.0 = the end point of O + vector[iPlane]*/
	void GetProjectedMinMaxT(const vtkPoints* points, const LOCAL_FRAME& lf, int iPlane, 
		double& tmin, double& tmax);

	/** Projects the point onto the iPlane axis of the local frame lf and returns its distances from the origin of lf.
	The distance is returned in a fraction of  iPlane vector length, i.e., 0.0 = origin, 1.0 = the end point of O + vector[iPlane]*/
	inline double GetProjectedT(const double* x, const LOCAL_FRAME& lf, int iPlane);
#endif

	/** Computes centroid of points */
	void ComputeCentroid(const vtkPoints* points, double* centroid);

	/** Computes three eigen vectors of the given point set. 
	Centroid may be NULL, if it should be calculated automatically, otherwise, it must be centroid of points.
	The returned eigen vectors are ordered by their lengths. The longest (i.e., the principal axis) is denoted by the first one. */
	void ComputeEigenVects(const vtkPoints* points, const double* centroid, double eigenvects[3][3]);

	/** Computes the principal axis for the given point set. 
	N.B. the direction is normalized*/
	void ComputeAxisLine(vtkPoints* points, double* origin, double* direction); 

	/** Computes the principal axis for the given muscle point set and orgin and insertion areas. 
	N.B. the direction is normalized*/
	void ComputeAxisLine(vtkPoints* points, vtkPoints* ori_points, vtkPoints* ins_points,
		double* origin, double* direction); 

	/**
	Computes 4*nFrames vectors by rotating u around r vector. All vectors are
	normalized and stored in the order A, B, C, D where B is the vector
	opposite to A, C is vector perpendicular to A and r and D is vector
	opposite to C. N.B. vectors u and r must be normalized and perpendicular!
	The buffer pVectors must be capable enough to hold all vectors.*/
	void ComputeDirectionVectors(double* u, double* r, int nFrames, VCoord* pVectors);

	/** Adjusts the length of direction vectors (in pVects) to fit the given point set. 
	For each direction vector, the algorighm find a plane defined by the center 
	(it should be the centroid of points) and a normal of non-unit size that is 
	collinear with the input direction vector. This normal is chosen so that the 
	no point lies in the positive halfspace of the plane (i.e. in the direction 
	of normal from the plane). The computed normals are returned in pVects.
	N.B. the input vectors must be of unit size! */
	void FitDirectionVectorsToData(vtkPoints* points, double* center, 
		int nVects, VCoord* pVects);

	/**
	Computes local frame systems for various cubes defined by their center
	and two direction vectors in w and 4 direction vectors in u and v axis.
	Direction vectors in u and v are given in uv_dirs and have the structure
	compatible with the output of ComputeDirectionVectors method. 
	The computed LFs are stored in pLFS buffer. The buffer must be capable to
	hold 8*nCubes (= nFrames in ComputeDirectionVectors) entries. */
	void ComputeLFS(double* center, VCoord* w_dir, 
		int nCubes, VCoord* uv_dirs, LOCAL_FRAME* pLFS);

	/**
	Finds the best local frame system from those passed in pLFS that best
	maps template origin and insertion points to target origin and insertion
	points. N.B. any point set can be NULL, if it is not needed. Special
	case is when both target sets or template sets are NULL, then the
	routine returns the first LF. */
	int FindBestMatch(vtkPoints* template_O, vtkPoints* template_I,
		int nLFS, LOCAL_FRAME* pLFS, vtkPoints* target_O, vtkPoints* target_I);

	/** Returns the coordinates of surface point that is the closest to the given plane. */
	void FindClosestPoint(vtkPolyData* input, const double* origin, 
		const double* normal, double* x);

	/** Sorts the given points according to their iCoord coordinate.
	The resulting order is returned and the user is responsible for its
	deallocation when it is no longer needed. pPoints array is not touched. */
	int* SortPoints(const VCoord* pPoints, int nPoints, int iCoord);

	/**
	Gets edges from the contour and sorts them to form continuous path. The
	format of the returned array (the caller is responsible for its
	deallocation) is s1,s2,s2,s3,s3,s4, ... sn,s1 - for instance: 0,2,2,3,3,4,4,6,0 

	*/
	int* GetSortedEdges(vtkPolyData* contour
//		, const double* orientationNormal, bool orientationCCW
		);

	/** Adds new points into pContourPoints [in/out] so they form a polygon of at least 4 vertices. 
	Returns the new number of points in the list. 
	N.B, pContourPoints must be capable to hold at least 4 vertices.*/
	int FixPolygon(VCoord* pContourPoints, int nPoints);

	/** Divides the rectangle defined by one point and two vectors into
	nPoints edges such that the total square error between lengths of
	contour and rectangle edges is minimized. The routine stores beginning
	points of these rectangle edges into pOutRectPoints. 
	N.B. nPoints should be >= 4 and pOutRectPoints MUST BE capable
	to store max(nPoints, 4) coordinates.*/
	void DivideRectangle(double* origin, double* u, double* v, int nPoints,
		VCoord* pContourPoints, VCoord* pOutRectPoints);

	/**
	Computes new locations of given points (pPoints)lying inside the
	template polygon (pPolyTemplate) within the other polygon
	(pPolyTarget). Both polygons have the same number of edges. New
	coordinates are stored in pPoints buffer.*/
	void MapPoints(VCoord* pPoints, int nPoints, 
		VCoord* pPolyTemplate, VCoord* pPolyTarget, int nPolyPoints);

	/** Project the points onto the surface for which the locator was built and stores the projected points into out_points. */
	void ProjectAttachmentArea(const vtkPoints* points, const vtkCellLocator* cellLocator, vtkPoints* out_points);

	/** Creates the surface polydata that represents the attachment area on the muscle. 
	Attachment area points (projpts) must lie on the muscle surface (surface), i.e., use ProjectAttachmentArea method prior to this one.
	If ADV_KUKACKA conditional is specified, the method may optionally return also the rest of muscle surface after the attachment area is cut-off*/
	void GetAttachmentAreaSurface(const vtkPoints* projpts, const vtkPolyData* surface, vtkPolyData* output
#ifdef ADV_KUKACKA
		, vtkPolyData* cutSurface = NULL, vtkPolyData* cutPolyLine = NULL
#endif
		);

#ifdef ADV_SLICING_ORTHOPLANES
		/** Filter the fiber defined by the point sequence pPoints.
	N.B. pPoints is damaged!!! 
	It filters out a) redundant points (two points at the same location)
	and b) points whose projection onto the principal axis (lf, iPlane) is 
	not in the interval tmin-tmax (see also GetProjectedT).
	It returns the start position of fibers in nValidIndex and the number
	of points in the fiber in nValidPoints. If the result would be a single point,
	this is also filtered out, i.e., possible results are nValidPoints == 0 or >=2.*/
	void FilterFiber(VCoord* pPoints, int& nValidIndex, int& nValidPoints, 
		const LOCAL_FRAME& lf, int iPlane, double tmin, double tmax);
#else
	/** Filter the fiber defined by the point sequence pPoints.
	N.B. pPoints is damaged!!! 
	It filters out a) redundant points (two points at the same location)
	and b) if cutting planes are specified (not NULL), then also points that 
	lies in positive half spaces defined by both cutting planes.	
	It returns the start position of fibers in nValidIndex and the number
	of points in the fiber in nValidPoints. If the result would be a single point,
	this is also filtered out, i.e., possible results are nValidPoints == 0 or >=2.*/
	void FilterFiber(VCoord* pPoints, int& nValidIndex, int& nValidPoints, 
		const double* OCutPlaneOrigin, const double* OCutPlaneNormal,
		const double* ICutPlaneOrigin, const double* ICutPlaneNormal);

	/** Computes the plane for the given set of points that can be used to cut unwanted space. 
	The returned plane C is parallel with the best fitting plane R of the given points, whilst passing through
	the point P from the point set that lies in the same half space (defined by C) as the given centroid
	and its distance from the plane R is maximal. The returned normal directs in the opposite direction than 
	the given centroid. N.B. the method assumes that centroid does not lie on the plane R. */
	void ComputeCuttingPlane(const vtkPoints* pAreaPoints, const double* pMuscleCentroid,
		double* planeOrigin, double* planeNormal);
#endif

	/** Transforms the given points corresponding to vertices of the source mesh so that they correspond to vertices of the target mesh.
	N.B. the target mesh must be compatible with the source one, i.e., everything must be the same except for vertex coordinates. */
	void TransformPointsFromInputTemplateToInput(vtkPoints* points, vtkPolyData* source, vtkPolyData* target);

	/** Extrapolates the fiber defined by the point sequence in the given direction.
	If nDir is positive, the method computes fiber successor, otherwise its predecessor and
	stores the coordinates of the computed point into pOut. The method performs a simple 
	linear extrapolation.It is guaranteed that the point lies on the surface of input muscle.  
	N.B.  cellLocator is a locator built upon input muscle surface and may not be NULL!!
	The method returns false, if the calculation fails. */
	bool ExtrapolateFiber(const VCoord* pPoints, int nPoints, int nDir,
		const vtkCellLocator* cellLocator, VCoord* pOut);

	/** Extrapolates the fiber defined by the point sequence in the given direction.
	If nDir is positive, the method computes fiber successor, otherwise its predecessor and
	stores the coordinates of the computed point into pOut. It is guaranteed that the point
	lies in the area represented by cellLocatorORI (nDIr < 0) or by cellLocatorINS (nDir > 0).	
	N.B.  cellLocator is a locator built upon input muscle surface and may not be NULL!!
	The method returns false, if the calculation fails. */
	bool ExtrapolateFiber(const VCoord* pPoints, int nPoints, int nDir,
		const vtkCellLocator* cellLocator, 
		const vtkCellLocator* cellLocatorORI, const vtkCellLocator* cellLocatorINS, VCoord* pOut);

	/** Smooth the fiber defined by the given points. */
	void SmoothFiber(VCoord* pPoints, int nPoints);

#ifdef ADV_KUKACKA
	/** Calculates harmonic scalar function for the given surface.
	The surface may not contain attachment areas, i.e., it is an open mesh corresponding to the body of muscle.
	oriContour and insContour contains points (should geometrically correspond to some vertices of the surface mesh)
	that are on origin and insertion attachment area. oriVal and insVal are values of harmonic function in these points.
	Values for other points will be calculated and the final scalar filed stored into surface. */
	void ComputeHarmonicScalarFunction(vtkPolyData* surface, 
		vtkPolyData* oriContour, double oriVal, vtkPolyData* insContour, double insVal);

	/** Creates an external rendering window and displays the harmonic weights. */
	void DebugVisualizeHarmonicWeights(vtkPolyData* surface, vtkPoints* target_O, vtkPoints* target_I, vtkPoints* proj_O = NULL, vtkPoints* proj_I = NULL);
#endif

	/** Creates an external rendering window and displays the fitted cube */
	void DebugVisualizeFitting(vtkMAFVisualDebugger* vd, int nIndex, 
		int nCount, LOCAL_FRAME& lfs, 
		vtkPoints* template_O, vtkPoints* template_I, 
		vtkPoints* target_O, vtkPoints* target_I, double dblScore, 
		bool bBestOne = false);

	/** Adds a contour into VTK data sets. */
	void DebugAddContour(const VCoord* pPoints, int nPoints, vtkPoints* pOutPts, vtkCellArray* pOutCells);

#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
	/** Computes the best fitting plane for the given points and adds it into VTK data sets. */
	void DebugAddBestFittingPlane(const VCoord* pPoints, int nPoints, vtkPoints* pOutPts, vtkCellArray* pOutCells);
#endif

	/** Adds links between two points into VTK data sets. */
	void DebugAddMorphDir(const VCoord* pPointsFrom, const VCoord* pPointsTo, int nPoints, 
		vtkPoints* pOutPts, vtkCellArray* pOutCells);

	/** Creates an external rendering window and displays the slicing process */
	void DebugVisualizeSlicing(vtkMAFVisualDebugger* vd, LOCAL_FRAME& lfs, 		
		vtkPoints* target_O, vtkPoints* target_I, 
		vtkPolyData* template_Contours, vtkPolyData* target_Contours, 
		vtkPolyData* mapping_Links
#ifdef PUBLICATION_TEST_MUSCLE_CONTOUR_PLANARITY
		, vtkPolyData* target_Countours_Planes
#endif
		);

	/** Creates an external rendering window and displays the fibres produced */
	void DebugVisualizeFibresPostprocessing(vtkMAFVisualDebugger* vd, 
		vtkPoints* target_O, vtkPoints* target_I, vtkPolyData* fibres
#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
																   , vtkPolyData* analyticalFibres
#endif
	);

#ifdef PUBLICATION_TEST_GENERATED_DATA_OVERRIDE
public:
	enum DataType
	{
		Cylinder,
		Torus,
		Ellipsoid,
		Muscle1,
	};

	/** Creates parametric function. */
	static vtkParametricFunction* CreateParametricFunction(enum DataType dataType);	

	/** Generates the input data for the testing. */
	void GenerateInputData(enum DataType dataType);	

	/** Fit the analytical curve to the constructed fibre and sample this curve to a new fibre in pOut.
	The fibre is described by the cell in pFibres with Id = fibreId. The method returns MSE error of fitting.
	Fitting is done by non-linear least-square fitting. */
	double FitCurveToFibre(enum DataType dataType, vtkPolyData* pFibres, int fibreId, vtkPolyData* pOut);

	typedef struct FIT_CONTEXT_DATA
	{
		VCoord* pSamples;		//samples of produced fibre	
		int nSamples;
		
		vtkParametricFunction* pFunction;
	} FIT_CONTEXT_DATA;

	typedef struct FIT_OPTIMIZE_U_CONTEXT_DATA
	{
		VCoord Pt;							//Point

		vtkParametricFunction* pFunction;
		double uvw[3];						//for optimization of U, only parameters uvw[1] and uvw[2] are valid

	} FIT_OPTIMIZE_U_CONTEXT_DATA;
	

	/** Calculate the length of fibre*/
	double MeassureFibreLength(vtkPolyData* pFibres, int fibreId);

	/** Samples uniformly the fibre identified by the cell with Id = fibreId. The number of samples is returned in nSamples.*/
	VCoord* SampleFibre(vtkPolyData* pFibres, int fibreId, int& nSamples);

	/** Calculates objective function */
	static void FitCurveToFibre_fvec(const alglib::real_1d_array &x, alglib::real_1d_array &fi, void *ptr);

	/** Calculates Jacobian function */
	static void FitCurveToFibre_jac(const alglib::real_1d_array &x, alglib::real_1d_array &fi, alglib::real_2d_array &jac, void *ptr);
	
	/** Finds the optimal parameter u so that F(u_optimal; v, w) is the closest to the point pData->pSamples[ptIdx],
	where v = uvw[1], w = uvw[2] are fixed parameters of the curve and uvw[0] = u_optimal upon the exit of this method. */
	static void vtkMAFMuscleDecomposition::FitCurveToFibre_Optimize_U(FIT_CONTEXT_DATA* pData,  int ptIdx, double* uvw);

	/** Calculates objective function */
	static void FitCurveToFibre_Optimize_U_fvec(const alglib::real_1d_array &x, double &func, void *ptr);

	/** Calculates Jacobian function */
	static void FitCurveToFibre_Optimize_U_jac(const alglib::real_1d_array &x, alglib::real_1d_array &fi, alglib::real_2d_array &jac, void *ptr);

#endif

private:
	vtkMAFMuscleDecomposition(const vtkMAFMuscleDecomposition&);  // Not implemented.
	void operator = (const vtkMAFMuscleDecomposition&);  // Not implemented.  
};

#ifdef ADV_SLICING_ORTHOPLANES
//------------------------------------------------------------------------
//Projects the point onto the iPlane axis of the local frame lf and returns its distances from the origin of lf.
//The distance is returned in a fraction of  iPlane vector length, i.e., 0.0 = origin, 1.0 = the end point of O + vector[iPlane]
inline double vtkMAFMuscleDecomposition::GetProjectedT(const double* x, const LOCAL_FRAME& lf, int iPlane) {
	return //dot product is enough to get what we want
		lf.uvw[iPlane][0]*(x[0] - lf.O[0]) + 
		lf.uvw[iPlane][1]*(x[1] - lf.O[1]) +
		lf.uvw[iPlane][2]*(x[2] - lf.O[2]);
}
#endif


#endif // vtkMAFMuscleDecomposition_h__