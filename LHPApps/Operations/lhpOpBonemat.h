/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpBonemat.h,v $
  Language:  C++
  Date:      $Date: 2011-12-12 12:33:41 $
  Version:   $Revision: 1.1.1.1.2.2 $
  Authors:   Daniele Giunchi , Stefano Perticoni, Gianluigi Crimi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpBonemat_H__
#define __lhpOpBonemat_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "lhpOpBonematCommon.h"
#include "vtkMath.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafMatrix;
class vtkUnstructuredGrid;
class vtkTransform;
class mafVMEVolumeGray;
class Element;
class vtkCell;
class vtkIntArray;
class vtkDoubleArray;
class vtkFieldData;

// Element Properties: element ID, density, Young Module
typedef struct {
	vtkIdType elementID;  
	double rhoQCT;
	double rhoAsh;
	double E;
} ElementProp;


//----------------------------------------------------------------------------
// lhpOpBonemat :
//----------------------------------------------------------------------------
/** Operation to map CT Volume properties on the operation input finite element mesh*/
class LHP_OPERATIONS_EXPORT lhpOpBonemat: public lhpOpBonematCommon
{
public:

  lhpOpBonemat(wxString label);
  ~lhpOpBonemat(); 

	/** Static function for Young Module conversion, you must set staticCalibration before call this function*/ 
	static double YoungModuleFromHu(double HU);

	/** Static function witch compares two ElementProps used for qsort in decreasing order*/
	static int compareE(const void *p1, const void *p2);

	/** Static function that computes the interpolation of the volume components, you must set staticVolumeInterpolatorPointer before call this function */
	static double ComputeVolumeIntegrand(double pos[3]);

	/** Static function that computes the interpolation the Young module of the volume components,
	    you must set staticVolumeInterpolatorPointer and  staticCalibration before call this function */
	static double ComputeVolumeYoungIntegrand(double pos[3]) ;
	
  /** Execute the procedure that maps TAC values on the finite element mesh; */
  int Execute();

	/** Operation copy */
  mafOp* Copy();

protected:

	/**Computes HU integration and fill element props vector */
	void HUIntegration(vtkUnstructuredGrid *inputTrasformedUG, ElementProp *elProps);
	
	/**Computes E integration and fill element props vector*/
	void EIntegration(vtkUnstructuredGrid *inputTrasformedUG, ElementProp *elProps);

	/** Generates Arrays And Field Data from element properties vector and save frequency file */
	void GenerateArraysAndFieldData(FILE *freq_fp, vtkIdType numElements, ElementProp *elProps, vtkUnstructuredGrid * inputUG);

	/** Generates field data for output mesh */
	vtkFieldData* GenerateFieldData(std::vector<ElementProp> materialProperties);

	/** updates output mesh with arrays and field data */
	void UpdateOutputMesh(vtkUnstructuredGrid *inputUG, vtkIntArray *arrayMaterial, vtkDoubleArray *arrayE, vtkDoubleArray *arrayPoisson, vtkDoubleArray *arrayRho, vtkFieldData *fdata);

	/** Creates a new Element from vtkCell */
	Element* CreateElementFromCell(vtkCell * cell);

	/** Creates material Bins By grouping element props, internally sorts elProps vector */
	void lhpOpBonemat::CreateBins(FILE *freq_fp,  int numElements, ElementProp *elProps,  std::vector<ElementProp> *materialProperties, std::vector <int> *frequences);

	/** Fills Data Arrays */
	void  FillDataArrays(int numElements, std::vector<ElementProp> materialProperties, std::vector<int> frequences, ElementProp *elemProperties, vtkFieldData *fdata,vtkDoubleArray *arrayRho, vtkDoubleArray *arrayE, vtkIntArray *arrayMaterial,vtkDoubleArray *arrayPoisson);

	/** static function for HU to RhoQct Conversion */
	static double RhoQCTFromHU(double HU, Calibration *calibration);
	/** static function for RhoQct to RhoAsh Conversion */
	static double RhoAshFromRhoQCT(double rhoQCT, Calibration *calibration);
	/** static function for RhoAsh to Young Module Conversion */
	static double YoungModuleFromRho(double rho, Calibration *calibration);

	/** Generate transformed mesh for Volume <-> Mesh align */
	bool lhpOpBonemat::GetTrasformdedMesh(vtkUnstructuredGrid *inputUG,vtkUnstructuredGrid **outputUG, mafMatrix meshAbsMatrix, mafMatrix volumeAbsMatrix);

	// friend test
  friend class lhpOpBonematTest;
};
#endif
