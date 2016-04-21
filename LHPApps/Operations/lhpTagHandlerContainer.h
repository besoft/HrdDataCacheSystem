/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpTagHandlerContainer.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Stefano Perticoni - Daniele Giunchi - Roberto Mucci
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/
#ifndef __lhpTagHandlerContainer_h
#define __lhpTagHandlerContainer_h

//----------------------------------------------------------------------------
// includes :
//----------------------------------------------------------------------------

#include "lhpTagHandler.h"


class lhpTagHandler_L0000_resource_DictionaryURI: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_DictionaryURI,mafObject);

  lhpTagHandler_L0000_resource_DictionaryURI();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_DictionaryVersion: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_DictionaryVersion,mafObject);

  lhpTagHandler_L0000_resource_DictionaryVersion();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_DataType_Field: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_DataType_Field,mafObject);

    lhpTagHandler_L0000_resource_data_DataType_Field();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};


class lhpTagHandler_L0000_resource_data_DataType_Dimension: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_DataType_Dimension,mafObject);

    lhpTagHandler_L0000_resource_data_DataType_Dimension();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_data_DataType_VolumeType: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_DataType_VolumeType,mafObject);

    lhpTagHandler_L0000_resource_data_DataType_VolumeType();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};
 
class lhpTagHandler_L0000_resource_data_DataType_Timevarying: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_DataType_Timevarying,mafObject);

    lhpTagHandler_L0000_resource_data_DataType_Timevarying();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_data_Size_FileSize: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Size_FileSize,mafObject);

    lhpTagHandler_L0000_resource_data_Size_FileSize();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_data_Size_EntityCount: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Size_EntityCount,mafObject);

    lhpTagHandler_L0000_resource_data_Size_EntityCount();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_data_Size_TimeFramesCount: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Size_TimeFramesCount,mafObject);

    lhpTagHandler_L0000_resource_data_Size_TimeFramesCount();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_data_Dataset_DatasetURI: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Dataset_DatasetURI,mafObject);

    lhpTagHandler_L0000_resource_data_Dataset_DatasetURI();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_data_Dataset_UploadDate: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Dataset_UploadDate,mafObject);

    lhpTagHandler_L0000_resource_data_Dataset_UploadDate();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_data_Dataset_LocalFileCheckSum: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Dataset_LocalFileCheckSum,mafObject);

  lhpTagHandler_L0000_resource_data_Dataset_LocalFileCheckSum();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};


class lhpTagHandler_L0000_resource_data_Dataset_FileType_FileFormat: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Dataset_FileType_FileFormat,mafObject);

    lhpTagHandler_L0000_resource_data_Dataset_FileType_FileFormat();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_data_Dataset_FileType_Endianity: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Dataset_FileType_Endianity,mafObject);

    lhpTagHandler_L0000_resource_data_Dataset_FileType_Endianity();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_data_Dataset_FileType_Encryption: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Dataset_FileType_Encryption,mafObject);

    lhpTagHandler_L0000_resource_data_Dataset_FileType_Encryption();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_MAF_VmeType: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_MAF_VmeType,mafObject);

  lhpTagHandler_L0000_resource_MAF_VmeType();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_MAF_TimeSpace_VMEabsoluteMatrixPose: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_MAF_TimeSpace_VMEabsoluteMatrixPose,mafObject);

    lhpTagHandler_L0000_resource_MAF_TimeSpace_VMEabsoluteMatrixPose();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_MAF_TimeSpace_TimeStampVector: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_MAF_TimeSpace_TimeStampVector,mafObject);

    lhpTagHandler_L0000_resource_MAF_TimeSpace_TimeStampVector();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootURI: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootURI,mafObject);

    lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootURI();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootName: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootName,mafObject);

    lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootName();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeChildURI1: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeChildURI1,mafObject);

    lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeChildURI1();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_MAF_Procedural: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_MAF_Procedural,mafObject);

  lhpTagHandler_L0000_resource_MAF_Procedural();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_MAF_Procedural_VMElinkURI1: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_MAF_Procedural_VMElinkURI1,mafObject);

  lhpTagHandler_L0000_resource_MAF_Procedural_VMElinkURI1();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeTreeCreationDate: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeTreeCreationDate,mafObject);

    lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeTreeCreationDate();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};


class lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Operation: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Operation,mafObject);

  lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Operation();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_CreationDate: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_CreationDate,mafObject);

  lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_CreationDate();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Application: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Application,mafObject);

  lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Application();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_IsNatural: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_IsNatural,mafObject);

  lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_IsNatural();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_OperatorID: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_OperatorID,mafObject);

  lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_OperatorID();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Parameters: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Parameters,mafObject);

  lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Parameters();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Operation: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Operation,mafObject);

  lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Operation();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_ModifyDate: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_ModifyDate,mafObject);

  lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_ModifyDate();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Application: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Application,mafObject);

  lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Application();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_OperatorID: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_OperatorID,mafObject);

  lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_OperatorID();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Parameters: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Parameters,mafObject);

  lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Parameters();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Traceability_Ownership: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_Ownership,mafObject);

    lhpTagHandler_L0000_resource_data_Traceability_Ownership();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};

class lhpTagHandler_L0000_resource_data_Traceability_Ownership_OwnerID: public lhpTagHandler
{
  public:
    mafTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_Ownership_OwnerID,mafObject);

    lhpTagHandler_L0000_resource_data_Traceability_Ownership_OwnerID();
    virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);
  
};


class lhpTagHandler_L0000_resource_data_Ownership_OwnerID: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Ownership_OwnerID,mafObject);

  lhpTagHandler_L0000_resource_data_Ownership_OwnerID();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Quality_QualityScore1_URL: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Quality_QualityScore1_URL,mafObject);

  lhpTagHandler_L0000_resource_data_Quality_QualityScore1_URL();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Quality_QualityScore1_Score: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Quality_QualityScore1_Score,mafObject);

  lhpTagHandler_L0000_resource_data_Quality_QualityScore1_Score();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Quality_QualityScore1_ScoreDate: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Quality_QualityScore1_ScoreDate,mafObject);

  lhpTagHandler_L0000_resource_data_Quality_QualityScore1_ScoreDate();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_Access_Policy1_GroupID: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_Access_Policy1_GroupID,mafObject);

  lhpTagHandler_L0000_resource_Access_Policy1_GroupID();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_Access_Policy1_Price: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_Access_Policy1_Price,mafObject);

  lhpTagHandler_L0000_resource_Access_Policy1_Price();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_Access_Policy1_Usage: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_Access_Policy1_Usage,mafObject);

  lhpTagHandler_L0000_resource_Access_Policy1_Usage();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_Access_Publishing_PublishingStatus: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_Access_Publishing_PublishingStatus,mafObject);

  lhpTagHandler_L0000_resource_Access_Publishing_PublishingStatus();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy,mafObject);

  lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryURI: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryURI,mafObject);

  lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryURI();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryVersion: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryVersion,mafObject);

  lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryVersion();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryURI: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryURI,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryURI();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryVersion: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryVersion,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryVersion();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_MASource: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_MASource,mafObject);

  lhpTagHandler_L0000_resource_data_Source_MASource();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryURI: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryURI,mafObject);

  lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryURI();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryVersion: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryVersion,mafObject);

  lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryVersion();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_MASource_MASource_General_SamplingFrequency: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_MASource_MASource_General_SamplingFrequency,mafObject);

  lhpTagHandler_L0000_resource_data_Source_MASource_MASource_General_SamplingFrequency();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DataType_IsLandmark_NumberOfLandmarks: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DataType_IsLandmark_NumberOfLandmarks,mafObject);

  lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DataType_IsLandmark_NumberOfLandmarks();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};



class lhpTagHandler_L0000_resource_data_Source_MicroCTSource: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_MicroCTSource,mafObject);

  lhpTagHandler_L0000_resource_data_Source_MicroCTSource();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryURI: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryURI,mafObject);

  lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryURI();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryVersion: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryVersion,mafObject);

  lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryVersion();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class L0000_resource_Documentation: public lhpTagHandler
{
public:
  mafTypeMacro(L0000_resource_Documentation,mafObject);

  L0000_resource_Documentation();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};


class L0000_resource_Access_Policy1_GroupID: public lhpTagHandler
{
public:
  mafTypeMacro(L0000_resource_Access_Policy1_GroupID,mafObject);

  L0000_resource_Access_Policy1_GroupID();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};


class L0000_resource_Access_Policy1_Price: public lhpTagHandler
{
public:
  mafTypeMacro(L0000_resource_Access_Policy1_Price,mafObject);

  L0000_resource_Access_Policy1_Price();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};


class L0000_resource_Access_Policy1_Usage: public lhpTagHandler
{
public:
  mafTypeMacro(L0000_resource_Access_Policy1_Usage,mafObject);

  L0000_resource_Access_Policy1_Usage();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class L0000_resource_Access_Publishing_PublishingStatus: public lhpTagHandler
{
public:
  mafTypeMacro(L0000_resource_Access_Publishing_PublishingStatus,mafObject);

  L0000_resource_Access_Publishing_PublishingStatus();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyDate: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyDate,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyDate();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Modality: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Modality,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Modality();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Manufacturer: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Manufacturer,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Manufacturer();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_InstitutionName: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_InstitutionName,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_InstitutionName();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StationName: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StationName,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StationName();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ManufacturerModelName: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ManufacturerModelName,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ManufacturerModelName();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientID: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientID,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientID();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientSex: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientSex,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientSex();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ScanOptions: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ScanOptions,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ScanOptions();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_KVP: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_KVP,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_KVP();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DataCollectionDiameter: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DataCollectionDiameter,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DataCollectionDiameter();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ReconstructionDiameter: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ReconstructionDiameter,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ReconstructionDiameter();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToDetector: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToDetector,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToDetector();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToPatient: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToPatient,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToPatient();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_GantryDetectorTilt: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_GantryDetectorTilt,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_GantryDetectorTilt();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_TableHeight: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_TableHeight,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_TableHeight();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RotationDirection: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RotationDirection,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RotationDirection();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ExposureTime: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ExposureTime,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ExposureTime();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_XRayTubeCurrent: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_XRayTubeCurrent,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_XRayTubeCurrent();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Exposure: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Exposure,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Exposure();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FilterType: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FilterType,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FilterType();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FocalSpot: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FocalSpot,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FocalSpot();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ConvolutionKernel: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ConvolutionKernel,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ConvolutionKernel();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientPosition: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientPosition,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientPosition();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyID: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyID,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyID();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ImagePositionPatient: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ImagePositionPatient,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ImagePositionPatient();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelSpacing: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelSpacing,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelSpacing();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelPaddingValue: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelPaddingValue,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelPaddingValue();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowCenter: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowCenter,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowCenter();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowWidth: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowWidth,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowWidth();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleIntercept: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleIntercept,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleIntercept();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

class lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleSlope: public lhpTagHandler
{
public:
  mafTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleSlope,mafObject);

  lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleSlope();
  virtual void HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo);

};

#endif