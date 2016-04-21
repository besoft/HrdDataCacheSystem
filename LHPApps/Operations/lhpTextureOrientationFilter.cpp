/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpTextureOrientationFilter.cpp,v $
Language:  C++
Date:      $Date: 2009-07-07 14:46:31 $
Version:   $Revision: 1.1.1.1.2.3 $
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


#include "lhpTextureOrientationFilter.h"
#include "lhpTextureOrientationCalculator.h"
#include "lhpTextureOrientationUseful.h"

#include "vtkObjectFactory.h"
#include "vtkFloatArray.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkCommand.h"

#include <assert.h>
#include <ostream>


//----------------------------------------------------------------------------
using namespace lhpTextureOrientation ;
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// mandatory vtk macro
vtkCxxRevisionMacro(lhpTextureOrientationFilter, "$Revision: 1.1.1.1.2.3 $");


//----------------------------------------------------------------------------
// standard New() method
vtkStandardNewMacro(lhpTextureOrientationFilter);



//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
lhpTextureOrientationFilter::lhpTextureOrientationFilter() : m_target_component(0), 
m_outputFormat(VectorFormat), m_ProbeSizeVoxelsSet(false), m_ProbeSizeSceneUnitsSet(false), 
m_SampleSpacingVoxelsSet(false), m_SampleSpacingSceneUnitsSet(false), m_progress(NULL)
{
}



//----------------------------------------------------------------------------
// Destructor
lhpTextureOrientationFilter::~lhpTextureOrientationFilter() 
//----------------------------------------------------------------------------
{
}



//----------------------------------------------------------------------------
// Initialize method to refresh filter and output data
void lhpTextureOrientationFilter::Initialize()
//----------------------------------------------------------------------------
{
  m_output->Initialize() ;
}




//----------------------------------------------------------------------------
// set target component
void lhpTextureOrientationFilter::SetTargetComponent(int c)
//----------------------------------------------------------------------------
{
  if (c != m_target_component){
    m_target_component = c ;
    this->Modified() ;
  }
}



//----------------------------------------------------------------------------
// Set the dimensions of the probe
void lhpTextureOrientationFilter::SetProbeSizeVoxels(int dimsx, int dimsy, int dimsz)
//----------------------------------------------------------------------------
{
  m_ProbeSizeVoxels[0] = dimsx ;
  m_ProbeSizeVoxels[1] = dimsy ;
  m_ProbeSizeVoxels[2] = dimsz ;

  m_ProbeSizeVoxelsSet = true ;
  m_ProbeSizeSceneUnitsSet = false ;
  this->Modified() ;
}


//----------------------------------------------------------------------------
// Set the dimensions of the probe
void lhpTextureOrientationFilter::SetProbeSizeVoxels(const int dims[3])
//----------------------------------------------------------------------------
{
  SetProbeSizeVoxels(dims[0], dims[1], dims[2]) ;
}


//----------------------------------------------------------------------------
// Set the size of the probe in world coords
void lhpTextureOrientationFilter::SetProbeSizeSceneUnits(double size)
//----------------------------------------------------------------------------
{
  m_ProbeSizeSceneUnits = size ;

  m_ProbeSizeSceneUnitsSet = true ;
  m_ProbeSizeVoxelsSet = false ;
  this->Modified() ;
}


//----------------------------------------------------------------------------
// Set the step increment of the probe
void lhpTextureOrientationFilter::SetSampleSpacingVoxels(int stepx, int stepy, int stepz)
//----------------------------------------------------------------------------
{
  m_SampleSpacingVoxels[0] = stepx ;
  m_SampleSpacingVoxels[1] = stepy ;
  m_SampleSpacingVoxels[2] = stepz ;

  m_SampleSpacingVoxelsSet = true ;
  m_SampleSpacingSceneUnitsSet = false ;
  this->Modified() ;

}


//----------------------------------------------------------------------------
// Set the step increment of the probe
void lhpTextureOrientationFilter::SetSampleSpacingVoxels(const int step[3])
//----------------------------------------------------------------------------
{
  SetSampleSpacingVoxels(step[0], step[1], step[2]) ;
}


//----------------------------------------------------------------------------
// Set the step increment of the probe in world coords
void lhpTextureOrientationFilter::SetSampleSpacingSceneUnits(double step)
//----------------------------------------------------------------------------
{
  m_SampleSpacingSceneUnits = step ;

  m_SampleSpacingSceneUnitsSet = true ;
  m_SampleSpacingVoxelsSet = false ;
  this->Modified() ;
}




//----------------------------------------------------------------------------
// The Execute function calls the appropriate execute template for the input scalar type
void lhpTextureOrientationFilter::Execute()
//----------------------------------------------------------------------------
{
  m_input = this->GetInput() ;
  m_output = this->GetOutput() ;

  // Refresh the filter and output data
  Initialize() ;

  // Run the filter
  FilterExecute() ;
}











//----------------------------------------------------------------------------
// Execute method, normally templated but no need in this case
void lhpTextureOrientationFilter::FilterExecute()
//----------------------------------------------------------------------------
{
  int x, y, z ;
  int inputExt[6], inputWholeExt[6], dims[3] ;
  double origin[3], spacing[3] ;


  // Get no. of components
  int ncompsIn = m_input->GetNumberOfScalarComponents() ;

  // Get properties of the input image
  m_input->GetWholeExtent(inputWholeExt) ;
  m_input->GetExtent(inputExt) ;
  m_input->GetOrigin(origin) ;
  m_input->GetSpacing(spacing) ;
  m_input->GetDimensions(dims) ;



  //--------------------------------------------------------------------------
  // Define the dimensions and increment step of the moving probe
  //--------------------------------------------------------------------------
  if (!m_ProbeSizeVoxelsSet){
    if (m_ProbeSizeSceneUnitsSet){
      // calculate dimensions from size in world coords
      int d0 = lhpTextureOrientationUseful::Round(this->m_ProbeSizeSceneUnits / spacing[0]) ;
      int d1 = lhpTextureOrientationUseful::Round(this->m_ProbeSizeSceneUnits / spacing[1]) ;
      int d2 = lhpTextureOrientationUseful::Round(this->m_ProbeSizeSceneUnits / spacing[2]) ;
      SetProbeSizeVoxels(d0, d1, d2) ;
    }
    else{
      // default is whole image
      SetProbeSizeVoxels(dims) ;
    }
  }

  if (!m_SampleSpacingVoxelsSet){
    if (m_SampleSpacingSceneUnitsSet){
      // calculate step from step size in world coords
      int s0 = lhpTextureOrientationUseful::Round(this->m_SampleSpacingSceneUnits / spacing[0]) ;
      int s1 = lhpTextureOrientationUseful::Round(this->m_SampleSpacingSceneUnits / spacing[1]) ;
      int s2 = lhpTextureOrientationUseful::Round(this->m_SampleSpacingSceneUnits / spacing[2]) ;
      SetSampleSpacingVoxels(s0, s1, s2) ;
    }
    else{
      // default is zero step (no movement at all)
      SetSampleSpacingVoxels(0,0,0) ;
    }
  }

  //--------------------------------------------------------------------------
  // Calculate how many whole probes you can get into the image
  // and calculate the start and finish positions of the probes
  //--------------------------------------------------------------------------
  if (m_ProbeSizeVoxels[0] == 0)
    m_numx = 1 ;
  else
    m_numx = (dims[0] - m_ProbeSizeVoxels[0]) / m_SampleSpacingVoxels[0] + 1 ;
  int xstart = inputWholeExt[0] ;
  int xlast = xstart + (m_numx-1)*m_SampleSpacingVoxels[0] ;
  assert(xlast + m_ProbeSizeVoxels[0] - 1 <= inputWholeExt[1]) ;

  if (m_ProbeSizeVoxels[1] == 0)
    m_numy = 1 ;
  else
    m_numy = (dims[1] - m_ProbeSizeVoxels[1]) / m_SampleSpacingVoxels[1] + 1 ;
  int ystart = inputWholeExt[2] ;
  int ylast = ystart + (m_numy-1)*m_SampleSpacingVoxels[1] ;
  assert(ylast + m_ProbeSizeVoxels[1] - 1 <= inputWholeExt[3]) ;

  if (m_ProbeSizeVoxels[2] == 0)
    m_numz = 1 ;
  else
    m_numz = (dims[2] - m_ProbeSizeVoxels[2]) / m_SampleSpacingVoxels[2] + 1 ;
  int zstart = inputWholeExt[4] ;
  int zlast = zstart + (m_numz-1)*m_SampleSpacingVoxels[2] ;
  assert(zlast + m_ProbeSizeVoxels[2] - 1 <= inputWholeExt[5]) ;





  //--------------------------------------------------------------------------
  // Create points and attribute arrays (vector or tensor as requested by user)
  //--------------------------------------------------------------------------
  vtkPoints *points = vtkPoints::New() ;

  vtkFloatArray *scalars ;
  vtkFloatArray *vectors ;
  vtkFloatArray *tensors ;

  scalars = vtkFloatArray::New() ;
  scalars->SetNumberOfComponents(1) ;
  scalars->SetName("scalars") ;

  if (m_outputFormat == VectorFormat){
    vectors = vtkFloatArray::New() ;
    vectors->SetNumberOfComponents(3) ;
    vectors->SetName("vectors") ;
  }
  else if (m_outputFormat == TensorFormat){
    tensors = vtkFloatArray::New() ;
    tensors->SetNumberOfComponents(9) ;
    tensors->SetName("tensors") ;
  }
  else{
    // unknown format
    assert(false) ;
  }



  //--------------------------------------------------------------------------
  // Set up the orientation calculator
  //--------------------------------------------------------------------------
  lhpTextureOrientationCalculator *texCalc = new lhpTextureOrientationCalculator() ;
  texCalc->SetInputImage(m_input) ;
  texCalc->SetVoiDimensions(m_ProbeSizeVoxels) ;
  texCalc->SetTextureStatistic(CooCorrelation) ;
  texCalc->SetResolutionOfSamples(7, 12, 15) ;


  //--------------------------------------------------------------------------
  // set up progress reporting
  //--------------------------------------------------------------------------
  int numTotal = m_numx * m_numy * m_numz ;
  double progressInc = 100.0 / (double)numTotal ;
  m_progress = 0.0 ;
  InvokeEvent(ProgressEventId, &m_progress) ;




  //--------------------------------------------------------------------------
  // Process the image
  //--------------------------------------------------------------------------
  for (z = zstart;  z <= zlast ;  z += m_SampleSpacingVoxels[2]){
    for (y = ystart;  y <= ylast ;  y += m_SampleSpacingVoxels[1]){
      for (x = xstart ;  x <= xlast ;  x += m_SampleSpacingVoxels[0]){
        //------------------------------------------------------
        // Run the orientation calculator
        //------------------------------------------------------
        texCalc->SetVoiPosition(x,y,z) ;
        texCalc->Execute() ;

        //------------------------------------------------------
        // Create new polydata point in the centre of the window and transfer data to it
        //------------------------------------------------------
        double xcoords[3] ;
        xcoords[0] = origin[0] + ((double)x + (double)m_ProbeSizeVoxels[0]/2.0) * spacing[0] ;
        xcoords[1] = origin[1] + ((double)y + (double)m_ProbeSizeVoxels[1]/2.0) * spacing[1] ;
        xcoords[2] = origin[2] + ((double)z + (double)m_ProbeSizeVoxels[2]/2.0) * spacing[2] ;
        points->InsertNextPoint(xcoords) ;

        double evals[3] ;
        texCalc->GetEigenvalues(evals) ;
        double scal = evals[0] / (evals[0] + evals[1] + evals[2]) ;
        scal = (3.0*scal - 1.0) / 2.0 ;
        scalars->InsertNextTuple(&scal) ;

        if (m_outputFormat == VectorFormat){
          double Vec[3] ;
          texCalc->GetEigenvectorScaled(Vec, 0) ;
          vectors->InsertNextTuple(Vec) ;
        }
        else if (m_outputFormat == TensorFormat){
          double Vmat[9] ;
          texCalc->GetEigenvectorMatrixScaled(Vmat) ; // the array is column-major, and the principal vectors are the columns.
          tensors->InsertNextTuple(Vmat) ;
        }
      } // for x


      //------------------------------------------------------
      // progress tracking
      //------------------------------------------------------
      m_progress += (double)m_numx * progressInc ;
      InvokeEvent(ProgressEventId, &m_progress) ;

    } // for y
  } // for z



  // attach points and tensors to output polydata
  m_output->SetPoints(points) ;
  points->Delete() ;

  m_output->GetPointData()->SetScalars(scalars) ;
  scalars->Delete() ;

  if (m_outputFormat == VectorFormat){
    m_output->GetPointData()->SetVectors(vectors) ;
    vectors->Delete() ;
  }
  else if (m_outputFormat == TensorFormat){
    m_output->GetPointData()->SetTensors(tensors) ;
    tensors->Delete() ;
  }
  else{
    // unknown format
    assert(false) ;
  }


  delete texCalc ;
}





//----------------------------------------------------------------------------
// PrintSelf method (CSV format)
void lhpTextureOrientationFilter::PrintSelf(ostream& os, vtkIndent indent)
//----------------------------------------------------------------------------
{
  // target component
  os << "Target component, " << this->GetTargetComponent() << std::endl ;

  // size and step or texture sample
  if (m_ProbeSizeVoxelsSet)
    os << "dimensions per sample, " << m_ProbeSizeVoxels[0] << ", " << m_ProbeSizeVoxels[1] << ", " << m_ProbeSizeVoxels[2] << std::endl ;
  else
    os << "dimensions per sample, Not Set" << std::endl ;

  if (m_SampleSpacingVoxelsSet)
    os << "step (dx,dy,dz) per sample, " << m_SampleSpacingVoxels[0] << ", " << m_SampleSpacingVoxels[1] << ", " << m_SampleSpacingVoxels[2] << std::endl ;
  else
    os << "step (dx,dy,dz) per sample, Not Set" << std::endl ;

  if (m_ProbeSizeSceneUnitsSet)
    os << "size per sample (world units), " << m_ProbeSizeSceneUnits << std::endl ;
  else
    os << "size per sample (world units), Not Set " << std::endl ;

  if (m_SampleSpacingSceneUnitsSet)
    os << "step per sample (world units), " << m_SampleSpacingSceneUnits << std::endl ;
  else
    os << "step per sample (world units), Not Set " << std::endl ;

  if ((m_ProbeSizeVoxelsSet) && (m_SampleSpacingVoxelsSet))
    os << "no. of texture samples, " << m_numx << ", " << m_numy << ", " << m_numz << std::endl ;

  os << "output format, " << m_outputFormat << std::endl ;
  os << std::endl ;
}




//----------------------------------------------------------------------------
// Print results (CSV format)
void lhpTextureOrientationFilter::PrintResults(ostream& os)
//----------------------------------------------------------------------------
{
  m_output->Update() ;

  PrintSelf(os, 0) ;

  vtkPoints *points = m_output->GetPoints() ;
  vtkPointData *PD = m_output->GetPointData() ;



  switch(m_outputFormat){
    case VectorFormat:
      {
        vtkDataArray *DA = PD->GetArray("vectors") ;

        os << "vector output: 1st principal component\n" << std::endl ;

        int ituple = 0 ;
        for (int iz = 0 ;  iz < this->m_numz ;  iz++){
          for (int iy = 0 ;  iy < this->m_numy ;  iy++){
            for (int ix = 0 ;  ix < this->m_numx ;  ix++, ituple++){
              double *x = points->GetPoint(ituple) ;
              double *vec = DA->GetTuple3(ituple) ;
              os << "coordinates, " << x[0] << ", " << x[1] << ", " << x[2] << ",\t" ;
              os << "1st component, " << vec[0] << ", " << vec[1] << ", " << vec[2] << std::endl ;
            }
            os << std::endl ;
          }
          os << std::endl ;
        }
      }
      break ;
    case TensorFormat:
      {
        vtkDataArray *DA = PD->GetArray("tensors") ;

        os << "tensor output: principal components\n" << std::endl ;

        int ituple = 0 ;
        for (int iz = 0 ;  iz < this->m_numz ;  iz++){
          for (int iy = 0 ;  iy < this->m_numy ;  iy++){
            for (int ix = 0 ;  ix < this->m_numx ;  ix++, ituple++){
              double *x = points->GetPoint(ituple) ;
              double *tens = DA->GetTuple9(ituple) ; // the array is column-major, and the principal vectors are the columns.
              os << "coordinates, " << x[0] << ", " << x[1] << ", " << x[2] << ",\t" ;
              os << "1st component, " << tens[0] << ", " << tens[1] << ", " << tens[2] << ",\t" ;
              os << "2nd component, " << tens[3] << ", " << tens[4] << ", " << tens[5] << ",\t" ;
              os << "3rd component, " << tens[6] << ", " << tens[7] << ", " << tens[8] << std::endl ;
            }
            os << std::endl ;
          }
          os << std::endl ;
        }
      }
      break ;
    default:
      // unknown format
      assert(false) ;
  }
}