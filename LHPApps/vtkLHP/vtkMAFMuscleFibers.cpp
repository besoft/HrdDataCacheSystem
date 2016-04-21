
/*========================================================================= 
Program: Multimod Application Framework RELOADED 
Module: $RCSfile: vtkMAFMuscleFibers.cpp,v $ 
Language: C++ 
Date: $Date: 2009-05-19 14:30:01 $ 
Version: $Revision: 1.1 $ 
Authors: Josef Kohout (Josef.Kohout *AT* beds.ac.uk)
========================================================================== 
Copyright (c) 2008 University of Bedfordshire (www.beds.ac.uk)
See the COPYINGS file for license details 
=========================================================================
*/
#include "vtkMAFMuscleFibers.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkIdList.h"

vtkCxxRevisionMacro(vtkMAFMuscleFibers, "$Revision: 1.1 $");

vtkStandardNewMacro(vtkMAFMuscleFibers);

#include "mafMemDbg.h"
#include "mafDbg.h"

#pragma region vtkMAFMuscleFibers
//------------------------------------------------------------------------
//Computes the Bezier control polygon for the give parameters r and s.
//Returns also weight of internal points in Ws2 (weight of end-points is always 1.0).
//The caller is responsible for the deleting of returned address when it is longer needed
/*virtual*/ vtkMAFMuscleFibers::VCoord* 
vtkMAFMuscleFibers::GetControlPolygon(double r, double s, int& nVerts, double& Ws2)
//------------------------------------------------------------------------
{
  double dblWs1, dblWs2;
  VCoord* pPolys = new VCoord[2*MAX_BCURVE_POINTS];

  int nVerts1 = ComputeControlPolygons(m_pFrontBlendings, s, pPolys, dblWs1);
  int nVerts2 = ComputeControlPolygons(m_pBackBlendings, s, &pPolys[nVerts1], dblWs2);

  _ASSERT(nVerts1 == nVerts2);  
  for (int i = 0; i < nVerts1; i++) 
  {
    for (int j = 0; j < 3; j++){
      pPolys[i][j] = (1 - r)*pPolys[i][j] + r*pPolys[nVerts1 + i][j];
    }
  }   

  nVerts = nVerts1;
  Ws2 = (1 - r)*dblWs1 + r*dblWs2;
  return pPolys;
}

//------------------------------------------------------------------------
//Gets the Cartesian coordinates of the point with the give parametric coordinates.
//N.B.: r, s, t parameters must be from 0 to 1
/*virtual*/void vtkMAFMuscleFibers::GetPoint(double r, double s, 
                                             double t, double* x_out)
//------------------------------------------------------------------------
{
  int nVerts = 0; 
  double Ws2 = 1.0;

  VCoord* pPolys = GetControlPolygon(r, s, nVerts, Ws2);
  double* pBern = new double[nVerts]; //Bernstein coeficients

  //compute Bezier function of degree nVerts-1 for the parameter t  
  //first, compute wi(s)*Bi(t) - see Blember paper.
  double t_1 = 1 - t;
  pBern[0] = t_1;
  for (int i = 1; i < nVerts - 1; i++) {
    pBern[0] *= t_1;  //should be faster than pow
  }

  double dblBernSum = pBern[0];
  double dblComb = 1.0;    
  for (int i = 1; i < nVerts - 1; i++)
  {
    //Combination C(n, k+1) = C(n,k)*(n-k)/(k+1); - see Wikipedia
    //in our case n = nVerts - 1, k = i-1; C(n,k) = dblComb    
    dblComb *= ((double)(nVerts - i)) / i;

    double tt1 = t_1;
    for (int j = 1; j < nVerts - i - 1; j++) {
      tt1 *= t_1;  //should be faster than pow
    }

    double tt2 = t;
    for (int j = 1; j < i; j++) {
      tt2 *= t;  //should be faster than pow
    }

    pBern[i] = Ws2*dblComb*tt1*tt2;
    dblBernSum += pBern[i];
  }

  pBern[nVerts - 1] = t;
  for (int i = 1; i < nVerts - 1; i++) {
    pBern[nVerts - 1] *= t;  //should be faster than pow
  }
  dblBernSum += pBern[nVerts - 1];

  //we have now everything computed in pBern[...]
  for (int i = 0; i < 3; i++)
  {
    x_out[i] = 0.0;     
    for (int j = 0; j < nVerts; j++) {
      x_out[i] += pPolys[j][i]*pBern[j];      
    }

    x_out[i] /= dblBernSum;
  }

  delete[] pBern;
  delete[] pPolys;
}

//------------------------------------------------------------------------
//Computes the control polygon for the given set of curves and parameter s.
//The polygon is returned in pOutPoly, which must be capable to hold at least 
//MAX_BCURVE_POINTS points. Returns number of coordinates written in pOutPoly
//and in Ws2 also the weight of internal points (weight of end-points is always 1.0)
/*virtual*/ int vtkMAFMuscleFibers::ComputeControlPolygons(
  BCURVE_BLENDING* pR_Info, double s, VCoord* pOutPoly, double& Ws2)
  //------------------------------------------------------------------------
{
  //we need to find the correct pair of curves according to s parameter
  if (s < 0.0)
    s = 0.0;
  else if (s > 1.0)
    s = 1.0;  //clamp s

  while (true)
  {
    if (pR_Info->dblSMin <= s && s  <= pR_Info->dblSMax)
      break;    //found

    pR_Info++;
  } //end while

  int* pCurveA = &m_pCurves[pR_Info->iCurveAPos];
  int* pCurveB = &m_pCurves[pR_Info->iCurveBPos];

  int nPointsA = *pCurveA; 
  int nPointsB = *pCurveB;
  pCurveA++; pCurveB++; //advance to indices

  if (s == pR_Info->dblSMin)
  {
    //return curve A
    for (int i = 0; i < nPointsA; i++)
    {
      for (int j = 0; j < 3; j++) {
        pOutPoly[i][j] = m_pCoords[pCurveA[i]][j];
      }
    }

    Ws2 = pR_Info->dblWSMin;
    return nPointsA;
  }

  if (s == pR_Info->dblSMax)
  {
    //return curve B
    for (int i = 0; i < nPointsB; i++)
    {
      for (int j = 0; j < 3; j++) {
        pOutPoly[i][j] = m_pCoords[pCurveB[i]][j];
      }
    }

    Ws2 = pR_Info->dblWSMax;
    return nPointsB;
  }

  //so we need to perform blending 
  double alfa = pR_Info->dblSMax - pR_Info->dblSMin;
  double beta = (s - pR_Info->dblSMin) / alfa;
  alfa = 1.0 - beta;

  Ws2 = alfa*pR_Info->dblWSMin + beta*pR_Info->dblWSMax;

  if (nPointsA == nPointsB)
  {
    //the number of points is the same                
    for (int i = 0; i < nPointsA; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        pOutPoly[i][j] = alfa*m_pCoords[pCurveA[i]][j] + 
          beta*m_pCoords[pCurveB[i]][j];
      }        
    }

    return nPointsA;
  }

  //polygons have different number of vertices 
  //and we need to blend between them => we will use the larger one as the base
  if (nPointsA < nPointsB)
  {
    nPointsA = nPointsB;
    pCurveA = pCurveB;

    pCurveB = &m_pCurves[pR_Info->iCurveAPos];
    nPointsB = *pCurveB;
    pCurveB++;  

    alfa = beta;
    beta = 1.0 - alfa;
  }

  //pCurveA is larger        
  for (int j = 0; j < 3; j++)
  {
    //the first/last vertex of A is mapped to the first/last vertex of B
    pOutPoly[0][j] = alfa*m_pCoords[pCurveA[0]][j] + beta*m_pCoords[pCurveB[0]][j];
    pOutPoly[nPointsA - 1][j] = alfa*m_pCoords[pCurveA[nPointsA - 1]][j] + 
      beta*m_pCoords[pCurveB[nPointsB - 1]][j];

    pOutPoly[1][j] = alfa*m_pCoords[pCurveA[1]][j] + 
      beta*m_pCoords[pCurveB[pR_Info->Match.Second]][j];

    if (nPointsA > 3) {
      pOutPoly[2][j] = alfa*m_pCoords[pCurveA[2]][j] + 
        beta*m_pCoords[pCurveB[pR_Info->Match.Third]][j];
    }
  }

  return nPointsA;
}

//------------------------------------------------------------------------
//Saves points representing the origin area into out_points.
//Data in out_points is reset in prior to this operation.
/*virtual*/ void vtkMAFMuscleFibers::GetOriginLandmarks(vtkPoints* out_points)
//------------------------------------------------------------------------
{
  //by the default, origin landmarks are all start points of curves     
  out_points->Reset();

  int iPos = 0;
  while (iPos < m_nCurvesEntries)
  {
    out_points->InsertNextPoint(m_pCoords[m_pCurves[iPos + 1]]);
    iPos += m_pCurves[iPos] + 1;
  }
}

//------------------------------------------------------------------------
//Saves points representing the insertion area into out_points.
//Data in out_points is reset in prior to this operation.
/*virtual*/ void vtkMAFMuscleFibers::GetInsertionLandmarks(vtkPoints* out_points)
//------------------------------------------------------------------------
{
  //by the default, insertion landmarks are all end points of curves
  out_points->Reset();

  int iPos = 0;
  while (iPos < m_nCurvesEntries)
  {
    out_points->InsertNextPoint(m_pCoords[m_pCurves[iPos + m_pCurves[iPos]]]);
    iPos += m_pCurves[iPos] + 1;
  }
}

#pragma endregion //vtkMAFMuscleFibers

#pragma region vtkMAFParallelMuscleFibers
vtkCxxRevisionMacro(vtkMAFParallelMuscleFibers, "$Revision: 1.1 $");
//------------------------------------------------------------------------
vtkMAFParallelMuscleFibers::vtkMAFParallelMuscleFibers()
//------------------------------------------------------------------------
{
  m_pCoords = new VCoord[8];
  for (int i = 0; i < 8; i++)
  {
    m_pCoords[i][0] = (double)(i % 2);        //x
    m_pCoords[i][1] = (double)(i / 4);        //y
    m_pCoords[i][2] = (double)((i / 2) % 2);  //z
  }

  m_pCurves = new int[m_nCurvesEntries = 4*(1+2)];               //4 line segments 
  for (int i = 0, iPos = 0; i < 4; i++)
  {
    m_pCurves[iPos++] = 2;
    m_pCurves[iPos++] = i;
    m_pCurves[iPos++] = i + 4;
  }

  m_pFrontBlendings = new BCURVE_BLENDING[m_nFrontBlendings = 1];
  m_pBackBlendings = new BCURVE_BLENDING[m_nBackBlendings = 1];
  memset(m_pFrontBlendings, 0, sizeof(BCURVE_BLENDING));
  memset(m_pBackBlendings, 0, sizeof(BCURVE_BLENDING));

  m_pBackBlendings->dblSMax = m_pFrontBlendings->dblSMax = 1.0;
  m_pBackBlendings->dblWSMin = m_pFrontBlendings->dblWSMin = 1.0;
  m_pBackBlendings->dblWSMax = m_pFrontBlendings->dblWSMax = 1.0;

  m_pFrontBlendings->iCurveAPos = 0*3; m_pFrontBlendings->iCurveBPos = 1*3; //3 = 1 + 2 pos
  m_pBackBlendings->iCurveAPos = 2*3; m_pBackBlendings->iCurveBPos = 3*3;    
}

#pragma endregion //vtkMAFParallelMuscleFibers

#pragma region vtkMAFPennateMuscleFibers
vtkCxxRevisionMacro(vtkMAFPennateMuscleFibers, "$Revision: 1.1 $");
//------------------------------------------------------------------------
//dblX_factor denotes the size of origin (O) and insertion (I) area in the unit cube.
//Both are from interval (0, 1). dblPennateFactor defines the degree of pennation (should by >= 1)
vtkMAFPennateMuscleFibers::vtkMAFPennateMuscleFibers(double dblO_factor, 
                                                     double dblI_factor, double dblPennateFactor)
//------------------------------------------------------------------------
{
  m_pCoords = new VCoord[18];
  for (int i = 0; i < 16; i++)
  {
    m_pCoords[i][0] = (double)(i % 2);        //x    
    m_pCoords[i][2] = (double)((i / 2) % 2);  //z
  }

  for (int i = 0; i < 8; i++){                //points on corner of cube
    m_pCoords[i][1] = (double)(i / 4);        //y
  }

  m_pCoords[10][1] = m_pCoords[8][1] = 1.0 - dblO_factor;
  m_pCoords[11][1] = m_pCoords[9][1] = dblI_factor;

  m_pCoords[12][1] = m_pCoords[14][1] = 1.0 - 0.5*dblO_factor;
  m_pCoords[13][1] = m_pCoords[15][1] = 0.5*dblI_factor;

  for (int i = 16; i < 18; i++)
  {
    m_pCoords[i][0] = 0.5;                     //x
    m_pCoords[i][1] = (m_pCoords[12][1] + m_pCoords[13][1]) * 0.5;    //y
    m_pCoords[i][2] = (double)(i - 16);        //z
  }

  const static int TOPOLOGY[] = { 
    8,0,1, 10,2,3,
    4,5,9, 6,7,11,
    12,16,13, 14,17,15,    
  };

  m_pCurves = new int[m_nCurvesEntries = 6*(1+3)];               //6 curves
  for (int i = 0, iPos = 0, iTPos = 0; i < 6; i++)
  {
    m_pCurves[iPos++] = 3;
    m_pCurves[iPos++] = TOPOLOGY[iTPos++];
    m_pCurves[iPos++] = TOPOLOGY[iTPos++];
    m_pCurves[iPos++] = TOPOLOGY[iTPos++];
  }   

  m_pFrontBlendings = new BCURVE_BLENDING[m_nFrontBlendings = 2];
  m_pBackBlendings = new BCURVE_BLENDING[m_nBackBlendings = 2];
  memset(m_pFrontBlendings, 0, 2*sizeof(BCURVE_BLENDING));
  memset(m_pBackBlendings, 0, 2*sizeof(BCURVE_BLENDING));

  m_pBackBlendings[0].dblSMax = m_pFrontBlendings[0].dblSMax = 0.5;
  m_pBackBlendings[0].dblWSMin = m_pFrontBlendings[0].dblWSMin = dblPennateFactor;
  m_pBackBlendings[0].dblWSMax = m_pFrontBlendings[0].dblWSMax = 1;

  m_pBackBlendings[1].dblSMin = m_pFrontBlendings[1].dblSMin = 0.5;
  m_pBackBlendings[1].dblSMax = m_pFrontBlendings[1].dblSMax = 1.0;
  m_pBackBlendings[1].dblWSMin = m_pFrontBlendings[1].dblWSMin = 1;
  m_pBackBlendings[1].dblWSMax = m_pFrontBlendings[1].dblWSMax = dblPennateFactor;   

  m_pFrontBlendings[0].iCurveAPos = 0*4; m_pFrontBlendings[0].iCurveBPos = 4*4; //4 = 1 len + 3 indices
  m_pFrontBlendings[1].iCurveAPos = 4*4; m_pFrontBlendings[1].iCurveBPos = 2*4;

  m_pBackBlendings[0].iCurveAPos = 1*4; m_pBackBlendings[0].iCurveBPos = 5*4; //4 = 1 len + 3 indices
  m_pBackBlendings[1].iCurveAPos = 5*4; m_pBackBlendings[1].iCurveBPos = 3*4;
}


#pragma endregion //vtkMAFPennateMuscleFibers

#pragma region vtkMAFCurvedMuscleFibers
vtkCxxRevisionMacro(vtkMAFCurvedMuscleFibers, "$Revision: 1.1 $");
//------------------------------------------------------------------------
//dblOI_factor is from interval (0 to 0.5)
//dblCurvedFactor defines the degree of attraction towards inner points (should be >= 1) 
vtkMAFCurvedMuscleFibers::vtkMAFCurvedMuscleFibers(double dblOI_factor, double dblCurvedFactor)
//------------------------------------------------------------------------
{
  m_pCoords = new VCoord[14];
  for (int i = 0; i < 8; i++)
  {
    m_pCoords[i][0] = (double)(i % 2);        //x    
    m_pCoords[i][1] = ((double)(i / 4));      //y
    m_pCoords[i][2] = (double)((i / 2) % 2);  //z
  }

  for (int i = 8; i < 14; i++)
  {
    m_pCoords[i][0] = 0.0;                    //x   
    m_pCoords[i][2] = (double)(i % 2);        //z
  }

  m_pCoords[8][1] = m_pCoords[9][1] = dblOI_factor;
  m_pCoords[10][1] = m_pCoords[11][1] = 0.5;
  m_pCoords[12][1] = m_pCoords[13][1] = 1.0 - dblOI_factor;    

  const static int TOPOLOGY[] = { 
    4,0,1,5,4, 4,2,3,7,6,
    3,8,10,12, 3,9,11,13,
  };

  m_pCurves = new int[m_nCurvesEntries = 2*((1+4) + (1+3))];               //4 curves  
  memcpy(m_pCurves, TOPOLOGY, sizeof(TOPOLOGY));

  m_pFrontBlendings = new BCURVE_BLENDING[m_nFrontBlendings = 1];
  m_pBackBlendings = new BCURVE_BLENDING[m_nBackBlendings = 1];
  memset(m_pFrontBlendings, 0, sizeof(BCURVE_BLENDING));
  memset(m_pBackBlendings, 0, sizeof(BCURVE_BLENDING));

  m_pBackBlendings->dblSMax = m_pFrontBlendings->dblSMax = 1.0;
  m_pBackBlendings->dblWSMin = m_pFrontBlendings->dblWSMin = dblCurvedFactor;
  m_pBackBlendings->dblWSMax = m_pFrontBlendings->dblWSMax = 1.0;

  m_pFrontBlendings->iCurveAPos = 0; m_pFrontBlendings->iCurveBPos = 2*5; //5 = 1 len + 4 pos
  m_pBackBlendings->iCurveAPos = 1*5; m_pBackBlendings->iCurveBPos = 2*5 + 1*4;

  m_pFrontBlendings->Match.Second = m_pFrontBlendings->Match.Third = 1;
  m_pBackBlendings->Match.Second = m_pBackBlendings->Match.Third = 1;

  /*m_pCoords = new VCoord[20];
  for (int i = 0; i < 12; i++)
  {
  m_pCoords[i][0] = (double)(i % 2);        //x    
  m_pCoords[i][1] = ((double)(i / 4)) / 2;  //y
  m_pCoords[i][2] = (double)((i / 2) % 2);  //z
  }

  for (int i = 12; i < 20; i++)
  {
  m_pCoords[i][0] = 0.0;                    //x   
  m_pCoords[i][2] = (double)(i % 2);        //z
  }

  m_pCoords[12][1] = m_pCoords[13][1] = dblOI_factor;
  m_pCoords[14][1] = m_pCoords[15][1] = 1.0 - dblOI_factor;

  m_pCoords[16][1] = m_pCoords[17][1] = 2*dblOI_factor;
  m_pCoords[18][1] = m_pCoords[19][1] = 1.0 - 2*dblOI_factor;

  const static int TOPOLOGY[] = { 
  0,1,5,12,16,4, 8,9,5,14,18,4, 
  2,3,7,13,17,6, 10,11,7,15,19,6 
  };


  m_pCurves = new int[8*(1+3)];               //8 curves  
  for (int i = 0, iPos = 0, iTPos = 0; i < 8; i++)
  {
  m_pCurves[iPos++] = 3;
  m_pCurves[iPos++] = TOPOLOGY[iTPos++];
  m_pCurves[iPos++] = TOPOLOGY[iTPos++];
  m_pCurves[iPos++] = TOPOLOGY[iTPos++];
  }

  m_pFrontBlendings = new BCURVE_BLENDING[2];
  m_pBackBlendings = new BCURVE_BLENDING[2];

  for (int i = 0; i < 2; i++)
  {
  m_pBackBlendings[i].dblSMin = m_pFrontBlendings[i].dblSMin = 0.5*i;
  m_pBackBlendings[i].dblSMax = m_pFrontBlendings[i].dblSMax = 0.5*(1 + i);
  m_pBackBlendings[i].Correspondence = m_pFrontBlendings[i].Correspondence = 0;

  m_pFrontBlendings[i].iCurveAPos = (2*i)*4; //4 = 1 len + 3 pts
  m_pFrontBlendings[i].iCurveBPos = (2*i + 1)*4;
  m_pBackBlendings[i].iCurveAPos = (2*i + 4)*4; 
  m_pBackBlendings[i].iCurveBPos = (2*i + 5)*4;   
  }      
  */
}

#pragma endregion //vtkMAFCurvedMuscleFibers

#pragma region vtkMAFFannedMuscleFibers
vtkCxxRevisionMacro(vtkMAFFannedMuscleFibers, "$Revision: 1.1 $");
//------------------------------------------------------------------------
//dblX1_factor denotes the size of origin (O) and insertion (I) area of the 
//outer band of fibers (in unit cube). dblFanCenter is y-position of middle 
//control point of fanned polygons (inner polygons)
vtkMAFFannedMuscleFibers::vtkMAFFannedMuscleFibers(double dblO1_factor,
                                double dblI1_factor, double dblFanCenter)
//------------------------------------------------------------------------
{  
  m_pCoords = new VCoord[30];
  for (int i = 0; i < 12; i++)
  {
    m_pCoords[i][0] = (double)(i % 2);        //x    
    m_pCoords[i][1] = ((double)(i / 4)) / 2;  //y
    m_pCoords[i][2] = (double)((i / 2) % 2);  //z
  }

  for (int i = 12; i < 16; i++)
  {
    m_pCoords[i][0] = (double)(i % 2);        //x    
    m_pCoords[i][1] = 1.0 - dblI1_factor;     //y
    m_pCoords[i][2] = (double)((i / 2) % 2);  //z
  }

  double b = 0.5 - dblO1_factor;
  for (int i = 16; i < 28; i++)
  {
    int iVal = i - 16;
    m_pCoords[i][0] = dblO1_factor + (iVal % 3)*b;   //x   
    m_pCoords[i][1] = dblFanCenter * (iVal / 6);      //y
    m_pCoords[i][2] = (double)((iVal / 3) % 2);      //z
  }

  for (int i = 28; i < 30; i++)
  {
    m_pCoords[i][0] = 0.5;
    m_pCoords[i][1] = 1.0;
    m_pCoords[i][2] = (double)(i % 2);
  }

  m_pCurves = new int[m_nCurvesEntries = 10*(1+3)];               //10 curves  
  int iPos = 0;
  for (int i = 0; i < 4; i++)
  {
    m_pCurves[iPos++] = 3;
    m_pCurves[iPos++] = i;
    m_pCurves[iPos++] = i + 4;
    m_pCurves[iPos++] = i + 12;
  }

  for (int i = 16; i < 22; i++)
  {
    m_pCurves[iPos++] = 3;
    m_pCurves[iPos++] = i;
    m_pCurves[iPos] = i + 6;
    iPos += 2;
  }

  m_pCurves[16 + 3] = 8;
  m_pCurves[20 + 3] = 28;
  m_pCurves[24 + 3] = 9;
  m_pCurves[28 + 3] = 10;
  m_pCurves[32 + 3] = 29;
  m_pCurves[36 + 3] = 11;

  //blending is also more complex
  m_pFrontBlendings = new BCURVE_BLENDING[m_nFrontBlendings = 4];
  m_pBackBlendings = new BCURVE_BLENDING[m_nBackBlendings = 4];

  memset(m_pFrontBlendings, 0, 4*sizeof(BCURVE_BLENDING));
  memset(m_pBackBlendings, 0, 4*sizeof(BCURVE_BLENDING));

  m_pBackBlendings[0].dblSMax = m_pFrontBlendings[0].dblSMax = dblO1_factor;

  m_pBackBlendings[1].dblSMin = m_pFrontBlendings[1].dblSMin = dblO1_factor;
  m_pBackBlendings[1].dblSMax = m_pFrontBlendings[1].dblSMax = 0.5;

  m_pBackBlendings[2].dblSMin = m_pFrontBlendings[2].dblSMin = 0.5;
  m_pBackBlendings[2].dblSMax = m_pFrontBlendings[2].dblSMax = 1.0 - dblO1_factor;

  m_pBackBlendings[3].dblSMin = m_pFrontBlendings[3].dblSMin = 1.0 - dblO1_factor;
  m_pBackBlendings[3].dblSMax = m_pFrontBlendings[3].dblSMax = 1.0;

  for (int i = 0; i < 4; i++)
  {    
    m_pFrontBlendings[i].iCurveAPos = (i + 3)*4; //4 = 1 len + 3 pts
    m_pFrontBlendings[i].iCurveBPos = (i + 4)*4;     
    m_pBackBlendings[i].iCurveAPos = (i + 6)*4; 
    m_pBackBlendings[i].iCurveBPos = (i + 7)*4;   

    m_pBackBlendings[i].dblWSMin = m_pFrontBlendings[i].dblWSMin = 1.0;
    m_pBackBlendings[i].dblWSMax = m_pFrontBlendings[i].dblWSMax = 1.0;
  }

  m_pFrontBlendings[0].iCurveAPos = 0*4; //4 = 1 len + 3 pts
  m_pFrontBlendings[3].iCurveBPos = 1*4;
  m_pBackBlendings[0].iCurveAPos = 2*4;
  m_pBackBlendings[3].iCurveBPos = 3*4;
}

#pragma endregion //vtkMAFFannedMuscleFibers


#pragma region vtkMAFRectusMuscleFibers
vtkCxxRevisionMacro(vtkMAFRectusMuscleFibers, "$Revision: 1.1 $");
//------------------------------------------------------------------------
//See Blemker's paper about rectus modeling. O1_x and O2_x denote the x-coordinates
//of origin rectangular area on the first side of unit cube, O12_y is denotes the 
//y-coordinate of this area (the other y-coordinate is 1.0). I*_? defines the
//insertion area on the opposite side. Note: angle of fibers is governed by y-coordinates.
//dblBoundFactor defines how fibers attract to sides of cube (should be >= 1)
vtkMAFRectusMuscleFibers::vtkMAFRectusMuscleFibers(double dblO1_x, double dblO2_x, double dblO12_y, 
                                                   double dblI1_x, double dblI2_x, double dblI12_y,
                                                   double dblBoundFactor)
//------------------------------------------------------------------------
{
  m_pCoords = new VCoord[36];
  for (int i = 0; i < 20; i++)
  {
    m_pCoords[i][0] = (double)(i % 2);        //x        
    m_pCoords[i][2] = (double)((i / 2) % 2);  //z
  }

  //corners
  for (int i = 0; i < 8; i++) {
    m_pCoords[i][1] = ((double)(i / 4));      //y
  }

  //compute important measurement
  double L1 = (1 + dblO12_y) / 3;
  double L2 = L1 - dblO12_y;
  //double L3 = (L2*dblI1_x) / (1 - L2 - dblI12_y);
  //double L4 = (L2*(1 - dblI2_x)) / (1 - L2 - dblI12_y);

  //points on edges
  for (int i = 0; i < 2; i++)
  {
    m_pCoords[8 + i][1] = L2;    
    m_pCoords[12 + i][1] = (1 - L2) / 2;    
    m_pCoords[16 + i][1] = (1 - L2);

    m_pCoords[18 + i][1] =  1.0 - L2;
    m_pCoords[14 + i][1] =  m_pCoords[18 + i][1] - 1.5*L2;
    m_pCoords[10 + i][1] =  m_pCoords[14 + i][1] - 0.5;    
  }

  //complete "front" face
  for (int i = 0; i < 4; i++)
  {      
    m_pCoords[20 + i][0] = dblO1_x;
    m_pCoords[24 + i][0] = dblO2_x;
    m_pCoords[24 + i][2] = m_pCoords[20 + i][2] = 0.0;
  }

  m_pCoords[24][1] = m_pCoords[20][1] = dblO12_y;
  m_pCoords[25][1] = m_pCoords[21][1] = dblO12_y + L2;
  m_pCoords[26][1] = m_pCoords[22][1] = 1.0 - L1;
  m_pCoords[27][1] = m_pCoords[23][1] = 1.0;

  //complete "back" face
  for (int i = 0; i < 3; i++)
  {
    m_pCoords[28 + i][0] = dblI1_x;
    m_pCoords[31 + i][0] = dblI2_x;
    m_pCoords[28 + i][2] = m_pCoords[31 + i][2] = 1.0;
  }

  m_pCoords[31][1] = m_pCoords[28][1] = 0.0;
  m_pCoords[32][1] = m_pCoords[29][1] = dblI12_y - L2;
  m_pCoords[33][1] = m_pCoords[30][1] = dblI12_y;
  
  //and last two coordinates
  m_pCoords[35][0] = dblI2_x - L2; 
  m_pCoords[34][0] = dblI1_x + L2;
  m_pCoords[35][1] = m_pCoords[34][1] = dblI12_y;
  m_pCoords[35][2] = m_pCoords[34][2] = 1.0;
    
  const static int TOPOLOGY[] = 
  { 
    //control polygons for right winding (will be m_pFrontBlendings)
    2,20,0, 3,21,8,2, 4,22,12,10,28, 4,23,16,14,29, 3,4,18,30, 2,6,34, 

    //control polygons for left winding (will be m_pBackBlendings)
    2,24,1, 3,25,9,3, 4,26,13,11,31, 4,27,17,15,32, 3,5,19,33, 2,7,35, 
  };  

  m_pCurves = new int[m_nCurvesEntries = sizeof(TOPOLOGY)/sizeof(int)];               //many curves  
  memcpy(m_pCurves, TOPOLOGY, sizeof(TOPOLOGY));

  m_pFrontBlendings = new BCURVE_BLENDING[m_nFrontBlendings = 5];
  m_pBackBlendings = new BCURVE_BLENDING[m_nBackBlendings = 5];
  memset(m_pFrontBlendings, 0, 5*sizeof(BCURVE_BLENDING));
  memset(m_pBackBlendings, 0, 5*sizeof(BCURVE_BLENDING));

  //create blending
  double SBnd[6];
  SBnd[5] = 1.0;
  SBnd[4] = 1.0 / (1.0 + L2);
  for (int i = 3; i >= 1; i--){
    SBnd[i] = m_pCoords[4 + 4*i][1] * SBnd[4];
  }
  SBnd[0] = 0.0;

  int iTPos = 0;
  double dblBoundFactor2 = 1.5*dblBoundFactor;
    
    // 2*dblBoundFactor;
  for (int i = 0; i < 5; i++)
  {  
    //the first and the last blending uses Bezier of degree 2, the rest uses degree 3
    //in order to have smooth connection, we need to to have different bound factor
    if ((i & 3) == 0)
    {      
      m_pFrontBlendings[i].dblWSMin = m_pFrontBlendings[i].dblWSMax = dblBoundFactor;
      m_pBackBlendings[i].dblWSMin = m_pBackBlendings[i].dblWSMax = dblBoundFactor;
    }
    else
    {
      m_pFrontBlendings[i].dblWSMin = m_pFrontBlendings[i].dblWSMax = dblBoundFactor2;
      m_pBackBlendings[i].dblWSMin = m_pBackBlendings[i].dblWSMax = dblBoundFactor2;
    }

    m_pBackBlendings[i].dblSMin = m_pFrontBlendings[i].dblSMin = SBnd[i];
    m_pBackBlendings[i].dblSMax = m_pFrontBlendings[i].dblSMax = SBnd[i + 1];

    m_pFrontBlendings[i].iCurveAPos = iTPos;    
    iTPos += TOPOLOGY[iTPos] + 1;

    m_pFrontBlendings[i].iCurveBPos = iTPos;
  }

  iTPos += TOPOLOGY[iTPos] + 1;
  for (int i = 0; i < 5; i++)
  {
    m_pBackBlendings[i].iCurveAPos = iTPos;    
    iTPos += TOPOLOGY[iTPos] + 1;

    m_pBackBlendings[i].iCurveBPos = iTPos;
  }

  m_pFrontBlendings[0].Match.Second = m_pBackBlendings[0].Match.Second = 1;
  m_pFrontBlendings[1].Match.Second = m_pBackBlendings[1].Match.Second = 1;
  m_pFrontBlendings[1].Match.Third = m_pBackBlendings[1].Match.Third = 2;

  m_pFrontBlendings[3].Match.Second = m_pBackBlendings[3].Match.Second = 0;
  m_pFrontBlendings[3].Match.Third = m_pBackBlendings[3].Match.Third = 1;
  //m_pFrontBlendings[4].Match.Second = m_pBackBlendings[4].Match.Second = 0;
}

#pragma endregion //vtkMAFRectusMuscleFibers
