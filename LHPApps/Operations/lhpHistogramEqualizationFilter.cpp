/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpHistogramEqualizationFilter.cpp,v $
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


#include "lhpHistogramEqualizationFilter.h"
#include "lhpHistogram1D.h"

#include "vtkObjectFactory.h"
#include "vtkImageAccumulate.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkDataSet.h"

#include <cstdlib>
#include <assert.h>

#ifndef M_PI
  #define _USE_MATH_DEFINES
#endif

#include <cmath>


//----------------------------------------------------------------------------
// mandatory vtk macro
vtkCxxRevisionMacro(lhpHistogramEqualizationFilter, "$Revision: 1.1 $");


//----------------------------------------------------------------------------
// standard New() method
vtkStandardNewMacro(lhpHistogramEqualizationFilter);


//----------------------------------------------------------------------------
// Constructor
lhpHistogramEqualizationFilter::lhpHistogramEqualizationFilter() : m_target_component(0), m_numberOfGreyLevels(256)
{
}



//----------------------------------------------------------------------------
// Destructor
lhpHistogramEqualizationFilter::~lhpHistogramEqualizationFilter() 
{
}



//----------------------------------------------------------------------------
// set target component
void lhpHistogramEqualizationFilter::SetTargetComponent(int c)
{
  if (c != m_target_component){
    m_target_component = c ;
    this->Modified() ;
  }
}

//----------------------------------------------------------------------------
// get target component
int lhpHistogramEqualizationFilter::GetTargetComponent() const
{
  return m_target_component ;
}



//----------------------------------------------------------------------------
// Set the number of grey levels
void lhpHistogramEqualizationFilter::SetNumberOfGreyLevels(int numberOfGreyLevels) 
//----------------------------------------------------------------------------
{
  if (numberOfGreyLevels != m_numberOfGreyLevels){
    m_numberOfGreyLevels = numberOfGreyLevels ;  

    this->Modified() ;
  }
}



//----------------------------------------------------------------------------
// Configure the output image
void lhpHistogramEqualizationFilter::ExecuteInformation(vtkImageData *inData, vtkImageData *outData)
//----------------------------------------------------------------------------
{
  // Mustn't multithread this filter
  this->SetNumberOfThreads(1) ;

  inData->GetDimensions(m_dims) ;
  inData->GetSpacing(m_spacing) ;
  inData->GetOrigin(m_origin) ;

  outData->SetDimensions(m_dims) ;
  outData->SetWholeExtent(0, m_dims[0]-1, 0, m_dims[1]-1, 0, m_dims[2]-1) ;
  outData->SetOrigin(m_origin) ;
  outData->SetSpacing(m_spacing) ;
  outData->SetNumberOfScalarComponents(1) ;
  outData->SetScalarTypeToUnsignedChar() ;

}




//----------------------------------------------------------------------------
// Set the input UpdateExtent.
// You need this function if the input and output images
// have different extents.
void lhpHistogramEqualizationFilter::ComputeInputUpdateExtent(int inExt[6], int *vtkNotUsed(outExt))
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
                        lhpHistogramEqualizationFilter *self,
                        vtkImageData *inData,
                        IT *inPtr,
                        vtkImageData *outData, 
                        OT *outPtr,
                        int outExt[6], 
                        int id
                        )
{
  int x, y, z, compId ;


  // Get no. of components
  int ncompsIn = inData->GetNumberOfScalarComponents() ;
  int ncompsOut = outData->GetNumberOfScalarComponents() ;

  // Get whole extent of the output image
  int *outWholeExt = outData->GetWholeExtent() ;

  // this sets the limits of x, y and z
  int zstart = outExt[4] ;
  int zlast = outExt[5] ;
  int ystart = outExt[2] ;
  int ylast = outExt[3] ;
  int xstart = outExt[0] ;
  int xlast = outExt[1] ;

  // Get scalar range of input
  double inputRange[2] ;
  inData->GetPointData()->GetScalars()->GetRange(inputRange, self->m_target_component) ;


 
  // get increments to neighbours and no. of input components
  int inIncX, inIncY, inIncZ, outIncX, outIncY, outIncZ  ;
  inData->GetIncrements(inIncX, inIncY, inIncZ) ;
  outData->GetIncrements(outIncX, outIncY, outIncZ) ;




  //--------------------------------------------------------------------------
  // Calculate image histogram
  //--------------------------------------------------------------------------

  // Construct the histogram.
  // Note how we stretch the range a bit so that the given min and max values are in 
  // the centres of the first and last bins.
  int numberOfBins = 1000 ;
  double binsize = (inputRange[1] - inputRange[0]) / (double)(numberOfBins-1) ;
  lhpHistogram1D hist(numberOfBins, inputRange[0]-binsize/2.0, inputRange[1]+binsize/2.0) ;
  
  for (z = zstart ;  z <= zlast ;  z++){
    for (y = ystart ;  y <= ylast ;  y++){
      inPtr = (IT*)inData->GetScalarPointer(xstart,y,z) ;

      for (x = xstart ;  x <= xlast ;  x++){
        for (compId = 0 ;  compId < ncompsIn ;  compId++, inPtr++){
          if (compId == self->GetTargetComponent()){						
            hist.AddValue(*inPtr) ;
          }
        }
      }
    }
  }



  //--------------------------------------------------------------------------
  // Equalize image
  //--------------------------------------------------------------------------
  hist.CalculateQuantilePoints(self->m_numberOfGreyLevels) ;

  for (z = zstart ;  z <= zlast ;  z++){
    for (y = ystart ;  y <= ylast ;  y++){
      inPtr = (IT*)inData->GetScalarPointer(xstart,y,z) ;
      outPtr = (OT*)outData->GetScalarPointer(xstart,y,z) ;

      for (x = xstart ;  x <= xlast ;  x++){
        for (compId = 0 ;  compId < ncompsIn ;  compId++, inPtr++){
          if (compId == self->GetTargetComponent()){
            *outPtr++ = hist.WhichQuantile(*inPtr) ;
          }
        }
      }
    }
  }


/*
  //--------------------------------------------------------------------------
  // Calculate final image histogram and print
  //--------------------------------------------------------------------------
  lhpHistogram1D hist2(self->m_numberOfGreyLevels, -0.5, self->m_numberOfGreyLevels-0.5) ;

  for (z = zstart ;  z <= zlast ;  z++){
    for (y = ystart ;  y <= ylast ;  y++){
      outPtr = (OT*)outData->GetScalarPointer(xstart,y,z) ;

      for (x = xstart ;  x <= xlast ;  x++, outPtr++)			
        hist2.AddValue(*outPtr) ;
    }
  }

  hist2.PrintSelf(std::cout, 0) ;
*/

}






//----------------------------------------------------------------------------
// This intermediate function calls lhpHistogramEqualizationFilterExecute() with arguments of
// the correct input and output type
template <class T>
void ImageFilterExecute1(
                         lhpHistogramEqualizationFilter *self,
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
void lhpHistogramEqualizationFilter::ThreadedExecute(
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
void lhpHistogramEqualizationFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << "\nComponent: " << this->GetTargetComponent() << std::endl ;
}
