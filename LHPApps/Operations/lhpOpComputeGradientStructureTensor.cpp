/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpComputeGradientStructureTensor.cpp,v $
Language:  C++
Date:      $Date: 2011-10-20 14:50:44 $
Version:   $Revision: 1.1.2.3 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpBuilderDecl.h"

#include "lhpOpComputeGradientStructureTensor.h"

#include "wx/busyinfo.h"
#include "mafGUI.h"
#include "mafDecl.h"

// New includes (clean previous)
#include <itkImage.h>
#include <lhpStructureTensorRecursiveGaussianImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkSymmetricEigenAnalysisImageFilter.h>
#include <lhpSymmetricEigenVectorAnalysisImageFilter.h>
#include <itkMatrix.h>
#include <itkVectorImage.h>
#include <itkVariableLengthVector.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include "mafString.h"
#include "wx/filename.h"
#include "vtkMAFSmartPointer.h"
#include "vtkDataSetReader.h"
#include "vtkStructuredPoints.h"
#include "itkVTKImageToImageFilter.h"
#include "itkImageToVTKImageFilter.h"
#include "vtkImageCast.h"
#include "wx/mac/classic/filedlg.h"
#include "mafVMEVolumeGray.h"


//----------------------------------------------------------------------------
lhpOpComputeGradientStructureTensor::lhpOpComputeGradientStructureTensor(wxString label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_OP;
  m_Canundo = false;
  m_InputPreserving = true;
  m_PrimaryEigenVectorOutputImageAbsPath = "";
}
//----------------------------------------------------------------------------
lhpOpComputeGradientStructureTensor::~lhpOpComputeGradientStructureTensor()
//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
bool lhpOpComputeGradientStructureTensor::Accept(mafNode *node)
//----------------------------------------------------------------------------
{ 
  return (node && node->IsA("mafVMEVolumeGray") && mafVME::SafeDownCast(node)->GetOutput()->GetVTKData()->IsA("vtkStructuredPoints"));
}
//----------------------------------------------------------------------------
mafOp* lhpOpComputeGradientStructureTensor::Copy()   
//----------------------------------------------------------------------------
{
  lhpOpComputeGradientStructureTensor *cp = new lhpOpComputeGradientStructureTensor(m_Label);
  return cp;
}
//----------------------------------------------------------------------------
// constants
//----------------------------------------------------------------------------
enum BONEMAT_ID
{
  ID_FIRST = MINID,
  ID_CHOOSE_EV_OUTPUT_FILE_NAME,
  ID_OK,

};

//----------------------------------------------------------------------------
void lhpOpComputeGradientStructureTensor::CreateGui()
//----------------------------------------------------------------------------
{
  m_Gui = new mafGUI(this);
  m_Gui->Label(""); 

  m_Gui->Label("Primary Eigenvectors Output file");
  m_Gui->Button(ID_CHOOSE_EV_OUTPUT_FILE_NAME, "choose output filename");
  m_Gui->Label("");

  m_Gui->Button(ID_OK, "Compute GST");
  m_Gui->Enable(ID_OK, false);

  m_Gui->Label("");

  m_Gui->OkCancel();
  m_Gui->Label("");
  m_Gui->Label("");
}

//----------------------------------------------------------------------------
void lhpOpComputeGradientStructureTensor::OpRun()   
//----------------------------------------------------------------------------
{
  CreateGui();
  ShowGui();
}

//----------------------------------------------------------------------------
void lhpOpComputeGradientStructureTensor::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
    case wxOK:
      {
        OpStop(OP_RUN_OK);
      }
      break;
    case wxCANCEL:
      OpStop(OP_RUN_CANCEL);
      break;
	
	case ID_CHOOSE_EV_OUTPUT_FILE_NAME:
		{
			wxFileDialog *SaveDialog = new wxFileDialog(
				NULL, _("Save File As _?"), wxEmptyString, wxEmptyString,
				_("Meta image file (*.mha)|*.mha"));

			SaveDialog->ShowModal();
			m_PrimaryEigenVectorOutputImageAbsPath = SaveDialog->GetPath();

			if (m_PrimaryEigenVectorOutputImageAbsPath != "")
			{
				m_Gui->Enable(ID_OK, true);
			}
		}
		break;
	  
    case ID_OK	:
      {
        ComputeGradientStructureTensor();
      }
      break;
    default:
      mafEventMacro(*e);
      break;
    }
  }
}
//----------------------------------------------------------------------------
void lhpOpComputeGradientStructureTensor::OpStop(int result)
//----------------------------------------------------------------------------
{
  HideGui();
  mafEventMacro(mafEvent(this,result));        
}

//----------------------------------------------------------------------------
const char* lhpOpComputeGradientStructureTensor::GetPrimaryEigenVectorOutputImageAbsPath()
//----------------------------------------------------------------------------
{
  return m_PrimaryEigenVectorOutputImageAbsPath.GetCStr();
}

//----------------------------------------------------------------------------
void lhpOpComputeGradientStructureTensor::SetPrimaryEigenVectorOutputImageAbsPath(const char* name)
//----------------------------------------------------------------------------
{
  m_PrimaryEigenVectorOutputImageAbsPath = name;  
}

void lhpOpComputeGradientStructureTensor::ComputeGradientStructureTensor()
{
	wxBusyInfo wait("Please wait...");

	mafVMEVolumeGray *vg = mafVMEVolumeGray::SafeDownCast(m_Input);

	vtkStructuredPoints *sp = vtkStructuredPoints::SafeDownCast(vg->GetOutput()->GetVTKData());
	assert(sp);

	vtkMAFSmartPointer<vtkImageCast> vtkImageToShort;
	vtkImageToShort->SetOutputScalarTypeToShort();
	vtkImageToShort->SetInput(sp);
	vtkImageToShort->Modified();
	vtkImageToShort->Update();

	typedef itk::Image<short,3> ImageType;
	typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;
	VTKImageToImageType::Pointer converter=VTKImageToImageType::New();

	converter->SetInput(vtkImageToShort->GetOutput());
	converter->Update();

	assert(converter->GetOutput() != NULL);

	assert(sp);
	assert(sp->GetNumberOfPoints() != 0);

	// Define the dimension of the images
	const unsigned int Dimension = 3;

	// Define the pixel type
	typedef short PixelType;

	// Declare the types of the images
	typedef itk::Image<PixelType, Dimension>  InputImageType;

	// Declare the reader
	typedef itk::ImageFileReader< InputImageType > ReaderType;

	// Create the reader and writer
	/*ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName( inputImageAbsPath );
*/

	// Declare the type for the
	typedef itk::StructureTensorRecursiveGaussianImageFilter <
		InputImageType >  StructureTensorFilterType;

	typedef StructureTensorFilterType::OutputImageType myTensorImageType;

	// Create a  Filter
	StructureTensorFilterType::Pointer filter = StructureTensorFilterType::New();

	// Connect the input images
	filter->SetInput( converter->GetOutput() );

	// Set the value of sigma (if commented use default)
	// filter->SetSigma( sigma );

	// Execute the filter
	filter->Update();

	// Compute the eigenvectors and eigenvalues of the structure tensor matrix
	typedef  itk::FixedArray< double, Dimension>    EigenValueArrayType;
	typedef  itk::Image< EigenValueArrayType, Dimension> EigenValueImageType;

	typedef  StructureTensorFilterType::OutputImageType SymmetricSecondRankTensorImageType;

	typedef itk::
		SymmetricEigenAnalysisImageFilter<SymmetricSecondRankTensorImageType, EigenValueImageType> EigenAnalysisFilterType;

	EigenAnalysisFilterType::Pointer eigenAnalysisFilter = EigenAnalysisFilterType::New();
	eigenAnalysisFilter->SetDimension( Dimension );
	eigenAnalysisFilter->OrderEigenValuesBy(
		EigenAnalysisFilterType::FunctorType::OrderByValue );

	eigenAnalysisFilter->SetInput( filter->GetOutput() );
	eigenAnalysisFilter->Update();

	// Generate eigen vector image
	typedef  itk::Matrix< double, 3, 3>                         EigenVectorMatrixType;
	typedef  itk::Image< EigenVectorMatrixType, Dimension>      EigenVectorImageType;

	typedef itk::
		SymmetricEigenVectorAnalysisImageFilter<SymmetricSecondRankTensorImageType, EigenValueImageType, EigenVectorImageType> EigenVectorAnalysisFilterType;

	EigenVectorAnalysisFilterType::Pointer eigenVectorAnalysisFilter = EigenVectorAnalysisFilterType::New();
	eigenVectorAnalysisFilter->SetDimension( Dimension );
	eigenVectorAnalysisFilter->OrderEigenValuesBy(
		EigenVectorAnalysisFilterType::FunctorType::OrderByValue );

	eigenVectorAnalysisFilter->SetInput( filter->GetOutput() );
	eigenVectorAnalysisFilter->Update();

	//Generate an image with eigen vector pixel that correspond to the largest eigen value
	EigenVectorImageType::ConstPointer eigenVectorImage =
		eigenVectorAnalysisFilter->GetOutput();

	typedef itk::VectorImage< double, 3 >    VectorImageType;
	VectorImageType::Pointer primaryEigenVectorImage = VectorImageType::New();

	unsigned int vectorLength = 3; // Eigenvector length
	primaryEigenVectorImage->SetVectorLength ( vectorLength );

	VectorImageType::RegionType region;
	region.SetSize(eigenVectorImage->GetLargestPossibleRegion().GetSize());
	region.SetIndex(eigenVectorImage->GetLargestPossibleRegion().GetIndex());
	primaryEigenVectorImage->SetRegions( region );
	primaryEigenVectorImage->SetOrigin(eigenVectorImage->GetOrigin());
	primaryEigenVectorImage->SetSpacing(eigenVectorImage->GetSpacing());
	primaryEigenVectorImage->Allocate();

	//Fill up the buffer with null vector
	itk::VariableLengthVector< double > nullVector( vectorLength );
	for ( unsigned int i=0; i < vectorLength; i++ )
	{
		nullVector[i] = 0.0;
	}
	primaryEigenVectorImage->FillBuffer( nullVector );

	//Generate an image containing the largest eigen values
	typedef itk::Image< double, 3 >    PrimaryEigenValueImageType;
	PrimaryEigenValueImageType::Pointer primaryEigenValueImage = PrimaryEigenValueImageType::New();

	PrimaryEigenValueImageType::RegionType eigenValueImageRegion;
	eigenValueImageRegion.SetSize(eigenVectorImage->GetLargestPossibleRegion().GetSize());
	eigenValueImageRegion.SetIndex(eigenVectorImage->GetLargestPossibleRegion().GetIndex());
	primaryEigenValueImage->SetRegions( eigenValueImageRegion );
	primaryEigenValueImage->SetOrigin(eigenVectorImage->GetOrigin());
	primaryEigenValueImage->SetSpacing(eigenVectorImage->GetSpacing());
	primaryEigenValueImage->Allocate();
	primaryEigenValueImage->FillBuffer( 0.0 );


	//Setup the iterators
	//
	//Iterator for the eigenvector matrix image
	itk::ImageRegionConstIterator<EigenVectorImageType> eigenVectorImageIterator;
	eigenVectorImageIterator = itk::ImageRegionConstIterator<EigenVectorImageType>(
		eigenVectorImage, eigenVectorImage->GetRequestedRegion());
	eigenVectorImageIterator.GoToBegin();

	//Iterator for the output image with the largest eigenvector
	itk::ImageRegionIterator<VectorImageType> primaryEigenVectorImageIterator;
	primaryEigenVectorImageIterator = itk::ImageRegionIterator<VectorImageType>(
		primaryEigenVectorImage, primaryEigenVectorImage->GetRequestedRegion());
	primaryEigenVectorImageIterator.GoToBegin();

	//Iterator for the output image with the largest eigenvalue
	itk::ImageRegionIterator<PrimaryEigenValueImageType> primaryEigenValueImageIterator;
	primaryEigenValueImageIterator = itk::ImageRegionIterator<PrimaryEigenValueImageType>(
		primaryEigenValueImage, primaryEigenValueImage->GetRequestedRegion());
	primaryEigenValueImageIterator.GoToBegin();


	//Iterator for the eigen value image
	EigenValueImageType::ConstPointer eigenImage = eigenAnalysisFilter->GetOutput();
	itk::ImageRegionConstIterator<EigenValueImageType> eigenValueImageIterator;
	eigenValueImageIterator = itk::ImageRegionConstIterator<EigenValueImageType>(
		eigenImage, eigenImage->GetRequestedRegion());
	eigenValueImageIterator.GoToBegin();

	//Iterator for the structure tensor
	typedef StructureTensorFilterType::OutputImageType TensorImageType;
	TensorImageType::ConstPointer tensorImage = filter->GetOutput();
	itk::ImageRegionConstIterator<TensorImageType> tensorImageIterator;
	tensorImageIterator = itk::ImageRegionConstIterator<TensorImageType>(
		tensorImage, tensorImage->GetRequestedRegion());
	tensorImageIterator.GoToBegin();


	double toleranceEigenValues = 1e-4;

	while (!eigenValueImageIterator.IsAtEnd())
	{
		// Get the eigen value
		EigenValueArrayType eigenValue;
		eigenValue = eigenValueImageIterator.Get();

		// Find the smallest eigenvalue
		double smallest = vnl_math_abs( eigenValue[0] );
		double Lambda1 = eigenValue[0];
		unsigned int smallestEigenValueIndex=0;

		for ( unsigned int i=1; i <=2; i++ )
		{
			if ( vnl_math_abs( eigenValue[i] ) < smallest )
			{
				Lambda1 = eigenValue[i];
				smallest = vnl_math_abs( eigenValue[i] );
				smallestEigenValueIndex = i;
			}
		}

		// Find the largest eigenvalue
		double largest = vnl_math_abs( eigenValue[0] );
		double Lambda3 = eigenValue[0];
		unsigned int largestEigenValueIndex=0;

		for ( unsigned int i=1; i <=2; i++ )
		{
			if (  vnl_math_abs( eigenValue[i] > largest ) )
			{
				Lambda3 = eigenValue[i];
				largest = vnl_math_abs( eigenValue[i] );
				largestEigenValueIndex = i;
			}
		}

		// find Lambda2 so that |Lambda1| < |Lambda2| < |Lambda3|
		double Lambda2 = eigenValue[0];
		unsigned int middleEigenValueIndex=0;

		for ( unsigned int i=0; i <=2; i++ )
		{
			if ( eigenValue[i] != Lambda1 && eigenValue[i] != Lambda3 )
			{
				Lambda2 = eigenValue[i];
				middleEigenValueIndex = i;
				break;
			}
		}

		// Write out the largest eigen value
		primaryEigenValueImageIterator.Set( eigenValue[largestEigenValueIndex]);



		EigenValueImageType::IndexType pixelIndex;
		pixelIndex = eigenValueImageIterator.GetIndex();

		EigenVectorMatrixType   matrixPixel;
		matrixPixel = eigenVectorImageIterator.Get();

		//Tensor pixelType
		TensorImageType::PixelType tensorPixel;
		tensorPixel = tensorImageIterator.Get();

		/*
		std::cout << "[" << pixelIndex[0] << "," << pixelIndex[1] << "," << pixelIndex[2] << "]"
		<< "\t" << eigenValue[0] << "\t" << eigenValue[1] << "\t"  << eigenValue[2] << std::endl;
		std::cout << "[Smallest,Largest]" << "\t" << smallest << "\t" << largest << std::endl;
		std::cout <<"Eigen Vector" << std::endl;
		std::cout <<"\t" <<  matrixPixel << std::endl;
		std::cout <<"Tensor pixel" << std::endl;
		std::cout << "\t" << tensorPixel(0,0) << "\t" << tensorPixel(0,1) << "\t" << tensorPixel(0,2) << std::endl;
		std::cout << "\t" << tensorPixel(1,0) << "\t" << tensorPixel(1,1) << "\t" << tensorPixel(1,2) << std::endl;
		std::cout << "\t" << tensorPixel(2,0) << "\t" << tensorPixel(2,1) << "\t" << tensorPixel(2,2) << std::endl;
		*/

		if( fabs(largest) > toleranceEigenValues  )
		{
			//Assuming eigenvectors are rows
			itk::VariableLengthVector<double> primaryEigenVector( vectorLength );
			for ( unsigned int i=0; i < vectorLength; i++ )
			{
				primaryEigenVector[i] = matrixPixel[largestEigenValueIndex][i];
			}

			//std::cout << "\t" << "[" << primaryEigenVector[0] << "," << primaryEigenVector[1] << "," << primaryEigenVector[2] << "]" << std::endl;
			primaryEigenVectorImageIterator.Set( primaryEigenVector );
		}

		++eigenValueImageIterator;
		++eigenVectorImageIterator;
		++primaryEigenVectorImageIterator;
		++primaryEigenValueImageIterator;
		++tensorImageIterator;

	}

	typedef itk::ImageFileWriter< VectorImageType > WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName( m_PrimaryEigenVectorOutputImageAbsPath.GetCStr() );
	writer->SetInput( primaryEigenVectorImage );
	writer->Update();

	assert(wxFileExists(m_PrimaryEigenVectorOutputImageAbsPath.GetCStr()) == true);

	// Not needed for the moment...
	/*typedef itk::ImageFileWriter< PrimaryEigenValueImageType > EigenValueImageWriterType;
	EigenValueImageWriterType::Pointer eigenValueWriter = EigenValueImageWriterType::New();
	eigenValueWriter->SetFileName( primaryEigenValueOutputImageAbsPath );
	eigenValueWriter->SetInput( primaryEigenValueImage );
	eigenValueWriter->Update();

	assert(wxFileExists(primaryEigenValueOutputImageAbsPath.c_str()) == true);
*/
	// All objects should be automatically destroyed at this point
	// Success
	assert(true);

	wxString msg;
	msg = "Output eigen vectors file written to ";
	msg.append(m_PrimaryEigenVectorOutputImageAbsPath.GetCStr());
	
	mafLogMessage(msg);
	wxMessageBox(msg);

}

