/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpIsotropicSampleTable.cpp,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "lhpIsotropicSampleTable.h"
#include "lhpTextureOrientationUseful.h"

#include <ostream>

#ifndef M_PI
  #define _USE_MATH_DEFINES
#endif

#include <cmath>



//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
lhpIsotropicSampleTable::lhpIsotropicSampleTable(int numberOfTheta, int maxNumberOfPhi, int numberOfR, double rstepSize)
: m_numberOfTheta(numberOfTheta), m_numberOfPhiMax(maxNumberOfPhi), m_numberOfR(numberOfR)
{
  int i, j, k ;

  // tabulate theta such that 0 <= theta <= pi
  m_theta = new double[m_numberOfTheta] ;
  for (i = 0 ;  i < m_numberOfTheta ;  i++){
    m_theta[i] = M_PI * (double)i / (double)(m_numberOfTheta-1) ;
  }


  // How many phi's for each theta
  // This should be proportional to sin(theta) so that the points on each horizontal ring are have the same spacing
  m_numberOfPhi= new int[m_numberOfTheta] ;
  for (i = 0 ;  i < m_numberOfTheta ;  i++){
    m_numberOfPhi[i] = lhpTextureOrientationUseful::Round((double)m_numberOfPhiMax * sin(m_theta[i])) ;
    if (m_numberOfPhi[i] == 0)
      m_numberOfPhi[i] = 1 ;
  }

  // tabulate phi, such that 0 <= phi < 2pi
  m_phi = new double*[m_numberOfTheta] ;
  for (i = 0 ;  i < m_numberOfTheta ;  i++)
    m_phi[i] = new double[m_numberOfPhi[i]] ;

  for (i = 0 ;  i < m_numberOfTheta ;  i++){
    for (j = 0 ;  j < m_numberOfPhi[i];  j++)
      m_phi[i][j] = 2.0*M_PI * (double)j / (double)(m_numberOfPhi[i]) ;
  }

  // tabulate r, with steps equal to voxel spacing
  m_r = new double[m_numberOfR] ;
  for (k = 0 ;  k < m_numberOfR ;  k++)
    m_r[k] = rstepSize * (double)(k+1) ;


  // Tabulate corresponding cartesian coords
  m_x = new double[m_numberOfR * m_numberOfPhiMax * m_numberOfTheta] ;
  m_y = new double[m_numberOfR * m_numberOfPhiMax * m_numberOfTheta] ;
  m_z = new double[m_numberOfR * m_numberOfPhiMax * m_numberOfTheta] ;
  for (i = 0 ;  i < m_numberOfTheta ;  i++){
    for (j = 0 ;  j < m_numberOfPhi[i];  j++){
      for (k = 0 ;  k < m_numberOfR ;  k++){
        double x, y, z ;
        lhpTextureOrientationUseful::PolarToCart(m_r[k], m_theta[i], m_phi[i][j], x, y, z) ;
        SetXYZ(i,j,k,x,y,z) ;
      }
    }
  }

  // Allocate array for texture results
  m_sampleValue = new double[m_numberOfR * m_numberOfPhiMax * m_numberOfTheta] ;
}



//------------------------------------------------------------------------------
// Deconstructor
//------------------------------------------------------------------------------
lhpIsotropicSampleTable::~lhpIsotropicSampleTable()
{
  int i ;

  delete [] m_theta ;
  delete [] m_r ;

  for (i = 0 ;  i < m_numberOfTheta ;  i++)
    delete [] m_phi[i] ;
  delete [] m_phi ;

  delete [] m_numberOfPhi ;

  delete [] m_x ;
  delete [] m_y ;
  delete [] m_z ;
  delete [] m_sampleValue ;
}



//------------------------------------------------------------------------------
// Print self
void lhpIsotropicSampleTable::PrintSelf(std::ostream& os, int indent)
//------------------------------------------------------------------------------
{
  os << "no. of theta = " << m_numberOfTheta << std::endl ;
  os << "max no. of phi = " << m_numberOfPhiMax << std::endl ;
  os << "no. of r = " << m_numberOfR << std::endl ;
  
  for (int i = 0 ;  i < m_numberOfTheta ;  i++)
    os << i << " theta(i) = " << m_theta[i] << " no. of phi = " << m_numberOfPhi[i] << std::endl ;
  os << std::endl ;

  for (int i = 0 ;  i < m_numberOfTheta ;  i++){
    os << i << " " ;
    for (int j = 0 ;  j < m_numberOfPhi[i] ;  j++){
      os << m_phi[i][j] << " " ;
    }
    os << std::endl ;
  }
  os << std::endl ;

  for (int k = 0 ;  k < m_numberOfR ;  k++)
    os << k << " r(k) = " << m_r[k] << std::endl ;
  os << std::endl ;

}
