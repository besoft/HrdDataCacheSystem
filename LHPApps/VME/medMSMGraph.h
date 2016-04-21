/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medMSMGraph.h,v $
Language:  C++
Date:      $Date: 2012-02-08 07:22:52 $
Version:   $Revision: 1.1.2.5 $
Authors:   Josef Kohout
==========================================================================
Copyright (c) 2011
University of West Bohemia
=========================================================================*/
#ifndef __medMSMGraph_h
#define __medMSMGraph_h

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafNode.h"
#include "lhpVMEDefines.h"

#include <vector>
#include <stack>
#include <map>

/** This is a helper class for medVMEMusculoskeletalModel. It describes the whole model. */
class LHP_VME_EXPORT medMSMGraph
{
public:
#pragma region RegionEnum, BoneEnum, ...
	/** This template class provides methods for parsing string to enumerate value and back.
	It is derived from TBase that must contain:
	1) static const char* TAG_PREFIX; where is stored TAG used as a prefix in TagArray of VMEs
	2) static const char* NAMES[]; where are stored names for enumerate values MUST BE ORDERED!
	3) enum EnumValues; where are stored enumerate values corresponding to NAMES +
	the first value: Null = 0 and the last value: Invalid. */
	template< class TBase >
	class Enum : public TBase
	{
	public:
		typedef typename TBase::EnumValues EnumValues;

		// ------- static methods -----------
		/** Returns the prefix of all regions to be used in tag array of VMEs*/
		inline static const char* GetTagNamePrefix() { return TAG_PREFIX; }

		/** Returns the number of supported values */
		inline static int GetNumberOfValues() { return Invalid; }

		/** Converts the given identifier into string */
		static const char* GetName(EnumValues value);

		/** Converts the given identifier into tag name to be used in tag array of VMEs */
		static mafString GetTagName(EnumValues value);

		/** Converts the given string into the identifier (value).
		Returns Invalid, if the string could not be parsed. */
		static EnumValues ParseName(const char* name);

		/** Converts the given tag name  (used in tag array of VME) into the identifier (value) .
		Returns Null, if the tag name does not contain the prefix or Invalid, if the name could not be parsed. */
		static EnumValues ParseTagName(const char* name);
	protected:
		/** Callback from bsearch, compares two strings identified by a pointer*/
		static int Comparer(const char** ch1, const char** ch2) {
			return strcmp(*ch1, *ch2);
		}

	protected:
		EnumValues m_Value;		///<the stored value
	public:
		// ------- methods -----------
		Enum(EnumValues value = Null) : m_Value(value) { }
		inline Enum& operator = (EnumValues value) { m_Value = value;  return *this; }

		/** Converts the given identifier into string */
		inline const char* GetName() const { return GetName(m_Value); }

		/** Converts the given identifier into tag name to be used in tag array of VMEs */
		inline const char* GetTagName() const {	return GetTagName(m_Value); }

		/** cast operators */
		inline operator EnumValues() const { return m_Value; }
		inline operator EnumValues&() { return m_Value; }
	};

	/** This class encapsulates enum of Regions*/
	class RegionType
	{
	public:
		/** enum with region names */
		enum EnumValues {
			Null = 0,

			Abdomen, Back, Head, LeftFoot,
			LeftFootDorsal, LeftFootPlantar, LeftForeArm, LeftForeArmAnterior,
			LeftForeArmLateral, LeftForeArmPosterior, LeftHand, LeftHandAnterior,
			LeftHandPosterior, LeftLowerLimb, LeftPelvis, LeftShank,
			LeftShankAnterior, LeftShankLateral, LeftShankPosterior, LeftShoulder,
			LeftSpine, LeftThigh, LeftThighAnterior, LeftThighMedial,
			LeftThighPosterior, LeftUpperArm, LeftUpperArmAnterior, LeftUpperArmPosterior,
			LeftUpperLimb, Neck, Pelvis, RightFoot,
			RightFootDorsal, RightFootPlantar, RightForeArm, RightForeArmAnterior,
			RightForeArmLateral, RightForeArmPosterior, RightHand, RightHandAnterior,
			RightHandPosterior, RightLowerLimb, RightPelvis, RightShank,
			RightShankAnterior, RightShankLateral, RightShankPosterior, RightShoulder,
			RightSpine, RightThigh, RightThighAnterior, RightThighMedial,
			RightThighPosterior, RightUpperArm, RightUpperArmAnterior, RightUpperArmPosterior,
			RightUpperLimb, Thorax,

			Invalid
		};
		static const char* TAG_PREFIX;		///<contains L000_...._Regions_ prefix used for VME Tags
		static const char* NAMES[];				///<array of region names corresponding to RegionEnum values
	};

	/** This class encapsulates enum of Bones*/
	class BoneType
	{
	public:
		/** enum with bone names */
		enum EnumValues {
			Null = 0,

			CervicalVertebrae1, CervicalVertebrae2, CervicalVertebrae3, CervicalVertebrae4,
			CervicalVertebrae5, CervicalVertebrae6, CervicalVertebrae7, Coccyx,
			Jaw, LeftCalcaneus, LeftCapitate, LeftClavicle,
			LeftCostalCartilage1, LeftCostalCartilage10, LeftCostalCartilage11, LeftCostalCartilage12,
			LeftCostalCartilage2, LeftCostalCartilage3, LeftCostalCartilage4, LeftCostalCartilage5,
			LeftCostalCartilage6, LeftCostalCartilage7, LeftCostalCartilage8, LeftCostalCartilage9,
			LeftCuboid, LeftFemur, LeftFibula, LeftFootRay1LateralSesamoid,
			LeftFootRay1MedialSesamoid, LeftFootRay1Phalanx1, LeftFootRay1Phalanx2, LeftFootRay2Phalanx1,
			LeftFootRay2Phalanx2, LeftFootRay2Phalanx3, LeftFootRay3Phalanx1, LeftFootRay3Phalanx2,
			LeftFootRay3Phalanx3, LeftFootRay4Phalanx1, LeftFootRay4Phalanx2, LeftFootRay4Phalanx3,
			LeftFootRay5Phalanx1, LeftFootRay5Phalanx2, LeftFootRay5Phalanx3, LeftFootScaphoid,
			LeftHamate, LeftHandRay1LateralSesamoid, LeftHandRay1MedialSesamoid, LeftHandRay1Phalanx1,
			LeftHandRay1Phalanx2, LeftHandRay2Phalanx1, LeftHandRay2Phalanx2, LeftHandRay2Phalanx3,
			LeftHandRay3Phalanx1, LeftHandRay3Phalanx2, LeftHandRay3Phalanx3, LeftHandRay4Phalanx1,
			LeftHandRay4Phalanx2, LeftHandRay4Phalanx3, LeftHandRay5Phalanx1, LeftHandRay5Phalanx2,
			LeftHandRay5Phalanx3, LeftHandScaphoid, LeftHumerus, LeftIliac,
			LeftIntermediateCuneiform, LeftLateralCuneiform, LeftLunate, LeftMedialCuneiform,
			LeftMetacarpal1, LeftMetacarpal2, LeftMetacarpal3, LeftMetacarpal4,
			LeftMetacarpal5, LeftMetatarsal1, LeftMetatarsal2, LeftMetatarsal3,
			LeftMetatarsal4, LeftMetatarsal5, LeftPatella, LeftPisiformis,
			LeftRadius, LeftRib1, LeftRib10, LeftRib11,
			LeftRib12, LeftRib2, LeftRib3, LeftRib4,
			LeftRib5, LeftRib6, LeftRib7, LeftRib8,
			LeftRib9, LeftScapula, LeftTalus, LeftTibia,
			LeftTrapezium, LeftTrapezoid, LeftTriquetrum, LeftUlna,
			LumbarVertebrae1, LumbarVertebrae2, LumbarVertebrae3, LumbarVertebrae4,
			LumbarVertebrae5, Manubrium, RightCalcaneus, RightCapitate,
			RightClavicle, RightCostalCartilage1, RightCostalCartilage10, RightCostalCartilage11,
			RightCostalCartilage12, RightCostalCartilage2, RightCostalCartilage3, RightCostalCartilage4,
			RightCostalCartilage5, RightCostalCartilage6, RightCostalCartilage7, RightCostalCartilage8,
			RightCostalCartilage9, RightCuboid, RightFemur, RightFibula,
			RightFootRay1LateralSesamoid, RightFootRay1MedialSesamoid, RightFootRay1Phalanx1, RightFootRay1Phalanx2,
			RightFootRay2Phalanx1, RightFootRay2Phalanx2, RightFootRay2Phalanx3, RightFootRay3Phalanx1,
			RightFootRay3Phalanx2, RightFootRay3Phalanx3, RightFootRay4Phalanx1, RightFootRay4Phalanx2,
			RightFootRay4Phalanx3, RightFootRay5Phalanx1, RightFootRay5Phalanx2, RightFootRay5Phalanx3,
			RightFootScaphoid, RightHamate, RightHandRay1LateralSesamoid, RightHandRay1MedialSesamoid,
			RightHandRay1Phalanx1, RightHandRay1Phalanx2, RightHandRay2Phalanx1, RightHandRay2Phalanx2,
			RightHandRay2Phalanx3, RightHandRay3Phalanx1, RightHandRay3Phalanx2, RightHandRay3Phalanx3,
			RightHandRay4Phalanx1, RightHandRay4Phalanx2, RightHandRay4Phalanx3, RightHandRay5Phalanx1,
			RightHandRay5Phalanx2, RightHandRay5Phalanx3, RightHandScaphoid, RightHumerus,
			RightIliac, RightIntermediateCuneiform, RightLateralCuneiform, RightLunate,
			RightMedialCuneiform, RightMetacarpal1, RightMetacarpal2, RightMetacarpal3,
			RightMetacarpal4, RightMetacarpal5, RightMetatarsal1, RightMetatarsal2,
			RightMetatarsal3, RightMetatarsal4, RightMetatarsal5, RightPatella,
			RightPisiformis, RightRadius, RightRib1, RightRib10,
			RightRib11, RightRib12, RightRib2, RightRib3,
			RightRib4, RightRib5, RightRib6, RightRib7,
			RightRib8, RightRib9, RightScapula, RightTalus,
			RightTibia, RightTrapezium, RightTrapezoid, RightTriquetrum,
			RightUlna, Sacrum, Skull, SternalBody,
			ThoracicVertebrae1, ThoracicVertebrae10, ThoracicVertebrae11, ThoracicVertebrae12,
			ThoracicVertebrae2, ThoracicVertebrae3, ThoracicVertebrae4, ThoracicVertebrae5,
			ThoracicVertebrae6, ThoracicVertebrae7, ThoracicVertebrae8, ThoracicVertebrae9,
			XiphoidProcess,

			Invalid
		};
		static const char* TAG_PREFIX;	///<contains L000_...._Bones_ prefix used for VME Tags
		static const char* NAMES[];			///<array of bone names corresponding to BoneEnum values
	};

	/** This class encapsulates enum of Joints*/
	class JointType
	{
	public:
		/** enum with joint names */
		enum EnumValues {
			Null = 0,

			CoccoSacral, Intercorporeal1, Intercorporeal10, Intercorporeal11, 
			Intercorporeal12, Intercorporeal13, Intercorporeal14, Intercorporeal15, 
			Intercorporeal16, Intercorporeal17, Intercorporeal18, Intercorporeal19, 
			Intercorporeal2, Intercorporeal20, Intercorporeal21, Intercorporeal22, 
			Intercorporeal23, Intercorporeal24, Intercorporeal3, Intercorporeal4, 
			Intercorporeal5, Intercorporeal6, Intercorporeal7, Intercorporeal8, 
			Intercorporeal9, LeftAcromioClavicular, LeftAtlantoOccipital, LeftCalcaneoCuboidal, 
			LeftCapitatoHamate, LeftCarpoMetacarpal1, LeftCarpoMetacarpal2, LeftCarpoMetacarpal3, 
			LeftCarpoMetacarpal4, LeftCarpoMetacarpal5, LeftChondroChondral1, LeftChondroChondral2, 
			LeftChondroChondral3, LeftChondroSternal1, LeftChondroSternal2, LeftChondroSternal3, 
			LeftChondroSternal4, LeftChondroSternal5, LeftChondroSternal6, LeftChondroSternal7, 
			LeftCondyloTrochlear, LeftCostoChondral1, LeftCostoChondral10, LeftCostoChondral11, 
			LeftCostoChondral12, LeftCostoChondral2, LeftCostoChondral3, LeftCostoChondral4, 
			LeftCostoChondral5, LeftCostoChondral6, LeftCostoChondral7, LeftCostoChondral8, 
			LeftCostoChondral9, LeftCostoCorporeal1, LeftCostoCorporeal10, LeftCostoCorporeal11, 
			LeftCostoCorporeal12, LeftCostoCorporeal2, LeftCostoCorporeal3, LeftCostoCorporeal4, 
			LeftCostoCorporeal5, LeftCostoCorporeal6, LeftCostoCorporeal7, LeftCostoCorporeal8, 
			LeftCostoCorporeal9, LeftCostoTransverse1, LeftCostoTransverse10, LeftCostoTransverse2, 
			LeftCostoTransverse3, LeftCostoTransverse4, LeftCostoTransverse5, LeftCostoTransverse6, 
			LeftCostoTransverse7, LeftCostoTransverse8, LeftCostoTransverse9, LeftCoxoFemoral, 
			LeftCuneoCuboidal, LeftFemoroPatellar, LeftFemoroTibial, LeftFibuloTalar, 
			LeftFootRay1Interphalangeal, LeftFootRay2DistalInterphalangeal, LeftFootRay2ProximalInterphalangeal, LeftFootRay3DistalInterphalangeal, 
			LeftFootRay3ProximalInterphalangeal, LeftFootRay4DistalInterphalangeal, LeftFootRay4ProximalInterphalangeal, LeftFootRay5DistalInterphalangeal, 
			LeftFootRay5ProximalInterphalangeal, LeftHandRay1Interphalangeal, LeftHandRay2DistalInterphalangeal, LeftHandRay2ProximalInterphalangeal, 
			LeftHandRay3DistalInterphalangeal, LeftHandRay3ProximalInterphalangeal, LeftHandRay4DistalInterphalangeal, LeftHandRay4ProximalInterphalangeal, 
			LeftHandRay5DistalInterphalangeal, LeftHandRay5ProximalInterphalangeal, LeftHumeroRadial, LeftHumeroUlnar, 
			LeftInferiorRadioUlnar, LeftInferiorTibioFibular, LeftIntermediateScaphoCuneal, LeftLateralAtlantoAxial, 
			LeftLateralInterCuneal, LeftLateralScaphoCuneal, LeftLumbosacral, LeftLunoCapitate, 
			LeftLunoTriquetral, LeftMedialInterCuneal, LeftMedialScaphoCuneal, LeftMetacarpoPhalangeal1, 
			LeftMetacarpoPhalangeal2, LeftMetacarpoPhalangeal3, LeftMetacarpoPhalangeal4, LeftMetacarpoPhalangeal5, 
			LeftMetatarsoPhalangeal1, LeftMetatarsoPhalangeal2, LeftMetatarsoPhalangeal3, LeftMetatarsoPhalangeal4, 
			LeftMetatarsoPhalangeal5, LeftMiddleRadioUlnar, LeftMiddleTibioFibular, LeftPisoTriquetral, 
			LeftRadioLunate, LeftRadioScaphoid, LeftSacroIliac, LeftScaphoCapitate, 
			LeftScaphoLunate, LeftScaphoTalar, LeftScaphoTrapezoTrapezoid, LeftScapuloHumeral, 
			LeftSerratoScapular, LeftSerratoThoracic, LeftSternoCostoClavicular, LeftSubTalar, 
			LeftSuperiorRadioUlnar, LeftSuperiorTibioFibular, LeftSyndesmoCarpal, LeftSyndesmoUlnar, 
			LeftTarsoMetatarsal1, LeftTarsoMetatarsal2, LeftTarsoMetatarsal3, LeftTarsoMetatarsal4, 
			LeftTarsoMetatarsal5, LeftTemporoMandibular, LeftTibioTalar, LeftTibioTarsal, 
			LeftTrapezoCapitate, LeftTrapezoTrapezoid, LeftTriquetroHamate, LeftZygapophyseal10, 
			LeftZygapophyseal11, LeftZygapophyseal12, LeftZygapophyseal13, LeftZygapophyseal14, 
			LeftZygapophyseal15, LeftZygapophyseal16, LeftZygapophyseal17, LeftZygapophyseal18, 
			LeftZygapophyseal19, LeftZygapophyseal2, LeftZygapophyseal20, LeftZygapophyseal21, 
			LeftZygapophyseal22, LeftZygapophyseal23, LeftZygapophyseal3, LeftZygapophyseal4, 
			LeftZygapophyseal5, LeftZygapophyseal6, LeftZygapophyseal7, LeftZygapophyseal8, 
			LeftZygapophyseal9, ManubrioSternal, MedianAtlantoAxial, RightAcromioClavicular, 
			RightAtlantoOccipital, RightCalcaneoCuboidal, RightCapitatoHamate, RightCarpoMetacarpal1, 
			RightCarpoMetacarpal2, RightCarpoMetacarpal3, RightCarpoMetacarpal4, RightCarpoMetacarpal5, 
			RightChondroChondral1, RightChondroChondral2, RightChondroChondral3, RightChondroSternal1, 
			RightChondroSternal2, RightChondroSternal3, RightChondroSternal4, RightChondroSternal5, 
			RightChondroSternal6, RightChondroSternal7, RightCondyloTrochlear, RightCostoChondral1, 
			RightCostoChondral10, RightCostoChondral11, RightCostoChondral12, RightCostoChondral2, 
			RightCostoChondral3, RightCostoChondral4, RightCostoChondral5, RightCostoChondral6, 
			RightCostoChondral7, RightCostoChondral8, RightCostoChondral9, RightCostoCorporeal1, 
			RightCostoCorporeal10, RightCostoCorporeal11, RightCostoCorporeal12, RightCostoCorporeal2, 
			RightCostoCorporeal3, RightCostoCorporeal4, RightCostoCorporeal5, RightCostoCorporeal6, 
			RightCostoCorporeal7, RightCostoCorporeal8, RightCostoCorporeal9, RightCostoTransverse1, 
			RightCostoTransverse10, RightCostoTransverse2, RightCostoTransverse3, RightCostoTransverse4, 
			RightCostoTransverse5, RightCostoTransverse6, RightCostoTransverse7, RightCostoTransverse8, 
			RightCostoTransverse9, RightCoxoFemoral, RightCuneoCuboidal, RightFemoroPatellar, 
			RightFemoroTibial, RightFibuloTalar, RightFootRay1Interphalangeal, RightFootRay2DistalInterphalangeal, 
			RightFootRay2ProximalInterphalangeal, RightFootRay3DistalInterphalangeal, RightFootRay3ProximalInterphalangeal, RightFootRay4DistalInterphalangeal, 
			RightFootRay4ProximalInterphalangeal, RightFootRay5DistalInterphalangeal, RightFootRay5ProximalInterphalangeal, RightHandRay1Interphalangeal, 
			RightHandRay2DistalInterphalangeal, RightHandRay2ProximalInterphalangeal, RightHandRay3DistalInterphalangeal, RightHandRay3ProximalInterphalangeal, 
			RightHandRay4DistalInterphalangeal, RightHandRay4ProximalInterphalangeal, RightHandRay5DistalInterphalangeal, RightHandRay5ProximalInterphalangeal, 
			RightHumeroRadial, RightHumeroUlnar, RightInferiorRadioUlnar, RightInferiorTibioFibular, 
			RightIntermediateScaphoCuneal, RightLateralAtlantoAxial, RightLateralInterCuneal, RightLateralScaphoCuneal, 
			RightLumbosacral, RightLunoCapitate, RightLunoTriquetral, RightMedialInterCuneal, 
			RightMedialScaphoCuneal, RightMetacarpoPhalangeal1, RightMetacarpoPhalangeal2, RightMetacarpoPhalangeal3, 
			RightMetacarpoPhalangeal4, RightMetacarpoPhalangeal5, RightMetatarsoPhalangeal1, RightMetatarsoPhalangeal2, 
			RightMetatarsoPhalangeal3, RightMetatarsoPhalangeal4, RightMetatarsoPhalangeal5, RightMiddleRadioUlnar, 
			RightMiddleTibioFibular, RightPisoTriquetral, RightRadioLunate, RightRadioScaphoid, 
			RightSacroIliac, RightScaphoCapitate, RightScaphoLunate, RightScaphoTalar, 
			RightScaphoTrapezoTrapezoid, RightScapuloHumeral, RightSerratoScapular, RightSerratoThoracic, 
			RightSternoCostoClavicular, RightSubTalar, RightSuperiorRadioUlnar, RightSuperiorTibioFibular, 
			RightSyndesmoCarpal, RightSyndesmoUlnar, RightTarsoMetatarsal1, RightTarsoMetatarsal2, 
			RightTarsoMetatarsal3, RightTarsoMetatarsal4, RightTarsoMetatarsal5, RightTemporoMandibular, 
			RightTibioTalar, RightTibioTarsal, RightTrapezoCapitate, RightTrapezoTrapezoid, 
			RightTriquetroHamate, RightZygapophyseal10, RightZygapophyseal11, RightZygapophyseal12, 
			RightZygapophyseal13, RightZygapophyseal14, RightZygapophyseal15, RightZygapophyseal16, 
			RightZygapophyseal17, RightZygapophyseal18, RightZygapophyseal19, RightZygapophyseal2, 
			RightZygapophyseal20, RightZygapophyseal21, RightZygapophyseal22, RightZygapophyseal23, 
			RightZygapophyseal3, RightZygapophyseal4, RightZygapophyseal5, RightZygapophyseal6, 
			RightZygapophyseal7, RightZygapophyseal8, RightZygapophyseal9, XiphiSternal, 


			Invalid
		};
		static const char* TAG_PREFIX;		///<contains L000_...._Joints_ prefix used for VME Tags
		static const char* NAMES[];			///<array of joint names corresponding to JointEnum values
	};

	/** This class encapsulates enum of Muscles*/
	class MuscleType
	{
	public:
		/** enum with muscle names */
		enum EnumValues {
			Null = 0,

			LeftAbductorPollicisLongus, LeftAdductorBrevis, LeftAdductorLongus, LeftAdductorMagnus,
			LeftAnconeus, LeftArticularisGenu, LeftBicepsBrachiiCaputBrevis, LeftBicepsBrachiiCaputLongus,
			LeftBicepsFemorisCaputBrevis, LeftBicepsFemorisCaputLongus, LeftBrachialis, LeftBrachioRadialis,
			LeftCoracoBrachialis, LeftDeltoideus, LeftExtensorCarpiRadialisBrevis, LeftExtensorCarpiRadialisLongus,
			LeftExtensorCarpiUlnaris, LeftExtensorDigitiMinimi, LeftExtensorDigitorum, LeftExtensorDigitorumBrevis,
			LeftExtensorDigitorumLongus, LeftExtensorHallucisLongus, LeftExtensorIndicis, LeftExtensorPollicisBrevis,
			LeftExtensorPollicisLongus, LeftFlexorCarpiRadialis, LeftFlexorCarpiUlnaris, LeftFlexorDigitorumBrevis,
			LeftFlexorDigitorumLongus, LeftFlexorDigitorumProfondus, LeftFlexorDigitorumSuperficialis, LeftFlexorHallucisLongus,
			LeftFlexorPollicisLongus, LeftFootAbductorDigitiMinimi, LeftFootAbductorHallucis, LeftFootAdductorHallucis,
			LeftFootFlexorDigitiMinimiBrevis, LeftFootFlexorHallucisBrevis, LeftFootInterosseusDorsalis1, LeftFootInterosseusDorsalis2,
			LeftFootInterosseusDorsalis3, LeftFootInterosseusDorsalis4, LeftFootInterosseusPalmaris1, LeftFootInterosseusPalmaris2,
			LeftFootInterosseusPalmaris3, LeftFootLumbricalis1, LeftFootLumbricalis2, LeftFootLumbricalis3,
			LeftFootLumbricalis4, LeftFootOpponensDigitiMinimi, LeftGastrocnemiusCaputLateralis, LeftGastrocnemiusCaputMedialis,
			LeftGemellusInferior, LeftGemellusSuperior, LeftGluteusMaximus, LeftGluteusMedius,
			LeftGluteusMinimus, LeftGracilis, LeftHandAbductorDigitiMinimi, LeftHandAbductorPollicisBrevis,
			LeftHandAdductorPollicis, LeftHandFlexorDigitiMinimiBrevis, LeftHandFlexorPollicisBrevis, LeftHandInterosseusDorsalis1,
			LeftHandInterosseusDorsalis2, LeftHandInterosseusDorsalis3, LeftHandInterosseusDorsalis4, LeftHandInterosseusPalmaris1,
			LeftHandInterosseusPalmaris2, LeftHandInterosseusPalmaris3, LeftHandLumbricalis1, LeftHandLumbricalis2,
			LeftHandLumbricalis3, LeftHandLumbricalis4, LeftHandOpponensDigitiMinimi, LeftHandOpponensPollicis,
			LeftIlliacus, LeftInfraSpinatus, LeftLatissimusDorsi, LeftObturatorExternus,
			LeftObturatorInternus, LeftPalmarisLongus, LeftPectineus, LeftPectoralisMajor,
			LeftPectoralisMinor, LeftPeroneusBrevis, LeftPeroneusLongus, LeftPeroneusTertius,
			LeftPiriformis, LeftPlantaris, LeftPopliteus, LeftPronatorQuadratus,
			LeftPronatorTeres, LeftPsoasMajor, LeftPsoasMinor, LeftQuadratusFemoris,
			LeftQuadratusPlantae, LeftRectusFemoris, LeftSartorius, LeftSemiMembranosus,
			LeftSemiTendinosus, LeftSerratusAnterior, LeftSoleus,
			LeftSubclavius, LeftSubscapularis, LeftSupinator, LeftSupraSpinatus,
			LeftTensorFasciaLata, LeftTeresMajor, LeftTeresMinor, LeftTibialisAnterior,
			LeftTibialisPosterior, LeftTricepsBrachiiCaputLateralis, LeftTricepsBrachiiCaputLongus, LeftTricepsBrachiiCaputMedialis,
			LeftVastusIntermedius, LeftVastusLateralis, LeftVastusMedialis, RightAbductorPollicisLongus,
			RightAdductorBrevis, RightAdductorLongus, RightAdductorMagnus, RightAnconeus,
			RightArticularisGenu, RightBicepsBrachiiCaputBrevis, RightBicepsBrachiiCaputLongus, RightBicepsFemorisCaputBrevis,
			RightBicepsFemorisCaputLongus, RightBrachialis, RightBrachioRadialis, RightCoracoBrachialis,
			RightDeltoideus, RightExtensorCarpiRadialisBrevis, RightExtensorCarpiRadialisLongus, RightExtensorCarpiUlnaris,
			RightExtensorDigitiMinimi, RightExtensorDigitorum, RightExtensorDigitorumBrevis, RightExtensorDigitorumLongus,
			RightExtensorHallucisLongus, RightExtensorIndicis, RightExtensorPollicisBrevis, RightExtensorPollicisLongus,
			RightFlexorCarpiRadialis, RightFlexorCarpiUlnaris, RightFlexorDigitorumBrevis, RightFlexorDigitorumLongus,
			RightFlexorDigitorumProfondus, RightFlexorDigitorumSuperficialis, RightFlexorHallucisLongus, RightFlexorPollicisLongus,
			RightFootAbductorDigitiMinimi, RightFootAbductorHallucis, RightFootAdductorHallucis, RightFootFlexorDigitiMinimiBrevis,
			RightFootFlexorHallucisBrevis, RightFootInterosseusDorsalis1, RightFootInterosseusDorsalis2, RightFootInterosseusDorsalis3,
			RightFootInterosseusDorsalis4, RightFootInterosseusPalmaris1, RightFootInterosseusPalmaris2, RightFootInterosseusPalmaris3,
			RightFootLumbricalis1, RightFootLumbricalis2, RightFootLumbricalis3, RightFootLumbricalis4,
			RightFootOpponensDigitiMinimi, RightGastrocnemiusCaputLateralis, RightGastrocnemiusCaputMedialis, RightGemellusInferior,
			RightGemellusSuperior, RightGluteusMaximus, RightGluteusMedius, RightGluteusMinimus,
			RightGracilis, RightIlliacus, RightInfraSpinatus, RightLatissimusDorsi,
			RightObturatorExternus, RightObturatorInternus, RightPalmarisLongus, RightPectineus,
			RightPectoralisMajor, RightPectoralisMinor, RightPeroneusBrevis, RightPeroneusLongus,
			RightPeroneusTertius, RightPiriformis, RightPlantaris, RightPopliteus,
			RightPronatorQuadratus, RightPronatorTeres, RightPsoasMajor, RightPsoasMinor,
			RightQuadratusFemoris, RightQuadratusPlantae, RightRectusFemoris, RightSartorius,
			RightSemiMembranosus, RightSemiTendinosus, RightSerratusAnterior,
			RightSoleus, RightSubclavius, RightSubscapularis, RightSupinator,
			RightSupraSpinatus, RightTensorFasciaLata, RightTeresMajor, RightTeresMinor,
			RightTibialisAnterior, RightTibialisPosterior, RightTricepsBrachiiCaputLateralis, RightTricepsBrachiiCaputLongus,
			RightTricepsBrachiiCaputMedialis, RightVastusIntermedius, RightVastusLateralis, RightVastusMedialis,

			Invalid
		};
		static const char* TAG_PREFIX;		///<contains L000_...._Muscles_ prefix used for VME Tags
		static const char* NAMES[];			///<array of muscle names corresponding to MuscleEnum values
	};

	/** This class encapsulates enum of Ligaments*/
	class LigamentType
	{
	public:
		/** enum with ligament names */
		enum EnumValues {
			Null = 0,

			LeftAnnularLigamentHand, LeftAnteriorCruciateLigament, LeftForeArmInterosseusMembrane, LeftPalmarAponeurosisAnterior,
			LeftPatellarLigament, LeftPosteriorCruciateLigament, LeftShankInterosseusMembrane, LeftTriangularLig,
			RightAnnularLigamentHand, RightAnteriorCruciateLigament, RightForeArmInterosseusMembrane,
			RightPalmarAponeurosisAnterior, RightPatellarLigament, RightPosteriorCruciateLigament, RightShankInterosseusMembrane,
			RightTriangularLig,

			Invalid
		};
		static const char* TAG_PREFIX;	///<contains L000_...._Ligaments_ prefix used for VME Tags
		static const char* NAMES[];			///<array of ligament names corresponding to LigamentEnum values
	};

	/** This class encapsulates enum of Body Landmark*/
	class BodyLandmarkType
	{
	public:
		/** enum with muscle names */
		enum EnumValues {
			Null = 0,

			RightHipMostLateral,

			Invalid
		};

		static const char* TAG_PREFIX;	///<contains L000_...._Ligaments_ prefix used for VME Tags
		static const char* NAMES[];			///<array of ligament names corresponding to LigamentEnum values
	};

	typedef Enum< RegionType > RegionEnum;
	typedef Enum< BoneType > BoneEnum;
	typedef Enum< JointType > JointEnum;
	typedef Enum< MuscleType > MuscleEnum;
	typedef Enum< LigamentType > LigamentEnum;
	typedef Enum< BodyLandmarkType > BodyLandmarkEnum;

	typedef std::vector< RegionEnum > RegionEnumList;
	typedef std::vector< BoneEnum > BoneEnumList;
	typedef std::vector< JointEnum > JointEnumList;
	typedef std::vector< MuscleEnum > MuscleEnumList;
	typedef std::vector< LigamentEnum > LigamentEnumList;
	typedef std::vector< BodyLandmarkEnum > BodyLandmarkEnumList;

#pragma endregion RegionEnum, BoneEnum, ...

#pragma region GraphRelations
#define MAX_INITEM_ENTRIES	4
	/** Describes functional relationships between bones, muscles, regions, ... */
	class LHP_VME_EXPORT MSMGraphRelations
	{	
	public:
		/** Gets the bones to which the specified muscle (specified as MuscleType) are connected by tendons */ 				
		static void GetBonesByMuscle(int muscleType, BoneEnumList& output);

	protected:
		typedef struct ITEM_ENTRY 
		{
			int objId;
			int entries[MAX_INITEM_ENTRIES];
		} ITEM_ENTRY;

		static const ITEM_ENTRY MAP_MUSCLE_BONES[];	///<array of bone types names corresponding to muscle types

	protected:
		/** Callback from bsearch, compares two strings identified by a pointer*/
		static int Comparer(const int* key, const ITEM_ENTRY* data) {
			return *key - data->objId;
		}
	};
#pragma endregion

	/** Describes one node of the MSMGraph*/
	class LHP_VME_EXPORT MSMGraphNode
	{
	public:
		/** This class serves as a descriptor of MSMGraphNodes */
		class LHP_VME_EXPORT MSMGraphNodeDescriptor
		{
		public:
			/** Flags (can be combined) that describes the node*/
			enum DescFlags {
				UnknownNode = 0,

				//Tag based information
				RegionNode	= 1,										//the node contains some region data - m_RegionInfo is valid
				BoneNode		= 2,										//the node contains some bone data - m_BoneInfo is valid
				JointNode		= 4,										//the node contains some joint data - m_JointInfo is valid
				MuscleNode	= 8,										//the node contains some joint data - m_MuscleInfo is valid
				LigamentNode	= 16,									//the node contains some ligament data - m_LigamentInfo is valid
				BodyLandmarkCloudNode = 32,					//the node contains the landmarks for motion fusion of regions - BodyLandmarkInfo is valid

				//VME class based information
				MuscleWrapperNode = 64,							//medVMEMuscleWrapper (this may be accompanied with  MuscleNode)
				WrapperMeterNode = 128,							//mafVMEMeter, medVMEComputeWrapping (this may be also accompanied with  MuscleNode)
				LandmarkCloudNode = 256,						//mafVMELandmarkCloud
				LandmarkNode = 512,									//mafVMELandmark (for opened LandmarkClouds)
				GroupNode		= 1024,									//mafVMEGroup
				LinkNode		= 2048,									//mafVMEShortcut
				ParametricNode = 4096,							//mafVMESurfaceParametric (for constraints)

				//special
				LowResolutionNode = 8192,						//the node contains a lower resolution of data of another node
				HullNode = 16384,										//the node represents hull data of another node
			};

			DescFlags m_Flags;						///<describing flags

			RegionEnum m_RegionInfo;			///< info about region - valid only, if m_Flags contain RegionNode
			BoneEnum m_BoneInfo;					///< info about bone - valid only, if m_Flags contain BoneNode
			JointEnum m_JointInfo;				///< info about joint - valid only, if m_Flags contain JointNode
			MuscleEnum m_MuscleInfo;			///< info about muscle - valid only, if m_Flags contain MuscleNode
			LigamentEnum m_LigamentInfo;	///< info about ligament - valid only, if m_Flags contain LigamentNode
			BodyLandmarkEnum m_BodyLandmarkInfo;	///< info about body landmarks - valid only, if m_Flags contain BodyLandmarkCloudNode

		public:
			// ------- methods -----------
			MSMGraphNodeDescriptor(DescFlags value = UnknownNode) : m_Flags(value) { }

			/** Combines two descriptors together to give one common descriptor.
			If both input descriptors define the same type of Info (e.g., RegionInfo), Info is set to Invalid. */
			static MSMGraphNodeDescriptor Combine(const MSMGraphNodeDescriptor& desc1, const MSMGraphNodeDescriptor& desc2);

			/** Analyses the given node and returns the descriptor for it.*/
			static MSMGraphNodeDescriptor CreateDescriptor(mafNode* node);

			/** Returns the string representation  of this descriptor*/
			mafString ToString() const;
		};

	public:
		MSMGraphNodeDescriptor m_NodeDescriptor;						///<describes this node
		MSMGraphNodeDescriptor m_AncestorNodesDescriptor;		///<describes all nodes on the path from this node to root
		MSMGraphNodeDescriptor m_DescendantNodesDescriptor;	///<describes all nodes on the path from this node to its children (and further)

		mafNode* m_Vme;		///<VME described by this node

		MSMGraphNode* m_Parent;					///<parent node
		MSMGraphNode* m_LowerResNode;		///<node with the lower resolution of m_Vme
		MSMGraphNode* m_HigherResNode;	///<node with the higher resolution of m_Vme
		int m_LevelOfDetail;						///<level of detail of the m_Vme, 0 means the highest resolution,  -1 means unknown

		MSMGraphNode* m_HullNode;				///<node containing temporary data of m_Vme (Hull)
		MSMGraphNode* m_HullUserNode;		///<node for which we are a hull

		typedef std::vector< MSMGraphNode* > MSMGraphChildren;
		MSMGraphChildren m_Children;		///<children of the current node

	public:
		/** ctor */
		MSMGraphNode();

		/** dtor */
		~MSMGraphNode();
	public:

		/** Returns the string representation  of this node*/
		mafString ToString();
	};

protected:
	MSMGraphNode* m_RootNode;		///<Root level (typically a node for medVMEMusculoskeletalModel)

public:
	typedef std::vector< MSMGraphNode* > MSMGraphNodeList;
	typedef std::vector< mafNode* > mafVMENodeList;
	typedef MSMGraphNode::MSMGraphNodeDescriptor MSMGraphNodeDescriptor;

	/**
	Predefined values for Level of Details
	*/
	typedef struct LOD
	{
		enum EnumValues
		{
			Highest = 0,
			LR1 = 1,
			LR2 = 2,
			LR3 = 3,
			LR4 = 4,
			LR5 = 5,
			LR6 = 6,
			LR7 = 7,
			LR8 = 8,
			LR9 = 9,
			LR10 = 10,
			LR11 = 11,
			LR12 = 12,
			LR13 = 13,
			LR14 = 14,
			LR15 = 15,
			LR16 = 16,			
			Lowest =	0x0FFFFFF,		

			NoAlternative = 0x8000000,	///<cannot be used with Highest, Lowest, 
			Everything = 0xFFFFFFFF,		///<we will het all LOD	
		};
	} LOD;

public:
	medMSMGraph();
	~medMSMGraph();
public:

	/** Builds a graph for the given root node.
	N.B. when any VME changes, the graph needs to be rebuilt. */
	void BuildGraph(mafNode* node);

	/** Gets the root of the built graph */
	inline const MSMGraphNode* GetRoot()  const {
		return m_RootNode;
	}


#pragma region Get one-object methods
	/** Gets graph nodes for the specified region (specified as RegionType) that are descendants of root.
	LOD specifies which level of detail is requested - see GetMSMNodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetRegion(int region, MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const{
		GetMSMNodes(LOD::Highest, MSMGraphNodeDescriptor::RegionNode, 
			region, offsetof(MSMGraphNode, m_NodeDescriptor.m_RegionInfo),
			list, bRecursive, root);
	}

	/** Gets VMEs of the specified region (specified as RegionType) that are descendants of root.
	LOD specifies which level of detail is requested - see GetMSMNodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetRegion(int region, mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const{
		GetVMENodes(LOD::Highest, MSMGraphNodeDescriptor::RegionNode, 
			region, offsetof(MSMGraphNode, m_NodeDescriptor.m_RegionInfo),
			list, bRecursive, root);
	}

	/** Gets graph nodes for the specified bone (specified as BoneType) that are descendants of root.
	LOD specifies which level of detail is requested - see GetMSMNodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetBone(int bone, int lod, MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const{
		GetMSMNodes(lod, MSMGraphNodeDescriptor::BoneNode, 
			bone, offsetof(MSMGraphNode, m_NodeDescriptor.m_BoneInfo),
			list, bRecursive, root);
	}

	/** Gets VMEs of the specified bone (specified as BoneType) that are descendants of root.
	LOD specifies which level of detail is requested - see GetMSMNodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetBone(int bone, int lod, mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const{
		GetVMENodes(lod, MSMGraphNodeDescriptor::BoneNode, 
			bone, offsetof(MSMGraphNode, m_NodeDescriptor.m_BoneInfo),
			list, bRecursive, root);
	}

	/** Gets graph nodes for the specified muscle (specified as MuscleType) that are descendants of root.
	LOD specifies which level of detail is requested - see GetMSMNodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetMuscle(int msc, int lod, MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const{
		GetMSMNodes(lod, MSMGraphNodeDescriptor::MuscleNode, 
			msc, offsetof(MSMGraphNode, m_NodeDescriptor.m_MuscleInfo),
			list, bRecursive, root);
	}

	/** Gets VMEs of the specified muscle (specified as MuscleType) that are descendants of root.
	LOD specifies which level of detail is requested - see GetMSMNodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetMuscle(int msc, int lod, mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const{
		GetVMENodes(lod, MSMGraphNodeDescriptor::MuscleNode, 
			msc, offsetof(MSMGraphNode, m_NodeDescriptor.m_MuscleInfo),
			list, bRecursive, root);
	}		

	/** Gets graph nodes for the specified joint (specified as JointType) that are descendants of root.
	LOD specifies which level of detail is requested - see GetMSMNodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetJoint(int jnt, int lod, MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const{
		GetMSMNodes(lod, MSMGraphNodeDescriptor::JointNode, 
			jnt, offsetof(MSMGraphNode, m_NodeDescriptor.m_JointInfo),
			list, bRecursive, root);
	}

	/** Gets VMEs of the specified joint (specified as JointType) that are descendants of root.
	LOD specifies which level of detail is requested - see GetMSMNodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetJoint(int jnt, int lod, mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const{
		GetVMENodes(lod, MSMGraphNodeDescriptor::JointNode, 
			jnt, offsetof(MSMGraphNode, m_NodeDescriptor.m_JointInfo),
			list, bRecursive, root);
	}
#pragma endregion

#pragma region Get multiple-objects methods

	/** Gets graph nodes for regions that are descendants of root.	
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetRegions(MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		GetMSMNodes(LOD::Highest, MSMGraphNodeDescriptor::RegionNode, list, bRecursive, root);
	}

	/** Gets VMEs of regions that are descendants of root.	
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetRegions(mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL)  const  {
		GetVMENodes(LOD::Highest, MSMGraphNodeDescriptor::RegionNode, list, bRecursive, root);
	}	

	/** Gets graph nodes for bones that are descendants of root.
	LOD specifies which level of detail is requested - see GetMSMNodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetBones(int lod, MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const  {
		GetMSMNodes(lod, MSMGraphNodeDescriptor::BoneNode, list, bRecursive, root);
	}

	/** Gets VMEs of bones that are descendants of root.
	LOD specifies which level of detail is requested - see GetVMENodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetBones(int lod, mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const  {
		GetVMENodes(lod, MSMGraphNodeDescriptor::BoneNode, list, bRecursive, root);
	}

	/** Gets graph nodes for muscles that are descendants of root.
	LOD specifies which level of detail is requested - see GetMSMNodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetMuscles(int lod, MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL)  const {
		GetMSMNodes(lod, MSMGraphNodeDescriptor::MuscleNode, list, bRecursive, root);
	}

	/** Gets VMEs of muscles that are descendants of root.
	LOD specifies which level of detail is requested - see GetVMENodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetMuscles(int lod, mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		GetVMENodes(lod, MSMGraphNodeDescriptor::MuscleNode, list, bRecursive, root);
	}

	/** Gets graph nodes for joints that are descendants of root.
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetJoints(MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		GetMSMNodes(LOD::Highest, MSMGraphNodeDescriptor::JointNode, list, bRecursive, root);
	}

	/** Gets VMEs of joints that are descendants of root.
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetJoints(mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		GetVMENodes(LOD::Highest, MSMGraphNodeDescriptor::JointNode, list, bRecursive, root);
	}

	/** Gets graph nodes for body landmarks that are descendants of root.
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetBodyLandmarks(MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const  {
		GetMSMNodes(LOD::Highest, MSMGraphNodeDescriptor::BodyLandmarkCloudNode, list, bRecursive, root);
	}

	/** Gets VMEs of body landmarks that are descendants of root.
	LOD specifies which level of detail is requested - see GetVMENodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetBodyLandmarks(mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL)  const {
		GetVMENodes(LOD::Highest, MSMGraphNodeDescriptor::BodyLandmarkCloudNode, list, bRecursive, root);
	}

	/** Gets graph nodes for wrappers that are descendants of root.
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetWrappers(MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		GetMSMNodes(LOD::Highest, MSMGraphNodeDescriptor::WrapperMeterNode, list, bRecursive, root);
	}

	/** Gets VMEs of wrappers that are descendants of root.	
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetWrappers(mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		GetVMENodes(LOD::Highest, MSMGraphNodeDescriptor::WrapperMeterNode, list, bRecursive, root);
	}

	/** Gets graph nodes for muscle wrappers that are descendants of root.
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetMuscleWrappers(MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		GetMSMNodes(LOD::Highest, MSMGraphNodeDescriptor::MuscleWrapperNode, list, bRecursive, root);
	}

	/** Gets VMEs of muscle wrappers that are descendants of root.
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetMuscleWrappers(mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		GetVMENodes(LOD::Highest, MSMGraphNodeDescriptor::MuscleWrapperNode, list, bRecursive, root);
	}

	/** Gets graph nodes for ligaments that are descendants of root.
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetLigaments(MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		GetMSMNodes(LOD::Highest, MSMGraphNodeDescriptor::LigamentNode, list, bRecursive, root);
	}

	/** Gets VMEs of ligaments that are descendants of root.
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetLigaments(mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		GetVMENodes(LOD::Highest, MSMGraphNodeDescriptor::LigamentNode, list, bRecursive, root);
	}

	/** Gets graph nodes for landmark clouds that are descendants of root.
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetLandmarkClouds(MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		GetMSMNodes(LOD::Highest, MSMGraphNodeDescriptor::LandmarkCloudNode, list, bRecursive, root);
	}

	/** Gets VMEs of ligaments that are descendants of root.
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	inline void GetLandmarkClouds(mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		GetVMENodes(LOD::Highest, MSMGraphNodeDescriptor::LandmarkCloudNode, list, bRecursive, root);
	}
#pragma endregion
	/** Gets graph nodes that are descendants of root and match the given filter.
	Parameter lod specifies the desired level of details of nodes. Several nodes may represent the same object but
	each of them represents it in other quality, resolution, i.e., in various level of details. If lod is a non-negative 
	integer smaller or equal than LOD::Lowest, the method gets the node of the desired resolution (level of detail)
	or, if such a level does not exist, it gets the node of the closest existing higher level of detail. If the caller wants 
	to get the requested level of detail and nothing else, parameter lod must be a bitwise combination of required level 
	and LOD::NoAlternative. Hence, if for some object, there is no node of the required level-of-detail, the object
	will not be returned. It is important to point out that LOD::NoAlternative may not be used together with
	LOD::Lowest. Parameter lod can be also LOD::Everything to get all nodes regardles their level of details. 	
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	void GetMSMNodes(int lod, MSMGraphNodeDescriptor::DescFlags filter, MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const ;

	/** Gets VMEs that are descendants of root and match the given filter.
	LOD specifies which level of detail is requested - see GetMSMNodes for details. 
	If root is NULL, the graph is searched from GetRoot(). If bRecursive is false,
	only the root level is searched, otherwise, all regions in the subtree are found. */
	void GetVMENodes(int lod, MSMGraphNodeDescriptor::DescFlags filter, mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const ;

	/** Extracts VME nodes from graph nodes in source and stored them into dest*/
	void ExtractVMENodes(const MSMGraphNodeList& source, mafVMENodeList& dest) const ;

	/** FInds the MSMNode for the given vme in the tree starting from the specified root (or the current one, if not specified). 
	Returns NULL, if the node could not be found. */
	inline const MSMGraphNode* FindMSMNode(const mafNode* vme, bool bRecursive = false, const MSMGraphNode* root = NULL) const {
		return FindMSMNodeR(vme, ((root != NULL) ? root : GetRoot()), bRecursive);
	}

#if defined (_MSC_VER) && _MSC_VER >= 1600 && defined(_DEBUG)
	void DebugOutput(MSMGraphNode* root, int indent = 0);
#endif

protected:
	struct BUILD_GRAPH_INFO
	{
		std::map< mafNode*, MSMGraphNode* >  processed;		//already processed mafNode nodes and their MSMGraphNode association
		std::map< mafNode*, MSMGraphNode* >  resolutions;	//mafNode and its higher resolution MSMGraphNode
		std::map< mafNode*, MSMGraphNode* >  hulls;				//hull mafNode and its original MSMGraphNode
	};

	/** Recursive function that builds graph for the given node returning the created subtree.
	This method is supposed to be used by BuildGraph method. It should be initialized with
	parent = NULL	 and info == NULL */
	MSMGraphNode* BuildGraphR(mafNode* node, MSMGraphNode* parent = NULL, BUILD_GRAPH_INFO* info = NULL);

	/** Recursive method that adds graph nodes having the given descriptor into the  list.
	Paremeter lod is described in GetMSMNodes.
	The search starts at root (MAY NOT BE NULL). */
	void GetMSMNodesR(int lod, MSMGraphNodeDescriptor::DescFlags filter, const MSMGraphNode* root,
		MSMGraphNodeList& list, bool bRecursive) const ;	

	/** Gets graph nodes that are descendants of root and match the given filter.
	This version filters nodes to the exact type, e.g., LeftFemur - given in req_type parameter.
	Parameter offset specifies memory offset to MSMGraphNode, where the type is stored.
	All other parameters correlate to those in the public version.
	*/
	void GetMSMNodes(int lod, MSMGraphNodeDescriptor::DescFlags filter, 
		int req_type, int offset, MSMGraphNodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const ;

	/** Gets VMEs that are descendants of root and match the given filter.
	The meaning of all parameters is the same as in case of GetMSMNodes. */
	void GetVMENodes(int lod, MSMGraphNodeDescriptor::DescFlags filter, 
		int req_type, int offset, mafVMENodeList& list, bool bRecursive = false, const MSMGraphNode* root = NULL) const;

	/** Finds the MSMNode for the given vme in the tree starting from the given root
	Returns NULL, if the node could not be found. */
	const MSMGraphNode* FindMSMNodeR(const mafNode* vme ,const MSMGraphNode* root,  bool bRecursive) const ;
};

#pragma region Inlines
//-------------------------------------------------------------------------
//Converts the given region identifier into string
template < class TBase >
/*static*/  const char* medMSMGraph::Enum< TBase >::GetName(EnumValues value)
	//-------------------------------------------------------------------------
{
	int nVal = (int)value;
	if (nVal <= Null)
		return "Null";
	else if (nVal >= Invalid)
		return "Invalid";
	else
		return NAMES[nVal - 1];
}

//-------------------------------------------------------------------------
//Converts the given region identifier into tag name to be used in tag array of VMEs
template < class TBase >
/*static*/ mafString medMSMGraph::Enum< TBase >::GetTagName(EnumValues value)
	//-------------------------------------------------------------------------
{
	mafString ret = GetTagNamePrefix();
	if (value > Null && value < Invalid) {
		ret.Append("_");
		ret.Append(GetName(value));
	}
	return ret;
}

//-------------------------------------------------------------------------
//Converts the given string into the identifier (value).
//Returns Invalid, if the string could not be parsed.
template < class TBase >
/*static*/ typename medMSMGraph::Enum< TBase >::EnumValues medMSMGraph::Enum< TBase >::ParseName(const char* name)
	//-------------------------------------------------------------------------
{
	if (name == NULL || *name == '\0')
		return Invalid;	//nothing to found

	const char** pos_found = (const char**)bsearch(&name, NAMES, Invalid, sizeof(const char*),
		(int (*)(const void*, const void*)) Comparer );

	if (pos_found == NULL)
		return Invalid;

	return (EnumValues)((pos_found - NAMES) + 1);
}

//-------------------------------------------------------------------------
//Converts the given tag name  (used in tag array of VME) into the identifier (value) .
//Returns Null, if the tag name does not contain the prefix or Invalid, if the name could not be parsed.
template < class TBase >
/*static*/ typename medMSMGraph::Enum< TBase >::EnumValues medMSMGraph::Enum< TBase >::ParseTagName(const char* tagname)
	//-------------------------------------------------------------------------
{
	if (tagname == NULL)
		return Null;	//prefix not found

	const char* mask = GetTagNamePrefix();
	const char* name = tagname;
	while (*name == *mask && *name != '\0') {	//we are not at the end of any string
		mask++; name++;
	}

	if (*mask != '\0')
		return Null;	//we have not found prefix

	if (*name == '\0')
		return Invalid;

	//continue with parsing the name
	return ParseName(++name);	//skip '_'
}

#pragma endregion Inlines
#endif //__medMSMGraph_h