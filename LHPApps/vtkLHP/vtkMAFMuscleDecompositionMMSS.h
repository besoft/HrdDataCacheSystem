/*========================================================================= 
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: vtkMAFMuscleDecompositionMMSS.h,v $ 
  Language: C++ 
  Date: $Date: 2011-11-01 09:45:53 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Ivo Zelený
  ========================================================================== 
  Copyright (c) 2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
  vtkMAFMuscleDecompositionMMSS decomposes the muscle volume represented by
  the input surface into fibers represented by a set of polylines (in vtkPolyData). 
*/
#ifndef vtkMAFMuscleDecompositionMMSS_h__
#define vtkMAFMuscleDecompositionMMSS_h__

#pragma once

#pragma warning(push)
#pragma warning(disable:4996)
#include "vtkPolyDataToPolyDataFilter.h"
#pragma warning(pop)

#include <vtkQuadricClustering.h>
#include "vtkMAFMuscleFibers.h"
#include "MMSSVector3d.h"
#include <vector>
#include "vtkLHPConfigure.h"

class vtkPoints;
class vtkCellLocator;

class VTK_vtkLHP_EXPORT vtkMAFMuscleDecompositionMMSS : public vtkPolyDataToPolyDataFilter
{
public:
  static vtkMAFMuscleDecompositionMMSS *New();

  vtkTypeRevisionMacro(vtkMAFMuscleDecompositionMMSS, vtkPolyDataToPolyDataFilter);  

protected:
  vtkMAFMuscleDecompositionMMSS();           
  virtual ~vtkMAFMuscleDecompositionMMSS();

protected:
  //structures used by routines
  typedef double VCoord[3];

  typedef struct LOCAL_FRAME
  {    
    VCoord O;       //<origin
    VCoord uvw[3];  //<axis (may not be of unit size)    
  } LOCAL_FRAME;  

protected:
  vtkMAFMuscleFibers* FibersTemplate;    //<instance of fiber type to be used
  vtkPoints* OriginArea;                 //<origin area points for the input muscle
  vtkPoints* InsertionArea;              //<insertion area points for the input muscle

  int NumberOfFibres;   //<number of fibres to be generated; default = 50
  int Resolution;       //<number of segments per fibre; default = 9
  int SmoothFibers;		  //<non-zero, if fibres are smoothed (default)
  int SmoothSteps;      //<number of smoothing iterations
  double SmoothFactor;  //<smoothing weight (lower values mean more smoothed, higher less), default is 4
  int DebugMode;	//<masked debug mode - see below

public:
  typedef enum DebugModeFlags
  {
    dbgNone = 0,                  //no extra things
    dbgVisualizeFitting = 1,      //visualizes in an external renderer the cube fitting process
    dbgVisualizeFittingResult = 2,//visualizes in an external renderer the result of cube fitting process
    dbgDoNotProjectFibres = 4,    //does not project fibres 
  };
  

public:
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
protected:
  /** 
  By default, UpdateInformation calls this method to copy information
  unmodified from the input to the output.*/
  /*virtual*/void ExecuteInformation();

  /**
  This method is the one that should be used by subclasses, right now the 
  default implementation is to call the backwards compatibility method */
  /*virtual*/void ExecuteData(vtkDataObject *output);


 
  /**
  Computes the minimal oriented box that fits the input data so that all
  points are inside of this box (or on its boundary) and the total squared
  distance of template origin points from the input mesh origin points and
  the total squared distance of template insertion points from the input
  mesh origin points are minimized */
  void ComputeFittingOB(vtkPoints* points, LOCAL_FRAME& out_lf);

  /** Computes the principal axis for the given point set. 
  N.B. the direction is normalized*/
  void ComputeAxisLine(vtkPoints* points, double* origin, double* direction); 

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

 
  /** Smooth the fiber defined by the given points. */
  void SmoothFiber(VCoord* pPoints, int nPoints);
  
private:
  vtkMAFMuscleDecompositionMMSS(const vtkMAFMuscleDecompositionMMSS&);  // Not implemented.
  void operator = (const vtkMAFMuscleDecompositionMMSS&);  // Not implemented.  
};

#endif // vtkMAFMuscleDecompositionMMSS_h__