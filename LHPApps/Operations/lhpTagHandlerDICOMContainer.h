/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpTagHandlerDICOMContainer.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Stefano Perticoni - Daniele Giunchi
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/
#ifndef __lhpTagHandlerDICOMContainer_h
#define __lhpTagHandlerDICOMContainer_h

//----------------------------------------------------------------------------
// includes :
//----------------------------------------------------------------------------

#include "lhpTagHandler.h"

class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyDate: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyDate, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyDate();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Modality: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Modality, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Modality();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};




class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Manufacturer: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Manufacturer, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Manufacturer();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};




class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_InstitutionName: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_InstitutionName, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_InstitutionName();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StationName: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StationName, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StationName();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ManufacturerModelName: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ManufacturerModelName, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ManufacturerModelName();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientID: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientID, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientID();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientSex: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientSex, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientSex();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ScanOptions: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ScanOptions, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ScanOptions();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_KVP: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_KVP, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_KVP();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};




class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DataCollectionDiameter: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DataCollectionDiameter, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DataCollectionDiameter();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ReconstructionDiameter: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ReconstructionDiameter, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ReconstructionDiameter();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToDetector: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToDetector, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToDetector();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToPatient: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToPatient, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToPatient();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_GantryDetectorTilt: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_GantryDetectorTilt, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_GantryDetectorTilt();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};


class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_TableHeight: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_TableHeight, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_TableHeight();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RotationDirection: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RotationDirection, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RotationDirection();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ExposureTime: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ExposureTime, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ExposureTime();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_XRayTubeCurrent: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_XRayTubeCurrent, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_XRayTubeCurrent();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Exposure: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Exposure, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Exposure();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FilterType: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FilterType, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FilterType();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FocalSpot: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FocalSpot, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FocalSpot();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ConvolutionKernel: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ConvolutionKernel, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ConvolutionKernel();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientPosition: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientPosition, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientPosition();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyID: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyID, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyID();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ImagePositionPatient: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ImagePositionPatient, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ImagePositionPatient();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelSpacing: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelSpacing, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelSpacing();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelPaddingValue: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelPaddingValue, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelPaddingValue();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowCenter: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowCenter, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowCenter();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowWidth: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowWidth, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowWidth();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleIntercept: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleIntercept, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleIntercept();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};



class lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleSlope: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleSlope, mafObject);

    lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleSlope();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

#endif