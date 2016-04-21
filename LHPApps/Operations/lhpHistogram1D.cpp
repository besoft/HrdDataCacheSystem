/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpHistogram1D.cpp,v $
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


#include "lhpHistogram1D.h"
#include <iostream>
#include <assert.h>


//------------------------------------------------------------------------------
// Constructor
lhpHistogram1D::lhpHistogram1D(int numberOfBins, double minVal, double maxVal)
: m_numberOfBins(numberOfBins), m_minVal(minVal), m_maxVal(maxVal),
m_numberOfQuantiles(0), m_quantilePoints(NULL)
//------------------------------------------------------------------------------
{
  // calculate bin size
  m_binSize = (maxVal-minVal) / (double)m_numberOfBins ;

  // allocate bin array
  m_bin = new int[m_numberOfBins] ;

  Initialize() ;
}


//------------------------------------------------------------------------------
// Constructor
lhpHistogram1D::lhpHistogram1D(double nominalBinSize, double minVal, double maxVal)
: m_minVal(minVal), m_maxVal(maxVal),
m_numberOfQuantiles(0), m_quantilePoints(NULL)
//------------------------------------------------------------------------------
{
  // calculate no. of bins, which must be at least 1
  m_numberOfBins = (int)((maxVal-minVal) / nominalBinSize + 0.5) ;
  if (m_numberOfBins < 1)
    m_numberOfBins = 1 ;

  // calculate actual bin size
  m_binSize = (maxVal-minVal) / (double)m_numberOfBins ;

  // allocate bin array
  m_bin = new int[m_numberOfBins] ;

  Initialize() ;
}




//------------------------------------------------------------------------------
// Destructor
lhpHistogram1D::~lhpHistogram1D()
//------------------------------------------------------------------------------
{
  delete [] m_bin ;

  if (m_quantilePoints != NULL)
    delete [] m_quantilePoints ;
}


//------------------------------------------------------------------------------
// Set all data to zero
void lhpHistogram1D::Initialize()
//------------------------------------------------------------------------------
{
  // set bins and total to zero
  for (int i = 0 ;  i < m_numberOfBins ;  i++)
    m_bin[i] = 0 ;
  m_total = 0 ;

  // remove quantile data
  m_numberOfQuantiles = 0 ;
  if (m_quantilePoints != NULL){
    delete [] m_quantilePoints ;
    m_quantilePoints = NULL ;
  }
}



//------------------------------------------------------------------------------
// Get total in this bin (returns 0 if out of range)
int lhpHistogram1D::GetBin(int i) const 
//------------------------------------------------------------------------------
{
  if ((i >= 0) && (i < m_numberOfBins))
    return m_bin[i] ;
  else
    return 0 ;
}


//------------------------------------------------------------------------------
// Increment bin corresponding to this value
// Values outside the range are ignored
void lhpHistogram1D::AddValue(double val)
//------------------------------------------------------------------------------
{
  if ((val >= m_minVal) && (val <= m_maxVal)){
    int i = CalcBinId(val) ;
    m_bin[i]++ ;
    m_total++ ;
  }
}



//------------------------------------------------------------------------------
// Print Self
void lhpHistogram1D::PrintSelf(std::ostream& os, int indent) const
//------------------------------------------------------------------------------
{
  double range[2] ;

  os << "no. of bins = " << m_numberOfBins << std::endl ;
  os << "bin size = " << m_binSize << std::endl ;
  os << "range of values = " << m_minVal << " " << m_maxVal << std::endl ;
  os << "total = " << m_total << std::endl ;

  os << "bins" << std::endl ;
  for (int i = 0 ;  i < m_numberOfBins ;  i++){
    CalculateBinRange(i, range) ;
    os << i << "\t" << m_bin[i] << "\t" << "range " << range[0] << " - " << range[1] << std::endl ;
  }

  if (m_quantilePoints != NULL){
    os << "quantile points" << std::endl ;
    for (int i = 0 ;  i < m_numberOfQuantiles ;  i++){
      os << i << "\t" << m_quantilePoints[i] << std::endl ;
    }
    os << std::endl ;
  }
}





//------------------------------------------------------------------------------
// calculate id of bin from value
int lhpHistogram1D::CalcBinId(double val) const
//------------------------------------------------------------------------------
{
  if (val == m_maxVal)
    return m_numberOfBins-1 ;
  else
    return (int)((val-m_minVal) / m_binSize) ;
}




//------------------------------------------------------------------------------
// get range of values covered by bin i
// range[0] <= v < range[1]  where i < nbins-1
// range[0] <= v <= range[1] where i = nbins-1
void lhpHistogram1D::CalculateBinRange(int i, double range[2]) const
//------------------------------------------------------------------------------
{
  range[0] = m_minVal + m_binSize*(double)i ;
  range[1] = range[0] + m_binSize ;
}



//------------------------------------------------------------------------------
// calculate maximum bin
int lhpHistogram1D::MaxBin() const
//------------------------------------------------------------------------------
{
  int imax = 0 ;
  for (int i = 0 ;  i < m_numberOfBins ;  i++){
    if (m_bin[i] > m_bin[imax])
      imax = i ;
  }

  return imax ;
}


//------------------------------------------------------------------------------
// calculate mean bin
double lhpHistogram1D::MeanBin(int istart) const
//------------------------------------------------------------------------------
{
  double mean = 0.0 ;
  for (int i = istart ;  i < m_numberOfBins+istart ;  i++){
    int imod = i % m_numberOfBins ;
    mean += (double)(i*m_bin[imod]) ;
  }
  mean /= (double)m_total ;

  return mean ;
}



//------------------------------------------------------------------------------
// calculate median value
double lhpHistogram1D::MedianValue() const
//------------------------------------------------------------------------------
{
  double range[2] ;
  double cumHistOld = 0.0 ;
  double cumHist = 0.0 ;
  double target = 0.5 * (double)m_total ;
  double value = 0.0 ;

  for (int j = 0;  j < m_numberOfBins ;  j++){
    cumHistOld = cumHist ;
    cumHist += m_bin[j] ;

    if (cumHist >= target){
      // interpolate within bin for greater accuracy
      CalculateBinRange(j, range) ;
      double value = ((target-cumHistOld)*range[1] + (cumHist-target)*range[0]) / (cumHist - cumHistOld) ;
      break ;
    }
  }

  return value ;
}







//------------------------------------------------------------------------------
// Calculate quantile points - values which mark the start of each quantile range
// This allocates n+1 points, with quantile[n] marking the end of the range.
void lhpHistogram1D::CalculateQuantilePoints(int numberOfQuantiles)
//------------------------------------------------------------------------------
{
  //----------------------------------------------------------------------------
  // allocate or realloate memory
  //----------------------------------------------------------------------------
  if ((m_quantilePoints != NULL) && (numberOfQuantiles != m_numberOfQuantiles)){
    // delete and reallocate array
    delete [] m_quantilePoints ;
    m_quantilePoints = new double[numberOfQuantiles+1] ;
  }
  else if (m_quantilePoints == NULL){
    // allocate array
    m_quantilePoints = new double[numberOfQuantiles+1] ;
  }
  else{
    // do nothing - array is already allocated with requested no. of quantiles
  }

  m_numberOfQuantiles = numberOfQuantiles ;


  //----------------------------------------------------------------------------
  // calculate quantile points
  //----------------------------------------------------------------------------

  // pre-calculate target values for each quantile
  int i, j ;
  double *target = new double[m_numberOfQuantiles+1] ;
  for (i = 0 ;  i <= m_numberOfQuantiles ;  i++)
    target[i] = (double)(i * m_total) / (double)m_numberOfQuantiles ;
  
  // first quantile point is min val
  m_quantilePoints[0] = m_minVal ;

  double range[2] ;
  double cumHistOld = 0.0 ;
  double cumHist = 0.0 ;
  for (i = 1, j = -1 ;  i < m_numberOfQuantiles ;  i++){
    // add up cumulative histogram until cumHist is greater or equal to target
    // note that we increment j before calculating cumHist, otherwise we get 
    // the next value of j when we exit the while loop.
    while (cumHist < target[i]){
      j++ ;

      if (j >= m_numberOfBins){
        std::cout << "bin index " << j << " out of range in CalculateQuantilePoints()" << std::endl ;
        assert(false) ;
      }

      cumHistOld = cumHist ;
      cumHist += m_bin[j] ;
    }

    // Error check.
    if (m_bin[j] == 0){
      // If this bin is zero, the target should have been exceeded in the previous bin
      // and then moved on to the next target.  It would cause division by zero in the next step.
      std::cout << "Error at quantile " << i << " bin " << j << " in CalculateQuantilePoints()" << std::endl ;
      std::cout << "Zero bin cannot be a quantile" << std::endl ;
      assert(false) ;
    }

    // interpolate across bin j to find where it crossed the target
    CalculateBinRange(j, range) ;
    m_quantilePoints[i] = ((cumHist-target[i])*range[0] + (target[i]-cumHistOld)*range[1]) / (cumHist - cumHistOld) ;
  }

  // last quantile point is max val (not actually a quantile, but included for convenience)
  m_quantilePoints[m_numberOfQuantiles] = m_maxVal ;

  delete [] target ;
}



//------------------------------------------------------------------------------
// Which quantile is value v in.
// Possible return values are 0 <= i < n where n is no. of quantiles.
// Returns i = -1 if the value is out of range.
int lhpHistogram1D::WhichQuantile(double v) const
//------------------------------------------------------------------------------
{
  if ((v < m_minVal) || (v > m_maxVal))
    return -1 ;

  // Looking for quantile i such that q[i] <= v < q[i+1] 
  // Therefore return the first i for which v < q[i+1]
  // Note that this means that quantiles for which q[i] = q[i+1] will be empty.
  for (int i = 0 ;  i < m_numberOfQuantiles ;  i++){
    if (v < m_quantilePoints[i+1])
      return i ;
  }

  // value must be maximum, so return quantile n-1, which includes the max value
  return m_numberOfQuantiles - 1 ;
}

