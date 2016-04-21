/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpCooccurrenceMatrixFilter.cpp,v $
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


#include "lhpCooccurrenceMatrixFilter.h"

#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkDataSet.h"

#include <cstdlib>
#include <ostream>
#include <assert.h>

#ifndef M_PI
  #define _USE_MATH_DEFINES
#endif

#include <cmath>
#include <algorithm>


//----------------------------------------------------------------------------
// mandatory vtk macro
vtkCxxRevisionMacro(lhpCooccurrenceMatrixFilter, "$Revision: 1.1 $");


//----------------------------------------------------------------------------
// standard New() method
vtkStandardNewMacro(lhpCooccurrenceMatrixFilter);


//----------------------------------------------------------------------------
// Constructor
lhpCooccurrenceMatrixFilter::lhpCooccurrenceMatrixFilter() : m_target_component(0), m_dx(0), m_dy(0), m_dz(0), 
m_subsetRangeFlag(false), m_outDimsSet(false), m_voiDimsSet(false), m_voiPosSet(false),
m_voiSoftBoundary(true)
{
  // set all the calculation requests off
  SetCalculationRequest(CooAll, false) ;
}



//----------------------------------------------------------------------------
// Destructor
lhpCooccurrenceMatrixFilter::~lhpCooccurrenceMatrixFilter() 
{
}



//----------------------------------------------------------------------------
// set target component
void lhpCooccurrenceMatrixFilter::SetTargetComponent(int c)
//----------------------------------------------------------------------------
{
  if (c != m_target_component){
    m_target_component = c ;
    this->Modified() ;
  }
}

//----------------------------------------------------------------------------
// get target component
int lhpCooccurrenceMatrixFilter::GetTargetComponent() const
//----------------------------------------------------------------------------
{
  return m_target_component ;
}



//----------------------------------------------------------------------------
// Turn calculation on/off
void lhpCooccurrenceMatrixFilter::SetCalculationRequest(enum CooCalculationId id, bool requestCalc)
//----------------------------------------------------------------------------
{
  if (id == CooAll){
    // set or unset all calculations
    for (int i = 0 ;  i < 20 ;  i++)
      m_calculationFlag[i] = requestCalc ;
  }
  else{
    // set the requested calculation on/off
    m_calculationFlag[id] = requestCalc ;
  }

  this->Modified() ;
}



//----------------------------------------------------------------------------
// Set the volume of interest dimensions
void lhpCooccurrenceMatrixFilter::SetVoiDimensions(int dimsx, int dimsy, int dimsz) 
//----------------------------------------------------------------------------
{
  m_voiDimensions[0] = dimsx ;
  m_voiDimensions[1] = dimsy ;
  m_voiDimensions[2] = dimsz ;

  m_voiDimsSet = true ;
  this->Modified() ;
}


//----------------------------------------------------------------------------
// Set the volume of interest dimensions
void lhpCooccurrenceMatrixFilter::SetVoiDimensions(const int dims[3]) 
//----------------------------------------------------------------------------
{
  SetVoiDimensions(dims[0], dims[1], dims[2]) ;
}


//----------------------------------------------------------------------------
// Set the position (start corner) of the volume of interest
void lhpCooccurrenceMatrixFilter::SetVoiPosition(int x, int y, int z)
//----------------------------------------------------------------------------
{
  m_voiPos[0] = x ;
  m_voiPos[1] = y ;
  m_voiPos[2] = z ;

  m_voiPosSet = true ;
  this->Modified() ;
}


//----------------------------------------------------------------------------
// Set the position (start corner) of the volume of interest
void lhpCooccurrenceMatrixFilter::SetVoiPosition(const int pos[3])
//----------------------------------------------------------------------------
{
  SetVoiPosition(pos[0], pos[1], pos[2]) ;
}



//----------------------------------------------------------------------------
// Set the voi extents, assuming the dimensions and position are set
void lhpCooccurrenceMatrixFilter::SetVoiExtents()
//----------------------------------------------------------------------------
{
  assert(m_voiDimsSet) ;
  assert(m_voiPosSet) ;

  m_voiExtents[0] = m_voiPos[0] ;
  m_voiExtents[1] = m_voiPos[0] + m_voiDimensions[0] - 1 ;
  m_voiExtents[2] = m_voiPos[1] ;
  m_voiExtents[3] = m_voiPos[1] + m_voiDimensions[1] - 1 ;
  m_voiExtents[4] = m_voiPos[2] ;
  m_voiExtents[5] = m_voiPos[2] + m_voiDimensions[2] - 1 ;
}



//----------------------------------------------------------------------------
// Configure the output image
void lhpCooccurrenceMatrixFilter::ExecuteInformation(vtkImageData *inData, vtkImageData *outData)
//----------------------------------------------------------------------------
{
  // Mustn't multithread this filter
  this->SetNumberOfThreads(1) ;

  // The output dimensions depend on the input scalar range.
  // To set it here automatically we would have to call inData->Update(), which is a bit ropy, 
  // so we just check that the user has set it manually.
  assert(m_outDimsSet) ;

  outData->SetDimensions(m_dimsOut) ;
  outData->SetWholeExtent(0, m_dimsOut[0]-1, 0, m_dimsOut[1]-1, 0, m_dimsOut[2]-1) ;
  outData->SetOrigin(0,0,0) ;
  outData->SetSpacing(1,1,1) ;
  outData->SetNumberOfScalarComponents(1) ;
  outData->SetScalarTypeToInt() ;


  //----------------------------------------------------------------------------
  // Set the volume of interest
  //----------------------------------------------------------------------------
  int inDims[3], inExt[6] ;
  inData->GetDimensions(inDims) ;
  inData->GetWholeExtent(inExt) ;

  if (!m_voiDimsSet){
    // if not set, default dimensions for voi is whole image
    SetVoiDimensions(inDims) ;
  }
   
  if (!m_voiPosSet){
    // if not set, default position for voi is start corner of whole image
    SetVoiPosition(inExt[0], inExt[2], inExt[4]) ;
  }

  // set the voi extents
  SetVoiExtents() ;
}




//----------------------------------------------------------------------------
// Set the input UpdateExtent.
// You need this function if the input and output images
// have different extents.
void lhpCooccurrenceMatrixFilter::ComputeInputUpdateExtent(int inExt[6], int *vtkNotUsed(outExt))
{
  int *wholeExtent;

  wholeExtent = this->GetInput()->GetWholeExtent();
  memcpy(inExt, wholeExtent, 6*sizeof(int));
}





//----------------------------------------------------------------------------
// This function does the work of the filter.
// IT is the input scalar type, OT is the output scalar type.
template <class IT, class OT>
void ImageFilterExecute(
                        lhpCooccurrenceMatrixFilter *self,
                        vtkImageData *inData,
                        IT *inPtr1,
                        vtkImageData *outData, 
                        OT *outPtr,
                        int outExt[6], 
                        int id
                        )
{
  int x1, y1, z1, compId ;
  int x2, y2, z2 ;
  IT *inPtr2 ;

  // Get no. of components
  int ncompsIn = inData->GetNumberOfScalarComponents() ;
  int ncompsOut = outData->GetNumberOfScalarComponents() ;

  // Get input Extent
  int *inExt = inData->GetWholeExtent() ;



  //--------------------------------------------------------------------------
  // Set the limits of x, y and z for the first voxel
  //--------------------------------------------------------------------------
  int zstart1, zlast1, ystart1, ylast1, xstart1, xlast1 ;

  // calculate nominal range of x, y and z for the given voi
  if (self->m_voiSoftBoundary){
    // soft boundary - only one voxel per sample need be inside the voi
    // n.b. this can create a range where not all of the voxels are valid
    if (self->m_dz >= 0){
      zstart1 = std::max(inExt[4], self->m_voiExtents[4] - self->m_dz) ;
      zlast1 = std::min(inExt[5] - self->m_dz, self->m_voiExtents[5]) ;
    }
    else{
      zstart1 = std::max(inExt[4] - self->m_dz, self->m_voiExtents[4]) ;
      zlast1 = std::min(inExt[5], self->m_voiExtents[5] - self->m_dz) ;
    }    

    if (self->m_dy >= 0){
      ystart1 = std::max(inExt[2], self->m_voiExtents[2] - self->m_dy) ;
      ylast1 = std::min( inExt[3] - self->m_dy, self->m_voiExtents[3]) ;
    }
    else{
      ystart1 = std::max(inExt[2] - self->m_dy, self->m_voiExtents[2]) ;
      ylast1 = std::min(inExt[3], self->m_voiExtents[3] - self->m_dy) ;
    }    

    if (self->m_dx >= 0){
      xstart1 = std::max(inExt[0], self->m_voiExtents[0] - self->m_dx) ;
      xlast1 = std::min(inExt[1] - self->m_dx, self->m_voiExtents[1]) ;
    }
    else{
      xstart1 = std::max(inExt[0] - self->m_dx, self->m_voiExtents[0]) ;
      xlast1 = std::min(inExt[1], self->m_voiExtents[1] - self->m_dx) ;
    }    
  }
  else{
    // hard boundary - both voxels per sample must be inside the voi
    if (self->m_dz >= 0){
      zstart1 = std::max(inExt[4], self->m_voiExtents[4]) ;
      zlast1 = std::min(inExt[5] - self->m_dz, self->m_voiExtents[5] - self->m_dz) ;
    }
    else{
      zstart1 = std::max(inExt[4] - self->m_dz, self->m_voiExtents[4] - self->m_dz) ;
      zlast1 = std::min(inExt[5], self->m_voiExtents[5]) ;
    }

    if (self->m_dy >= 0){
      ystart1 = std::max(inExt[2], self->m_voiExtents[2]) ;
      ylast1 = std::min(inExt[3] - self->m_dy, self->m_voiExtents[3] - self->m_dy) ;
    }
    else{
      ystart1 = std::max(inExt[2] - self->m_dy, self->m_voiExtents[2] - self->m_dy) ;
      ylast1 = std::min(inExt[3], self->m_voiExtents[3]) ;
    }

    if (self->m_dx >= 0){
      xstart1 = std::max(inExt[0], self->m_voiExtents[0]) ;
      xlast1 = std::min(inExt[1] - self->m_dx, self->m_voiExtents[1] - self->m_dx) ;
    }
    else{
      xstart1 = std::max(inExt[0] - self->m_dx, self->m_voiExtents[0] - self->m_dx) ;
      xlast1 = std::min(inExt[1], self->m_voiExtents[1]) ;
    }
  }


  // Create lut of which values between zstart1 and zlast1 etc are actually valid
  // We have to do this because in the case of the "soft boundary", the voi can have a range which is disjoint
  // Eg if voiExts is 15-19 and dz = 10, the valid values are z1 = 5-9 and 15-19
  int *zlutArray = new int[zlast1-zstart1+1] ;
  int *zlut = zlutArray - zstart1 ;
  for (z1 = zstart1 ;  z1 <= zlast1 ;  z1++){
    z2 = z1 + self->m_dx ;
    if (self->m_voiSoftBoundary){
      if (((z1 >= self->m_voiExtents[4]) && (z1 <= self->m_voiExtents[5])) || ((z2 >= self->m_voiExtents[4]) && (z2 <= self->m_voiExtents[5])))
        zlut[z1] = 1 ;
      else
        zlut[z1] = 0 ;
    }
    else{
      if (((z1 >= self->m_voiExtents[4]) && (z1 <= self->m_voiExtents[5])) && ((z2 >= self->m_voiExtents[4]) && (z2 <= self->m_voiExtents[5])))
        zlut[z1] = 1 ;
      else
        zlut[z1] = 0 ;
    }
  }

  // lut for valid y values
  int *ylutArray = new int[ylast1-ystart1+1] ;
  int *ylut = ylutArray - ystart1 ;
  for (y1 = ystart1 ;  y1 <= ylast1 ;  y1++){
    y2 = y1 + self->m_dx ;
    if (self->m_voiSoftBoundary){
      if (((y1 >= self->m_voiExtents[2]) && (y1 <= self->m_voiExtents[3])) || ((y2 >= self->m_voiExtents[2]) && (y2 <= self->m_voiExtents[3])))
        ylut[y1] = 1 ;
      else
        ylut[y1] = 0 ;
    }
    else{
      if (((y1 >= self->m_voiExtents[2]) && (y1 <= self->m_voiExtents[3])) && ((y2 >= self->m_voiExtents[2]) && (y2 <= self->m_voiExtents[3])))
        ylut[y1] = 1 ;
      else
        ylut[y1] = 0 ;
    }
  }

  // lut for valid x values
  int *xlutArray = new int[xlast1-xstart1+1] ;
  int *xlut = xlutArray - xstart1 ;
  for (x1 = xstart1 ;  x1 <= xlast1 ;  x1++){
    x2 = x1 + self->m_dx ;
    if (self->m_voiSoftBoundary){
      if (((x1 >= self->m_voiExtents[0]) && (x1 <= self->m_voiExtents[1])) || ((x2 >= self->m_voiExtents[0]) && (x2 <= self->m_voiExtents[1])))
        xlut[x1] = 1 ;
      else
        xlut[x1] = 0 ;
    }
    else{
      if (((x1 >= self->m_voiExtents[0]) && (x1 <= self->m_voiExtents[1])) && ((x2 >= self->m_voiExtents[0]) && (x2 <= self->m_voiExtents[1])))
        xlut[x1] = 1 ;
      else
        xlut[x1] = 0 ;
    }
  }


  // calculate range for the second voxel in the sample
  int zstart2, zlast2, ystart2, ylast2, xstart2, xlast2 ;
  zstart2 = zstart1 + self->m_dz ;
  zlast2 = zlast1 + self->m_dz ;
  ystart2 = ystart1 + self->m_dy ;
  ylast2 = ylast1 + self->m_dy ;
  xstart2 = xstart1 + self->m_dx ;
  xlast2 = xlast1 + self->m_dx ;

  // error checking
  assert((zstart1 >= inExt[4]) && (zstart1 <= inExt[5])) ;
  assert((zstart2 >= inExt[4]) && (zstart2 <= inExt[5])) ;
  assert((zlast1 >= inExt[4]) && (zlast1 <= inExt[5])) ;
  assert((zlast2 >= inExt[4]) && (zlast2 <= inExt[5])) ;
  assert((ystart1 >= inExt[2]) && (ystart1 <= inExt[3])) ;
  assert((ystart2 >= inExt[2]) && (ystart2 <= inExt[3])) ;
  assert((ylast1 >= inExt[2]) && (ylast1 <= inExt[3])) ;
  assert((ylast2 >= inExt[2]) && (ylast2 <= inExt[3])) ;
  assert((xstart1 >= inExt[0]) && (xstart1 <= inExt[1])) ;
  assert((xstart2 >= inExt[0]) && (xstart2 <= inExt[1])) ;
  assert((xlast1 >= inExt[0]) && (xlast1 <= inExt[1])) ;
  assert((xlast2 >= inExt[0]) && (xlast2 <= inExt[1])) ;
 




  //--------------------------------------------------------------------------
  // Calculate unnormalised coocurrence matrix
  // nb cmat[g1][g2] = cmat[n*g1+g2] where g1 and g2 are grey levels
  //--------------------------------------------------------------------------
  int n = self->m_dimsOut[0] ;
  int *cmat = new int[n*n] ;
  self->m_totalSamples = 0 ;
  for (int i = 0 ;  i < n*n ;  i++)
    cmat[i] = 0 ;

  if ((zlast1 >= zstart1) && (ylast1 >= ystart1) && (xlast1 >= xstart1)){ // no point in pretending to do this if any loops are degenerate
    for (z1 = zstart1, z2 = zstart2 ;  z1 <= zlast1 ;  z1++, z2++){
      if (zlut[z1] == 1){
        for (y1 = ystart1, y2 = ystart2 ;  y1 <= ylast1 ;  y1++, y2++){
          if (ylut[y1] == 1){
            inPtr1 = (IT*)inData->GetScalarPointer(xstart1,y1,z1) ;
            inPtr2 = (IT*)inData->GetScalarPointer(xstart2, y2, z2) ;

            for (x1 = xstart1, x2 = xstart2 ;  x1 <= xlast1 ;  x1++, x2++){
              if (xlut[x1] == 1){
                for (compId = 0 ;  compId < ncompsIn ;  compId++, inPtr1++, inPtr2++){
                  if (compId == self->GetTargetComponent()){
                    // get the grey level pair (g1,g2)
                    int g1 = *inPtr1 ;
                    int g2 = *inPtr2 ;
                    if (g1 >= n)
                      assert(false) ;
                    if (g2 >= n)
                      assert(false) ;
                    assert(g1 < n) ;
                    assert(g2 < n) ;

                    // compare grey level pair with any sub-range before adding to matrix
                    bool acceptPair = true ;
                    if (self->m_subsetRangeFlag){
                      acceptPair = (g1 >= self->m_subsetRange[0]) && (g1 <= self->m_subsetRange[1])
                        || (g2 >= self->m_subsetRange[0]) && (g2 <= self->m_subsetRange[1]) ;
                    }

                    if (acceptPair){
                      // increment cmat[g1][g2] and cmat[g2][g1]
                      cmat[n*g1 + g2]++ ;
                      cmat[n*g2 + g1]++ ;
                      self->m_totalSamples += 2 ;
                    }
                  } // if compId = target
                } // for compId
              } // if xlut
            } // for x
          } // if ylut
        } // for y
      } // if zlut
    } // for z
  } // if last >= start


  //--------------------------------------------------------------------------
  // Calculate output image from matrix
  // Image coords (x,y) correspond to grey level pair (g2,g1)
  // NB The y axis of a matrix points down, but an image normally points up,
  // so the matrix will display upside down unless you deliberately turn the image over.
  //--------------------------------------------------------------------------
  for (y1 = 0 ;  y1 < n ;  y1++){
    for (x1 = 0 ;  x1 < n ;  x1++){
      outPtr = (OT*)outData->GetScalarPointer(x1,y1,0) ;
      *outPtr = cmat[n*y1 + x1] ;
    }
  }


  //--------------------------------------------------------------------------
  // Calculate normalised cooccurrence matrix P(i,j) for calculation purposes
  // Set all elements to zero if matrix total is zero
  //--------------------------------------------------------------------------
  double *P = new double[n*n] ;
  if (self->m_totalSamples > 0){
    for (int i = 0 ;  i < n*n ;  i++)
      P[i] = (double)cmat[i] / (double)self->m_totalSamples ;
  }
  else{
    for (int i = 0 ;  i < n*n ;  i++)
      P[i] = 0.0 ;
  }



  //--------------------------------------------------------------------------
  // Do any requested calculations
  // To add a new calculation, just write it here and add an id to the enum
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // Sum of matrix elements (Should always be 1 if the matrix is valid)
  //--------------------------------------------------------------------------
  if (self->GetCalculationRequest(CooSumMatrix)){
    double sum = 0.0 ;
    for (int i = 0 ;  i < n ;  i++){
      for (int j = 0 ;  j < n ;  j++){
        sum += P[n*i+j] ;
      }
    }
    self->m_calculationResult[CooSumMatrix] = sum ;
  }

  //--------------------------------------------------------------------------
  // Sum Squares Variance
  // NB For a uniform matrix (ie a random image) this should be (n+1)(n-1)/6
  //--------------------------------------------------------------------------
  if (self->GetCalculationRequest(CooSumSquaresVar)){
    double sumVar = 0.0 ;
    for (int i = 0 ;  i < n ;  i++){
      for (int j = 0 ;  j < n ;  j++){
        sumVar += (double)((i-j)*(i-j))*P[n*i+j] ;
      }
    }
    self->m_calculationResult[CooSumSquaresVar] = sumVar ;
  }




  //--------------------------------------------------------------------------
  // Correlation
  //--------------------------------------------------------------------------
  if (self->GetCalculationRequest(CooCorrelation)){
    if (self->GetTotalSamples() > 0){
      double mui = 0.0 ;
      double muj = 0.0 ;
      for (int i = 0 ;  i < n ;  i++){
        for (int j = 0 ;  j < n ;  j++){
          mui += (double)i * P[n*i+j] ;
          muj += (double)j * P[n*i+j] ;
        }
      }

      double vari = 0.0 ;
      double varj = 0.0 ;
      for (int i = 0 ;  i < n ;  i++){
        for (int j = 0 ;  j < n ;  j++){
          double devi = (double)i - mui ;
          double devj = (double)j - muj ;
          vari += devi*devi * P[n*i+j] ;
          varj += devj*devj * P[n*i+j] ;
        }
      }

      double correl = 0.0 ;
      for (int i = 0 ;  i < n ;  i++){
        for (int j = 0 ;  j < n ;  j++){
          double devi = (double)i - mui ;
          double devj = (double)j - muj ;
          correl += devi * devj * P[n*i+j] ;
        }
      }
      if (vari*varj > 0.0)
        correl /= sqrt(vari*varj) ;
      else
        correl = 0.0 ;

      self->m_calculationResult[CooCorrelation] = correl ;
    }
    else{
      // set result to zero if matrix is empty
      self->m_calculationResult[CooCorrelation] = 0.0 ;
    }
  }


  //--------------------------------------------------------------------------
  // Trace
  //--------------------------------------------------------------------------
  double trace = 0.0 ;
  for (int i = 0 ;  i < n ;  i++)
    trace += P[n*i+i] ;

  self->m_calculationResult[CooTrace] = trace ;





  //--------------------------------------------------------------------------
  // delete heap memory
  //--------------------------------------------------------------------------
  delete [] cmat ;
  delete [] P ;

  delete [] zlutArray ;
  delete [] ylutArray ;
  delete [] xlutArray ;

}






//----------------------------------------------------------------------------
// This intermediate function calls lhpCooccurrenceMatrixFilterExecute() with arguments of
// the correct input and output type
template <class T>
void ImageFilterExecute1(
                         lhpCooccurrenceMatrixFilter *self,
                         vtkImageData *inData,
                         T *inPtr,
                         vtkImageData *outData,
                         int outExt[6], 
                         int id)
{
  void *outPtr = outData->GetScalarPointerForExtent(outExt) ;

  switch (outData->GetScalarType())
  {
    // The T* argument is a null pointer of type inData->GetScalarType()
    // The VTK_TT* argument is set to a null pointer of type outData->GetScalarType()
    vtkTemplateMacro7(ImageFilterExecute, self, inData, inPtr, outData,
      static_cast<VTK_TT *>(outPtr), outExt, id) ;
  default:
    vtkGenericWarningMacro("Execute: Unknown input ScalarType");
    return;
  }
}






//----------------------------------------------------------------------------
// This method is passed a input and output data, and executes the filter
// algorithm to fill the output from the input.
// The filter is executed via a pair of switches which determine the
// input and output types.
void lhpCooccurrenceMatrixFilter::ThreadedExecute(
                                  vtkImageData *inData, 
                                  vtkImageData *outData,
                                  int outExt[6], 
                                  int id
                                  )
{
  void *inPtr = inData->GetScalarPointerForExtent(outExt) ;

  switch (inData->GetScalarType())
  {
    // The VTK_TT argument is set to a null pointer of type inData->GetScalarType()
    vtkTemplateMacro6(ImageFilterExecute1, this, inData,
      static_cast<VTK_TT*>(inPtr), outData, outExt, id);
  default:
    vtkErrorMacro(<< "Execute: Unknown input ScalarType");
    return;
  }
}



//----------------------------------------------------------------------------
// PrintSelf method
void lhpCooccurrenceMatrixFilter::PrintSelf(ostream& os, vtkIndent indent)
//----------------------------------------------------------------------------
{

  os << "target component: " << m_target_component << std::endl ;
  
  if (m_outDimsSet)
    os << "output dims: " << m_dimsOut[0] << " " << m_dimsOut[1] << " " << m_dimsOut[2] << std::endl ;
  else
    os << "output dims: not set" << std::endl ;

  os << "displacement vector: " << m_dx << " " << m_dy << " " << m_dz << std::endl ;

  if (m_subsetRangeFlag)
    os << "grey level subset: " << m_subsetRange[0] << " " << m_subsetRange[1] << std::endl ;
  else
    os << "grey level subset: not set" << std::endl ;

  os << "requested statistics" << std::endl ;
  for (int i = 0 ;  i < 20 ;  i++){
    if (m_calculationFlag[i])
      os << "\t" << i << "\t" << m_calculationResult[i] << std::endl ;
  }

  if (m_voiDimsSet)
    os << "voi dims: " << m_voiDimensions[0] << " " << m_voiDimensions[1] << " " << m_voiDimensions[2] << std::endl ;
  else
    os << "voi dims: not set" << std::endl ;

  if (m_voiPosSet)
    os << "voi pos: " << m_voiPos[0] << " " << m_voiPos[1] << " " << m_voiPos[2] << std::endl ;
  else
    os << "voi pos: not set" << std::endl ;

  if ((m_voiDimsSet) && (m_voiPosSet))
    os << "voi extents: " << m_voiExtents[0] << " " << m_voiExtents[1] << " " << m_voiExtents[2] << " " << m_voiExtents[3] << " " << m_voiExtents[4] << " " << m_voiExtents[5] << std::endl ;

  os << std::endl ;


}
