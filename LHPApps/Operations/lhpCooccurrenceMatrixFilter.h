#ifndef __lhpCooccurrenceMatrixFilter_H__
#define __lhpCooccurrenceMatrixFilter_H__

#include <vtkImageToImageFilter.h>
#include <ostream>


//------------------------------------------------------------------------------
// Image to image filter whose output is Grey Level Cooccurrence Matrix (GLCM)
// or Spatial Grey Level Dependence Matrix (SGLDM).
// Output image has one int component, containing the matrix.  (Display with y axis pointing down if you want it to look correct)
// The input image should be histogram-equalized with a small number of grey levels first.
//
// Methods are provided to request SGLDM calculations.
// The calculations must be requested before running the filter.
//
// The input image should be of integer type.
// The dimensions of the matrix and the output image must be set equal to the number of input grey levels
// It is difficult or impossible in vtk to set the output dims to depend on the input attributes,
// therefore the user must make sure that the dimensions are set correctly before execution.
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// enum of id's of texture statistics which can be requested
//------------------------------------------------------------------------------
enum CooCalculationId {
  CooAll = 0,
  CooSumMatrix,
  CooSumSquaresVar,
  CooCorrelation,
  CooTrace
};



//------------------------------------------------------------------------------
// Cooccurrence matrix class
//------------------------------------------------------------------------------
class lhpCooccurrenceMatrixFilter : public vtkImageToImageFilter
{
public:
  static lhpCooccurrenceMatrixFilter *New();
  vtkTypeRevisionMacro(lhpCooccurrenceMatrixFilter,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Set/get the output dimensions
  // The user should set this equal to the number of input grey levels.
  void SetOutputDimensions(int dims) {m_dimsOut[0] = dims ;  m_dimsOut[1] = dims ;  m_dimsOut[2] = 1 ;  m_outDimsSet = true ;  this->Modified() ;}
  int GetOutputDimensions(int dims) const {return m_dimsOut[0] ;}

  // set/get which scalar component to use (0, 1 or 2)
  void SetTargetComponent(int) ;
  int GetTargetComponent() const ;

  // set displacement vector for this matrix
  void SetDisplacement(const int dx[3]) {m_dx = dx[0] ;  m_dy = dx[1] ;  m_dz = dx[2] ;  this->Modified() ;}
  void SetDisplacement(int dx, int dy, int dz) {m_dx = dx ;  m_dy = dy ;  m_dz = dz ;  this->Modified() ;}

  // Set calculations on or off and get results
  // The calculations must be requested before executing the filter
  void SetCalculationRequest(enum CooCalculationId id, bool requestCalc) ;
  bool GetCalculationRequest(enum CooCalculationId id) const {return m_calculationFlag[id] ;}
  double GetCalculationResult(enum CooCalculationId id) const {return m_calculationResult[id] ;}

  // Set a subset of the grey level range
  // Only pairs where at least one of the voxels is in this range will be counted
  void SetGreyLevelSubsetRange(int g0, int g1) {m_subsetRange[0] = g0 ;  m_subsetRange[1] = g1 ;  m_subsetRangeFlag = true ;  this->Modified() ;}

  // Remove any subset range of grey levels which might have been set
  void SetGreyLevelRangeToDefault() {m_subsetRangeFlag = false ;  this->Modified() ;}

  // Set volume of interest to soft boundary (default setting).
  // Only one voxel in each displacement pair need be inside the voi.
  void SetVoiToSoftBoundary() {m_voiSoftBoundary = true ;  this->Modified() ;}

  // Set volume of interest to hard boundary (the default is soft).
  // Both voxels in each displacement pair must be inside the voi.
  void SetVoiToHardBoundary() {m_voiSoftBoundary = false ;  this->Modified() ;}

  // Set the volume of interest dimensions
  void SetVoiDimensions(int dimsx, int dimsy, int dimsz) ;

  // Set the volume of interest dimensions
  void SetVoiDimensions(const int dims[3]) ;

  // Set the position (start corner) of the volume of interest
  void SetVoiPosition(int x, int y, int z) ;

  // Set the position (start corner) of the volume of interest
  void SetVoiPosition(const int pos[3]) ;

  // How many voxel pairs were sampled to produce matrix
  // If this is zero, the matrix is empty and invalid.
  int GetTotalSamples() {return m_totalSamples ;}

protected:
  int m_target_component ;	// image component
  bool m_outDimsSet ;       // flag indicating if output dims have been set
  int m_dimsOut[3] ;        // dims of output image
  int m_dx, m_dy, m_dz ;    // displacement vector for this cooccurrence mat

  // subset range of grey levels
  bool m_subsetRangeFlag ;
  int m_subsetRange[2] ;

  // results of calculations
  int m_totalSamples ;
  bool m_calculationFlag[20] ;
  double m_calculationResult[20] ;

  // volume of interest
  bool m_voiDimsSet ;
  bool m_voiPosSet ;
  bool m_voiSoftBoundary ;
  int m_voiDimensions[3] ;
  int m_voiPos[3] ;
  int m_voiExtents[6] ;


  void SetVoiExtents() ;


  lhpCooccurrenceMatrixFilter();
  ~lhpCooccurrenceMatrixFilter();

  // This gives the subclass a chance to configure the output
  // see VTK user's guide p 246
  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};
  void ComputeInputUpdateExtent(int inExt[6], int outExt[6]);

  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
    int extent[6], int id);


  // Declare this function as a friend so that self->func() can access private members
  // Otherwise all methods have to be public
  template <class IT, class OT>
  friend void ImageFilterExecute(
    lhpCooccurrenceMatrixFilter *self, 
    vtkImageData *inData,
    IT *inPtr,
    vtkImageData *outData, 
    OT *outPtr,
    int outExt[6], 
    int id
    ) ;
} ;



#endif