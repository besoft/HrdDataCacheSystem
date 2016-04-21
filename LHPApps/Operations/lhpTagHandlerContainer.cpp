/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpTagHandlerContainer.cpp,v $
  Language:  C++
  Date:      $Date: 2012-03-20 17:15:11 $
  Version:   $Revision: 1.1.1.1.2.3 $
  Authors:   Stefano Perticoni - Daniele Giunchi - Roberto Mucci
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

#include "lhpTagHandlerContainer.h"
#include <wx/zipstrm.h>
#include <wx/zstream.h>
#include <wx/wfstream.h>
#include <wx/fs_zip.h>
#include <wx/dir.h>

#include "mafTagArray.h"
#include "mafVME.h"
#include "mafVMERoot.h"
#include "mafAbsMatrixPipe.h"
#include "mafDataVector.h"
#include "vtkImageData.h"
#include "vtkRectilinearGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPolyData.h"
#include "mafVMEOutputSurface.h"
#include "mafVMEOutputPolyline.h"
#include "mafVMEOutputVolume.h"
#include "mafVMEOutputPointSet.h"
#include "mafVMEGenericAbstract.h"
#include "mafVMELandmarkCloud.h"
#include "mafAttributeTraceability.h"


#include <string>
#include <fstream>
#include <iostream>

using namespace std; 


mafCxxTypeMacro(lhpTagHandler_L0000_resource_DictionaryURI);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_DictionaryURI::lhpTagHandler_L0000_resource_DictionaryURI()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_DictionaryURI::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("https://www.biomedtown.org/biomed_town/LHDL/users/swclient/dictionaries/LHDL_dictionary");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_DictionaryVersion);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_DictionaryVersion::lhpTagHandler_L0000_resource_DictionaryVersion()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_DictionaryVersion::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafString dictionaryFileNamePrefix = "lhpXMLDictionary_";
  mafString dictionaryFileName = "NOT FOUND";
  wxString oldDir = wxGetCwd();

  wxSetWorkingDirectory(m_VMEUploaderDownloaderDir.GetCStr());

  wxArrayString files;
  wxString filePattern = dictionaryFileNamePrefix ;
  filePattern.Append("*.xml");

  wxDir::GetAllFiles(wxGetWorkingDirectory(), &files, filePattern, wxDIR_FILES);

  if (files.size() == 0)
  {
    // tag handling code
    cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
  }
  if (files.size() != 1)
  {
    // tag handling code
    cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
  }
  else
  {
    assert(files.size() == 1);
    dictionaryFileName = files[0];
    int pos = dictionaryFileName.FindLast("\\");
    dictionaryFileName.Erase(0, pos);
  }

  wxSetWorkingDirectory(oldDir);

  int pos = dictionaryFileName.FindLast("_");
  dictionaryFileName.Erase(0, pos);
  pos = dictionaryFileName.FindLast(".");
  dictionaryFileName.Erase(pos);
  cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_DataType_Field);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_DataType_Field::lhpTagHandler_L0000_resource_data_DataType_Field()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_DataType_Field::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_DataType_Dimension);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_DataType_Dimension::lhpTagHandler_L0000_resource_data_DataType_Dimension()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_DataType_Dimension::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
	mafVME *vme = cargo->GetInputVme();
	mafString value;
  if(mafVMEOutputSurface::SafeDownCast(vme->GetOutput()))
	{
		value = "SURFACE";
	}
  else if(mafVMEOutputPolyline::SafeDownCast(vme->GetOutput())) 
  {
    value = "CURVE";
  }
  else if(mafVMEOutputVolume::SafeDownCast(vme->GetOutput()))
  {
    value = "VOLUME";
  }
  else if(mafVMEOutputPointSet::SafeDownCast(vme->GetOutput()))
  {
    value = "POINT";
  }
	else
	{
		value = "NOT PRESENT";
	}

	// tag handling code
	cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_DataType_VolumeType);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_DataType_VolumeType::lhpTagHandler_L0000_resource_data_DataType_VolumeType()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_DataType_VolumeType::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
	mafVME *vme = cargo->GetInputVme();
	mafString value;
  if(vtkImageData::SafeDownCast(vme->GetOutput()->GetVTKData()))
	{
		value = "STRUCTURED";
	}
  else if (vtkRectilinearGrid::SafeDownCast(vme->GetOutput()->GetVTKData()))
  {
    value = "CARTESIAN";
  }
  else if (vtkUnstructuredGrid::SafeDownCast(vme->GetOutput()->GetVTKData()))
  {
    value = "UNSTRUCTURED";
  }
/*  else if (? ::SafeDownCast(vme->GetOutput()->GetVTKData())) BREP
  {
    value = "BREP"; //not yet supported
  }*/
	else
	{
		value = "NOT PRESENT";
	}

	// tag handling code
	cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_DataType_Timevarying);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_DataType_Timevarying::lhpTagHandler_L0000_resource_data_DataType_Timevarying()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_DataType_Timevarying::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  value = vme->IsAnimated()?"1":"0";

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Size_FileSize);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Size_FileSize::lhpTagHandler_L0000_resource_data_Size_FileSize()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Size_FileSize::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  //here there is also the controller for zip archive
	long length = 0;
	mafString inputMSF = cargo->GetInputMSF();
	mafString id ;
	id << cargo->GetInputVme()->GetId();

  if(cargo->GetInputVme()->GetId() == -1) return; 

	//here put code for filename
	wxString oldDir = wxGetCwd();
	mafLogMessage( _T("Current working directory is: '%s' "), wxGetCwd().c_str() );
	wxSetWorkingDirectory(m_VMEUploaderDownloaderDir.GetCStr());
	mafLogMessage( _T("Now current working directory is: '%s' "), wxGetCwd().c_str() );

	// get manual tags
	wxString command2execute;
	command2execute.Clear();
	command2execute = m_PythonwExe.GetCStr();

	command2execute.Append(" lhpVMEBinaryDataChecker.py ");
  command2execute.Append("\"");
	command2execute.Append(inputMSF.GetCStr());
  command2execute.Append("\"");
	command2execute.Append(" ");
	command2execute.Append(id.GetCStr());

	//mafLogMessage( _T("Executing command: '%s'"), command2execute.c_str() );

	long pid = wxExecute(command2execute, wxEXEC_SYNC);

	wxArrayString output;
	wxArrayString errors;

	pid = wxExecute(command2execute, output, errors);

  if(output== NULL)
  {
    return;
  }
	wxString result = output[output.size() - 1];
  
	wxSetWorkingDirectory(oldDir);

  //if result == "", no binary data has been found
  if (result == "")
  {
    cargo->SetTagHandlerGeneratedString(wxString::Format("%d",0));
    return;
  }

	////////////////////////////

	wxString temp;
	temp.Append(inputMSF.GetCStr());
	temp = temp.BeforeLast('/');
	temp.Append("/");
	temp.Append(result);

  wxString extension;
  extension.Append(result);
  extension = extension.AfterLast('.');

  bool fileOpened = false;
  mafString fileToComputeLength = temp;
 
  fstream fp;
  fp.open(fileToComputeLength);

  if(fp.fail() == false)
  {
    fp.seekg(0, ios::end);
    length = fp.tellg();
    fp.close();
    fileOpened = true;
  }


  //Process with length
	if(fileOpened) cargo->SetTagHandlerGeneratedString(wxString::Format("%d",length));
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Size_EntityCount);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Size_EntityCount::lhpTagHandler_L0000_resource_data_Size_EntityCount()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Size_EntityCount::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag , Not Handled (instance exists)");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Size_TimeFramesCount);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Size_TimeFramesCount::lhpTagHandler_L0000_resource_data_Size_TimeFramesCount()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Size_TimeFramesCount::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();

  mafString value;
  std::vector<mafTimeStamp> timeStamps;
  vme->GetTimeStamps(timeStamps);
  value << (long) timeStamps.size();

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Dataset_DatasetURI);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Dataset_DatasetURI::lhpTagHandler_L0000_resource_data_Dataset_DatasetURI()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Dataset_DatasetURI::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Dataset_UploadDate);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Dataset_UploadDate::lhpTagHandler_L0000_resource_data_Dataset_UploadDate()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Dataset_UploadDate::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Dataset_LocalFileCheckSum);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Dataset_LocalFileCheckSum::lhpTagHandler_L0000_resource_data_Dataset_LocalFileCheckSum()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Dataset_LocalFileCheckSum::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Dataset_FileType_FileFormat);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Dataset_FileType_FileFormat::lhpTagHandler_L0000_resource_data_Dataset_FileType_FileFormat()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Dataset_FileType_FileFormat::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafString value = "MAF2"; //for now the only file format supported
 
  // tag handling code
  cargo->SetTagHandlerGeneratedString(value.GetCStr());
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Dataset_FileType_Endianity);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Dataset_FileType_Endianity::lhpTagHandler_L0000_resource_data_Dataset_FileType_Endianity()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Dataset_FileType_Endianity::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
	
	mafString value;
	value = mafIsLittleEndian()? "Little Endian" : "Big Endian";

	// tag handling code
	cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Dataset_FileType_Encryption);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Dataset_FileType_Encryption::lhpTagHandler_L0000_resource_data_Dataset_FileType_Encryption()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Dataset_FileType_Encryption::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  value = vme->GetCrypting()?"1":"0";

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);

}
mafCxxTypeMacro(lhpTagHandler_L0000_resource_MAF_VmeType);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_MAF_VmeType::lhpTagHandler_L0000_resource_MAF_VmeType()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_MAF_VmeType::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  value = vme->GetTypeName();
  cargo->SetTagHandlerGeneratedString(value.GetCStr());
}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_MAF_TimeSpace_VMEabsoluteMatrixPose);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_MAF_TimeSpace_VMEabsoluteMatrixPose::lhpTagHandler_L0000_resource_MAF_TimeSpace_VMEabsoluteMatrixPose()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_MAF_TimeSpace_VMEabsoluteMatrixPose::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
	mafVME *vme = cargo->GetInputVme();
	mafString value;
  std::vector<mafTimeStamp> timeStamps;
  mafAbsMatrixPipe *absMatrixPipe;
  long finalTimeStamps = 0;

//   if (vme->IsAnimated())
//   {
//   why only generic?
//   mafVMEGenericAbstract *vmeGeneric = mafVMEGenericAbstract::SafeDownCast(vme);
//   if (vme != NULL)
//   {
//     mafVMEGenericAbstract->GetMatrixTimeStamps(timeStamps);
//     finalTimeStamps = timeStamps.size();
//   }
//   }
//   else
//   {
//     vme->GetAbsTimeStamps(timeStamps);
//     finalTimeStamps = 1;
//   }

  vme->GetAbsTimeStamps(timeStamps);
  absMatrixPipe = vme->GetAbsMatrixPipe();

	long timeCount;
  // It seems that this doesn't work! why?
  // (only one matrix is stored)
	for(timeCount = 0; timeCount < timeStamps.size(); timeCount++)
	{
		absMatrixPipe->SetTimeStamp(timeStamps[timeCount]);
		value << absMatrixPipe->GetMatrix();
	}

	// tag handling code
	cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_MAF_TimeSpace_TimeStampVector);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_MAF_TimeSpace_TimeStampVector::lhpTagHandler_L0000_resource_MAF_TimeSpace_TimeStampVector()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_MAF_TimeSpace_TimeStampVector::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;
  std::vector<mafTimeStamp> timeStamps;
  vme->GetAbsTimeStamps(timeStamps);
  long timeCount;
  for(timeCount = 0; timeCount < timeStamps.size(); timeCount++)
  {
    value << timeStamps[timeCount];
    if(timeCount < timeStamps.size() - 1 )
    {
      value << " ";
    }
  }

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootURI);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootURI::lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootURI()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootURI::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // Bug #2068 fix
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("RootURI"))
  {
    value = vme->GetTagArray()->GetTag("RootURI")->GetValue();
    cargo->SetTagHandlerGeneratedString(value.GetCStr());
  }
  else
  {
    cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
  }
    // tag handling code
  // cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootName);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootName::lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootName()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeRootName::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafString value;
  mafVME *vme = cargo->GetInputVme();
  value = vme->GetRoot()->GetName();
  
  // tag handling code
  cargo->SetTagHandlerGeneratedString(value.GetCStr());
}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeChildURI1);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeChildURI1::lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeChildURI1()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeChildURI1::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_MAF_Procedural);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_MAF_Procedural::lhpTagHandler_L0000_resource_MAF_Procedural()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_MAF_Procedural::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafString value = "false";
  mafVME *vme = cargo->GetInputVme();
  if (vme->GetNumberOfLinks() != 0)
    value = "true";

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value.GetCStr());
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_MAF_Procedural_VMElinkURI1);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_MAF_Procedural_VMElinkURI1::lhpTagHandler_L0000_resource_MAF_Procedural_VMElinkURI1()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_MAF_Procedural_VMElinkURI1::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeTreeCreationDate);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeTreeCreationDate::lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeTreeCreationDate()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_MAF_TreeInfo_VmeTreeCreationDate::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetRoot()->GetTagArray()->IsTagPresent("Creation_Date"))
  {
    value = vme->GetRoot()->GetTagArray()->GetTag("Creation_Date")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Operation);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Operation::lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Operation()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Operation::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  mafAttributeTraceability *trial = (mafAttributeTraceability *)vme->GetAttribute("TrialAttribute");
  if (trial != NULL)
  {
    value = trial->m_TraceabilityVector[0].m_OperationName;
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());
}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_CreationDate);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_CreationDate::lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_CreationDate()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_CreationDate::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  mafAttributeTraceability *trial = (mafAttributeTraceability *)vme->GetAttribute("TrialAttribute");
  if (trial != NULL)
  {
    value = trial->m_TraceabilityVector[0].m_Date;
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Application);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Application::lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Application()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Application::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  mafAttributeTraceability *trial = (mafAttributeTraceability *)vme->GetAttribute("TrialAttribute");
  if (trial != NULL)
  {
    value = trial->m_TraceabilityVector[0].m_AppStamp;
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_IsNatural);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_IsNatural::lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_IsNatural()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_IsNatural::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value = "false";

  mafAttributeTraceability *trial = (mafAttributeTraceability *)vme->GetAttribute("TrialAttribute");
  if (trial != NULL)
  {
    value = trial->m_TraceabilityVector[0].m_IsNatural;
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_OperatorID);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_OperatorID::lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_OperatorID()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_OperatorID::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  mafAttributeTraceability *trial = (mafAttributeTraceability *)vme->GetAttribute("TrialAttribute");
  if (trial != NULL)
  {
    value = trial->m_TraceabilityVector[0].m_OperatorID;
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Parameters);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Parameters::lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Parameters()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_CreateEvent_Parameters::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  mafAttributeTraceability *trial = (mafAttributeTraceability *)vme->GetAttribute("TrialAttribute");
  if (trial != NULL)
  {
     value = trial->m_TraceabilityVector[0].m_Parameters;
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Operation);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Operation::lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Operation()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Operation::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_ModifyDate);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_ModifyDate::lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_ModifyDate()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_ModifyDate::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Application);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Application::lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Application()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Application::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_OperatorID);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_OperatorID::lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_OperatorID()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_OperatorID::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Parameters);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Parameters::lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Parameters()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_ModifyEvent1_Parameters::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_Ownership);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_Ownership::lhpTagHandler_L0000_resource_data_Traceability_Ownership()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_Ownership::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Traceability_Ownership_OwnerID);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Traceability_Ownership_OwnerID::lhpTagHandler_L0000_resource_data_Traceability_Ownership_OwnerID()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Traceability_Ownership_OwnerID::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafString value = cargo->GetInputUser()->GetName();

  // tag handling code
  cargo->SetTagHandlerGeneratedString(value);
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Ownership_OwnerID)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Ownership_OwnerID::lhpTagHandler_L0000_resource_data_Ownership_OwnerID()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Ownership_OwnerID::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Quality_QualityScore1_URL)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Quality_QualityScore1_URL::lhpTagHandler_L0000_resource_data_Quality_QualityScore1_URL()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Quality_QualityScore1_URL::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Quality_QualityScore1_Score)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Quality_QualityScore1_Score::lhpTagHandler_L0000_resource_data_Quality_QualityScore1_Score()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Quality_QualityScore1_Score::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Quality_QualityScore1_ScoreDate)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Quality_QualityScore1_ScoreDate::lhpTagHandler_L0000_resource_data_Quality_QualityScore1_ScoreDate()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Quality_QualityScore1_ScoreDate::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_Access_Policy1_GroupID)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_Access_Policy1_GroupID::lhpTagHandler_L0000_resource_Access_Policy1_GroupID()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_Access_Policy1_GroupID::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_Access_Policy1_Price)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_Access_Policy1_Price::lhpTagHandler_L0000_resource_Access_Policy1_Price()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_Access_Policy1_Price::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_Access_Policy1_Usage)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_Access_Policy1_Usage::lhpTagHandler_L0000_resource_Access_Policy1_Usage()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_Access_Policy1_Usage::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("This tag will be filled during the upload process");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_Access_Publishing_PublishingStatus)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_Access_Publishing_PublishingStatus::lhpTagHandler_L0000_resource_Access_Publishing_PublishingStatus()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_Access_Publishing_PublishingStatus::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("private");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy::lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("https://www.biomedtown.org/biomed_town/LHDL/users/swclient/dictionaries/FA_onto");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryURI);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryURI::lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryURI()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryURI::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("https://www.biomedtown.org/biomed_town/LHDL/users/swclient/dictionaries/FA_onto");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryVersion);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryVersion::lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryVersion()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_DictionaryVersion::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafString dictionaryFileNamePrefix = "lhpXMLFASourceSubdictionary_";
  mafString dictionaryFileName = "NOT FOUND";
  wxString oldDir = wxGetCwd();

  wxSetWorkingDirectory(m_VMEUploaderDownloaderDir.GetCStr());

  wxArrayString files;
  wxString filePattern = dictionaryFileNamePrefix ;
  filePattern.Append("*.xml");

  wxDir::GetAllFiles(wxGetWorkingDirectory(), &files, filePattern, wxDIR_FILES);

  if (files.size() == 0)
  {
    // tag handling code
    cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
  }
  if (files.size() != 1)
  {
    // tag handling code
    cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
  }
  else
  {
    assert(files.size() == 1);
    dictionaryFileName = files[0];
    int pos = dictionaryFileName.FindLast("\\");
    dictionaryFileName.Erase(0, pos);
  }

  wxSetWorkingDirectory(oldDir);

  int pos = dictionaryFileName.FindLast("_");
  dictionaryFileName.Erase(0, pos);
  pos = dictionaryFileName.FindLast(".");
  dictionaryFileName.Erase(pos);
  cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource::lhpTagHandler_L0000_resource_data_Source_DicomSource()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("https://www.biomedtown.org/biomed_town/LHDL/users/swclient/dictionaries/DicomSource");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryURI);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryURI::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryURI()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryURI::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("https://www.biomedtown.org/biomed_town/LHDL/users/swclient/dictionaries/DicomSource");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryVersion);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryVersion::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryVersion()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DictionaryVersion::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafString dictionaryFileNamePrefix = "lhpXMLDicomSourceSubdictionary_";
  mafString dictionaryFileName = "NOT FOUND";
  wxString oldDir = wxGetCwd();

  wxSetWorkingDirectory(m_VMEUploaderDownloaderDir.GetCStr());

  wxArrayString files;
  wxString filePattern = dictionaryFileNamePrefix ;
  filePattern.Append("*.xml");

  wxDir::GetAllFiles(wxGetWorkingDirectory(), &files, filePattern, wxDIR_FILES);

  if (files.size() == 0)
  {
    // tag handling code
    cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
  }
  if (files.size() != 1)
  {
    // tag handling code
    cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
  }
  else
  {
    assert(files.size() == 1);
    dictionaryFileName = files[0];
    int pos = dictionaryFileName.FindLast("\\");
    dictionaryFileName.Erase(0, pos);
  }

  wxSetWorkingDirectory(oldDir);

  int pos = dictionaryFileName.FindLast("_");
  dictionaryFileName.Erase(0, pos);
  pos = dictionaryFileName.FindLast(".");
  dictionaryFileName.Erase(pos);
  cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_MASource)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_MASource::lhpTagHandler_L0000_resource_data_Source_MASource()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_MASource::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("https://www.biomedtown.org/biomed_town/LHDL/users/swclient/dictionaries/MASource");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryURI);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryURI::lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryURI()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryURI::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("https://www.biomedtown.org/biomed_town/LHDL/users/swclient/dictionaries/MASource");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryVersion);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryVersion::lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryVersion()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DictionaryVersion::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafString dictionaryFileNamePrefix = "lhpXMLMotionAnalysisSourceSubdictionary_";
  mafString dictionaryFileName = "NOT FOUND";
  wxString oldDir = wxGetCwd();

  wxSetWorkingDirectory(m_VMEUploaderDownloaderDir.GetCStr());

  wxArrayString files;
  wxString filePattern = dictionaryFileNamePrefix ;
  filePattern.Append("*.xml");

  wxDir::GetAllFiles(wxGetWorkingDirectory(), &files, filePattern, wxDIR_FILES);

  if (files.size() == 0)
  {
    // tag handling code
    cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
  }
  if (files.size() != 1)
  {
    // tag handling code
    cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
  }
  else
  {
    assert(files.size() == 1);
    dictionaryFileName = files[0];
    int pos = dictionaryFileName.FindLast("\\");
    dictionaryFileName.Erase(0, pos);
  }

  wxSetWorkingDirectory(oldDir);

  int pos = dictionaryFileName.FindLast("_");
  dictionaryFileName.Erase(0, pos);
  pos = dictionaryFileName.FindLast(".");
  dictionaryFileName.Erase(pos);
  cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_MASource_MASource_General_SamplingFrequency)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_MASource_MASource_General_SamplingFrequency::lhpTagHandler_L0000_resource_data_Source_MASource_MASource_General_SamplingFrequency()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_MASource_MASource_General_SamplingFrequency::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
 

  if (vme->IsAnimated())
  {
    double freq = 0;
    std::vector<mafTimeStamp> timeStamps;
    vme->GetTimeStamps(timeStamps);
    if (timeStamps.size() > 1 && (timeStamps[1] != timeStamps[0]))
    {
      double dif = timeStamps[1] - timeStamps[0];
      freq = 1/dif;
      freq = mafRoundToPrecision(freq, 0);
    }
    cargo->SetTagHandlerGeneratedString(wxString::Format("%f",freq));
  }
  else
  {
    mafString value;
    cargo->SetTagHandlerGeneratedString(value);
  }
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DataType_IsLandmark_NumberOfLandmarks)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DataType_IsLandmark_NumberOfLandmarks::lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DataType_IsLandmark_NumberOfLandmarks()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_MASource_MASource_DataType_IsLandmark_NumberOfLandmarks::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafString value;
  mafVME *vme = cargo->GetInputVme();
  if (vme->IsA("mafVMELandmarkCloud"))
  {
    value << ((mafVMELandmarkCloud*)vme)->GetNumberOfLandmarks();
  }
  cargo->SetTagHandlerGeneratedString(value);
}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_MicroCTSource)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_MicroCTSource::lhpTagHandler_L0000_resource_data_Source_MicroCTSource()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_MicroCTSource::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("https://www.biomedtown.org/biomed_town/LHDL/users/swclient/dictionaries/MicroCTSource");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryURI);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryURI::lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryURI()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryURI::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("https://www.biomedtown.org/biomed_town/LHDL/users/swclient/dictionaries/MicroCTSource");
}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryVersion);
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryVersion::lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryVersion()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_MicroCTSource_MicroCTSource_DictionaryVersion::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafString dictionaryFileNamePrefix = "lhpXMLMicroCTSourceSubdictionary_";
  mafString dictionaryFileName = "NOT FOUND";
  wxString oldDir = wxGetCwd();

  wxSetWorkingDirectory(m_VMEUploaderDownloaderDir.GetCStr());

  wxArrayString files;
  wxString filePattern = dictionaryFileNamePrefix ;
  filePattern.Append("*.xml");

  wxDir::GetAllFiles(wxGetWorkingDirectory(), &files, filePattern, wxDIR_FILES);

  if (files.size() == 0)
  {
    // tag handling code
    cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
  }
  if (files.size() != 1)
  {
    // tag handling code
    cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
  }
  else
  {
    assert(files.size() == 1);
    dictionaryFileName = files[0];
    int pos = dictionaryFileName.FindLast("\\");
    dictionaryFileName.Erase(0, pos);
  }

  wxSetWorkingDirectory(oldDir);

  int pos = dictionaryFileName.FindLast("_");
  dictionaryFileName.Erase(0, pos);
  pos = dictionaryFileName.FindLast(".");
  dictionaryFileName.Erase(pos);
  cargo->SetTagHandlerGeneratedString(dictionaryFileName.GetCStr());
}



mafCxxTypeMacro(L0000_resource_Documentation)
//------------------------------------------------------------------------------------
L0000_resource_Documentation::L0000_resource_Documentation()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void L0000_resource_Documentation::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}



mafCxxTypeMacro(L0000_resource_Access_Policy1_GroupID)
//------------------------------------------------------------------------------------
L0000_resource_Access_Policy1_GroupID::L0000_resource_Access_Policy1_GroupID()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void L0000_resource_Access_Policy1_GroupID::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}



mafCxxTypeMacro(L0000_resource_Access_Policy1_Price)
//------------------------------------------------------------------------------------
L0000_resource_Access_Policy1_Price::L0000_resource_Access_Policy1_Price()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void L0000_resource_Access_Policy1_Price::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}


mafCxxTypeMacro(L0000_resource_Access_Policy1_Usage)
//------------------------------------------------------------------------------------
L0000_resource_Access_Policy1_Usage::L0000_resource_Access_Policy1_Usage()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void L0000_resource_Access_Policy1_Usage::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}


mafCxxTypeMacro(L0000_resource_Access_Publishing_PublishingStatus)
//------------------------------------------------------------------------------------
L0000_resource_Access_Publishing_PublishingStatus::L0000_resource_Access_Publishing_PublishingStatus()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void L0000_resource_Access_Publishing_PublishingStatus::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  // tag handling code
  cargo->SetTagHandlerGeneratedString("Automated Tag Not Handled (instance exists)");
}

//////////
//DICOM///
//////////
mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyDate)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyDate::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyDate()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyDate::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("StudyDate"))
  {
    value = vme->GetTagArray()->GetTag("StudyDate")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Modality)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Modality::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Modality()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Modality::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("Modality"))
  {
    value = vme->GetTagArray()->GetTag("Modality")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Manufacturer)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Manufacturer::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Manufacturer()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Manufacturer::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("Manufacturer"))
  {
    value = vme->GetTagArray()->GetTag("Manufacturer")->GetValue();
  }


  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_InstitutionName)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_InstitutionName::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_InstitutionName()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_InstitutionName::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("InstitutionName"))
  {
    value = vme->GetTagArray()->GetTag("InstitutionName")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StationName)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StationName::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StationName()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StationName::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("StationName"))
  {
    value = vme->GetTagArray()->GetTag("StationName")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ManufacturerModelName)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ManufacturerModelName::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ManufacturerModelName()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ManufacturerModelName::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("ManufacturerModelName"))
  {
    value = vme->GetTagArray()->GetTag("ManufacturerModelName")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientID)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientID::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientID()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientID::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("PatientID"))
  {
    value = vme->GetTagArray()->GetTag("PatientID")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientSex)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientSex::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientSex()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientSex::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("PatientSex"))
  {
    value = vme->GetTagArray()->GetTag("PatientSex")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ScanOptions)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ScanOptions::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ScanOptions()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ScanOptions::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("ScanOptions"))
  {
    value = vme->GetTagArray()->GetTag("ScanOptions")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_KVP)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_KVP::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_KVP()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_KVP::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("KVP"))
  {
    value = vme->GetTagArray()->GetTag("KVP")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DataCollectionDiameter)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DataCollectionDiameter::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DataCollectionDiameter()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DataCollectionDiameter::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("DataCollectionDiameter"))
  {
    value = vme->GetTagArray()->GetTag("DataCollectionDiameter")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ReconstructionDiameter)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ReconstructionDiameter::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ReconstructionDiameter()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ReconstructionDiameter::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("ReconstructionDiameter"))
  {
    value = vme->GetTagArray()->GetTag("ReconstructionDiameter")->GetValue();
  }
  else

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToDetector)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToDetector::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToDetector()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToDetector::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("DistanceSourceToDetector"))
  {
    value = vme->GetTagArray()->GetTag("DistanceSourceToDetector")->GetValue();
  }
 
  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToPatient)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToPatient::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToPatient()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_DistanceSourceToPatient::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("DistanceSourceToPatient"))
  {
    value = vme->GetTagArray()->GetTag("DistanceSourceToPatient")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_GantryDetectorTilt)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_GantryDetectorTilt::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_GantryDetectorTilt()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_GantryDetectorTilt::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("GantryDetectorTilt"))
  {
    value = vme->GetTagArray()->GetTag("GantryDetectorTilt")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_TableHeight)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_TableHeight::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_TableHeight()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_TableHeight::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("TableHeight"))
  {
    value = vme->GetTagArray()->GetTag("TableHeight")->GetValue();
  }
 
  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RotationDirection)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RotationDirection::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RotationDirection()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RotationDirection::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("RotationDirection"))
  {
    value = vme->GetTagArray()->GetTag("RotationDirection")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ExposureTime)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ExposureTime::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ExposureTime()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ExposureTime::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("ExposureTime"))
  {
    value = vme->GetTagArray()->GetTag("ExposureTime")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_XRayTubeCurrent)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_XRayTubeCurrent::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_XRayTubeCurrent()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_XRayTubeCurrent::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("XrayTubeCurrent"))
  {
    value = vme->GetTagArray()->GetTag("XrayTubeCurrent")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Exposure)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Exposure::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Exposure()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_Exposure::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("Exposure"))
  {
    value = vme->GetTagArray()->GetTag("Exposure")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}


mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FilterType)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FilterType::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FilterType()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FilterType::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("FilterType"))
  {
    value = vme->GetTagArray()->GetTag("FilterType")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FocalSpot)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FocalSpot::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FocalSpot()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_FocalSpot::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("FocalSpot"))
  {
    value = vme->GetTagArray()->GetTag("FocalSpot")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ConvolutionKernel)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ConvolutionKernel::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ConvolutionKernel()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ConvolutionKernel::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("ConvolutionKernel"))
  {
    value = vme->GetTagArray()->GetTag("ConvolutionKernel")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientPosition)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientPosition::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientPosition()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PatientPosition::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("PatientPosition"))
  {
    value = vme->GetTagArray()->GetTag("PatientPosition")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyID)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyID::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyID()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_StudyID::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("StudyID"))
  {
    value = vme->GetTagArray()->GetTag("StudyID")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ImagePositionPatient)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ImagePositionPatient::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ImagePositionPatient()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_ImagePositionPatient::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("ImagePositionPatient"))
  {
    value = vme->GetTagArray()->GetTag("ImagePositionPatient")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelSpacing)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelSpacing::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelSpacing()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelSpacing::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("PixelSpacing"))
  {
    value = vme->GetTagArray()->GetTag("PixelSpacing")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelPaddingValue)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelPaddingValue::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelPaddingValue()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_PixelPaddingValue::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("PixelPaddingValue"))
  {
    value = vme->GetTagArray()->GetTag("PixelPaddingValue")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowCenter)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowCenter::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowCenter()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowCenter::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("WindowCenter"))
  {
    value = vme->GetTagArray()->GetTag("WindowCenter")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowWidth)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowWidth::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowWidth()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_WindowWidth::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("WindowWidth"))
  {
    value = vme->GetTagArray()->GetTag("WindowWidth")->GetValue();
  }
  else
  {
    value = "Not found";
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleIntercept)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleIntercept::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleIntercept()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleIntercept::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("RescaleIntercept"))
  {
    value = vme->GetTagArray()->GetTag("RescaleIntercept")->GetValue();
  }

  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}

mafCxxTypeMacro(lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleSlope)
//------------------------------------------------------------------------------------
lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleSlope::lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleSlope()
//------------------------------------------------------------------------------------
{
  ExtractTagName();
}
//------------------------------------------------------------------------------------
void lhpTagHandler_L0000_resource_data_Source_DicomSource_DicomSource_RescaleSlope::HandleAutoTag(lhpTagHandlerInputOutputParametersCargo *cargo)
//------------------------------------------------------------------------------------
{
  mafVME *vme = cargo->GetInputVme();
  mafString value;

  if(vme->GetTagArray()->IsTagPresent("RescaleSlope"))
  {
    value = vme->GetTagArray()->GetTag("RescaleSlope")->GetValue();
  }


  cargo->SetTagHandlerGeneratedString(value.GetCStr());

}
