/*========================================================================= 
Program: Multimod Application Framework RELOADED 
Module: $RCSfile: vtkMAFMuscleFibers.h,v $ 
Language: C++ 
Date: $Date: 2009-05-19 14:30:01 $ 
Version: $Revision: 1.1 $ 
Authors: Josef Kohout (Josef.Kohout *AT* beds.ac.uk)
========================================================================== 
Copyright (c) 2008 University of Bedfordshire (www.beds.ac.uk)
See the COPYINGS file for license details 
=========================================================================
*/
#ifndef vtkMAFMuscleFibers_h__
#define vtkMAFMuscleFibers_h__

#include "vtkObject.h"
#include "vtkPoints.h"
#include "vtkLHPConfigure.h"

#pragma region vtkMAFMuscleFibers
//abstract class for all muscle fibers geometries
//IMPORTANT NOTE: fiber templates use left hand oriented coordinate system
class VTK_vtkLHP_EXPORT vtkMAFMuscleFibers : public vtkObject
{
public:  
  vtkTypeRevisionMacro(vtkMAFMuscleFibers, vtkObject); 

  /** create an instance of the object */
  static vtkMAFMuscleFibers *New();

  typedef double VCoord[3];

protected:
  const static int MAX_BCURVE_POINTS = 4; //<maximal number of supported control points per curve
  //<IF changed CORRESPONDENCE must be updated as well
  
  VCoord* m_pCoords;              //<coordinates of primary control points  
  int* m_pCurves;                 //<the definition of all primary curves
  //the structure contains one int as the number of control points for the following curve
  //followed by indices into m_pCoords
  int m_nCurvesEntries;           //<number of entries in m_pCurves

  typedef struct BCURVE_BLENDING
  {
    double dblSMin;               //<interval of s parameter for this blending - min
    double dblSMax;               //<interval of s parameter for this blending - max
    double dblWSMin;              //<weight used for inner points of curve A (when s = SMin)
    double dblWSMax;              //<weight used for inner points of curve B (when s = SMax)

    int iCurveAPos;               //<index to m_pCurves where the curve A starts
    int iCurveBPos;               //<index to m_pCurves where the curve A starts

    //if polygons A and B have different number of vertices,
    //the first/last vertex of the polygon with larger number of points (P1) is mapped 
    //always to the first/last vertex of the other polygon (P2), and for the rest is here Match struct
    union {
      unsigned char Correspondence;
      struct CORRESPONDENCE
      {        
        unsigned char Second     : 2;  //As MAX_BCURVE_POINTS = 4, the second vertex (index 1) of P1 can be mapped either to vertex of P2 with index 0-3 => 2 bits
        unsigned char Third      : 2;  //RELEASE NOTE: WHEN UPDATED ComputeControlPolygons must be updated as well
        unsigned char Reserved   : 4;  
      } Match;
    };
  } BCURVE_BLENDING;    

  BCURVE_BLENDING* m_pFrontBlendings; //<specifies how the front curves (r=0)should be blended
  BCURVE_BLENDING* m_pBackBlendings;  //<specifies how the back curves (r=1) should be blended    
  int m_nFrontBlendings;               //<number of entries in m_pFrontBlendings
  int m_nBackBlendings;               //<number of entries in m_pBackBlendings

protected:
  vtkMAFMuscleFibers() 
  {
    m_pCoords = NULL; m_pCurves = NULL; m_nCurvesEntries = 0;
    m_pFrontBlendings = m_pBackBlendings = NULL;  
    m_nFrontBlendings = m_nBackBlendings = 0;    
  }

  virtual ~vtkMAFMuscleFibers()
  {
    delete[] m_pCoords;
    delete[] m_pCurves;
    delete[] m_pFrontBlendings;      
    delete[] m_pBackBlendings;

    m_pCoords = NULL; m_pCurves = NULL; m_nCurvesEntries = 0;
    m_pFrontBlendings = m_pBackBlendings = NULL;
    m_nFrontBlendings = m_nBackBlendings = 0;
  }

public:
  /** Gets the Cartesian coordinates of the point with the give parametric coordinates.
  N.B.: parametric coordinates range from 0 to 1*/
  inline void GetPoint(double* xpar_in, double* x_out) {
    GetPoint(xpar_in[0], xpar_in[1], xpar_in[2], x_out);
  }

  /** Gets the Cartesian coordinates of the point with the give parametric coordinates.
  N.B.: r, s, t parameters must be from 0 to 1 */
  virtual void GetPoint(double r, double s, double t, double* x_out);

  /** Computes the Bezier control polygon for the give parameters r and s.
  Returns also weight of internal points in Ws2 (weight of end-points is always 1.0)
  The caller is responsible for the deleting of returned address when it is longer needed. */
  virtual VCoord* GetControlPolygon(double r, double s, int& nVerts, double& Ws2);

  /** Saves points representing the origin area into out_points.
  Data in out_points is reset in prior to this operation. */
  virtual void GetOriginLandmarks(vtkPoints* out_points);

  /** Saves points representing the insertion area into out_points.
  Data in out_points is reset in prior to this operation. */
  virtual void GetInsertionLandmarks(vtkPoints* out_points);

protected:
  /** Computes the control polygon for the given set of curves and parameter s.
  The polygon is returned in pOutPoly, which must be capable to hold at least 
  MAX_BCURVE_POINTS points. Returns number of coordinates written in pOutPoly and 
  in Ws2 also the weight of internal points (weight of end-points is always 1.0) */
  virtual int ComputeControlPolygons(BCURVE_BLENDING* pR_Info, double s, VCoord* pOutPoly, double& Ws2);

private:
  vtkMAFMuscleFibers(const vtkMAFMuscleFibers&);  // Not implemented.
  void operator = (const vtkMAFMuscleFibers&);  // Not implemented.  
};
#pragma endregion

#pragma region vtkMAFParallelMuscleFibers
class VTK_vtkLHP_EXPORT vtkMAFParallelMuscleFibers : public vtkMAFMuscleFibers
{
public:  
  vtkTypeRevisionMacro(vtkMAFParallelMuscleFibers, vtkMAFMuscleFibers); 
  inline static vtkMAFParallelMuscleFibers* New() {
    return new vtkMAFParallelMuscleFibers();
  }

protected:
  vtkMAFParallelMuscleFibers();
private:
  vtkMAFParallelMuscleFibers(const vtkMAFParallelMuscleFibers&);  // Not implemented.
  void operator = (const vtkMAFParallelMuscleFibers&);  // Not implemented.  
};
#pragma  endregion //vtkMAFParallelMuscleFibers

#pragma region vtkMAFPennateMuscleFibers
class VTK_vtkLHP_EXPORT vtkMAFPennateMuscleFibers : public vtkMAFMuscleFibers
{
public:  
  vtkTypeRevisionMacro(vtkMAFPennateMuscleFibers, vtkMAFMuscleFibers); 

  /** dblX_factor denotes the size of origin (O) and insertion (I) area in the unit cube. 
  Both are from interval (0, 1). dblPennateFactor defines the degree of pennation (should be >= 1)*/
  inline static vtkMAFPennateMuscleFibers* New(double dblO_factor = 2 / 3.0, double dblI_factor = 2 / 3.0,
    double dblPennateFactor = 10.0) {
      return new vtkMAFPennateMuscleFibers(dblO_factor, dblI_factor, dblPennateFactor);
  }

protected:
  //see New
  vtkMAFPennateMuscleFibers(double dblO_factor, double dblI_factor, double dblPennateFactor);
private:
  vtkMAFPennateMuscleFibers(const vtkMAFPennateMuscleFibers&);  // Not implemented.
  void operator = (const vtkMAFPennateMuscleFibers&);  // Not implemented.  
};
#pragma endregion //vtkMAFPennateMuscleFibers

#pragma region vtkMAFCurvedMuscleFibers
class VTK_vtkLHP_EXPORT vtkMAFCurvedMuscleFibers : public vtkMAFMuscleFibers
{
public:  
  vtkTypeRevisionMacro(vtkMAFCurvedMuscleFibers, vtkMAFMuscleFibers); 

  /** dblOI_factor is from interval (0 to 0.5)
  dblCurvedFactor defines the degree of attraction towards inner points (should be >= 1) */
  inline static vtkMAFCurvedMuscleFibers* New(double dblOI_factor = 0.2, double dblCurvedFactor = 20.0) {
    return new vtkMAFCurvedMuscleFibers(dblOI_factor, dblCurvedFactor);
  }
protected:    
  //see New
  vtkMAFCurvedMuscleFibers(double dblOI_factor, double dblCurvedFactor);
private:
  vtkMAFCurvedMuscleFibers(const vtkMAFCurvedMuscleFibers&);  // Not implemented.
  void operator = (const vtkMAFCurvedMuscleFibers&);  // Not implemented.  
};
#pragma endregion //vtkMAFCurvedMuscleFibers

#pragma region vtkMAFFannedMuscleFibers
class VTK_vtkLHP_EXPORT vtkMAFFannedMuscleFibers : public vtkMAFMuscleFibers
{
public:  
  vtkTypeRevisionMacro(vtkMAFFannedMuscleFibers, vtkMAFMuscleFibers); 

  /** dblX1_factor denotes the size of origin (O) and insertion (I) area of the 
  outer band of fibers (in unit cube). dblFanCenter is y-position of middle 
  control point of fanned polygons (inner polygons)*/
  inline static vtkMAFFannedMuscleFibers* New(double dblO1_factor = 0.2, 
    double dblI1_factor = 0.2, double dblFanCenter = 0.75) {
      return new vtkMAFFannedMuscleFibers(dblO1_factor, dblI1_factor, dblFanCenter);
  }
protected:
  //See New
  vtkMAFFannedMuscleFibers(double dblO1_factor, double dblI1_factor, 
    double dblFanCenter);
private:
  vtkMAFFannedMuscleFibers(const vtkMAFFannedMuscleFibers&);  // Not implemented.
  void operator = (const vtkMAFFannedMuscleFibers&);  // Not implemented.  
};
#pragma endregion //vtkMAFFannedMuscleFibers

#pragma region vtkMAFRectusMuscleFibers
class VTK_vtkLHP_EXPORT vtkMAFRectusMuscleFibers : public vtkMAFMuscleFibers
{
public:  
  vtkTypeRevisionMacro(vtkMAFRectusMuscleFibers, vtkMAFMuscleFibers); 

  /** See Blemker's paper about rectus modeling. O1_x and O2_x denote the x-coordinates
  of origin rectangular area on the first side of unit cube, O12_y is denotes the 
  y-coordinate of this area (the other y-coordinate is 1.0). I*_? defines the
  insertion area on the opposite side. Note: angle of fibers is governed by y-coordinates. 
  dblBoundFactor defines how fibers attract to sides of cube (should be >= 1)*/
  inline static vtkMAFRectusMuscleFibers* New(
    double dblO1_x = 0.6, double dblO2_x = 0.8, double dblO12_y = 0.25, 
    double dblI1_x = 0.25, double dblI2_x = 0.65, double dblI12_y = 0.6,
    double dblBoundFactor = 10.0) {
      return new vtkMAFRectusMuscleFibers(dblO1_x, dblO2_x, dblO12_y,
        dblI1_x, dblI2_x, dblI12_y, dblBoundFactor);
  }
protected:
  //See New
  vtkMAFRectusMuscleFibers(double dblO1_x, double dblO2_x, double dblO12_y, 
    double dblI1_x, double dblI2_x, double dblI12_y, double dblBoundFactor);
private:
  vtkMAFRectusMuscleFibers(const vtkMAFRectusMuscleFibers&);  // Not implemented.
  void operator = (const vtkMAFRectusMuscleFibers&);  // Not implemented.  
};
#pragma endregion //vtkMAFFannedMuscleFibers
#endif // vtkMAFMuscleFibers_h__