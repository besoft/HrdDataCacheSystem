/*========================================================================= 
  Program: Musculoskeletal Modeling (VPHOP WP10)
  Module: $RCSfile: vtkProgressiveHullCPU.h,v $ 
  Language: C++ 
  Date: $Date: 2011-09-07 05:42:26 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: David Cholt, Tomas Janak
  Notes:     Done raw cleanup, Done final cleanup
  ========================================================================== 
  Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/


#ifndef __vtkProgressiveHullCPU_h
#define __vtkProgressiveHullCPU_h

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "vtkstd/vector"
#include "vtkstd/algorithm"
#include "vtkProgressiveHull.h"
#include "vtkCellArray.h"
#include "priorityQueue.h"

//TODO: remove this conditional when lpsolve55 is integrated by SCS into LHPBuilder
#ifdef _WINDOWS	
#define _USE_LP_SOLVE
#endif

#ifdef _USE_LP_SOLVE
#include "lp_lib.h"
#pragma comment(lib, "lpsolve55")
#else
#pragma message("WARNING: vtkProgressiveHullCPU will not work because LP_SOLVE library is not available.")
#endif

class VTK_vtkLHP_EXPORT vtkProgressiveHullCPU : public vtkProgressiveHull
{
public:
#pragma region Nested classes
  /** 
    class name: CVertex 
    Nested Vertex class 
    This is a list of the triangles, edges and vertices which are joined to this vertex.*/
  class CVertex
    {
    public:
      double  DCoord[3];
      int     Id;
      bool    BDeleted;
	  bool    BBoundary;
	  
      vtkstd::vector<int> OneRingTriangle;
      vtkstd::vector<int> OneRingEdge;
      vtkstd::vector<int> OneRingVertex;

	  /** constructor */
      CVertex(double *pCoord);
      /** destructor */
      ~CVertex();
    };

  /** 
    class name: CTriangle
    Nested Triangle class 
    Each triangle has three edges and three vertices.
    */
  class  CTriangle
    {
    public:
      double DNormal[3];
	  bool  BDeleted;
      int   AVertex[3];
      int   AEdge[3];
      int   Id;
	  double Volume;
	  bool    Modified;
    public:
      /** constructor */
      CTriangle();
      /** destructor */
      CTriangle(int v0,int v1,int v2);
      /** set the edge */
      void SetEdge(int e0,int e1,int e2);
			void SetNormal(double Normal[3]);
    };

  
  /**  
  class name: CEdge
  Nested Edge class 
  Normally, each edge has two neighbor triangles. Two vertices consist of an edge.
  */
  class CEdge
    {
    public:
      bool   BMarked;
	  bool   BBoundary;
	  bool   BDeleted;
      int    AVertex[4];       //first, second, left, right
      int    ATriangle[2];     //left right
      int    Id;
	  double DeciPoint[3];		// Edge will be decimated to this point
	  double DeciVolumeChange;  // Volume increase caused by this decimation
      /** constructor */
      CEdge();
      /** overloaded constructor */ 
      CEdge(int v0,int v1);
      /** verloaded constructor */
      CEdge(int v0,int v1,int v2,int v3);
      /** Set Triangle to the edge */
      void SetTriangle(int t0,int t1);
	  void SetDeciPoint(double point[3]);
    };
  
#pragma endregion Nested classes

	///
	///  Filter instance setup methods.
	///
public:

  vtkTypeMacro(vtkProgressiveHullCPU,vtkProgressiveHull);

	/** Retrieve instance of the class */
	static vtkProgressiveHullCPU *New();	


	/** Get tolerance of decimation */
	vtkGetMacro(MeshQuality, double);
	/** Set tolerance of edge decimation, from -1 to 1, 1 means all decimations are tolerated (even bad ones),
	-1 means only really flat decimation are allowed. Optimal value is around 0.5. Greater value means mode edges will be decimated overall */
	vtkSetMacro(MeshQuality, double);

	/** Get enlarge amount */
	vtkGetMacro(EnlargeMeshAmount, double);

	/** Set enlarge amount. Final mesh will be enlarged by this value.
	-1 value disables enlarging (default) */
	vtkSetMacro(EnlargeMeshAmount, double);

	///
	///  Used variables and structures, support methods 
	///
protected:
  /** constructor */
	vtkProgressiveHullCPU();           
  /** destructor */
  ~vtkProgressiveHullCPU();

  vtkstd::vector<CVertex*>    Vertexes;             // list of vertices of the mesh
  vtkstd::vector<CTriangle*>  Triangles;            // list of triangles of the mesh
  vtkstd::vector<CEdge*>      Edges;                // list of edge of the mesh

  int NumOfVertex;              // the number of vertices of the mesh
  int NumOfTriangle;            // the number of triangles of the mesh
  int NumOfEdge;                // the number of edges of the mesh

  CPriorityQueue<double, CEdge *> Queue; // prioritized queue of edges for decimation

#ifdef _USE_LP_SOLVE
  lprec *lp;					// linear programming solver
#endif

  double VolumeDifference;	    // Difference between original volume and actual volume
  double OriginalVolume;		// Original volume of the mesh, just to print out
	int actualPoints;
	double MeshQuality;             // Quality of triangles after of edge decimation
	double EnlargeMeshAmount;		  // Enlarging factor	


  /* Calculates unsigned volume of tetrahedron defined by three vertices of triangle 
  tID and 4th point v */
  double TetraVolume(int tID, double v[3]);
  /** Calculate volume of tetrahedron defined by three vertices of triangle
  and origin point (0,0,0) */
  double TriangleVolume(double v1[3],double v2[3],double v3[3]); 
   /** Calculate volume of tetrahedron defined by three vertices of triangle
  and origin point (0,0,0) - CTriangle Parameter overload*/
  double TriangleVolume(CTriangle* T);

  /** Checks if decimation produces weird spike artifacts */
	bool CheckSpikeAndTriangles(CEdge* e, double treshold, double angle, bool checkAngles);

  /** Calculates the normal of triangle T and updates it */
  void UpdateTriangleNormal(CTriangle* T);

  /** Calculates the normal of triangle abc*/
  void ComputeTriangleNormal(double a[3], double b[3], double c[3], double normal[3]);
  
	/** Computes greatest triangle inner angle */
	double ComputeTriangleAngle(double a[3], double b[3], double c[3]);

  /** Makes Cross product of 2 triangle vertices that differ from reference point.*/
  void MakeCross(int triangleID, int refPointID, double cross[3]);
  /** Creates a triangle from vertex data.*/
  CTriangle* CreateTriangle(vtkIdList *ptids, int v0ID,int v2ID, int v3ID, int tID);

	///
	///  Filter execution methods
	///
protected:
  vtkPolyData *InputMesh, *OutputMesh;

	/** Build internal mesh. It expects the input mesh to be manifold */
  void InitMesh();
  /** Build internal mesh relationships */
  void BuildMesh();
  /** Filter execution */
  void Execute();
  /** Prioritize edges based on volume change */
  void PrioritizeEdges();
	/** Compute decimation point of the edge */
	void ComputeDecimationPoint(int edgeID);
  /** Faster computation of priority of single edge */
  double ComputePriorityFast(int edgeID);
  /** Solve new vertex position for given neighborhood parameters */
  int SolveLP(vtkstd::vector<int>ATriangles, double a, double b, double c, double& outVolume, double* V);
  /** Decimate maxDecimatedEdges edges (or all of them) */
  void Decimate();
  /** Decimate one edge */
  void Substitute(CEdge* e);
	/** Enlarges output mesh */
	void EnlargeMesh();
  /** Hand over finished mesh */
  void DoneMesh();
	/** Clears the mesh from memory */
  void ClearMesh();
};
#endif

