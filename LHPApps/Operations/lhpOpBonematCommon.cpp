/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpBonematCommon.cpp,v $
  Language:  C++
  Date:      $Date: 2014-10-07 12:33:41 $
  Version:   $Revision: 1.1.1.1.2.4 $
  Authors:   Gianluigi Crimi
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

#include "lhpOpBonematCommon.h"
#include "lhpProceduralElements.h"
#include "lhpDefines.h"

#include "wx/busyinfo.h"
#include "mafGUI.h"

#include "mafDecl.h"
#include "mafVMERoot.h"
#include "mafVMEMesh.h"
#include "mafString.h"
#include "mafAbsMatrixPipe.h"
#include "mafNodeIterator.h"
#include "mafGUIRollOut.h"

#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <math.h>
#include <stdlib.h>

#include "vtkMAFSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkRectilinearGrid.h"
#include "vtkImageData.h"
#include "vtkDoubleArray.h"
#include "vtkDoubleArray.h"
#include "vtkGenericCell.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkIntArray.h"
#include "vtkFloatArray.h"
#include "vtkTransformFilter.h"
#include "vtkTransform.h"

#include "mmuDOMTreeErrorReporter.h"
#include "mafXMLElement.h"
#include "mmuXMLDOMElement.h"
#include "mafXMLString.h"

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/sax/ErrorHandler.hpp>

// #define _DEBUG_BONEMAT
// #define _DEBUG_BONEMAT_GUI
// #define _USE_MAXIMUM_DENSITY_FOR_GROUPING

//----------------------------------------------------------------------------
// constants
//----------------------------------------------------------------------------
const bool DEBUG_MODE = false;
const int decimalNumbersNumber = 16;

enum BONEMAT_ID
{
  ID_FIRST = MINID,
  ID_OPEN_CONFIGURATION_FILE,
  ID_SAVE_CONFIGURATION_FILE,
  ID_SAVE_CONFIGURATION_FILE_AS,
  ID_OPEN_INPUT_MESH,
  ID_OPEN_INPUT_TAC,
  ID_VOLUME_CHOOSE,
  ID_OUTPUT_MESH_NAME,
  ID_OUTPUT_FREQUENCY_FILE_NAME,
  ID_OK,

  ID_FIRST_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_0,
  ID_FIRST_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_1,
  ID_FIRST_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_2,

  ID_SECOND_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_0,
  ID_SECOND_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_1,
  ID_SECOND_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_2,

  ID_THIRD_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_0,
  ID_THIRD_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_1,
  ID_THIRD_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_2,
  
  ID_HU_THRESHOLD,

  ID_CALIBRATION_FIRST_POINT,
  ID_CALIBRATION_SECOND_POINT,
  ID_STEPS_NUMBER,
  ID_GAP_VALUE,
  ID_DENSITY_INTERVAL_0,
  ID_DENSITY_INTERVAL_1,
  
  ID_EXPONENTIAL_COEFFICIENTS_VECTOR_V2_0,
  ID_EXPONENTIAL_COEFFICIENTS_VECTOR_V2_1,
  ID_EXPONENTIAL_COEFFICIENTS_VECTOR_V2_2,

  ID_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_SINGLE_DENSITY_INTERVAL_0,
  ID_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_SINGLE_DENSITY_INTERVAL_1,
  ID_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_SINGLE_DENSITY_INTERVAL_2,

  ID_RO_INTERCEPT,
  ID_RO_SLOPE,
  ID_YOUNG_MODULE_CALCULATION_MODALITY,
  ID_RHOASH_DENSITY_INTERVALS_NUMBER, 

  //correction section
  ID_FLAG_RO_CORRECTION,
  ID_TYPE_RHOQCT_CORRECTION,

  ID_RO_CORRECTION_DENSITY_INTERVAL_0,
  ID_RO_CORRECTION_DENSITY_INTERVAL_1,

  //possible single interval
  ID_RO_CORRECTION_FIRST_EXPONENTIAL_COEFFICIENTS_SINGLE_0,
  ID_RO_CORRECTION_FIRST_EXPONENTIAL_COEFFICIENTS_SINGLE_1,

  //three intervals

  ID_RO_CORRECTION_FIRST_EXPONENTIAL_COEFFICIENTS_VECTOR_0,
  ID_RO_CORRECTION_FIRST_EXPONENTIAL_COEFFICIENTS_VECTOR_1,

  ID_RO_CORRECTION_SECOND_EXPONENTIAL_COEFFICIENTS_VECTOR_0,
  ID_RO_CORRECTION_SECOND_EXPONENTIAL_COEFFICIENTS_VECTOR_1,

  ID_RO_CORRECTION_THIRD_EXPONENTIAL_COEFFICIENTS_VECTOR_0,
  ID_RO_CORRECTION_THIRD_EXPONENTIAL_COEFFICIENTS_VECTOR_1,

  ID_PRINT_DEBUG_INFO,

  ID_CALIBRATION_ONE_INTERVAL_ROLLOUT,
  ID_CALIBRATION_THREE_INTERVALS_ROLLOUT,
  ID_DENSITY_ONE_INTERVAL_ROLLOUT,
  ID_DENSITY_THREE_INTERVALS_ROLLOUT,

  // Advanced Configuration

  ID_ADVANCED_CONFIGURATION,
  ID_ADVANCED_CONFIGURATION_ROLLOUT,
  ID_RHO_SELECTION,
  ID_DENSITY_SELECTION,
  ID_MIN_ELASTICITY,
  ID_POISSON_RATIO,
};

//----------------------------------------------------------------------------
lhpOpBonematCommon::lhpOpBonematCommon(wxString label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_OP;
  m_Canundo = false;

  m_ConfigurationFileName = "";
  m_FrequencyFileName = "";
  	
  m_Calibration.rhoIntercept = 0;
  m_Calibration.rhoSlope = 1;

  m_Calibration.a_RhoAshLessThanRhoAsh1 = 0;
  m_Calibration.b_RhoAshLessThanRhoAsh1 = 1;
  m_Calibration.c_RhoAshLessThanRhoAsh1 = 1;

  m_Calibration.a_RhoAshBetweenRhoAsh1andRhoAsh2 = 0;
  m_Calibration.b_RhoAshBetweenRhoAsh1andRhoAsh2 = 1;
  m_Calibration.c_RhoAshBetweenRhoAsh1andRhoAsh2 = 1;

  m_Calibration.a_RhoAshBiggerThanRhoAsh2 = 0;
  m_Calibration.b_RhoAshBiggerThanRhoAsh2 = 1;
  m_Calibration.c_RhoAshBiggerThanRhoAsh2 = 1;
  
  m_Egap = 50;
  m_StepsNumber = 4;
  m_Calibration.rhoAsh1 = m_Calibration.rhoAsh2 = 0;

  m_Calibration.densityIntervalsNumber = SINGLE_INTERVAL;

  m_Calibration.a_OneInterval = 0;
  m_Calibration.b_OneInterval = 1;
  m_Calibration.c_OneInterval = 1;

  m_YoungModuleCalculationModality = HU_INTEGRATION;

  m_InputVolume = NULL;
  m_OutputMesh = NULL;

  //Rho Calibration Flag
  m_Calibration.rhoCalibrationCorrectionIsActive = 0;
  m_Calibration.rhoCalibrationCorrectionType     = 0; //equals to single interval

  m_Calibration.rhoQCT1 = 0;
  m_Calibration.rhoQCT2 = 0;

  //single interval rho calibration
  m_Calibration.a_CalibrationCorrection = 0;
  m_Calibration.b_CalibrationCorrection = 1;

  //three intervals rho calibration
  m_Calibration.a_RhoQCTLessThanRhoQCT1 = 0;
  m_Calibration.b_RhoQCTLessThanRhoQCT1 = 1;

  m_Calibration.a_RhoQCTBetweenRhoQCT1AndRhoQCT2 = 0;
  m_Calibration.b_RhoQCTBetweenRhoQCT1AndRhoQCT2 = 1;

  m_Calibration.a_RhoQCTBiggerThanRhoQCT2 = 0;
  m_Calibration.b_RhoQCTBiggerThanRhoQCT2 = 1;

  // Advanced Configuration
  m_RhoSelection = USE_RHO_QCT;
  m_DensitySelection = USE_MEAN_DENSISTY;
  m_PoissonRatio = 0.3;
  m_Calibration.minElasticity = 1e-6;

  m_BusyInfo = NULL;
  m_Gui=NULL;
}
//----------------------------------------------------------------------------
lhpOpBonematCommon::~lhpOpBonematCommon()
//----------------------------------------------------------------------------
{
  
}
//----------------------------------------------------------------------------
bool lhpOpBonematCommon::Accept(mafNode *node)
//----------------------------------------------------------------------------
{ 
  return (node && mafVMEMesh::SafeDownCast(node));
}
//----------------------------------------------------------------------------
void lhpOpBonematCommon::CreateGui()
//----------------------------------------------------------------------------
{
  m_Gui = new mafGUI(this);
  m_Gui->Label("", true);

  //////////////////////////////////////////////////////////////////////////
  // Configuration
  m_Gui->Label("Configuration File:", true);
  m_Gui->Button(ID_OPEN_CONFIGURATION_FILE, "open configuration file");
  m_Gui->Button(ID_SAVE_CONFIGURATION_FILE, "save configuration file");
  m_Gui->Enable(ID_SAVE_CONFIGURATION_FILE, false); 
  m_Gui->Button(ID_SAVE_CONFIGURATION_FILE_AS, "save configuration file as");
  m_Gui->Divider(2);
 
  //////////////////////////////////////////////////////////////////////////
  m_Gui->Label("CT densitometric calibration", true);
  m_Gui->Label("RhoQCT = a + b * HU", false);
  
  m_Gui->Double(ID_RO_INTERCEPT, "a", &m_Calibration.rhoIntercept);
  m_Gui->Double(ID_RO_SLOPE, "b", &m_Calibration.rhoSlope)   ;
  m_Gui->Divider(2);
  
  m_Gui->Enable(ID_RO_INTERCEPT, true);
  m_Gui->Enable(ID_RO_SLOPE, true);

  //////////////////////////////////////////////////////////////////////////
  // Calibration

  m_Gui->Label("Correction of the calibration",true);
  m_Gui->Label("RhoAsh = a + b * RhoQCT",false);

  m_Gui->Bool(ID_FLAG_RO_CORRECTION, "Apply calibration correction", &m_Calibration.rhoCalibrationCorrectionIsActive,1);

  const wxString densityChoicesRoCalibration[] = {"Single interval", "Three intervals"};
  m_Gui->Combo(ID_TYPE_RHOQCT_CORRECTION,"", &m_Calibration.rhoCalibrationCorrectionType,2,densityChoicesRoCalibration);  
  m_Gui->Divider();

  m_GuiASCalibrationOneInterval = new mafGUI(this);
  
  m_GuiASCalibrationOneInterval->Double(ID_RO_CORRECTION_FIRST_EXPONENTIAL_COEFFICIENTS_SINGLE_0, "a", &m_Calibration.a_CalibrationCorrection);
  m_GuiASCalibrationOneInterval->Double(ID_RO_CORRECTION_FIRST_EXPONENTIAL_COEFFICIENTS_SINGLE_1, "b", &m_Calibration.b_CalibrationCorrection);

  m_GuiRollOutCalibrationOneInterval = m_Gui->RollOut(ID_CALIBRATION_ONE_INTERVAL_ROLLOUT,_("Single interval"),m_GuiASCalibrationOneInterval);
  
  m_Gui->Divider();

  m_GuiASCalibrationThreeIntervals = new mafGUI(this);

  m_GuiASCalibrationThreeIntervals->Double(ID_RO_CORRECTION_DENSITY_INTERVAL_0, "RhoQCT1",&m_Calibration.rhoQCT1);
  m_GuiASCalibrationThreeIntervals->Double(ID_RO_CORRECTION_DENSITY_INTERVAL_1, "RhoQCT2",&m_Calibration.rhoQCT2);
  
  m_GuiASCalibrationThreeIntervals->Divider();

  m_GuiASCalibrationThreeIntervals->Label("RhoQCT < RhoQCT1");
  m_GuiASCalibrationThreeIntervals->Double(ID_RO_CORRECTION_FIRST_EXPONENTIAL_COEFFICIENTS_VECTOR_0, "a", &m_Calibration.a_RhoQCTLessThanRhoQCT1);
  m_GuiASCalibrationThreeIntervals->Double(ID_RO_CORRECTION_FIRST_EXPONENTIAL_COEFFICIENTS_VECTOR_1, "b", &m_Calibration.b_RhoQCTLessThanRhoQCT1);
  
  m_GuiASCalibrationThreeIntervals->Label("RhoQCT1 <= RhoQCT <= RhoQCT2");
  m_GuiASCalibrationThreeIntervals->Double(ID_RO_CORRECTION_SECOND_EXPONENTIAL_COEFFICIENTS_VECTOR_0, "a", &m_Calibration.a_RhoQCTBetweenRhoQCT1AndRhoQCT2);
  m_GuiASCalibrationThreeIntervals->Double(ID_RO_CORRECTION_SECOND_EXPONENTIAL_COEFFICIENTS_VECTOR_1, "b", &m_Calibration.b_RhoQCTBetweenRhoQCT1AndRhoQCT2);
  
  m_GuiASCalibrationThreeIntervals->Label("RhoQCT > RhoQCT2");
  m_GuiASCalibrationThreeIntervals->Double(ID_RO_CORRECTION_THIRD_EXPONENTIAL_COEFFICIENTS_VECTOR_0, "a", &m_Calibration.a_RhoQCTBiggerThanRhoQCT2);
  m_GuiASCalibrationThreeIntervals->Double(ID_RO_CORRECTION_THIRD_EXPONENTIAL_COEFFICIENTS_VECTOR_1, "b", &m_Calibration.b_RhoQCTBiggerThanRhoQCT2);
  
  m_GuiRollOutCalibrationThreeIntervals = m_Gui->RollOut(ID_CALIBRATION_THREE_INTERVALS_ROLLOUT,_("Three intervals"),m_GuiASCalibrationThreeIntervals);
  
  m_Gui->Divider(2);

  //////////////////////////////////////////////////////////////////////////
  // Density - Elasticity

  m_Gui->Label("Density-elasticity relationship", true);
  m_Gui->Label("E = a + b * RhoAsh^c", false);
  
  m_Gui->Label("Minimum Elasticity Modulus", false);
  m_Gui->Double(ID_MIN_ELASTICITY, "", &m_Calibration.minElasticity);
  m_Gui->Divider();
  m_Gui->Divider();
  m_Gui->Divider();

  const wxString densityChoices[] = {"Single interval", "Three intervals"};
  m_Gui->Combo(ID_RHOASH_DENSITY_INTERVALS_NUMBER,"", &m_Calibration.densityIntervalsNumber,2,densityChoices);  
  
  m_GuiASDensityOneInterval = new mafGUI(this);

  m_GuiASDensityOneInterval->Double(ID_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_SINGLE_DENSITY_INTERVAL_0, "a", &m_Calibration.a_OneInterval);
  m_GuiASDensityOneInterval->Double(ID_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_SINGLE_DENSITY_INTERVAL_1, "b", &m_Calibration.b_OneInterval);
  m_GuiASDensityOneInterval->Double(ID_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_SINGLE_DENSITY_INTERVAL_2, "c", &m_Calibration.c_OneInterval);

  m_GuiRollOutDensityOneInterval = m_Gui->RollOut(ID_DENSITY_ONE_INTERVAL_ROLLOUT,_("Single interval"),m_GuiASDensityOneInterval);
  
  m_Gui->Divider();

  m_GuiASDensityThreeIntervals = new mafGUI(this);

  m_GuiASDensityThreeIntervals->Double(ID_DENSITY_INTERVAL_0, "RhoAsh1",&m_Calibration.rhoAsh1);
  m_GuiASDensityThreeIntervals->Double(ID_DENSITY_INTERVAL_1, "RhoAsh2",&m_Calibration.rhoAsh2);

  m_GuiASDensityThreeIntervals->Label("RhoAsh < RhoAsh1");
  m_GuiASDensityThreeIntervals->Double(ID_FIRST_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_0, "a", &m_Calibration.a_RhoAshLessThanRhoAsh1);
  m_GuiASDensityThreeIntervals->Double(ID_FIRST_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_1, "b", &m_Calibration.b_RhoAshLessThanRhoAsh1);
  m_GuiASDensityThreeIntervals->Double(ID_FIRST_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_2, "c", &m_Calibration.c_RhoAshLessThanRhoAsh1);
    
  m_GuiASDensityThreeIntervals->Label("RhoAsh1 <= RhoAsh <= RhoAsh2");
  m_GuiASDensityThreeIntervals->Double(ID_SECOND_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_0, "a", &m_Calibration.a_RhoAshBetweenRhoAsh1andRhoAsh2);
  m_GuiASDensityThreeIntervals->Double(ID_SECOND_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_1, "b", &m_Calibration.b_RhoAshBetweenRhoAsh1andRhoAsh2);
  m_GuiASDensityThreeIntervals->Double(ID_SECOND_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_2, "c", &m_Calibration.c_RhoAshBetweenRhoAsh1andRhoAsh2);

  m_GuiASDensityThreeIntervals->Label("RhoAsh > RhoAsh2");
  m_GuiASDensityThreeIntervals->Double(ID_THIRD_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_0, "a", &m_Calibration.a_RhoAshBiggerThanRhoAsh2);
  m_GuiASDensityThreeIntervals->Double(ID_THIRD_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_1, "b", &m_Calibration.b_RhoAshBiggerThanRhoAsh2);
  m_GuiASDensityThreeIntervals->Double(ID_THIRD_EXPONENTIAL_COEFFICIENTS_VECTOR_V3_2, "c", &m_Calibration.c_RhoAshBiggerThanRhoAsh2);
    
  m_GuiASDensityThreeIntervals->Divider(2);

  //   EnableTwoIntervals(true);
  
  m_GuiASDensityThreeIntervals->Enable(ID_DENSITY_INTERVAL_0, true);
  m_GuiASDensityThreeIntervals->Enable(ID_DENSITY_INTERVAL_1, true);

  m_GuiASDensityThreeIntervals->Enable(ID_RHOASH_DENSITY_INTERVALS_NUMBER, true);
  
  m_GuiRollOutDensityThreeIntervals = m_Gui->RollOut(ID_DENSITY_THREE_INTERVALS_ROLLOUT,_("Three intervals"),m_GuiASDensityThreeIntervals);
  
  /*m_Gui->Label("second coefficients vector");
  m_Gui->Double(ID_SECOND_EXPONENTIAL_COEFFICIENTS_VECTOR_V3, "", m_Ea1_Eb1_Ec1_V3);
  m_Gui->Label("hu threshold");
  m_Gui->Double(::ID_HU_THRESHOLD, "", &m_HUThreshold);*/

  //////////////////////////////////////////////////////////////////////////

  m_Gui->Divider(); 
  const wxString choices[] = {"HU integration", "E integration"};
  m_Gui->Label("Young's modulus ( E ) calculation modality","",TRUE);
  m_Gui->Combo(ID_YOUNG_MODULE_CALCULATION_MODALITY, "", &m_YoungModuleCalculationModality, 2, choices);
  m_Gui->Label("Integration steps");
  m_Gui->Integer(ID_STEPS_NUMBER, "", &m_StepsNumber);
  m_Gui->Label("Gap value");
  m_Gui->Double(::ID_GAP_VALUE, "", &m_Egap);
  m_Gui->Divider();

  //////////////////////////////////////////////////////////////////////////
  // Advanced 

  m_Gui->Divider(2);

  m_GuiASAdvancedConfig = new mafGUI(this);

  const wxString choices2[] = {"Use rhoQCT", "Use rhoAsh"};
  m_GuiASAdvancedConfig->Label("Density Output","",TRUE);
  m_GuiASAdvancedConfig->Combo(ID_RHO_SELECTION, "", &m_RhoSelection, 2, choices2);

  const wxString choices3[] = {"Mean", "Maximum"};
  m_GuiASAdvancedConfig->Label("Grouping Density","",TRUE);
  m_GuiASAdvancedConfig->Combo(ID_DENSITY_SELECTION, "", &m_DensitySelection, 2, choices3);

  m_GuiASAdvancedConfig->Label("Poisson's Ratio", TRUE);
  m_GuiASAdvancedConfig->Double(ID_POISSON_RATIO, "", &m_PoissonRatio);

  // Frequency

  m_GuiASAdvancedConfig->Label("Output Frequency file: ",true);
 
  mafString wildc = "Frequency File (*.*)|*.*";
  m_FrequencyFileName = mafGetApplicationDirectory().c_str();
  m_FrequencyFileName +=  "\\" ;
  m_FrequencyFileName +=  m_Input->GetName();
  m_FrequencyFileName +=  "-Freq.txt";

  m_GuiASAdvancedConfig->FileSave(ID_OUTPUT_FREQUENCY_FILE_NAME, _("Freq file"), &m_FrequencyFileName, wildc.GetCStr());

  //
  m_GuiASAdvancedConfig->Enable(ID_ADVANCED_CONFIGURATION, true);
  m_GuiRollOutAdvancedConfig = m_Gui->RollOut(ID_ADVANCED_CONFIGURATION_ROLLOUT,_("Advanced Configuration"),m_GuiASAdvancedConfig);


  m_Gui->Divider(2);
  //////////////////////////////////////////////////////////////////////////
  m_Gui->Label("");

  m_Gui->OkCancel();
  m_Gui->Label("");
  m_Gui->Label("");

  m_Gui->FitGui();
  m_Gui->Update();
}
//---------------------------------------------------------------------------
void lhpOpBonematCommon::InitProgressBar(wxString label ="")
//----------------------------------------------------------------------------
{
  m_OperationProgress = 0;

  if (GetTestMode() == false)
  {
    m_BusyInfo = new wxBusyInfo(label);
    mafEventMacro(mafEvent(this,PROGRESSBAR_SHOW));
  }
  else
  {
    printf("\n" + label + "\n");
    printf("%c", 179);
  }
}
//---------------------------------------------------------------------------
void lhpOpBonematCommon::CloseProgressBar()
//----------------------------------------------------------------------------
{
  if (GetTestMode() == false)
  {
    cppDEL(m_BusyInfo);
    mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));
  }
  else
  {
    printf("%c\n", 179);
  }

  m_OperationProgress = 0;
}
//---------------------------------------------------------------------------
void lhpOpBonematCommon::UpdateProgressBar(long progress)
//----------------------------------------------------------------------------
{
	if (GetTestMode() == false)
	{
		if(progress != m_OperationProgress)
		{
			m_OperationProgress = progress;
			mafEventMacro(mafEvent(this,PROGRESSBAR_SET_VALUE,progress));
		}
	}
	else
	{
		while(progress - m_OperationProgress > 2)
		{
			m_OperationProgress += 2;
			printf("%c", 177);
		}
	}
}

//----------------------------------------------------------------------------
void lhpOpBonematCommon::OpRun()   
//----------------------------------------------------------------------------
{
  CreateGui();
  VolumeSelection();

  if (m_InputVolume)
  {
	  ShowGui();
		m_GuiRollOutAdvancedConfig->RollOut(false);
    UpdateGuiConfiguration();
  }
  else 
    OpStop(OP_RUN_CANCEL);
}
//----------------------------------------------------------------------------
void lhpOpBonematCommon::OpStop(int result)
//----------------------------------------------------------------------------
{
  if (result==OP_RUN_CANCEL && m_OutputMesh)
		m_OutputMesh->ReparentTo(NULL);
	
	mafDEL(m_OutputMesh);
  
  if(m_Gui)
	  HideGui();

	mafEventMacro(mafEvent(this,result));        
}
//----------------------------------------------------------------------------
void lhpOpBonematCommon::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
    case wxOK:
      {
        int result = Execute();

        if(result == MAF_OK)
        {
          mafEventMacro(mafEvent(this,VME_SELECT, m_OutputMesh, true));
          mafEventMacro(mafEvent(this,VME_SHOW, m_OutputMesh, true));
          OpStop(OP_RUN_OK);
        }
        else
        {
          OpStop(OP_RUN_CANCEL);
        }
      }
      break;
    case wxCANCEL:
      OpStop(OP_RUN_CANCEL);
      break;

    case ID_OPEN_CONFIGURATION_FILE:
      {
        OpenConfigurationFile();
      }  
      break;

    case ID_SAVE_CONFIGURATION_FILE:
      {
        SaveConfigurationFile(m_ConfigurationFileName.GetCStr());
      }
      break;

    case ID_SAVE_CONFIGURATION_FILE_AS	:
      {
        SaveConfigurationFileAs();
      }
      break;

    case ID_RHOASH_DENSITY_INTERVALS_NUMBER:
    case ID_FLAG_RO_CORRECTION:
    case ID_TYPE_RHOQCT_CORRECTION:
      {
        UpdateGuiConfiguration();
      }
      break;
    case ID_YOUNG_MODULE_CALCULATION_MODALITY	:
      {
        if (m_YoungModuleCalculationModality == HU_INTEGRATION)
        {            
          // DisableRhoAshThreeIntervals();
        } 
        else if (m_YoungModuleCalculationModality == YOUNG_MODULE_INTEGRATION )
        {
          // UpdateRhoAshGuiOnRhoAshIntervalsNumberChange();
        }
      }
      break;
    case VME_ADD:
      {
        //trap the VME_ADD of the mmoCollapse and mmoExplode to update the
        //m_Input, then forward the message to mafDMLlogicMDI
        this->m_Input = e->GetVme();
        mafEventMacro(mafEvent(this,VME_ADD,this->m_Input));
      }
      break;
    default:
      mafEventMacro(*e);
      break;
    }
  }
}

//----------------------------------------------------------------------------
const char* lhpOpBonematCommon::GetConfigurationFileName()
//----------------------------------------------------------------------------
{
  return m_ConfigurationFileName.GetCStr();
}
//----------------------------------------------------------------------------
void lhpOpBonematCommon::SetConfigurationFileName(const char* name)
//----------------------------------------------------------------------------
{
  m_ConfigurationFileName = name;  
}

//----------------------------------------------------------------------------
int lhpOpBonematCommon::OpenConfigurationFile()
//----------------------------------------------------------------------------
{
  mafString wildcconf = "configuration xml file (*.xml)|*.xml|configuration file (*.conf)|*.conf";

  std::string initial = mafGetApplicationDirectory().c_str();

  std::string returnString = mafGetOpenFile("", wildcconf);

  if (returnString == "")
  {
    return 1;
  }

  m_ConfigurationFileName = returnString.c_str();  

  int result = LoadConfigurationFile(m_ConfigurationFileName.GetCStr());

  m_Gui->Enable(ID_SAVE_CONFIGURATION_FILE,true);
  OnEvent(&mafEvent(this,ID_FLAG_RO_CORRECTION));

  m_Gui->Update();

  return result;
}
//---------------------------------------------------------------------------
int lhpOpBonematCommon::LoadConfigurationFile(const char *configurationFileName)
//----------------------------------------------------------------------------
{
  int result = MAF_ERROR;

  if(mafString(configurationFileName).EndsWith(".conf"))
    result = LoadConfigurationTxtFile(configurationFileName);
  else
    result = LoadConfigurationXmlFile(configurationFileName);

  return result;
}
//---------------------------------------------------------------------------
int lhpOpBonematCommon::LoadConfigurationTxtFile(const char *configurationFileName)
//----------------------------------------------------------------------------
{
  std::ifstream inputFile(configurationFileName, std::ios::in);

  if (inputFile.fail()) {
    std::cerr << "Error opening " << configurationFileName << "\n";
    assert(false);
    return MAF_ERROR;
  }

  mafString LoadConfigurationFileCacheFileName = configurationFileName;
  LoadConfigurationFileCacheFileName.Append(".Load.cache");

  std::ofstream LoadConfigurationFileCache(LoadConfigurationFileCacheFileName.GetCStr(), std::ios::out);

  if (LoadConfigurationFileCache.fail())
  {
    if (GetTestMode() == false)
    {

      wxMessageBox("Error creating configuration file cache");
    }
    assert(false);
    return MAF_ERROR;
  }

  
  std::string buf;
  while(getline(inputFile, buf)) 
  {
    // Find marker at start of line:finish
    size_t pos = buf.find('#');
    if(pos  == 0) 
    {
      // skip line    
    }
    else
    { 
      LoadConfigurationFileCache << buf << std::endl;
    }
  }

  LoadConfigurationFileCache.close();
  //<< std::endl;

  std::ifstream inputFileFromCache(LoadConfigurationFileCacheFileName.GetCStr(), std::ios::in);

  if (inputFileFromCache.fail()) {
    std::cerr << "Error opening " << LoadConfigurationFileCacheFileName.GetCStr() << "\n";
    assert(false);
    return MAF_ERROR;
  }

  //<< "# #### CT Densitometric Calibration ####" << std::endl;

  //<< "# a (m_ROIntercept) : " << std::endl;
  inputFileFromCache >> m_Calibration.rhoIntercept // << std::endl;
  //   << "# b (m_ROSlope): " // << std::endl;
  >> m_Calibration.rhoSlope// << std::endl;

  // // << std::endl;
  // << "# apply calibration correction (m_ROCalibrationCorrectionIsActive): " // << std::endl;
  >> m_Calibration.rhoCalibrationCorrectionIsActive// << std::endl;

  /*// << std::endl;
  << "# #### Correction of the calibration ####" // << std::endl;

  << "# Intervals Type (m_ROCalibrationCorrectionType) {SINGLE_INTERVAL = 0, THREE_INTERVALS = 1}: "  // << std::endl;*/
  >> m_Calibration.rhoCalibrationCorrectionType// << std::endl;


  // << "# R01 (m_RhoQCT1) : " // << std::endl;
  >> m_Calibration.rhoQCT1// << std::endl;
  // << "# R02 (m_RhoQCT2) : " // << std::endl;
  >> m_Calibration.rhoQCT2// << std::endl;

  // << "# a (m_a_CalibrationCorrection): " // << std::endl;
  >> m_Calibration.a_CalibrationCorrection// << std::endl;
  // << "# b (m_b_CalibrationCorrection): " // << std::endl;
  >> m_Calibration.b_CalibrationCorrection// << std::endl;

  /*// << std::endl;
  << "# R0 < R01" // << std::endl;*/
  // << "# a (m_a_RhoQCTLessThanRhoQCT1): " // << std::endl;
  >> m_Calibration.a_RhoQCTLessThanRhoQCT1 // << std::endl;
  // << "# b (m_b_RhoQCTLessThanRhoQCT1): " // << std::endl;
  >> m_Calibration.b_RhoQCTLessThanRhoQCT1 // << std::endl;

  /*// << std::endl;
  << "# R01 <= R0 <= R02" // << std::endl;  
  << "# a (m_a_RhoQCTBetweenRhoQCT1AndRhoQCT2): " // << std::endl;*/
  >> m_Calibration.a_RhoQCTBetweenRhoQCT1AndRhoQCT2 // << std::endl;
  // << "# b (m_b_RhoQCTBetweenRhoQCT1AndRhoQCT2): " // << std::endl;
  >> m_Calibration.b_RhoQCTBetweenRhoQCT1AndRhoQCT2 // << std::endl;

  // << std::endl;

  /*<< "# R0 > R02" // << std::endl;
  << "# a (m_a_RhoQCTBiggerThanRhoQCT2): " // << std::endl;*/
  >> m_Calibration.a_RhoQCTBiggerThanRhoQCT2// << std::endl;
  // << "# b (m_b_RhoQCTBiggerThanRhoQCT2): " // << std::endl;
  >> m_Calibration.b_RhoQCTBiggerThanRhoQCT2// << std::endl;

  // << std::endl;
  // << "# ####  density-elasticity relationship #### " // << std::endl;

  /*// << std::endl;
  << "# density-intervals for E integration" // << std::endl;
  << "# Density intervals type (m_DensityIntervalsNumber) {SINGLE_INTERVAL = 0,THREE_INTERVALS = 1}: " // << std::endl;*/
  >> m_Calibration.densityIntervalsNumber // << std::endl;

  // << std::endl;
  // << "# R01 (m_RhoAsh1): " // << std::endl;
  >> m_Calibration.rhoAsh1// << std::endl;
  // << "# R02 (m_RhoAsh2): " // << std::endl;
  >> m_Calibration.rhoAsh2// << std::endl;
  // << "# a (m_a_OneInterval): " // << std::endl;
  >> m_Calibration.a_OneInterval// << std::endl;
  // << "# b (m_b_OneInterval): " // << std::endl;
  >> m_Calibration.b_OneInterval// << std::endl;
  // << "# c (m_c_OneInterval): " // << std::endl;
  >> m_Calibration.c_OneInterval// << std::endl;

  /*// << std::endl;
  << "# R0 < R01" // << std::endl;
  << "# a (m_a_RhoAshLessThanRhoAsh1): " // << std::endl;*/
  >> m_Calibration.a_RhoAshLessThanRhoAsh1// << std::endl;
  // << "# b (m_b_RhoAshLessThanRhoAsh1): " // << std::endl;
  >> m_Calibration.b_RhoAshLessThanRhoAsh1// << std::endl;
  // << "# c (m_c_RhoAshLessThanRhoAsh1): " // << std::endl;
  >> m_Calibration.c_RhoAshLessThanRhoAsh1// << std::endl;
  /*// << std::endl;
  << "# R01 <= R0 <= R02" // << std::endl;  
  << "# a (m_a_RhoAshBetweenRhoAsh1andRhoAsh2): " // << std::endl;*/
  >> m_Calibration.a_RhoAshBetweenRhoAsh1andRhoAsh2// << std::endl;
  // << "# b (m_b_RhoAshBetweenRhoAsh1andRhoAsh2): " // << std::endl;
  >> m_Calibration.b_RhoAshBetweenRhoAsh1andRhoAsh2// << std::endl;
  // << "# c (m_c_RhoAshBetweenRhoAsh1andRhoAsh2): " // << std::endl;
  >> m_Calibration.c_RhoAshBetweenRhoAsh1andRhoAsh2// << std::endl;
  /*// << std::endl;
  << "# R0 > R02" // << std::endl;
  << "# a (m_a_RhoAshBiggerThanRhoAsh2): " // << std::endl;*/
  >> m_Calibration.a_RhoAshBiggerThanRhoAsh2// << std::endl;
  // << "# b (m_b_RhoAshBiggerThanRhoAsh2): " // << std::endl;
  >> m_Calibration.b_RhoAshBiggerThanRhoAsh2// << std::endl;
  // << "# c (m_c_RhoAshBiggerThanRhoAsh2): " // << std::endl;
  >> m_Calibration.c_RhoAshBiggerThanRhoAsh2// << std::endl;

  // << std::endl;
  // << std::endl;
  /*<< "# Young`s modulus (E) calculation modality" // << std::endl;
  << "# Modality (m_YoungModuleCalculationModality) {HU_INTEGRATION = 0,YOUNG_MODULE_INTEGRATION = 1}: " // << std::endl;*/
  >> m_YoungModuleCalculationModality // << std::endl;

  // << "# Integration Steps (m_StepsNumber): " // << std::endl;
  >> m_StepsNumber// << std::endl;

  // << std::endl;

  // << "# Gap Value (m_Egap): " // << std::endl;
  >> m_Egap; // << std::endl;

  // << std::endl;

  if(m_YoungModuleCalculationModality == HU_INTEGRATION)
  {
    m_RhoSelection = USE_RHO_ASH;
    m_DensitySelection = USE_MAXIMUM_DENSITY;
  }
  else
  {
    m_RhoSelection = USE_RHO_QCT;
    m_DensitySelection = USE_MEAN_DENSISTY;
  }

	if(!m_TestMode)
		wxMessageBox("This configuration file is obsolete.\nPlease check all parameters and save configuration in new format.","Warning", wxOK|wxICON_WARNING , NULL);

  return MAF_OK;
}
//---------------------------------------------------------------------------
int lhpOpBonematCommon::LoadConfigurationXmlFile(const char *configurationFileName)
//----------------------------------------------------------------------------
{
  //Open the file xml
  try 
  {
    XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::Initialize();
  }
  catch (const XERCES_CPP_NAMESPACE_QUALIFIER XMLException& toCatch) 
  {
    // Do your failure processing here
    return MAF_ERROR;
  }

  XERCES_CPP_NAMESPACE_QUALIFIER XercesDOMParser *XMLParser = new  XERCES_CPP_NAMESPACE_QUALIFIER XercesDOMParser;

  XMLParser->setValidationScheme(XERCES_CPP_NAMESPACE_QUALIFIER XercesDOMParser::Val_Auto);
  XMLParser->setDoNamespaces(false);
  XMLParser->setDoSchema(false);
  XMLParser->setCreateEntityReferenceNodes(false);

  mmuDOMTreeErrorReporter *errReporter = new mmuDOMTreeErrorReporter();
  XMLParser->setErrorHandler(errReporter);

  mafString FileName = configurationFileName;

  try 
  {
    XMLParser->parse(FileName.GetCStr());
    int errorCount = XMLParser->getErrorCount(); 

    if (errorCount != 0)
    {
      // errors while parsing...
      mafErrorMessage("Errors while parsing XML file");
      return MAF_ERROR;
    }

    // extract the root element and wrap inside a mafXMLElement
    XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc = XMLParser->getDocument();
    XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *root = doc->getDocumentElement();
    assert(root);

    if (CheckNodeElement(root,"CONFIGURATION"))
    {
      mafString version = GetElementAttribute(root, "Version");

      //Check Config file version
      if (version != mafString(BONEMAT_COMMON_VERSION))
      {
        mafLogMessage("Wrong Configuration file Version:\n version:%s",version.GetCStr());
        return MAF_ERROR;
      }
    }
    else 
    {
      mafLogMessage("Wrong check root node");
      return MAF_ERROR;
    }

    XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList *typesChildren=root->getChildNodes();

    for (unsigned int i = 0; i<typesChildren->getLength();i++)
    {
      //Reading Type nodes 
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *typeNode=typesChildren->item(i);

      if (CheckNodeElement(typeNode,"CT_DENSITOMETRIC_CALIBRATION"))
      {
        m_Calibration.rhoIntercept = GetDoubleElementAttribute(typeNode, "ROIntercept");
        m_Calibration.rhoSlope = GetDoubleElementAttribute(typeNode, "ROSlope");

        if(GetElementAttribute(typeNode, "ROCalibrationCorrectionIsActive") == "true") 
        {
          m_Calibration.rhoCalibrationCorrectionIsActive = 1;
        }
        else
        {
          m_Calibration.rhoCalibrationCorrectionIsActive = 0;
        }
      }
      else if (CheckNodeElement(typeNode, "CORRECTION_OF_CALIBRATION"))
      {
        if(GetElementAttribute(typeNode, "IntervalsType") == "SINGLE")
        {
          m_Calibration.densityIntervalsNumber = SINGLE_INTERVAL;
        }
        else
        {
          m_Calibration.densityIntervalsNumber = THREE_INTERVALS;
        }

        XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList *calibrationTypesChildren=typeNode->getChildNodes();

        for (unsigned int j = 0; j<calibrationTypesChildren->getLength();j++)
        {
          //Reading Type nodes 
          XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *calibrationTypeNode=calibrationTypesChildren->item(j);

          if (CheckNodeElement(calibrationTypeNode,"CALIBRATION_SINGLE_INTERVAL"))
          {
            m_Calibration.a_CalibrationCorrection = GetDoubleElementAttribute(calibrationTypeNode, "a");
            m_Calibration.b_CalibrationCorrection = GetDoubleElementAttribute(calibrationTypeNode, "b");
          }
          else if (CheckNodeElement(calibrationTypeNode,"CALIBRATION_LIMITS"))
          {
            m_Calibration.rhoQCT1 = GetDoubleElementAttribute(calibrationTypeNode, "RhoQCT1");
            m_Calibration.rhoQCT2 = GetDoubleElementAttribute(calibrationTypeNode, "RhoQCT2");
          }
          else if (CheckNodeElement(calibrationTypeNode,"CALIBRATION_INTERVAL_1"))
          {
            m_Calibration.a_RhoQCTLessThanRhoQCT1 = GetDoubleElementAttribute(calibrationTypeNode, "a");
            m_Calibration.b_RhoQCTLessThanRhoQCT1 = GetDoubleElementAttribute(calibrationTypeNode, "b");
          }
          else if (CheckNodeElement(calibrationTypeNode,"CALIBRATION_INTERVAL_2"))
          {
            m_Calibration.a_RhoQCTBetweenRhoQCT1AndRhoQCT2 = GetDoubleElementAttribute(calibrationTypeNode, "a");
            m_Calibration.b_RhoQCTBetweenRhoQCT1AndRhoQCT2 = GetDoubleElementAttribute(calibrationTypeNode, "b");
          }
          else if (CheckNodeElement(calibrationTypeNode,"CALIBRATION_INTERVAL_3"))
          {
            m_Calibration.a_RhoQCTBiggerThanRhoQCT2 = GetDoubleElementAttribute(calibrationTypeNode, "a");
            m_Calibration.b_RhoQCTBiggerThanRhoQCT2 = GetDoubleElementAttribute(calibrationTypeNode, "b");
          }
        }
      }
      else if (CheckNodeElement(typeNode, "DENSITY_ELASTICITY_RELATIONSHIP"))
      {
        if(GetElementAttribute(typeNode, "IntervalsType") == "SINGLE")
        {
          m_Calibration.densityIntervalsNumber = SINGLE_INTERVAL;
        }
        else
        {
          m_Calibration.densityIntervalsNumber = THREE_INTERVALS;
        }

        m_Calibration.minElasticity = GetDoubleElementAttribute(typeNode, "MinElasticity");

        XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList *densityTypesChildren=typeNode->getChildNodes();

        for (unsigned int j = 0; j<densityTypesChildren->getLength();j++)
        {
          //Reading Type nodes 
          XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *densityTypeNode=densityTypesChildren->item(j);

          if (CheckNodeElement(densityTypeNode,"DENSITY_SINGLE_INTERVAL"))
          {
            m_Calibration.a_OneInterval = GetDoubleElementAttribute(densityTypeNode, "a");
            m_Calibration.b_OneInterval = GetDoubleElementAttribute(densityTypeNode, "b");
            m_Calibration.c_OneInterval = GetDoubleElementAttribute(densityTypeNode, "c");
          }
          else if (CheckNodeElement(densityTypeNode,"DENSITY_LIMITS"))
          {
            m_Calibration.rhoAsh1 = GetDoubleElementAttribute(densityTypeNode, "RhoAsh1");
            m_Calibration.rhoAsh2 = GetDoubleElementAttribute(densityTypeNode, "RhoAsh2");
          }
          else if (CheckNodeElement(densityTypeNode,"DENSITY_INTERVAL_1"))
          {
            m_Calibration.a_RhoAshLessThanRhoAsh1 = GetDoubleElementAttribute(densityTypeNode, "a");
            m_Calibration.b_RhoAshLessThanRhoAsh1 = GetDoubleElementAttribute(densityTypeNode, "b");
            m_Calibration.c_RhoAshLessThanRhoAsh1 = GetDoubleElementAttribute(densityTypeNode, "c");
          }
          else if (CheckNodeElement(densityTypeNode,"DENSITY_INTERVAL_2"))
          {
            m_Calibration.a_RhoAshBetweenRhoAsh1andRhoAsh2 = GetDoubleElementAttribute(densityTypeNode, "a");
            m_Calibration.b_RhoAshBetweenRhoAsh1andRhoAsh2 = GetDoubleElementAttribute(densityTypeNode, "b");
            m_Calibration.c_RhoAshBetweenRhoAsh1andRhoAsh2 = GetDoubleElementAttribute(densityTypeNode, "c");
          }
          else if (CheckNodeElement(densityTypeNode,"DENSITY_INTERVAL_3"))
          {
            m_Calibration.a_RhoAshBiggerThanRhoAsh2 = GetDoubleElementAttribute(densityTypeNode, "a");
            m_Calibration.b_RhoAshBiggerThanRhoAsh2 = GetDoubleElementAttribute(densityTypeNode, "b");
            m_Calibration.b_RhoAshBiggerThanRhoAsh2 = GetDoubleElementAttribute(densityTypeNode, "b");
            m_Calibration.c_RhoAshBiggerThanRhoAsh2 = GetDoubleElementAttribute(densityTypeNode, "c");
          }
        }
      }
      else if (CheckNodeElement(typeNode, "YOUNGMODULE"))
      {
        if(GetElementAttribute(typeNode, "CalculationModality") =="E")
        {
          m_YoungModuleCalculationModality = YOUNG_MODULE_INTEGRATION;
        }
        else
        {
          m_YoungModuleCalculationModality = HU_INTEGRATION;
        }

        m_StepsNumber = GetDoubleElementAttribute(typeNode, "StepsNumber");
      }
      else if (CheckNodeElement(typeNode, "GROUPING"))
      {
        m_Egap = GetDoubleElementAttribute(typeNode, "GapValue");
      }
      else if (CheckNodeElement(typeNode, "ADVANCED"))
      {
        if(GetElementAttribute(typeNode, "RhoUsage") == "rhoQCT")
        {
          m_RhoSelection = USE_RHO_QCT;
        }
        else if(GetElementAttribute(typeNode, "RhoUsage") == "rhoAsh")
        {
          m_RhoSelection = USE_RHO_ASH;
        }

        if(GetElementAttribute(typeNode, "DensitySelection") == "Mean")
        {
          m_DensitySelection = USE_MEAN_DENSISTY;
        }
        else if(GetElementAttribute(typeNode, "DensitySelection") == "Maximum")
        {
          m_DensitySelection = USE_MAXIMUM_DENSITY;
        }

        m_PoissonRatio = GetDoubleElementAttribute(typeNode, "PoissonRatio");
      }
    }
  }	
  catch (const  XERCES_CPP_NAMESPACE_QUALIFIER XMLException& toCatch) 
  {
    return MAF_ERROR;
  }
  catch (const  XERCES_CPP_NAMESPACE_QUALIFIER DOMException& toCatch) 
  {
    return MAF_ERROR;
  }
  catch (...) 
  {
    return MAF_ERROR;
  }

  cppDEL (errReporter);
  delete XMLParser;

  // terminate the XML library
  XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::Terminate();

  mafLogMessage(_("Configuration file Loaded"));

  return MAF_OK;
}
//--------------------------------------------------------------------------

//----------------------------------------------------------------------------
int lhpOpBonematCommon::SaveConfigurationFileAs()
//----------------------------------------------------------------------------
{
  mafString initialFileName;
  initialFileName = mafGetApplicationDirectory().c_str();
  initialFileName.Append("\\newConfigurationFile.xml");

  mafString wildc = "configuration xml file (*.xml)|*.xml";
  mafString newFileName = mafGetSaveFile(initialFileName.GetCStr(), wildc).c_str();

  if (newFileName == "") return MAF_ERROR;

  return SaveConfigurationFile(newFileName.GetCStr());
}
//---------------------------------------------------------------------------
int lhpOpBonematCommon::SaveConfigurationFile(const char *configurationFileName)
//----------------------------------------------------------------------------
{
  //Open the file xml
  try 
  {
    XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::Initialize();
  }
  catch (const XERCES_CPP_NAMESPACE_QUALIFIER XMLException& toCatch) 
  {
    // Do your failure processing here
    return MAF_ERROR;
  }

  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc;
  XMLCh tempStr[100];
  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode("LS", tempStr, 99);
  XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation *impl = XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementationRegistry::getDOMImplementation(tempStr);
  XERCES_CPP_NAMESPACE_QUALIFIER DOMWriter* theSerializer = ((XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementationLS*)impl)->createDOMWriter();
  theSerializer->setNewLine(mafXMLString("\r") );

  if (theSerializer->canSetFeature(XERCES_CPP_NAMESPACE_QUALIFIER XMLUni::fgDOMWRTFormatPrettyPrint, true))
    theSerializer->setFeature(XERCES_CPP_NAMESPACE_QUALIFIER XMLUni::fgDOMWRTFormatPrettyPrint, true);

  doc = impl->createDocument(NULL, mafXMLString("CONFIGURATION"), NULL);

  doc->setEncoding(mafXMLString("UTF-8") );
  doc->setStandalone(true);
  doc->setVersion(mafXMLString("1.0") );

  // extract root element and wrap it with an mafXMLElement object
  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *root = doc->getDocumentElement();
  assert(root);

  // attach version attribute to the root node
  root->setAttribute(mafXMLString("Version"),mafXMLString(mafString(BONEMAT_COMMON_VERSION)));

  // CT_DENSITOMETRIC_CALIBRATION
  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *densitometricCalibrationNode=doc->createElement(mafXMLString("CT_DENSITOMETRIC_CALIBRATION"));
  densitometricCalibrationNode->setAttribute(mafXMLString("ROIntercept"),mafXMLString(mafString(m_Calibration.rhoIntercept)));
  densitometricCalibrationNode->setAttribute(mafXMLString("ROSlope"),mafXMLString(mafString(m_Calibration.rhoSlope)));
  if(m_Calibration.rhoCalibrationCorrectionIsActive)
  {
    densitometricCalibrationNode->setAttribute(mafXMLString("ROCalibrationCorrectionIsActive"),mafXMLString("true"));
  }
  else
  {
    densitometricCalibrationNode->setAttribute(mafXMLString("ROCalibrationCorrectionIsActive"),mafXMLString("false"));
  }
  root->appendChild(densitometricCalibrationNode);

  // CORRECTION_OF_CALIBRATION
  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *correctionCalibrationNode=doc->createElement(mafXMLString("CORRECTION_OF_CALIBRATION"));

  if(m_Calibration.rhoCalibrationCorrectionType == SINGLE_INTERVAL)
  {
    correctionCalibrationNode->setAttribute(mafXMLString("IntervalsType"),mafXMLString("SINGLE"));
  }
  else
  {
    correctionCalibrationNode->setAttribute(mafXMLString("IntervalsType"),mafXMLString("THREE"));
  }

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *calibrationSingleIntervalNode=doc->createElement(mafXMLString("CALIBRATION_SINGLE_INTERVAL"));
  calibrationSingleIntervalNode->setAttribute(mafXMLString("a"),mafXMLString(mafString(m_Calibration.a_CalibrationCorrection)));
  calibrationSingleIntervalNode->setAttribute(mafXMLString("b"),mafXMLString(mafString(m_Calibration.b_CalibrationCorrection)));
  correctionCalibrationNode->appendChild(calibrationSingleIntervalNode);

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *calibrationLimitsNode=doc->createElement(mafXMLString("CALIBRATION_LIMITS"));
  calibrationLimitsNode->setAttribute(mafXMLString("RhoQCT1"),mafXMLString(mafString(m_Calibration.rhoQCT1)));
  calibrationLimitsNode->setAttribute(mafXMLString("RhoQCT2"),mafXMLString(mafString(m_Calibration.rhoQCT2)));
  correctionCalibrationNode->appendChild(calibrationLimitsNode);

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *calibrationInterval1Node=doc->createElement(mafXMLString("CALIBRATION_INTERVAL_1"));
  calibrationInterval1Node->setAttribute(mafXMLString("a"),mafXMLString(mafString(m_Calibration.a_RhoQCTLessThanRhoQCT1)));
  calibrationInterval1Node->setAttribute(mafXMLString("b"),mafXMLString(mafString(m_Calibration.b_RhoQCTLessThanRhoQCT1)));
  correctionCalibrationNode->appendChild(calibrationInterval1Node);

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *calibrationInterval2Node=doc->createElement(mafXMLString("CALIBRATION_INTERVAL_2"));
  calibrationInterval2Node->setAttribute(mafXMLString("a"),mafXMLString(mafString(m_Calibration.a_RhoQCTBetweenRhoQCT1AndRhoQCT2)));
  calibrationInterval2Node->setAttribute(mafXMLString("b"),mafXMLString(mafString(m_Calibration.b_RhoQCTBetweenRhoQCT1AndRhoQCT2)));
  correctionCalibrationNode->appendChild(calibrationInterval2Node);

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *calibrationInterval3Node=doc->createElement(mafXMLString("CALIBRATION_INTERVAL_3"));
  calibrationInterval3Node->setAttribute(mafXMLString("a"),mafXMLString(mafString(m_Calibration.a_RhoQCTBiggerThanRhoQCT2)));
  calibrationInterval3Node->setAttribute(mafXMLString("b"),mafXMLString(mafString(m_Calibration.b_RhoQCTBiggerThanRhoQCT2)));
  correctionCalibrationNode->appendChild(calibrationInterval3Node);

  root->appendChild(correctionCalibrationNode);

  // DENSITY_ELASTICITY_RELATIONSHIP
  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *densityRelationshipNode=doc->createElement(mafXMLString("DENSITY_ELASTICITY_RELATIONSHIP"));

  if(m_Calibration.densityIntervalsNumber == SINGLE_INTERVAL)
  {
    densityRelationshipNode->setAttribute(mafXMLString("IntervalsType"),mafXMLString("SINGLE"));
  }
  else
  {
    densityRelationshipNode->setAttribute(mafXMLString("IntervalsType"),mafXMLString("THREE"));
  }
  
  densityRelationshipNode->setAttribute(mafXMLString("MinElasticity"),mafXMLString(mafString(m_Calibration.minElasticity)));

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *densitySingleIntervalNode=doc->createElement(mafXMLString("DENSITY_SINGLE_INTERVAL"));
  densitySingleIntervalNode->setAttribute(mafXMLString("a"),mafXMLString(mafString(m_Calibration.a_OneInterval)));
  densitySingleIntervalNode->setAttribute(mafXMLString("b"),mafXMLString(mafString(m_Calibration.b_OneInterval)));
  densitySingleIntervalNode->setAttribute(mafXMLString("c"),mafXMLString(mafString(m_Calibration.c_OneInterval)));
  densityRelationshipNode->appendChild(densitySingleIntervalNode);

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *densityLimitsNode=doc->createElement(mafXMLString("DENSITY_LIMITS"));
  densityLimitsNode->setAttribute(mafXMLString("RhoAsh1"),mafXMLString(mafString(m_Calibration.rhoAsh1)));
  densityLimitsNode->setAttribute(mafXMLString("RhoAsh2"),mafXMLString(mafString(m_Calibration.rhoAsh2)));
  densityRelationshipNode->appendChild(densityLimitsNode);

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *densityInterval1Node=doc->createElement(mafXMLString("DENSITY_INTERVAL_1"));
  densityInterval1Node->setAttribute(mafXMLString("a"),mafXMLString(mafString(m_Calibration.a_RhoAshLessThanRhoAsh1)));
  densityInterval1Node->setAttribute(mafXMLString("b"),mafXMLString(mafString(m_Calibration.b_RhoAshLessThanRhoAsh1)));
  densityInterval1Node->setAttribute(mafXMLString("c"),mafXMLString(mafString(m_Calibration.c_RhoAshLessThanRhoAsh1)));
  densityRelationshipNode->appendChild(densityInterval1Node);

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *densityInterval2Node=doc->createElement(mafXMLString("DENSITY_INTERVAL_2"));
  densityInterval2Node->setAttribute(mafXMLString("a"),mafXMLString(mafString(m_Calibration.a_RhoAshBetweenRhoAsh1andRhoAsh2)));
  densityInterval2Node->setAttribute(mafXMLString("b"),mafXMLString(mafString(m_Calibration.b_RhoAshBetweenRhoAsh1andRhoAsh2)));
  densityInterval2Node->setAttribute(mafXMLString("c"),mafXMLString(mafString(m_Calibration.c_RhoAshBetweenRhoAsh1andRhoAsh2)));
  densityRelationshipNode->appendChild(densityInterval2Node);

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *densityInterval3Node=doc->createElement(mafXMLString("DENSITY_INTERVAL_3"));
  densityInterval3Node->setAttribute(mafXMLString("a"),mafXMLString(mafString(m_Calibration.a_RhoAshBiggerThanRhoAsh2)));
  densityInterval3Node->setAttribute(mafXMLString("b"),mafXMLString(mafString(m_Calibration.b_RhoAshBiggerThanRhoAsh2)));
  densityInterval3Node->setAttribute(mafXMLString("c"),mafXMLString(mafString(m_Calibration.c_RhoAshBiggerThanRhoAsh2)));
  densityRelationshipNode->appendChild(densityInterval3Node);

  root->appendChild(densityRelationshipNode);

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *youngModuleNode=doc->createElement(mafXMLString("YOUNGMODULE"));
  if(m_YoungModuleCalculationModality == HU_INTEGRATION)
    youngModuleNode->setAttribute(mafXMLString("CalculationModality"),mafXMLString("HU"));
  else
    youngModuleNode->setAttribute(mafXMLString("CalculationModality"),mafXMLString("E"));

  youngModuleNode->setAttribute(mafXMLString("StepsNumber"),mafXMLString(mafString(m_StepsNumber)));
  root->appendChild(youngModuleNode);

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *groupingNode=doc->createElement(mafXMLString("GROUPING"));
  groupingNode->setAttribute(mafXMLString("GapValue"),mafXMLString(mafString(m_Egap)));
  root->appendChild(groupingNode);

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *advancedConfig=doc->createElement(mafXMLString("ADVANCED"));

  if(m_RhoSelection == USE_RHO_QCT)
  {
    advancedConfig->setAttribute(mafXMLString("RhoUsage"),mafXMLString("rhoQCT"));
  }
  else if(m_RhoSelection == USE_RHO_ASH)
  {
    advancedConfig->setAttribute(mafXMLString("RhoUsage"),mafXMLString("rhoAsh"));
  }

  if(m_DensitySelection == USE_MEAN_DENSISTY)
  {
    advancedConfig->setAttribute(mafXMLString("DensitySelection"),mafXMLString("Mean"));
  }
  else if(m_DensitySelection == USE_MAXIMUM_DENSITY)
  {
    advancedConfig->setAttribute(mafXMLString("DensitySelection"),mafXMLString("Maximum"));
  }

  advancedConfig->setAttribute(mafXMLString("PoissonRatio"),mafXMLString(mafString(m_PoissonRatio)));

  root->appendChild(advancedConfig);
  // 

  XERCES_CPP_NAMESPACE_QUALIFIER XMLFormatTarget *XMLTarget;
  mafString fileName = configurationFileName;

  XMLTarget = new XERCES_CPP_NAMESPACE_QUALIFIER LocalFileFormatTarget(fileName);

  try 
  {
    // do the serialization through DOMWriter::writeNode();
    theSerializer->writeNode(XMLTarget,*doc);
  }
  catch (const XERCES_CPP_NAMESPACE_QUALIFIER  XMLException& toCatch) 
  {
    char* message = XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(toCatch.getMessage());
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&message);
    return MAF_ERROR;
  }
  catch (const XERCES_CPP_NAMESPACE_QUALIFIER DOMException& toCatch) 
  {
    char* message = XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(toCatch.msg);
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&message);
    return MAF_ERROR;
  }
  catch (...) {
    return MAF_ERROR;
  }

  theSerializer->release();
  cppDEL (XMLTarget);
  doc->release();

  XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::Terminate();

  mafLogMessage(wxString::Format("New configuration file has been written %s",fileName.GetCStr()));

  return MAF_OK;
}
//--------------------------------------------------------------------------

//----------------------------------------------------------------------------
const char* lhpOpBonematCommon::GetFrequencyFileName()
//----------------------------------------------------------------------------
{
  return m_FrequencyFileName.GetCStr();
}
//----------------------------------------------------------------------------
void lhpOpBonematCommon::SetFrequencyFileName(const char* name)
//----------------------------------------------------------------------------
{
  m_FrequencyFileName = name;  
}

//---------------------------------------------------------------------------
bool lhpOpBonematCommon::CheckNodeElement(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node, const char *elementName)
//----------------------------------------------------------------------------
{
  //Reading nodes
  if (node->getNodeType()!= XERCES_CPP_NAMESPACE_QUALIFIER DOMNode::ELEMENT_NODE)
    return false;

  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *nodeElement = (XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*)node;
  mafString nameElement = ""; 
  nameElement = mafXMLString(nodeElement->getTagName());

  return (nameElement == elementName);
}
//--------------------------------------------------------------------------
mafString lhpOpBonematCommon::GetElementAttribute(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node, const char *attributeName)
//----------------------------------------------------------------------------
{
  if (node->getNodeType()!= XERCES_CPP_NAMESPACE_QUALIFIER DOMNode::ELEMENT_NODE)
    return "";

  return mafXMLString(((XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *)node)->getAttribute(mafXMLString(attributeName)));
}
//--------------------------------------------------------------------------
double lhpOpBonematCommon::GetDoubleElementAttribute(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node, const char *attributeName)
//----------------------------------------------------------------------------
{
  double val = 0;
  wxString result = wxString(mafXMLString(((XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *)node)->getAttribute(mafXMLString(attributeName))).GetCStr());
  result.ToDouble(&val);

  return val;
}

//---------------------------------------------------------------------------
void lhpOpBonematCommon::VolumeSelection()
//----------------------------------------------------------------------------
{
  m_InputVolume = NULL;

  mafVMERoot *root = mafVMERoot::SafeDownCast(m_Input->GetRoot());

  int numVolume = 0;

  mafNodeIterator *iter = root->NewIterator();

  for (mafNode *Inode = iter->GetFirstNode(); Inode; Inode = iter->GetNextNode())
  {
    if (mafVMEVolumeGray::SafeDownCast(Inode))
    {
      if(m_InputVolume == NULL)
      {
        m_InputVolume = mafVMEVolumeGray::SafeDownCast(Inode);
      }
      numVolume++;
    }
  }

  iter->Delete();

  if(numVolume > 1)
  {
    mafString title = _("Choose Volume");
    mafEvent *e; 
    e = new mafEvent();
    e->SetId(VME_CHOOSE);
    e->SetArg((long)&lhpOpBonematCommon::VolumeAccept);
    e->SetString(&title);

    mafEventMacro(*e);

    mafNode *n = e->GetVme();
    delete e;

    m_InputVolume = mafVMEVolumeGray::SafeDownCast(n);
  }

  if(m_InputVolume)
  {
    m_InputVolume->GetOutput()->GetVTKData()->Update();
    m_InputVolume->Update();
    m_InputVolumeName =  m_InputVolume->GetName();
  }

}
//---------------------------------------------------------------------------
void lhpOpBonematCommon::CreateMeshCopy()
//----------------------------------------------------------------------------
{
  mafString outputName = m_Input->GetName();
  outputName += " Bonemat";
  mafNEW(m_OutputMesh);
  m_OutputMesh->DeepCopy(mafVMEMesh::SafeDownCast(m_Input));
  m_OutputMesh->SetName(outputName);
  m_OutputMesh->ReparentTo(m_Input->GetParent());
  m_OutputMesh->Update();
}
//--------------------------------------------------------------------------
void lhpOpBonematCommon::UpdateGuiConfiguration()
//----------------------------------------------------------------------------
{
  m_Gui->Enable(ID_TYPE_RHOQCT_CORRECTION,m_Calibration.rhoCalibrationCorrectionIsActive?true:false);

  m_GuiRollOutCalibrationOneInterval->RollOut(m_Calibration.rhoCalibrationCorrectionIsActive && m_Calibration.rhoCalibrationCorrectionType == SINGLE_INTERVAL ? true:false);
  m_Gui->Enable(ID_CALIBRATION_ONE_INTERVAL_ROLLOUT, m_Calibration.rhoCalibrationCorrectionIsActive && m_Calibration.rhoCalibrationCorrectionType == SINGLE_INTERVAL ? true:false);

  m_GuiRollOutCalibrationThreeIntervals->RollOut(m_Calibration.rhoCalibrationCorrectionIsActive && m_Calibration.rhoCalibrationCorrectionType == THREE_INTERVALS ? true:false);
  m_Gui->Enable(ID_CALIBRATION_THREE_INTERVALS_ROLLOUT, m_Calibration.rhoCalibrationCorrectionIsActive && m_Calibration.rhoCalibrationCorrectionType == THREE_INTERVALS ? true:false);


  m_GuiRollOutDensityOneInterval->RollOut(m_Calibration.densityIntervalsNumber == SINGLE_INTERVAL ? true:false);
  m_Gui->Enable(ID_DENSITY_ONE_INTERVAL_ROLLOUT, m_Calibration.densityIntervalsNumber == SINGLE_INTERVAL ? true:false);

  m_GuiRollOutDensityThreeIntervals->RollOut(m_Calibration.densityIntervalsNumber == THREE_INTERVALS ? true:false);
  m_Gui->Enable(ID_DENSITY_THREE_INTERVALS_ROLLOUT, m_Calibration.densityIntervalsNumber == THREE_INTERVALS ? true:false);

  m_GuiASCalibrationOneInterval->Update();
  m_GuiASCalibrationThreeIntervals->Update();
  m_GuiASDensityOneInterval->Update();
  m_GuiASDensityThreeIntervals->Update();
  m_GuiASAdvancedConfig->Update();

  m_Gui->FitGui();
  m_Gui->Update();
}
//--------------------------------------------------------------------------
mafNode * lhpOpBonematCommon::GetOutput()
//--------------------------------------------------------------------------
{
	return mafNode::SafeDownCast(m_OutputMesh);
}
