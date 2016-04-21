/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpTagHandlerDICOMContainer.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Stefano Perticoni - Daniele Giunchi
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------------

#include "lhpTagHandlerDICOMContainer.h"

#include "mafTagArray.h"
#include "mafVME.h"

#include <string>
#include <fstream>
#include <iostream>

using namespace std; 


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}



mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyDate);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyDate::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyDate()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyDate::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("StudyDate"))
  {
    vme->GetTagArray()->GetTag("StudyDate")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Modality);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Modality::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Modality()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Modality::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("Modality"))
  {
    vme->GetTagArray()->GetTag("Modality")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Manufacturer);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Manufacturer::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Manufacturer()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Manufacturer::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("Manufacturer"))
  {
    vme->GetTagArray()->GetTag("Manufacturer")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_InstitutionName);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_InstitutionName::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_InstitutionName()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_InstitutionName::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("InstitutionName"))
  {
    vme->GetTagArray()->GetTag("InstitutionName")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StationName);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StationName::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StationName()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StationName::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("StationName"))
  {
    vme->GetTagArray()->GetTag("StationName")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ManufacturerModelName);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ManufacturerModelName::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ManufacturerModelName()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ManufacturerModelName::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("ManufacturerModelName"))
  {
    vme->GetTagArray()->GetTag("ManufacturerModelName")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientID);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientID::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientID()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientID::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("PatientID"))
  {
    vme->GetTagArray()->GetTag("PatientID")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientSex);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientSex::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientSex()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientSex::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("PatientSex"))
  {
    vme->GetTagArray()->GetTag("PatientSex")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ScanOptions);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ScanOptions::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ScanOptions()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ScanOptions::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("ScanOptions"))
  {
    vme->GetTagArray()->GetTag("ScanOptions")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_KVP);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_KVP::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_KVP()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_KVP::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("KVP"))
  {
    vme->GetTagArray()->GetTag("KVP")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DataCollectionDiameter);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DataCollectionDiameter::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DataCollectionDiameter()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DataCollectionDiameter::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("DataCollectionDiameter"))
  {
    vme->GetTagArray()->GetTag("DataCollectionDiameter")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ReconstructionDiameter);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ReconstructionDiameter::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ReconstructionDiameter()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ReconstructionDiameter::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("ReconstructionDiameter"))
  {
    vme->GetTagArray()->GetTag("ReconstructionDiameter")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToDetector);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToDetector::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToDetector()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToDetector::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("DistanceSourceToDetector"))
  {
    vme->GetTagArray()->GetTag("DistanceSourceToDetector")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToPatient);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToPatient::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToPatient()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_DistanceSourceToPatient::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("DistanceSourceToPatient"))
  {
    vme->GetTagArray()->GetTag("DistanceSourceToPatient")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_GantryDetectorTilt);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_GantryDetectorTilt::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_GantryDetectorTilt()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_GantryDetectorTilt::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("GantryDetectorTilt"))
  {
    vme->GetTagArray()->GetTag("GantryDetectorTilt")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_TableHeight);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_TableHeight::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_TableHeight()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_TableHeight::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("TableHeight"))
  {
    vme->GetTagArray()->GetTag("TableHeight")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RotationDirection);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RotationDirection::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RotationDirection()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RotationDirection::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("RotationDirection"))
  {
    vme->GetTagArray()->GetTag("RotationDirection")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ExposureTime);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ExposureTime::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ExposureTime()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ExposureTime::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("ExposureTime"))
  {
    vme->GetTagArray()->GetTag("ExposureTime")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_XRayTubeCurrent);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_XRayTubeCurrent::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_XRayTubeCurrent()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_XRayTubeCurrent::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("XRayTubeCurrent"))
  {
    vme->GetTagArray()->GetTag("XRayTubeCurrent")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Exposure);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Exposure::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Exposure()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_Exposure::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("Exposure"))
  {
    vme->GetTagArray()->GetTag("Exposure")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FilterType);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FilterType::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FilterType()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FilterType::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("FilterType"))
  {
    vme->GetTagArray()->GetTag("FilterType")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FocalSpot);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FocalSpot::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FocalSpot()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_FocalSpot::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("FocalSpot"))
  {
    vme->GetTagArray()->GetTag("FocalSpot")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ConvolutionKernel);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ConvolutionKernel::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ConvolutionKernel()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ConvolutionKernel::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("ConvolutionKernel"))
  {
    vme->GetTagArray()->GetTag("ConvolutionKernel")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientPosition);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientPosition::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientPosition()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PatientPosition::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("PatientPosition"))
  {
    vme->GetTagArray()->GetTag("PatientPosition")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyID);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyID::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyID()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_StudyID::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("StudyID"))
  {
    vme->GetTagArray()->GetTag("StudyID")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ImagePositionPatient);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ImagePositionPatient::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ImagePositionPatient()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_ImagePositionPatient::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("ImagePositionPatient"))
  {
    vme->GetTagArray()->GetTag("ImagePositionPatient")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelSpacing);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelSpacing::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelSpacing()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelSpacing::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("PixelSpacing"))
  {
    vme->GetTagArray()->GetTag("PixelSpacing")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelPaddingValue);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelPaddingValue::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelPaddingValue()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_PixelPaddingValue::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("PixelPaddingValue"))
  {
    vme->GetTagArray()->GetTag("PixelPaddingValue")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowCenter);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowCenter::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowCenter()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowCenter::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("WindowCenter"))
  {
    vme->GetTagArray()->GetTag("WindowCenter")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowWidth);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowWidth::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowWidth()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_WindowWidth::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("WindowWidth"))
  {
    vme->GetTagArray()->GetTag("WindowWidth")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleIntercept);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleIntercept::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleIntercept()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleIntercept::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("RescaleIntercept"))
  {
    vme->GetTagArray()->GetTag("RescaleIntercept")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleSlope);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleSlope::lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleSlope()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Attributes_SourceAttributes_SourceType_SourceDir_Type_RescaleSlope::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  if(vme->GetTagArray()->IsTagPresent("RescaleSlope"))
  {
    vme->GetTagArray()->GetTag("RescaleSlope")->GetValueAsSingleString(value);
  }
  else
  {
    value = "NOT PRESENT";
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}