/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: medVMEMusculoSkeletalModel.h,v $
  Language:  C++
  Date:      $Date: 2012-04-30 14:52:43 $
  Version:   $Revision: 1.1.2.8 $
  Authors:   Josef Kohout
==========================================================================
  Copyright (c) 2011
  University of West Bohemia
=========================================================================*/
#ifndef __medVMEMusculoSkeletalModel_h
#define __medVMEMusculoSkeletalModel_h

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafVMEGroup.h"
#include "medMSMGraph.h"
#include "lhpVMEDefines.h"
#include <set>

//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------
class mafGUIDialog;

/** medVMEMusculoSkeletalModel - a VME use to create hierarchical assemblies of VME's of musculoskeletal model.  
  @sa mafVME mafMatrixVector */
class LHP_VME_EXPORT medVMEMusculoSkeletalModel : public mafVMEGroup
{
public:
	typedef medMSMGraph::LOD LOD;	//level of details

	typedef medMSMGraph::RegionType RegionsType;
	typedef medMSMGraph::BoneType BonesType;
	typedef medMSMGraph::MuscleType MusclesType;
	

protected:
	enum MSMMODEL_WIDGET_ID
	{
		ID_ENABLE_FILTERING = Superclass::ID_LAST,
		ID_ADD_REGIONS_BONES,
		ID_ADD_REGIONS_MUSCLES,
		ID_ADD_ITEMS_BONES,
		ID_ADD_ITEMS_MUSCLES,
		ID_REMOVE_BONES,
		ID_REMOVE_MUSCLES,
		ID_DEFORM_METHOD,
		ID_MULTIPLE_OBJECTS,
		ID_PROGRESSIVE_HULL,
        ID_PARTICLE_ITERATION_STEP,
        ID_PARTICLE_ITERATION_NUMBER,
		ID_COMMIT_CHANGES,
		ID_USE_FIXED_TIMESTEP,

		ID_ADDGUI_OK,
		ID_ADDGUI_CANCEL,

		ID_LAST,

		ID_INTERPOLATE_STEP_COUNT,
	};

#pragma region Settings
	int m_DeformationMethod;		///<0 = fast skinning, 1 = a new PK (energy minimization )method is used, it preserves volume, 2 = mass spring method
	int m_UseMultipleObjects;		///<1 = penetration should be prevented - valid, if m_DeformationMethod == 1
	int m_UseProgressiveHulls;		///<1 = use progressive hulls instead of QuadricDecimation - valid, if m_DeformationMethod == 1
	int m_EnableFiltering;			///<1 = filtering allowed	 (only, if m_DeformationMethod == 1)		
	int m_UseFixedTimestep;			///<1 = fixed time step should be used (only, if m_DeformationMethod == 2)		
	double m_IterationStep;         ///<the time step between the penetration checking between particles in different musucles for the mass spring method (only, if m_DeformationMethod == 2)
    int m_IterationNum;             ///<the total iteration number for the mass spring method (only, if m_DeformationMethod == 2)
	int m_InterpolateStepCount;		///<number of steps used for the interpolated PK deformation method (1 = original PK method, < 1 no steps are used>
    
	typedef RegionsType::EnumValues  medRegionsType;
	typedef BonesType::EnumValues  medBonesType;
	typedef MusclesType::EnumValues  medMusclesType;

	typedef std::set< medRegionsType > medRegionsSet;
	typedef std::set< medBonesType > medBonesSet;
	typedef std::set< medMusclesType > medMusclesSet;

	medRegionsSet m_Regions_Bones;		///<bones regions to be excluded
	medRegionsSet m_Regions_Muscles;	///<muscles regions to be excluded
	medBonesSet m_Items_Bones;				///<bones regions to be excluded
	medMusclesSet m_Items_Muscles;		///<muscles regions to be excluded

#pragma endregion Settings

	medMSMGraph m_MSMGraph;		///< Graph constructed for the underlying tree
	bool m_MSMGraphValid;			///< true, if m_MSMGraph is valid

	bool m_bWarningDisplayed;	///<false, if Warning should be displayed
	int m_ConfirmTimestamp;		///<counter of confirmed changes

	//typedef std::map < std::string,  mafVMENodeList;	///<typedef to make alias

public:
  mafTypeMacro(medVMEMusculoSkeletalModel, mafVMEGroup);

	typedef medMSMGraph::mafVMENodeList mafVMENodeList;	///<typedef to make alias
	

  /** return icon */
  static char** GetIcon();
  
	/** Gets the instance to MSMGraph. 
	N.B. The returned instance becomes invalid when any node in the tree is changed! */
	inline const medMSMGraph* GetMSMGraph(bool bForceRebuilt = false)
	{
		if (!m_MSMGraphValid || bForceRebuilt) 
		{
			m_MSMGraph.BuildGraph(this);
			m_MSMGraphValid = true;
		}

		return &m_MSMGraph;
	}

	/** Returns the current time stamp of confirm changes - to be used by medVMEMuscleWrapper
	to detect, if it should change because its muscleskeletonmodel has changed*/
	inline int GetConfirmTimeStamp() const {
		return m_ConfirmTimestamp;
	}

	/** Returns the code of the deformation method that should be used.
	0 = original fast skinning.
	1 = PK energy minimization method, which guarantee volume preservation but 
	may not work as expected for non-manifolds or coarse meshes, is supposed to be used for 
	muscle deformations in medVMEMuscleWrapper
	2 = Mass spring system method, which uses mass spring deformable models to represent muscles
	and takes into account both the bones and other muscles when deforming the muscle
	3 = Interpolated PK method - PK method repeated in several iteration for
	bodes positions interpolated between the moved rest pose to current pose*/
	inline int GetDeformationMethod(){
		return m_DeformationMethod;
	}

	/** Returns the number of iteration steps for Interpolated PK deformation method.
	computation. */
	inline int GetInterpolateStepCount(){
		return m_InterpolateStepCount;
	}
	/** Returns true, if PK energy minimization method is supposed to be used
	and also penetration prevention technique should be used, i.e., 
	methods GetMuscleWrappers and GetBoneXX should be called and
	all returned VMEs should be set to be input to PK Method */
	inline bool UsePenetrationPrevention(){
		return m_DeformationMethod != 0 && m_UseMultipleObjects != 0; 
	}

	/** Returns true, if PK energy minimization method is supposed to be used
	and that the user should set it to use progressive hulls. */
	inline bool UseProgressiveHulls(){
		return m_DeformationMethod != 0 && m_UseProgressiveHulls != 0; 
	}

	/** Returns true, if mass-spring method is supposed to be used
	and that the user should set it to use fixed time step. */
	inline bool UseFixedTimestep(){
		return m_DeformationMethod == 2 && m_UseFixedTimestep != 0;
	}

    /** Returns the time step between the penetration checking between 
    particles in different musucles for the mass spring method. */
    inline float GetParticleIterStep() { 
        return m_IterationStep; 
    }

    /** Returns the total iteration number for the mass spring method. */
    inline int GetParticleMaxIterNum() { 
        return m_IterationNum; 
    }

	/** Gets the list of muscle wrappers active in this model. 
	N.B. VMEs marked to be ignored in the model are not returned. 
	In order to get all muscle wrappers, use GetMSMGraph()->GetMuscleWrappers method. */
	virtual void GetMuscleWrappers(mafVMENodeList& output);

	/** Gets the list of bones active in this model. 
	N.B. VMEs marked to be ignored in the model are not returned. 
	In order to get all bones, use GetMSMGraph()->GetMuscles method. */
	virtual void GetBones(mafVMENodeList& output, int lod = LOD::Highest);

	/** Gets the list of group of bones active in this model. 
	This returns less objects than GetBones and hence should be preferred.
	N.B. VMEs marked to be ignored in the model are not returned. */
	virtual void GetBoneGroups(mafVMENodeList& output, int lod = LOD::Highest);

protected:
  medVMEMusculoSkeletalModel();
  virtual ~medVMEMusculoSkeletalModel();    

public:
	/** Process events coming from other objects */ 
	/*virtual*/ void OnEvent(mafEventBase *maf_event);

	/** Copy the contents of another musculoskeletal model VME into this one. */
	/*virtual*/ int DeepCopy(mafNode *a);

	/** Compare with another musculoskeletal model VME. */
	/*virtual*/ bool Equals(mafVME *vme);

protected:
	/** Stores this object into the storage element passed as argument. */
	/*virtual*/ int InternalStore(mafStorageElement *parent);
	
	/** Restores this object from the storage element passed as argument. */
	/*virtual*/ int InternalRestore(mafStorageElement *node);
	
	/** Internally used to create a new instance of the GUI.*/
	/*virtual*/ mafGUI *CreateGui();

	/** Populates both exclude lists with the data */
	virtual void PopulateExcludeLists();

	/** Gets the data from both exclude lists*/
	virtual void RetrieveExcludeLists();

	/** Handles the commit changes event 
	It stores changes from lists into data structures*/
	virtual void OnCommitChanges();

	/** Handles the event of adding regions into the given list*/
	virtual void OnAddRegions(wxListBox* list);

	/** Handles the event of adding bones into the given list*/
	virtual void OnAddBones(wxListBox* list);

	/** Handles the event of adding muscles into the given list*/
	virtual void OnAddMuscles(wxListBox* list);

	/** Handles the event of removing selected items from the given list*/
	virtual void OnRemoveItems(wxListBox* list);

	/** Displays a warning that any change must be confirmed (committed)*/
	virtual void DisplayWarning(bool bOnlyIfNotDisplayedPreviously = true);

	/** Creates m_AddDialog dialog to be used for adding items */
	virtual void CreateAddDialog();

	/** Destroys m_AddDialog dialog  */
	virtual void DeleteAddDialog();	

	/** Extracts from the given msm_list VMEs whose region is included in reg_set set.*/
	void ExtractVMEsExcludedByRegions(const medMSMGraph::MSMGraphNodeList& msm_list, 
		const medRegionsSet& reg_set, std::set < mafVME* >& out_set);

private:
	/** template method to restore the set (std::set) identified by its name.
	returns MAF_OK or MAF_ERROR depending on, if it succeeds*/
	template < typename T >
	int InternalRestore(mafStorageElement *node, const char* name, std::set < T >& out_set);

	/** template method to store the set (std::set) identified by its name.
	returns MAF_OK or MAF_ERROR depending on, if it succeeds*/
	template < typename T >
	int InternalStore(mafStorageElement *node, const char* name, const std::set < T >& input_set);
			
	/** populates the exclude list with regions or items (determined by isregion) 
	TConvert is the class used to convert enums to strings */
	template < typename T, class TConvert >
	void PopulateList(wxListBox* list, const std::set < T >& input_items_set, bool isregion);

	/** retrieves regions or items (determined by isregion)  from the given list
	TConvert is the class used to convert enums to strings */
	template < typename T, class TConvert >
	void RetrieveList(const wxListBox* list, std::set < T >& out_set, bool isregion);
	
	/** populates m_listAddDialog list with all items in TConvert and checks those from input_items_set 
	TConvert is the class used to convert enums to strings */
	template < typename T, class TConvert >
	void PopulateListAddDialog(const std::set < T >& input_items_set);

	/** retrieves checked items from m_listAddDialog list
	TConvert is the class used to convert enums to strings */
	template < typename T, class TConvert >
	void RetrieveListAddDialog(std::set < T >& out_set);

#pragma region GUI stuff
protected:
	int m_EnableFilteringGui;
	int m_DeformationMethodGui;			///<0 - original fast skinning, 1 = a new PK (energy minimization )method is used, it preserves volume, 2 - mss method
	int m_UseMultipleObjectsGui;		///<1 = penetration should be prevented - valid, if m_DeformationMethod == 1
	int m_UseProgressiveHullsGui;		///<1 = use progressive hulls instead of QuadricDecimation - valid, if m_DeformationMethod == 1
    float m_IterationStepGui;           ///<the time step integrated between the penetration checking between particles in different musucles for the mass spring method (only, if m_DeformationMethod == 2)
	int m_UseFixedTimestepGui;			///<enables the fixed time step for the mass spring method (only, if m_DeformationMethod == 2)
    int m_IterationNumGui;              ///<the total iteration number for the mass spring method (only, if m_DeformationMethod == 2)

	wxListBox* m_listExludedBones;
	wxListBox* m_listExludedMuscles;

	mafGUIDialog* m_AddDialog;
	wxListBox* m_listAddDialog;
#pragma endregion GUI stuff

private:
  medVMEMusculoSkeletalModel(const medVMEMusculoSkeletalModel&); // Not implemented
  void operator=(const medVMEMusculoSkeletalModel&); // Not implemented
};

#endif
