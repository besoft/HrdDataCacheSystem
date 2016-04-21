/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medMSMGraph.cpp,v $
Language:  C++
Date:      $Date: 2012-02-08 07:22:52 $
Version:   $Revision: 1.1.2.5 $
Authors:   Josef Kohout
==========================================================================
Copyright (c) 2011
University of West Bohemia
=========================================================================*/

#include "mafDefines.h"
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "medMSMGraph.h"
#include "mafVMEGroup.h"
#include "mafVMEShortcut.h"
#include "medVMEMuscleWrapper.h"
#include "medVMEWrappedMeter.h"
#include "medVMEComputeWrapping.H"
#include "mafVMELandmarkCloud.h"
#include "mafVMELandmark.h"
#include "mafVMESurfaceParametric.h"
#include "mafTagArray.h"
#include "mafVMEMeter.h"

#pragma region RegionType, BoneType, ...
/*static*/ const char* medMSMGraph::RegionType::TAG_PREFIX =
	"L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_Anatomy_Body_Regions";

/*static*/ const char* medMSMGraph::RegionType::NAMES[] = {
	"Abdomen", "Back", "Head", "LeftFoot",
	"LeftFootDorsal", "LeftFootPlantar", "LeftForeArm", "LeftForeArmAnterior",
	"LeftForeArmLateral", "LeftForeArmPosterior", "LeftHand", "LeftHandAnterior",
	"LeftHandPosterior", "LeftLowerLimb", "LeftPelvis", "LeftShank",
	"LeftShankAnterior", "LeftShankLateral", "LeftShankPosterior", "LeftShoulder",
	"LeftSpine", "LeftThigh", "LeftThighAnterior", "LeftThighMedial",
	"LeftThighPosterior", "LeftUpperArm", "LeftUpperArmAnterior", "LeftUpperArmPosterior",
	"LeftUpperLimb", "Neck", "Pelvis", "RightFoot",
	"RightFootDorsal", "RightFootPlantar", "RightForeArm", "RightForeArmAnterior",
	"RightForeArmLateral", "RightForeArmPosterior", "RightHand", "RightHandAnterior",
	"RightHandPosterior", "RightLowerLimb", "RightPelvis", "RightShank",
	"RightShankAnterior", "RightShankLateral", "RightShankPosterior", "RightShoulder",
	"RightSpine", "RightThigh", "RightThighAnterior", "RightThighMedial",
	"RightThighPosterior", "RightUpperArm", "RightUpperArmAnterior", "RightUpperArmPosterior",
	"RightUpperLimb", "Thorax",
};

/*static*/ const char* medMSMGraph::BoneType::TAG_PREFIX =
	"L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_Anatomy_Body_Organs_Bones";

/*static*/ const char* medMSMGraph::BoneType::NAMES[] = {
	"CervicalVertebrae1", "CervicalVertebrae2", "CervicalVertebrae3", "CervicalVertebrae4",
	"CervicalVertebrae5", "CervicalVertebrae6", "CervicalVertebrae7", "Coccyx",
	"Jaw", "LeftCalcaneus", "LeftCapitate", "LeftClavicle",
	"LeftCostalCartilage1", "LeftCostalCartilage10", "LeftCostalCartilage11", "LeftCostalCartilage12",
	"LeftCostalCartilage2", "LeftCostalCartilage3", "LeftCostalCartilage4", "LeftCostalCartilage5",
	"LeftCostalCartilage6", "LeftCostalCartilage7", "LeftCostalCartilage8", "LeftCostalCartilage9",
	"LeftCuboid", "LeftFemur", "LeftFibula", "LeftFootRay1LateralSesamoid",
	"LeftFootRay1MedialSesamoid", "LeftFootRay1Phalanx1", "LeftFootRay1Phalanx2", "LeftFootRay2Phalanx1",
	"LeftFootRay2Phalanx2", "LeftFootRay2Phalanx3", "LeftFootRay3Phalanx1", "LeftFootRay3Phalanx2",
	"LeftFootRay3Phalanx3", "LeftFootRay4Phalanx1", "LeftFootRay4Phalanx2", "LeftFootRay4Phalanx3",
	"LeftFootRay5Phalanx1", "LeftFootRay5Phalanx2", "LeftFootRay5Phalanx3", "LeftFootScaphoid",
	"LeftHamate", "LeftHandRay1LateralSesamoid", "LeftHandRay1MedialSesamoid", "LeftHandRay1Phalanx1",
	"LeftHandRay1Phalanx2", "LeftHandRay2Phalanx1", "LeftHandRay2Phalanx2", "LeftHandRay2Phalanx3",
	"LeftHandRay3Phalanx1", "LeftHandRay3Phalanx2", "LeftHandRay3Phalanx3", "LeftHandRay4Phalanx1",
	"LeftHandRay4Phalanx2", "LeftHandRay4Phalanx3", "LeftHandRay5Phalanx1", "LeftHandRay5Phalanx2",
	"LeftHandRay5Phalanx3", "LeftHandScaphoid", "LeftHumerus", "LeftIliac",
	"LeftIntermediateCuneiform", "LeftLateralCuneiform", "LeftLunate", "LeftMedialCuneiform",
	"LeftMetacarpal1", "LeftMetacarpal2", "LeftMetacarpal3", "LeftMetacarpal4",
	"LeftMetacarpal5", "LeftMetatarsal1", "LeftMetatarsal2", "LeftMetatarsal3",
	"LeftMetatarsal4", "LeftMetatarsal5", "LeftPatella", "LeftPisiformis",
	"LeftRadius", "LeftRib1", "LeftRib10", "LeftRib11",
	"LeftRib12", "LeftRib2", "LeftRib3", "LeftRib4",
	"LeftRib5", "LeftRib6", "LeftRib7", "LeftRib8",
	"LeftRib9", "LeftScapula", "LeftTalus", "LeftTibia",
	"LeftTrapezium", "LeftTrapezoid", "LeftTriquetrum", "LeftUlna",
	"LumbarVertebrae1", "LumbarVertebrae2", "LumbarVertebrae3", "LumbarVertebrae4",
	"LumbarVertebrae5", "Manubrium", "RightCalcaneus", "RightCapitate",
	"RightClavicle", "RightCostalCartilage1", "RightCostalCartilage10", "RightCostalCartilage11",
	"RightCostalCartilage12", "RightCostalCartilage2", "RightCostalCartilage3", "RightCostalCartilage4",
	"RightCostalCartilage5", "RightCostalCartilage6", "RightCostalCartilage7", "RightCostalCartilage8",
	"RightCostalCartilage9", "RightCuboid", "RightFemur", "RightFibula",
	"RightFootRay1LateralSesamoid", "RightFootRay1MedialSesamoid", "RightFootRay1Phalanx1", "RightFootRay1Phalanx2",
	"RightFootRay2Phalanx1", "RightFootRay2Phalanx2", "RightFootRay2Phalanx3", "RightFootRay3Phalanx1",
	"RightFootRay3Phalanx2", "RightFootRay3Phalanx3", "RightFootRay4Phalanx1", "RightFootRay4Phalanx2",
	"RightFootRay4Phalanx3", "RightFootRay5Phalanx1", "RightFootRay5Phalanx2", "RightFootRay5Phalanx3",
	"RightFootScaphoid", "RightHamate", "RightHandRay1LateralSesamoid", "RightHandRay1MedialSesamoid",
	"RightHandRay1Phalanx1", "RightHandRay1Phalanx2", "RightHandRay2Phalanx1", "RightHandRay2Phalanx2",
	"RightHandRay2Phalanx3", "RightHandRay3Phalanx1", "RightHandRay3Phalanx2", "RightHandRay3Phalanx3",
	"RightHandRay4Phalanx1", "RightHandRay4Phalanx2", "RightHandRay4Phalanx3", "RightHandRay5Phalanx1",
	"RightHandRay5Phalanx2", "RightHandRay5Phalanx3", "RightHandScaphoid", "RightHumerus",
	"RightIliac", "RightIntermediateCuneiform", "RightLateralCuneiform", "RightLunate",
	"RightMedialCuneiform", "RightMetacarpal1", "RightMetacarpal2", "RightMetacarpal3",
	"RightMetacarpal4", "RightMetacarpal5", "RightMetatarsal1", "RightMetatarsal2",
	"RightMetatarsal3", "RightMetatarsal4", "RightMetatarsal5", "RightPatella",
	"RightPisiformis", "RightRadius", "RightRib1", "RightRib10",
	"RightRib11", "RightRib12", "RightRib2", "RightRib3",
	"RightRib4", "RightRib5", "RightRib6", "RightRib7",
	"RightRib8", "RightRib9", "RightScapula", "RightTalus",
	"RightTibia", "RightTrapezium", "RightTrapezoid", "RightTriquetrum",
	"RightUlna", "Sacrum", "Skull", "SternalBody",
	"ThoracicVertebrae1", "ThoracicVertebrae10", "ThoracicVertebrae11", "ThoracicVertebrae12",
	"ThoracicVertebrae2", "ThoracicVertebrae3", "ThoracicVertebrae4", "ThoracicVertebrae5",
	"ThoracicVertebrae6", "ThoracicVertebrae7", "ThoracicVertebrae8", "ThoracicVertebrae9",
	"XiphoidProcess",
};

/*static*/ const char* medMSMGraph::JointType::TAG_PREFIX =
	"L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_Anatomy_Body_Joints";

/*static*/ const char* medMSMGraph::JointType::NAMES[] = {
	"CoccoSacral", "Intercorporeal1", "Intercorporeal10", "Intercorporeal11", 
	"Intercorporeal12", "Intercorporeal13", "Intercorporeal14", "Intercorporeal15", 
	"Intercorporeal16", "Intercorporeal17", "Intercorporeal18", "Intercorporeal19", 
	"Intercorporeal2", "Intercorporeal20", "Intercorporeal21", "Intercorporeal22", 
	"Intercorporeal23", "Intercorporeal24", "Intercorporeal3", "Intercorporeal4", 
	"Intercorporeal5", "Intercorporeal6", "Intercorporeal7", "Intercorporeal8", 
	"Intercorporeal9", "LeftAcromioClavicular", "LeftAtlantoOccipital", "LeftCalcaneoCuboidal", 
	"LeftCapitatoHamate", "LeftCarpoMetacarpal1", "LeftCarpoMetacarpal2", "LeftCarpoMetacarpal3", 
	"LeftCarpoMetacarpal4", "LeftCarpoMetacarpal5", "LeftChondroChondral1", "LeftChondroChondral2", 
	"LeftChondroChondral3", "LeftChondroSternal1", "LeftChondroSternal2", "LeftChondroSternal3", 
	"LeftChondroSternal4", "LeftChondroSternal5", "LeftChondroSternal6", "LeftChondroSternal7", 
	"LeftCondyloTrochlear", "LeftCostoChondral1", "LeftCostoChondral10", "LeftCostoChondral11", 
	"LeftCostoChondral12", "LeftCostoChondral2", "LeftCostoChondral3", "LeftCostoChondral4", 
	"LeftCostoChondral5", "LeftCostoChondral6", "LeftCostoChondral7", "LeftCostoChondral8", 
	"LeftCostoChondral9", "LeftCostoCorporeal1", "LeftCostoCorporeal10", "LeftCostoCorporeal11", 
	"LeftCostoCorporeal12", "LeftCostoCorporeal2", "LeftCostoCorporeal3", "LeftCostoCorporeal4", 
	"LeftCostoCorporeal5", "LeftCostoCorporeal6", "LeftCostoCorporeal7", "LeftCostoCorporeal8", 
	"LeftCostoCorporeal9", "LeftCostoTransverse1", "LeftCostoTransverse10", "LeftCostoTransverse2", 
	"LeftCostoTransverse3", "LeftCostoTransverse4", "LeftCostoTransverse5", "LeftCostoTransverse6", 
	"LeftCostoTransverse7", "LeftCostoTransverse8", "LeftCostoTransverse9", "LeftCoxoFemoral", 
	"LeftCuneoCuboidal", "LeftFemoroPatellar", "LeftFemoroTibial", "LeftFibuloTalar", 
	"LeftFootRay1Interphalangeal", "LeftFootRay2DistalInterphalangeal", "LeftFootRay2ProximalInterphalangeal", "LeftFootRay3DistalInterphalangeal", 
	"LeftFootRay3ProximalInterphalangeal", "LeftFootRay4DistalInterphalangeal", "LeftFootRay4ProximalInterphalangeal", "LeftFootRay5DistalInterphalangeal", 
	"LeftFootRay5ProximalInterphalangeal", "LeftHandRay1Interphalangeal", "LeftHandRay2DistalInterphalangeal", "LeftHandRay2ProximalInterphalangeal", 
	"LeftHandRay3DistalInterphalangeal", "LeftHandRay3ProximalInterphalangeal", "LeftHandRay4DistalInterphalangeal", "LeftHandRay4ProximalInterphalangeal", 
	"LeftHandRay5DistalInterphalangeal", "LeftHandRay5ProximalInterphalangeal", "LeftHumeroRadial", "LeftHumeroUlnar", 
	"LeftInferiorRadioUlnar", "LeftInferiorTibioFibular", "LeftIntermediateScaphoCuneal", "LeftLateralAtlantoAxial", 
	"LeftLateralInterCuneal", "LeftLateralScaphoCuneal", "LeftLumbosacral", "LeftLunoCapitate", 
	"LeftLunoTriquetral", "LeftMedialInterCuneal", "LeftMedialScaphoCuneal", "LeftMetacarpoPhalangeal1", 
	"LeftMetacarpoPhalangeal2", "LeftMetacarpoPhalangeal3", "LeftMetacarpoPhalangeal4", "LeftMetacarpoPhalangeal5", 
	"LeftMetatarsoPhalangeal1", "LeftMetatarsoPhalangeal2", "LeftMetatarsoPhalangeal3", "LeftMetatarsoPhalangeal4", 
	"LeftMetatarsoPhalangeal5", "LeftMiddleRadioUlnar", "LeftMiddleTibioFibular", "LeftPisoTriquetral", 
	"LeftRadioLunate", "LeftRadioScaphoid", "LeftSacroIliac", "LeftScaphoCapitate", 
	"LeftScaphoLunate", "LeftScaphoTalar", "LeftScaphoTrapezoTrapezoid", "LeftScapuloHumeral", 
	"LeftSerratoScapular", "LeftSerratoThoracic", "LeftSternoCostoClavicular", "LeftSubTalar", 
	"LeftSuperiorRadioUlnar", "LeftSuperiorTibioFibular", "LeftSyndesmoCarpal", "LeftSyndesmoUlnar", 
	"LeftTarsoMetatarsal1", "LeftTarsoMetatarsal2", "LeftTarsoMetatarsal3", "LeftTarsoMetatarsal4", 
	"LeftTarsoMetatarsal5", "LeftTemporoMandibular", "LeftTibioTalar", "LeftTibioTarsal", 
	"LeftTrapezoCapitate", "LeftTrapezoTrapezoid", "LeftTriquetroHamate", "LeftZygapophyseal10", 
	"LeftZygapophyseal11", "LeftZygapophyseal12", "LeftZygapophyseal13", "LeftZygapophyseal14", 
	"LeftZygapophyseal15", "LeftZygapophyseal16", "LeftZygapophyseal17", "LeftZygapophyseal18", 
	"LeftZygapophyseal19", "LeftZygapophyseal2", "LeftZygapophyseal20", "LeftZygapophyseal21", 
	"LeftZygapophyseal22", "LeftZygapophyseal23", "LeftZygapophyseal3", "LeftZygapophyseal4", 
	"LeftZygapophyseal5", "LeftZygapophyseal6", "LeftZygapophyseal7", "LeftZygapophyseal8", 
	"LeftZygapophyseal9", "ManubrioSternal", "MedianAtlantoAxial", "RightAcromioClavicular", 
	"RightAtlantoOccipital", "RightCalcaneoCuboidal", "RightCapitatoHamate", "RightCarpoMetacarpal1", 
	"RightCarpoMetacarpal2", "RightCarpoMetacarpal3", "RightCarpoMetacarpal4", "RightCarpoMetacarpal5", 
	"RightChondroChondral1", "RightChondroChondral2", "RightChondroChondral3", "RightChondroSternal1", 
	"RightChondroSternal2", "RightChondroSternal3", "RightChondroSternal4", "RightChondroSternal5", 
	"RightChondroSternal6", "RightChondroSternal7", "RightCondyloTrochlear", "RightCostoChondral1", 
	"RightCostoChondral10", "RightCostoChondral11", "RightCostoChondral12", "RightCostoChondral2", 
	"RightCostoChondral3", "RightCostoChondral4", "RightCostoChondral5", "RightCostoChondral6", 
	"RightCostoChondral7", "RightCostoChondral8", "RightCostoChondral9", "RightCostoCorporeal1", 
	"RightCostoCorporeal10", "RightCostoCorporeal11", "RightCostoCorporeal12", "RightCostoCorporeal2", 
	"RightCostoCorporeal3", "RightCostoCorporeal4", "RightCostoCorporeal5", "RightCostoCorporeal6", 
	"RightCostoCorporeal7", "RightCostoCorporeal8", "RightCostoCorporeal9", "RightCostoTransverse1", 
	"RightCostoTransverse10", "RightCostoTransverse2", "RightCostoTransverse3", "RightCostoTransverse4", 
	"RightCostoTransverse5", "RightCostoTransverse6", "RightCostoTransverse7", "RightCostoTransverse8", 
	"RightCostoTransverse9", "RightCoxoFemoral", "RightCuneoCuboidal", "RightFemoroPatellar", 
	"RightFemoroTibial", "RightFibuloTalar", "RightFootRay1Interphalangeal", "RightFootRay2DistalInterphalangeal", 
	"RightFootRay2ProximalInterphalangeal", "RightFootRay3DistalInterphalangeal", "RightFootRay3ProximalInterphalangeal", "RightFootRay4DistalInterphalangeal", 
	"RightFootRay4ProximalInterphalangeal", "RightFootRay5DistalInterphalangeal", "RightFootRay5ProximalInterphalangeal", "RightHandRay1Interphalangeal", 
	"RightHandRay2DistalInterphalangeal", "RightHandRay2ProximalInterphalangeal", "RightHandRay3DistalInterphalangeal", "RightHandRay3ProximalInterphalangeal", 
	"RightHandRay4DistalInterphalangeal", "RightHandRay4ProximalInterphalangeal", "RightHandRay5DistalInterphalangeal", "RightHandRay5ProximalInterphalangeal", 
	"RightHumeroRadial", "RightHumeroUlnar", "RightInferiorRadioUlnar", "RightInferiorTibioFibular", 
	"RightIntermediateScaphoCuneal", "RightLateralAtlantoAxial", "RightLateralInterCuneal", "RightLateralScaphoCuneal", 
	"RightLumbosacral", "RightLunoCapitate", "RightLunoTriquetral", "RightMedialInterCuneal", 
	"RightMedialScaphoCuneal", "RightMetacarpoPhalangeal1", "RightMetacarpoPhalangeal2", "RightMetacarpoPhalangeal3", 
	"RightMetacarpoPhalangeal4", "RightMetacarpoPhalangeal5", "RightMetatarsoPhalangeal1", "RightMetatarsoPhalangeal2", 
	"RightMetatarsoPhalangeal3", "RightMetatarsoPhalangeal4", "RightMetatarsoPhalangeal5", "RightMiddleRadioUlnar", 
	"RightMiddleTibioFibular", "RightPisoTriquetral", "RightRadioLunate", "RightRadioScaphoid", 
	"RightSacroIliac", "RightScaphoCapitate", "RightScaphoLunate", "RightScaphoTalar", 
	"RightScaphoTrapezoTrapezoid", "RightScapuloHumeral", "RightSerratoScapular", "RightSerratoThoracic", 
	"RightSternoCostoClavicular", "RightSubTalar", "RightSuperiorRadioUlnar", "RightSuperiorTibioFibular", 
	"RightSyndesmoCarpal", "RightSyndesmoUlnar", "RightTarsoMetatarsal1", "RightTarsoMetatarsal2", 
	"RightTarsoMetatarsal3", "RightTarsoMetatarsal4", "RightTarsoMetatarsal5", "RightTemporoMandibular", 
	"RightTibioTalar", "RightTibioTarsal", "RightTrapezoCapitate", "RightTrapezoTrapezoid", 
	"RightTriquetroHamate", "RightZygapophyseal10", "RightZygapophyseal11", "RightZygapophyseal12", 
	"RightZygapophyseal13", "RightZygapophyseal14", "RightZygapophyseal15", "RightZygapophyseal16", 
	"RightZygapophyseal17", "RightZygapophyseal18", "RightZygapophyseal19", "RightZygapophyseal2", 
	"RightZygapophyseal20", "RightZygapophyseal21", "RightZygapophyseal22", "RightZygapophyseal23", 
	"RightZygapophyseal3", "RightZygapophyseal4", "RightZygapophyseal5", "RightZygapophyseal6", 
	"RightZygapophyseal7", "RightZygapophyseal8", "RightZygapophyseal9", "XiphiSternal",  
};

/*static*/ const char* medMSMGraph::MuscleType::TAG_PREFIX =
	"L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_Anatomy_Body_Organs_Muscles";

/*static*/ const char* medMSMGraph::MuscleType::NAMES[] = {
	"LeftAbductorPollicisLongus", "LeftAdductorBrevis", "LeftAdductorLongus", "LeftAdductorMagnus",
	"LeftAnconeus", "LeftArticularisGenu", "LeftBicepsBrachiiCaputBrevis", "LeftBicepsBrachiiCaputLongus",
	"LeftBicepsFemorisCaputBrevis", "LeftBicepsFemorisCaputLongus", "LeftBrachialis", "LeftBrachioRadialis",
	"LeftCoracoBrachialis", "LeftDeltoideus", "LeftExtensorCarpiRadialisBrevis", "LeftExtensorCarpiRadialisLongus",
	"LeftExtensorCarpiUlnaris", "LeftExtensorDigitiMinimi", "LeftExtensorDigitorum", "LeftExtensorDigitorumBrevis",
	"LeftExtensorDigitorumLongus", "LeftExtensorHallucisLongus", "LeftExtensorIndicis", "LeftExtensorPollicisBrevis",
	"LeftExtensorPollicisLongus", "LeftFlexorCarpiRadialis", "LeftFlexorCarpiUlnaris", "LeftFlexorDigitorumBrevis",
	"LeftFlexorDigitorumLongus", "LeftFlexorDigitorumProfondus", "LeftFlexorDigitorumSuperficialis", "LeftFlexorHallucisLongus",
	"LeftFlexorPollicisLongus", "LeftFootAbductorDigitiMinimi", "LeftFootAbductorHallucis", "LeftFootAdductorHallucis",
	"LeftFootFlexorDigitiMinimiBrevis", "LeftFootFlexorHallucisBrevis", "LeftFootInterosseusDorsalis1", "LeftFootInterosseusDorsalis2",
	"LeftFootInterosseusDorsalis3", "LeftFootInterosseusDorsalis4", "LeftFootInterosseusPalmaris1", "LeftFootInterosseusPalmaris2",
	"LeftFootInterosseusPalmaris3", "LeftFootLumbricalis1", "LeftFootLumbricalis2", "LeftFootLumbricalis3",
	"LeftFootLumbricalis4", "LeftFootOpponensDigitiMinimi", "LeftGastrocnemiusCaputLateralis", "LeftGastrocnemiusCaputMedialis",
	"LeftGemellusInferior", "LeftGemellusSuperior", "LeftGluteusMaximus", "LeftGluteusMedius",
	"LeftGluteusMinimus", "LeftGracilis", "LeftHandAbductorDigitiMinimi", "LeftHandAbductorPollicisBrevis",
	"LeftHandAdductorPollicis", "LeftHandFlexorDigitiMinimiBrevis", "LeftHandFlexorPollicisBrevis", "LeftHandInterosseusDorsalis1",
	"LeftHandInterosseusDorsalis2", "LeftHandInterosseusDorsalis3", "LeftHandInterosseusDorsalis4", "LeftHandInterosseusPalmaris1",
	"LeftHandInterosseusPalmaris2", "LeftHandInterosseusPalmaris3", "LeftHandLumbricalis1", "LeftHandLumbricalis2",
	"LeftHandLumbricalis3", "LeftHandLumbricalis4", "LeftHandOpponensDigitiMinimi", "LeftHandOpponensPollicis",
	"LeftIlliacus", "LeftInfraSpinatus", "LeftLatissimusDorsi", "LeftObturatorExternus",
	"LeftObturatorInternus", "LeftPalmarisLongus", "LeftPectineus", "LeftPectoralisMajor",
	"LeftPectoralisMinor", "LeftPeroneusBrevis", "LeftPeroneusLongus", "LeftPeroneusTertius",
	"LeftPiriformis", "LeftPlantaris", "LeftPopliteus", "LeftPronatorQuadratus",
	"LeftPronatorTeres", "LeftPsoasMajor", "LeftPsoasMinor", "LeftQuadratusFemoris",
	"LeftQuadratusPlantae", "LeftRectusFemoris", "LeftSartorius", "LeftSemiMembranosus",
	"LeftSemiTendinosus", "LeftSerratusAnterior", "LeftSoleus",
	"LeftSubclavius", "LeftSubscapularis", "LeftSupinator", "LeftSupraSpinatus",
	"LeftTensorFasciaLata", "LeftTeresMajor", "LeftTeresMinor", "LeftTibialisAnterior",
	"LeftTibialisPosterior", "LeftTricepsBrachiiCaputLateralis", "LeftTricepsBrachiiCaputLongus", "LeftTricepsBrachiiCaputMedialis",
	"LeftVastusIntermedius", "LeftVastusLateralis", "LeftVastusMedialis", "RightAbductorPollicisLongus",
	"RightAdductorBrevis", "RightAdductorLongus", "RightAdductorMagnus", "RightAnconeus",
	"RightArticularisGenu", "RightBicepsBrachiiCaputBrevis", "RightBicepsBrachiiCaputLongus", "RightBicepsFemorisCaputBrevis",
	"RightBicepsFemorisCaputLongus", "RightBrachialis", "RightBrachioRadialis", "RightCoracoBrachialis",
	"RightDeltoideus", "RightExtensorCarpiRadialisBrevis", "RightExtensorCarpiRadialisLongus", "RightExtensorCarpiUlnaris",
	"RightExtensorDigitiMinimi", "RightExtensorDigitorum", "RightExtensorDigitorumBrevis", "RightExtensorDigitorumLongus",
	"RightExtensorHallucisLongus", "RightExtensorIndicis", "RightExtensorPollicisBrevis", "RightExtensorPollicisLongus",
	"RightFlexorCarpiRadialis", "RightFlexorCarpiUlnaris", "RightFlexorDigitorumBrevis", "RightFlexorDigitorumLongus",
	"RightFlexorDigitorumProfondus", "RightFlexorDigitorumSuperficialis", "RightFlexorHallucisLongus", "RightFlexorPollicisLongus",
	"RightFootAbductorDigitiMinimi", "RightFootAbductorHallucis", "RightFootAdductorHallucis", "RightFootFlexorDigitiMinimiBrevis",
	"RightFootFlexorHallucisBrevis", "RightFootInterosseusDorsalis1", "RightFootInterosseusDorsalis2", "RightFootInterosseusDorsalis3",
	"RightFootInterosseusDorsalis4", "RightFootInterosseusPalmaris1", "RightFootInterosseusPalmaris2", "RightFootInterosseusPalmaris3",
	"RightFootLumbricalis1", "RightFootLumbricalis2", "RightFootLumbricalis3", "RightFootLumbricalis4",
	"RightFootOpponensDigitiMinimi", "RightGastrocnemiusCaputLateralis", "RightGastrocnemiusCaputMedialis", "RightGemellusInferior",
	"RightGemellusSuperior", "RightGluteusMaximus", "RightGluteusMedius", "RightGluteusMinimus",
	"RightGracilis", "RightIlliacus", "RightInfraSpinatus", "RightLatissimusDorsi",
	"RightObturatorExternus", "RightObturatorInternus", "RightPalmarisLongus", "RightPectineus",
	"RightPectoralisMajor", "RightPectoralisMinor", "RightPeroneusBrevis", "RightPeroneusLongus",
	"RightPeroneusTertius", "RightPiriformis", "RightPlantaris", "RightPopliteus",
	"RightPronatorQuadratus", "RightPronatorTeres", "RightPsoasMajor", "RightPsoasMinor",
	"RightQuadratusFemoris", "RightQuadratusPlantae", "RightRectusFemoris", "RightSartorius",
	"RightSemiMembranosus", "RightSemiTendinosus", "RightSerratusAnterior",
	"RightSoleus", "RightSubclavius", "RightSubscapularis", "RightSupinator",
	"RightSupraSpinatus", "RightTensorFasciaLata", "RightTeresMajor", "RightTeresMinor",
	"RightTibialisAnterior", "RightTibialisPosterior", "RightTricepsBrachiiCaputLateralis", "RightTricepsBrachiiCaputLongus",
	"RightTricepsBrachiiCaputMedialis", "RightVastusIntermedius", "RightVastusLateralis", "RightVastusMedialis",
};

/*static*/ const char* medMSMGraph::LigamentType::TAG_PREFIX =
	"L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_Anatomy_Body_Organs_Ligaments";

/*static*/ const char* medMSMGraph::LigamentType::NAMES[] = {
	"LeftAnnularLigamentHand", "LeftAnteriorCruciateLigament", "LeftForeArmInterosseusMembrane", "LeftPalmarAponeurosisAnterior",
	"LeftPatellarLigament", "LeftPosteriorCruciateLigament", "LeftShankInterosseusMembrane", "LeftTriangularLig",
	"RightAnnularLigamentHand", "RightAnteriorCruciateLigament", "RightForeArmInterosseusMembrane",
	"RightPalmarAponeurosisAnterior", "RightPatellarLigament", "RightPosteriorCruciateLigament", "RightShankInterosseusMembrane",
	"RightTriangularLig",
};

/*static*/ const char* medMSMGraph::BodyLandmarkType::TAG_PREFIX =
	"L0000_resource_data_Representation_RepresentationType_Description_FunctionalAnatomy_FunctionalAnatomy_Anatomy_Body_BodyLandmarks";

/*static*/ const char* medMSMGraph::BodyLandmarkType::NAMES[] = {
	"RightHipMostLateral",
};

#pragma endregion RegionType, BoneType, ...

#pragma endregion RegionEnum, BoneEnum, ...

#pragma region GraphRelations
/*Static map between muscles and bones, structure is: MuscleType, Num of bones, BoneType 1, ...., -1 */
/*static*/ const medMSMGraph::MSMGraphRelations::ITEM_ENTRY medMSMGraph::MSMGraphRelations::MAP_MUSCLE_BONES[] =
{	
	{medMSMGraph::MuscleType::RightAdductorBrevis, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, -1, -1}},
	{medMSMGraph::MuscleType::RightAdductorLongus, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, -1, -1}},	
	{medMSMGraph::MuscleType::RightAdductorMagnus, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, -1, -1}},	
	{medMSMGraph::MuscleType::RightBicepsFemorisCaputBrevis, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, medMSMGraph::BoneType::RightTibia, medMSMGraph::BoneType::RightFibula}},
	{medMSMGraph::MuscleType::RightBicepsFemorisCaputLongus, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, medMSMGraph::BoneType::RightTibia, medMSMGraph::BoneType::RightFibula}},	
	{medMSMGraph::MuscleType::RightGluteusMaximus, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, medMSMGraph::BoneType::Sacrum, -1}},
	{medMSMGraph::MuscleType::RightGluteusMedius, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, -1, -1}},
	{medMSMGraph::MuscleType::RightGluteusMinimus, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, -1, -1}},
	{medMSMGraph::MuscleType::RightGracilis, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, medMSMGraph::BoneType::RightTibia, -1}},
	{medMSMGraph::MuscleType::RightIlliacus, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, -1, -1}},
	{medMSMGraph::MuscleType::RightObturatorExternus, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, -1, -1}},
	{medMSMGraph::MuscleType::RightObturatorInternus, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, -1, -1}},	
	{medMSMGraph::MuscleType::RightPectineus, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, -1, -1}},	
	{medMSMGraph::MuscleType::RightPiriformis, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, medMSMGraph::BoneType::Sacrum, -1}},	
	{medMSMGraph::MuscleType::RightPsoasMajor, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, medMSMGraph::BoneType::Sacrum, -1}},
	{medMSMGraph::MuscleType::RightPsoasMinor, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, medMSMGraph::BoneType::Sacrum, -1}},
	{medMSMGraph::MuscleType::RightQuadratusFemoris, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, -1, -1}},	
	{medMSMGraph::MuscleType::RightRectusFemoris, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, -1, -1}},
	{medMSMGraph::MuscleType::RightSartorius, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, medMSMGraph::BoneType::RightTibia, -1}},
	{medMSMGraph::MuscleType::RightSemiMembranosus, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, medMSMGraph::BoneType::RightTibia, -1}},
	{medMSMGraph::MuscleType::RightSemiTendinosus, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, medMSMGraph::BoneType::RightTibia, -1}},	
	{medMSMGraph::MuscleType::RightTensorFasciaLata, {medMSMGraph::BoneType::RightIliac, medMSMGraph::BoneType::RightFemur, medMSMGraph::BoneType::RightTibia, -1}},	
	{medMSMGraph::MuscleType::RightVastusIntermedius, {medMSMGraph::BoneType::RightFemur, -1, -1, -1}},
	{medMSMGraph::MuscleType::RightVastusLateralis, {medMSMGraph::BoneType::RightFemur, -1, -1, -1}},
	{medMSMGraph::MuscleType::RightVastusMedialis, {medMSMGraph::BoneType::RightFemur, -1, -1, -1}},
};

//-------------------------------------------------------------------------
//Gets the bones to which the specified muscle (specified as MuscleType) are connected by tendons
/*static*/ void medMSMGraph::MSMGraphRelations::GetBonesByMuscle(int muscleType, BoneEnumList& output)
{
	const MSMGraphRelations::ITEM_ENTRY* pos_found = (const MSMGraphRelations::ITEM_ENTRY*)bsearch(&muscleType, MAP_MUSCLE_BONES, 
		sizeof(MAP_MUSCLE_BONES) / sizeof(MAP_MUSCLE_BONES[0]), sizeof(MAP_MUSCLE_BONES[0]),
		(int (*)(const void*, const void*)) Comparer );

	output.clear();
	if (pos_found != NULL)
	{
		for (int i = 0; i < MAX_INITEM_ENTRIES; i++)
		{
			if (pos_found->entries[i] < 0)
				return;	//ready

			BoneEnum enBone = (BoneType::EnumValues)pos_found->entries[i];
			output.push_back(enBone);
		}	
	}
}
#pragma endregion

#pragma region MSMGraphNode

//-------------------------------------------------------------------------
//Combines two descriptors together to give one common descriptor.
//If both input descriptors define the same type of Info (e.g., RegionInfo), Info is set to Invalid.
medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor
	medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor::Combine(const MSMGraphNodeDescriptor& desc1, const MSMGraphNodeDescriptor& desc2)
{
	MSMGraphNodeDescriptor ret;
	ret.m_Flags = (DescFlags)(desc1.m_Flags | desc2.m_Flags);

	if ((desc1.m_Flags & desc2.m_Flags) == RegionNode)
		ret.m_RegionInfo = RegionEnum::Invalid;
	else
		ret.m_RegionInfo = (RegionEnum::EnumValues)(desc1.m_RegionInfo | desc2.m_RegionInfo);

	if ((desc1.m_Flags & desc2.m_Flags) == BoneNode)
		ret.m_BoneInfo = BoneEnum::Invalid;
	else
		ret.m_BoneInfo = (BoneEnum::EnumValues)(desc1.m_BoneInfo | desc2.m_BoneInfo);

	if ((desc1.m_Flags & desc2.m_Flags) == JointNode)
		ret.m_JointInfo = JointEnum::Invalid;
	else
		ret.m_JointInfo = (JointEnum::EnumValues)(desc1.m_JointInfo | desc2.m_JointInfo);

	if ((desc1.m_Flags & desc2.m_Flags) == LigamentNode)
		ret.m_LigamentInfo = LigamentEnum::Invalid;
	else
		ret.m_LigamentInfo = (LigamentEnum::EnumValues)(desc1.m_LigamentInfo | desc2.m_LigamentInfo);

	if ((desc1.m_Flags & desc2.m_Flags) == MuscleNode)
		ret.m_MuscleInfo = MuscleEnum::Invalid;
	else
		ret.m_MuscleInfo = (MuscleEnum::EnumValues)(desc1.m_MuscleInfo | desc2.m_MuscleInfo);

	if ((desc1.m_Flags & desc2.m_Flags) == BodyLandmarkCloudNode)
		ret.m_BodyLandmarkInfo = BodyLandmarkEnum::Invalid;
	else
		ret.m_BodyLandmarkInfo = (BodyLandmarkEnum::EnumValues)(desc1.m_BodyLandmarkInfo | desc2.m_BodyLandmarkInfo);

	return ret;
}

//-------------------------------------------------------------------------
//Returns the string representation  of this node
mafString medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor::ToString() const
{
	if (m_Flags == UnknownNode)
		return "Unknown";

	mafString ret;
	if ((m_Flags & RegionNode) != 0)
	{
		ret.Append("Region [");
		ret.Append(m_RegionInfo.GetName());
		ret.Append("]");
	}

	if ((m_Flags & BoneNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("Bone [");
		ret.Append(m_BoneInfo.GetName());
		ret.Append("]");
	}

	if ((m_Flags & JointNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("Joint [");
		ret.Append(m_JointInfo.GetName());
		ret.Append("]");
	}

	if ((m_Flags & MuscleNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("Muscle [");
		ret.Append(m_MuscleInfo.GetName());
		ret.Append("]");
	}

	if ((m_Flags & LigamentNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("Ligament [");
		ret.Append(m_LigamentInfo.GetName());
		ret.Append("]");
	}

	if ((m_Flags & BodyLandmarkCloudNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("BodyLandmarkCloud [");
		ret.Append(m_BodyLandmarkInfo.GetName());
		ret.Append("]");
	}

	if ((m_Flags & MuscleWrapperNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("MuscleWrapper");
	}

	if ((m_Flags & WrapperMeterNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("WrapperMeter");
	}

	if ((m_Flags & LandmarkCloudNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("LandmarkCloud");
	}

	if ((m_Flags & LandmarkNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("Landmark");
	}

	if ((m_Flags & GroupNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("Group");
	}

	if ((m_Flags & LinkNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("Link");
	}

	if ((m_Flags & ParametricNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("Parametric");
	}

	if ((m_Flags & LowResolutionNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("LowResolutionNode");
	}

	if ((m_Flags & HullNode) != 0)
	{
		if (!ret.IsEmpty())
			ret.Append(" | ");

		ret.Append("HullNode");
	}

	return ret;
}

//-------------------------------------------------------------------------
//ctor
medMSMGraph::MSMGraphNode::MSMGraphNode()
	//-------------------------------------------------------------------------
{
	this->m_AncestorNodesDescriptor = this->m_DescendantNodesDescriptor =
		this->m_NodeDescriptor = MSMGraphNodeDescriptor::UnknownNode;
	this->m_Parent = this->m_HigherResNode = this->m_LowerResNode =
		this->m_HullNode = this->m_HullUserNode = NULL;
	this->m_Vme = NULL;
	this->m_LevelOfDetail = -1;	//unknown level
}

//-------------------------------------------------------------------------
// dtor
//-------------------------------------------------------------------------
medMSMGraph::MSMGraphNode::~MSMGraphNode()
{
	for (MSMGraphChildren::iterator it = m_Children.begin();
		it != m_Children.end(); it++){
			delete (*it); //remove the child
	}

	m_Children.clear();
}

//-------------------------------------------------------------------------
//Returns the string representation  of this node
mafString medMSMGraph::MSMGraphNode::ToString()
{
	wxString hrstr, lrstr;
	hrstr = m_HigherResNode == NULL ? "(Null)" : 
		hrstr = wxString::Format("#%d: %s",
		(m_HigherResNode->m_Vme != NULL ? m_HigherResNode->m_Vme->GetId() : -1), 
		(m_HigherResNode->m_Vme != NULL ? m_HigherResNode->m_Vme->GetName() : ""));

	lrstr = m_LowerResNode == NULL ? "(Null)" : 
		lrstr = wxString::Format("#%d: %s",
		(m_LowerResNode->m_Vme != NULL ? m_LowerResNode->m_Vme->GetId() : -1), 
		(m_LowerResNode->m_Vme != NULL ? m_LowerResNode->m_Vme->GetName() : ""));

	return wxString::Format(
		"0x%p [#%d: %s]\n"
		"%s\nAncestors: %s\nDescentants: %s\n"
		"LOD: %d (HR = [%s], LR = [%s])",
		m_Vme, (m_Vme != NULL ? m_Vme->GetId() : -1), (m_Vme != NULL ? m_Vme->GetName() : ""),
		m_NodeDescriptor.ToString().GetCStr(), m_AncestorNodesDescriptor.ToString().GetCStr(),
		m_DescendantNodesDescriptor.ToString().GetCStr(),
		m_LevelOfDetail, hrstr.c_str(), lrstr.c_str()
		);
}

#pragma endregion MSMGraphNode

//-------------------------------------------------------------------------
medMSMGraph::medMSMGraph() : m_RootNode(NULL)
	//-------------------------------------------------------------------------
{
}

//-------------------------------------------------------------------------
medMSMGraph::~medMSMGraph()
	//-------------------------------------------------------------------------
{
	delete m_RootNode;
}

//-------------------------------------------------------------------------
//Analyses the given node and returns the descriptor for it.
medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor
	medMSMGraph::MSMGraphNode::MSMGraphNodeDescriptor::CreateDescriptor(mafNode* node)
	//-------------------------------------------------------------------------
{
	if (node == NULL)
		return MSMGraphNodeDescriptor::UnknownNode;

	MSMGraphNodeDescriptor ret;
	if (mafVMEShortcut::SafeDownCast(node) != NULL)
		ret.m_Flags = MSMGraphNodeDescriptor::LinkNode;
	else if (mafVMEGroup::SafeDownCast(node) != NULL)
		ret.m_Flags = MSMGraphNodeDescriptor::GroupNode;
	else if (medVMEWrappedMeter::SafeDownCast(node) != NULL ||
		medVMEComputeWrapping::SafeDownCast(node) != NULL ||
		mafVMEMeter::SafeDownCast(node) != NULL)
		ret.m_Flags = MSMGraphNodeDescriptor::WrapperMeterNode;
	else if (medVMEMuscleWrapper::SafeDownCast(node) != NULL)
		ret.m_Flags = MSMGraphNodeDescriptor::MuscleWrapperNode;
	else if (mafVMELandmarkCloud::SafeDownCast(node) != NULL)
		ret.m_Flags = MSMGraphNodeDescriptor::LandmarkCloudNode;
	else if (mafVMELandmark::SafeDownCast(node) != NULL)
		ret.m_Flags = MSMGraphNodeDescriptor::LandmarkNode;
	else if (mafVMESurfaceParametric::SafeDownCast(node) != NULL)
		ret.m_Flags = MSMGraphNodeDescriptor::ParametricNode;
	else
		ret.m_Flags = MSMGraphNodeDescriptor::UnknownNode;

	//get tag array and find whatever we have here
	mafTagArray* tgarr = node->GetTagArray();
	if (tgarr == NULL)
		return ret;

	std::vector< std::string > taglist;
	tgarr->GetTagList(taglist);
	for (std::vector< std::string >::const_iterator it = taglist.begin();
		it != taglist.end(); it++)
	{
		const char* tagname = (*it).c_str();

		if ((ret.m_RegionInfo = RegionEnum::ParseTagName(tagname)) != RegionEnum::Null) {
			ret.m_Flags = (MSMGraphNodeDescriptor::DescFlags)(ret.m_Flags | MSMGraphNodeDescriptor::RegionNode);
			break; //we have region here
		}

		if ((ret.m_BoneInfo = BoneEnum::ParseTagName(tagname)) != BoneEnum::Null) {
			ret.m_Flags = (MSMGraphNodeDescriptor::DescFlags)(ret.m_Flags | MSMGraphNodeDescriptor::BoneNode);
			break;//we have bone here
		}

		if ((ret.m_JointInfo = JointEnum::ParseTagName(tagname)) != JointEnum::Null) {
			ret.m_Flags = (MSMGraphNodeDescriptor::DescFlags)(ret.m_Flags | MSMGraphNodeDescriptor::JointNode);
			break; //we have joint here
		}

		if ((ret.m_MuscleInfo = MuscleEnum::ParseTagName(tagname)) != MuscleEnum::Null) {
			ret.m_Flags = (MSMGraphNodeDescriptor::DescFlags)(ret.m_Flags | MSMGraphNodeDescriptor::MuscleNode);
			break; //we have muscle here
		}

		if ((ret.m_LigamentInfo = LigamentEnum::ParseTagName(tagname)) != LigamentEnum::Null) {
			ret.m_Flags = (MSMGraphNodeDescriptor::DescFlags)(ret.m_Flags | MSMGraphNodeDescriptor::LigamentNode);
			break; //we have ligament here
		}

		if ((ret.m_BodyLandmarkInfo = BodyLandmarkEnum::ParseTagName(tagname)) != BodyLandmarkEnum::Null) {
			ret.m_Flags = (MSMGraphNodeDescriptor::DescFlags)(ret.m_Flags | MSMGraphNodeDescriptor::BodyLandmarkCloudNode);
			break; //we have body landmark here
		}
	}

	return ret;
}

//-------------------------------------------------------------------------
//Builds a graph for the given root node.
//N.B. when any VME changes, the graph needs to be rebuilt.
void medMSMGraph::BuildGraph(mafNode* node)
{
	delete m_RootNode;	//deletes the previous graph
	m_RootNode = NULL;

	m_RootNode = BuildGraphR(node);

#if defined (_MSC_VER) && _MSC_VER >= 1600 && defined(_DEBUG)
	DebugOutput(m_RootNode);
#endif
}

#if defined (_MSC_VER) && _MSC_VER >= 1600 && defined(_DEBUG)
void medMSMGraph::DebugOutput(MSMGraphNode* root, int indent)
{
	mafString strIndent;
	for (int i = 0; i < indent; i++) {
		strIndent.Append("\t");
	}

	mafString strIndentNL = "\n";
	strIndentNL.Append(strIndent);

	mafString str = root->ToString();
	wxString wsStr = str.GetCStr();
	wsStr.Replace("\n", strIndentNL.GetCStr());

	_RPT0(_CRT_WARN,  strIndent.GetCStr());
	_RPT0(_CRT_WARN,  wsStr.c_str());
	_RPT0(_CRT_WARN,  "\n\n");

	for (MSMGraphNode::MSMGraphChildren::const_iterator it = root->m_Children.begin();
		it != root->m_Children.end(); it++)
	{
		DebugOutput(*it, indent + 1);
	}
}
#endif

#include "lhpOpSetLowerRes.h"

//-------------------------------------------------------------------------
//Recursive function that builds graph for the given node returning the created subtree.
//This method is supposed to be used by BuildGraph method
medMSMGraph::MSMGraphNode* medMSMGraph::BuildGraphR(mafNode* node, MSMGraphNode* parent, BUILD_GRAPH_INFO* info)
{
	MSMGraphNode* ret = new MSMGraphNode();
	ret->m_NodeDescriptor = MSMGraphNode::MSMGraphNodeDescriptor::CreateDescriptor(node);	//create descriptor of this node
	if (NULL != (ret->m_Parent = parent))
	{
		//create m_AncestorNodesDescriptor
		ret->m_AncestorNodesDescriptor =
			MSMGraphNode::MSMGraphNodeDescriptor::Combine(
			parent->m_AncestorNodesDescriptor, parent->m_NodeDescriptor);
	}

	ret->m_Vme = node;

	bool bdelinfo = info == NULL;
	if (bdelinfo) {
		info = new BUILD_GRAPH_INFO;
		ret->m_LevelOfDetail = 0;	//highest resolution
	}
	else
	{
		//check, if node is not already in info->resolutions
		//which means that it has been referred by some node already processed
		std::map< mafNode*, MSMGraphNode* >::const_iterator it;		
		if ((it = info->resolutions.find(node)) == info->resolutions.end())
			ret->m_LevelOfDetail = 0;
		else
		{
			ret->m_HigherResNode = it->second;	//we have found	our higher res parent
			ret->m_HigherResNode->m_LowerResNode = ret;
			ret->m_NodeDescriptor.m_Flags = (MSMGraphNode::MSMGraphNodeDescriptor::DescFlags)
				(ret->m_NodeDescriptor.m_Flags | MSMGraphNode::MSMGraphNodeDescriptor::LowResolutionNode);

			//update our level of detail
			MSMGraphNode *pH = ret->m_HigherResNode;
			while (pH->m_LowerResNode != NULL) {
				pH->m_LowerResNode->m_LevelOfDetail = pH->m_LevelOfDetail + 1;
				pH = pH->m_LowerResNode;
			}			
		}

		if ((it = info->hulls.find(node)) != info->hulls.end())
		{
			ret->m_HullUserNode = it->second;	//we have found	our user
			ret->m_HullUserNode->m_HullNode = ret;
			ret->m_NodeDescriptor.m_Flags = (MSMGraphNode::MSMGraphNodeDescriptor::DescFlags)
				(ret->m_NodeDescriptor.m_Flags | MSMGraphNode::MSMGraphNodeDescriptor::HullNode);
		}
	}

	info->processed[node] = ret;	//include ourself 

	//check, if we have any lower resolution
	std::map< mafNode*, MSMGraphNode* >::const_iterator it;
	mafNode* nd = node->GetLink(lhpOpSetLowerRes::GetLinkNameLR());
	if (nd != NULL)
	{
		//check, if nd has not been already processed
		if ((it = info->processed.find(nd)) == info->processed.end())
			info->resolutions[nd] = ret;	//we will process nd later
		else
		{	//fix it now
			ret->m_LowerResNode = it->second;
			ret->m_LowerResNode->m_HigherResNode = ret;
			ret->m_LowerResNode->m_NodeDescriptor.m_Flags = (MSMGraphNode::MSMGraphNodeDescriptor::DescFlags)
				(ret->m_LowerResNode->m_NodeDescriptor.m_Flags | MSMGraphNode::MSMGraphNodeDescriptor::LowResolutionNode);

			//update our level of detail
			MSMGraphNode *pH = ret;
			while (pH->m_LowerResNode != NULL) {
				pH->m_LowerResNode->m_LevelOfDetail = pH->m_LevelOfDetail + 1;
				pH = pH->m_LowerResNode;
			}
		}
	}

	//check, if we have any hull resolution
	nd = node->GetLink(lhpOpSetLowerRes::GetLinkNameHULL());
	if (nd != NULL)
	{
		//check, if nd has not been already processed
		if ((it = info->processed.find(nd)) == info->processed.end())
			info->hulls[nd] = ret;	//we will process nd later
		else
		{	//fix it now
			ret->m_HullNode = it->second;
			ret->m_HullNode->m_HullUserNode = ret;
			ret->m_HullNode->m_NodeDescriptor.m_Flags = (MSMGraphNode::MSMGraphNodeDescriptor::DescFlags)
				(ret->m_HullNode->m_NodeDescriptor.m_Flags | MSMGraphNode::MSMGraphNodeDescriptor::HullNode);
		}		
	}

	//now, process children
	//this will also give us m_DescendantNodesDescriptor

	const mafNode::mafChildrenVector* children = node->GetChildren();
	int count = (int)children->size();

	for (int i = 0; i < count; i++)
	{
		MSMGraphNode* childnode = BuildGraphR(children->at(i), ret, info);
		ret->m_Children.push_back(childnode);

		//update m_DescendantNodesDescriptor
		ret->m_DescendantNodesDescriptor = MSMGraphNode::MSMGraphNodeDescriptor::Combine(
			ret->m_DescendantNodesDescriptor,
			MSMGraphNode::MSMGraphNodeDescriptor::Combine(
			childnode->m_DescendantNodesDescriptor, childnode->m_NodeDescriptor)
			);
	}

	if (bdelinfo)
		delete info;
	return ret;
}

//-------------------------------------------------------------------------
//Gets graph nodes that are descendants of root and match the given filter.
//	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
//	only the root level is searched, otherwise, all regions in the subtree are found.
void medMSMGraph::GetMSMNodes(int lod, MSMGraphNodeDescriptor::DescFlags filter, MSMGraphNodeList& list, bool bRecursive, const MSMGraphNode* root)  const
{
	list.clear();
	GetMSMNodesR(lod, filter, (root != NULL) ? root : GetRoot(), list, bRecursive);
}

//-------------------------------------------------------------------------
//Gets VMEs that are descendants of root and match the given filter.
//If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
//only the root level is searched, otherwise, all regions in the subtree are found.
void medMSMGraph::GetVMENodes(int lod, MSMGraphNodeDescriptor::DescFlags filter, mafVMENodeList& list, bool bRecursive, const MSMGraphNode* root) const
{
	MSMGraphNodeList msm_list;
	GetMSMNodesR(lod, filter, (root != NULL) ? root : GetRoot(), msm_list, bRecursive);
	ExtractVMENodes(msm_list, list);
}

//-------------------------------------------------------------------------
//Recursive method that adds graph nodes having the given descriptor into the  list.
//The search starts at root (MAY NOT BE NULL).
void medMSMGraph::GetMSMNodesR(int lod, MSMGraphNodeDescriptor::DescFlags filter, const MSMGraphNode* root,
	MSMGraphNodeList& list, bool bRecursive) const
{
	//Parameter lod specifies the desired level of details of nodes. Several nodes may represent the same object but
	//each of them represents it in other quality, resolution, i.e., in various level of details. If lod is a non-negative 
	//integer smaller or equal than LOD::Lowest, the method gets the node of the desired resolution (level of detail)
	//or, if such a level does not exist, it gets the node of the closest existing higher level of detail. If the caller wants 
	//to get the requested level of detail and nothing else, parameter lod must be a bitwise combination of required level 
	//and LOD::NoAlternative. Hence, if for some object, there is no node of the required level-of-detail, the object
	//will not be returned. It is important to point out that LOD::NoAlternative may not be used together with
	//LOD::Lowest. Parameter lod can be also LOD::Everything to get all nodes regardles their level of details. 	

	for (MSMGraphNode::MSMGraphChildren::const_iterator it = root->m_Children.begin();
		it != root->m_Children.end(); it++)
	{
		if (((*it)->m_NodeDescriptor.m_Flags & filter) == filter)
		{
			//check if the current node is of our level of detail
			if (lod == LOD::Everything || (*it)->m_LevelOfDetail == (lod & ~LOD::NoAlternative))
				list.push_back(*it);	//exact match			
			else if ((lod & LOD::NoAlternative) == 0)
			{
				//we accept also alternatives, so check, if this is the case
				if ((*it)->m_LowerResNode == NULL && (lod == LOD::Lowest || lod >= (*it)->m_LevelOfDetail))
					list.push_back(*it);	//best match
			}			
		}

		if (bRecursive)
			GetMSMNodesR(lod, filter, *it, list, bRecursive);
	}
}

//-------------------------------------------------------------------------
//Extracts VME nodes from graph nodes in source and stored them into dest
void medMSMGraph::ExtractVMENodes(const MSMGraphNodeList& source, mafVMENodeList& dest) const
{
	dest.clear();
	for (MSMGraphNodeList::const_iterator it = source.begin();
		it != source.end(); it++)
	{
		dest.push_back((*it)->m_Vme);
	}	
}

//-------------------------------------------------------------------------
//Gets graph nodes that are descendants of root and match the given filter.
//This version filters nodes to the exact type, e.g., LeftFemur - given in req_type parameter.
//Parameter offset specifies memory offset to MSMGraphNode, where the type is stored.
//All other parameters correlate to those in the public version.
void medMSMGraph::GetMSMNodes(int lod, MSMGraphNodeDescriptor::DescFlags filter, 
	int req_type, int offset, MSMGraphNodeList& list, bool bRecursive, const MSMGraphNode* root) const 
	//-------------------------------------------------------------------------
{	
	MSMGraphNodeList msm_list;
	GetMSMNodesR(lod, filter, (root != NULL) ? root : GetRoot(), msm_list, bRecursive);

	//filter data
	list.clear();
	for (MSMGraphNodeList::const_iterator it = msm_list.begin(); it != msm_list.end(); it++)
	{
		int value = *reinterpret_cast<const volatile int*>(((reinterpret_cast<const volatile char*>(*it)) + offset));
		if (value == req_type)	{
			list.push_back((*it));
		}
	}	
}

//-------------------------------------------------------------------------
//Gets VMEs that are descendants of root and match the given filter.
//The meaning of all parameters is the same as in case of GetMSMNodes. */
void medMSMGraph::GetVMENodes(int lod, MSMGraphNodeDescriptor::DescFlags filter, 
	int req_type, int offset, mafVMENodeList& list, bool bRecursive, const MSMGraphNode* root) const
	//-------------------------------------------------------------------------
{
	MSMGraphNodeList msm_list;
	GetMSMNodes(lod, filter, req_type, offset, msm_list, bRecursive);
	ExtractVMENodes(msm_list, list);
}


//-------------------------------------------------------------------------
//Finds the MSMNode for the given vme in the tree starting from NULL (or the current root, if not specified). 
//Returns NULL, if the node could not be found. 
const medMSMGraph::MSMGraphNode* medMSMGraph::FindMSMNodeR(const mafNode* vme, const MSMGraphNode* root, bool bRecursive) const 
	//-------------------------------------------------------------------------
{
	if (root == NULL)
		return NULL;

	if (root->m_Vme == vme)
		return root;

	if (bRecursive)
	{
		for (MSMGraphNode::MSMGraphChildren::const_iterator 
			it = root->m_Children.begin(); it != root->m_Children.end(); it++)
		{
			const medMSMGraph::MSMGraphNode* ret = FindMSMNodeR(vme, *it, bRecursive);
			if (ret != NULL)
				return ret;
		}
	}

	return NULL;	
}