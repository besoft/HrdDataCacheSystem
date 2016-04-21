/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medVMEMusculoSkeletalModel.cpp,v $
Language:  C++
Date:      $Date: 2012-04-30 14:52:43 $
Version:   $Revision: 1.1.2.9 $
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


#include "medVMEMusculoSkeletalModel.h"
#include "medVMEMuscleWrapper.h"
#include "mafGUI.h"
#include "mafGUIValidator.h"
#include "mafGUIDialog.h"
#include "mafGUICheckListBox.h"

#include "mafTransform.h"
#include "mafMatrixVector.h"
#include "mafNode.h"
#include "mafDbg.h"

//-------------------------------------------------------------------------
mafCxxTypeMacro(medVMEMusculoSkeletalModel)
	//-------------------------------------------------------------------------

#define DEFAULT_USE_PKMETHOD				1
#define DEFAULT_USE_MULTIPLEOBJECTS			1
#define DEFAULT_ENABLE_FILTERING			1
#define DEFAULT_USE_PROGRESSIVEHULL			1
#define DEFAULT_USE_FIXEDTIMESTEP			1
#define DEFAULT_PARTICLE_ITERATIONSTEP      0.4
#define DEFAULT_PARTICLE_PENEITERNUM        0
#define DEFAULT_PARTICLE_ITERATIONNUM       70
#define DEFAULT_INTERPOLATION_STEP_COUNT    3;

	//-------------------------------------------------------------------------
	medVMEMusculoSkeletalModel::medVMEMusculoSkeletalModel()
	//-------------------------------------------------------------------------
{
	m_MSMGraphValid = false;

	m_DeformationMethodGui = m_DeformationMethod = DEFAULT_USE_PKMETHOD;
	m_UseMultipleObjectsGui = m_UseMultipleObjects = DEFAULT_USE_MULTIPLEOBJECTS;
	m_EnableFilteringGui = m_EnableFiltering = DEFAULT_ENABLE_FILTERING;
	m_UseProgressiveHullsGui = m_UseProgressiveHulls = DEFAULT_USE_PROGRESSIVEHULL;
	m_UseFixedTimestepGui = m_UseProgressiveHulls = DEFAULT_USE_FIXEDTIMESTEP;
	m_IterationStepGui = m_IterationStep = DEFAULT_PARTICLE_ITERATIONSTEP;
	m_IterationNumGui = m_IterationNum = DEFAULT_PARTICLE_ITERATIONNUM;
	m_listExludedBones = m_listExludedMuscles = NULL;
	m_bWarningDisplayed = false;

	m_AddDialog = NULL;
	m_listAddDialog = NULL;
	m_ConfirmTimestamp = 0;

	m_InterpolateStepCount = DEFAULT_INTERPOLATION_STEP_COUNT;
}

//-------------------------------------------------------------------------
medVMEMusculoSkeletalModel::~medVMEMusculoSkeletalModel()
	//-------------------------------------------------------------------------
{

}

//-------------------------------------------------------------------------
//Extracts from the given msm_list VMEs whose region is included in reg_set set.
void medVMEMusculoSkeletalModel::ExtractVMEsExcludedByRegions(const medMSMGraph::MSMGraphNodeList& msm_list, 
	const medRegionsSet& reg_set, std::set < mafVME* >& out_set)
{
	for (medMSMGraph::MSMGraphNodeList::const_iterator 
		it = msm_list.begin(); it != msm_list.end(); it++)
	{
		//we need to find the first valid parent region
		const medMSMGraph::MSMGraphNode* node_reg = *it;
		while (node_reg != NULL && 
			node_reg->m_AncestorNodesDescriptor.m_RegionInfo == medMSMGraph::RegionEnum::Invalid) {
				node_reg = node_reg->m_Parent;
		}

		if (node_reg != NULL && 
			reg_set.find(node_reg->m_AncestorNodesDescriptor.m_RegionInfo) != reg_set.end()){
				out_set.insert((mafVME*)(*it)->m_Vme);
		}	
	}
}

//-------------------------------------------------------------------------
//Gets the list of muscle wrappers active in this model. 
//N.B. VMEs marked to be ignored in the model are not returned. 
//In order to get all muscle wrappers, use GetMSMGraph()->GetMuscleWrappers method.
/*virtual*/ void medVMEMusculoSkeletalModel::GetMuscleWrappers(mafVMENodeList& output)
{
	GetMSMGraph();	//make sure we have an updated graph	
	if (m_EnableFiltering == 0)
		m_MSMGraph.GetMuscleWrappers(output, true);
	else
	{
		//so this won't be as easy as we would like it
		output.clear();

		//get all muscle wrappers		
		medMSMGraph::MSMGraphNodeList msm_list;
		m_MSMGraph.GetMuscleWrappers(msm_list, true);	

		//find the musculoskeletal model of muscles referred in our muscle wrappers
		medVMEMusculoSkeletalModel* musclemodel = NULL;
		for (medMSMGraph::MSMGraphNodeList::const_iterator 
			it = msm_list.begin(); it != msm_list.end(); it++)
		{		
			//get muscle wrapper and its muscle
			medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast((*it)->m_Vme);
			mafVME* muscle = wrapper->GetMuscleVME_RP();
			_VERIFY_CMD(muscle != NULL, continue);	//muscle should not be NULL

			mafNode* parent = muscle->GetParent();
			while (parent != NULL && (musclemodel = medVMEMusculoSkeletalModel::SafeDownCast(parent)) == NULL)
			{
				parent = parent->GetParent();
			}

			if (musclemodel != NULL)
				break;	//we have found it
		}

		_VERIFY_RET(musclemodel != NULL);

		//first we need to get any muscle VMEs that are not excluded		
		medMSMGraph::MSMGraphNodeList msm_muscle_list;
		musclemodel->GetMSMGraph()->GetMuscles(LOD::Highest, msm_muscle_list, true);

		std::set< mafVME* > setExcludedVME;
		ExtractVMEsExcludedByRegions(msm_muscle_list, m_Regions_Muscles, setExcludedVME);

		//and extract excluded muscles items list 
		for (medMSMGraph::MSMGraphNodeList::const_iterator 
			it = msm_muscle_list.begin(); it != msm_muscle_list.end(); it++)
		{			
			const medMSMGraph::MSMGraphNode* node = *it;
			if (m_Items_Muscles.find(node->m_NodeDescriptor.m_MuscleInfo) != m_Items_Muscles.end()){
				setExcludedVME.insert((mafVME*)node->m_Vme);
			}			
		}

		//extract valid muscle wrappers				
		for (medMSMGraph::MSMGraphNodeList::const_iterator 
			it = msm_list.begin(); it != msm_list.end(); it++)
		{		
			//get muscle wrapper and its muscle
			medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast((*it)->m_Vme);
			mafVME* muscle = wrapper->GetMuscleVME_RP();

			//if the muscle is not in setExcludedVME, include it into output			
			if (setExcludedVME.find(muscle) == setExcludedVME.end()) {
				output.push_back(wrapper);
			}
		}
	}
}

//-------------------------------------------------------------------------
//Gets the list of bones active in this model. 
//N.B. VMEs marked to be ignored in the model are not returned. 
//In order to get all bones, use GetMSMGraph()->GetMuscles method.
/*virtual*/ void medVMEMusculoSkeletalModel::GetBones(mafVMENodeList& output, int lod)
{
	GetMSMGraph();	//make sure we have an updated graph	
	if (m_EnableFiltering == 0)
		m_MSMGraph.GetBones(lod, output, true);
	else
	{
		//so this won't be as easy as we would like it
		output.clear();

		//first get all bones
		medMSMGraph::MSMGraphNodeList msm_list;
		m_MSMGraph.GetBones(lod, msm_list, true);

		//extract VMEs that should be excluded by Regions
		std::set< mafVME* > setExcludedVME;
		ExtractVMEsExcludedByRegions(msm_list, m_Regions_Bones, setExcludedVME);

		//save to output VMEs not present in setExcludedVME and not excluded by items
		for (medMSMGraph::MSMGraphNodeList::const_iterator 
			it = msm_list.begin(); it != msm_list.end(); it++)
		{			
			const medMSMGraph::MSMGraphNode* node = *it;
			if (setExcludedVME.find(((mafVME*)node->m_Vme)) == setExcludedVME.end() &&
				m_Items_Bones.find(node->m_NodeDescriptor.m_BoneInfo) == m_Items_Bones.end()){
					output.push_back(node->m_Vme);
			}			
		}		
	}
}


//-------------------------------------------------------------------------
//Gets the list of group of bones active in this model. 
//	This returns less objects than GetBones and hence should be preferred.
//	N.B. VMEs marked to be ignored in the model are not returned.
/*virtual*/ void medVMEMusculoSkeletalModel::GetBoneGroups(mafVMENodeList& output, int lod)
{
	//this is the most difficult extraction since we want to get VMEs with set Bones flag but
	//not being a part of excluded region +  having at least one not excluded Bone item as its descendant	
	output.clear();

	//first get all bones and bones groups of the highest resolution
	medMSMGraph::MSMGraphNodeList msm_list;
	m_MSMGraph.GetBones(LOD::Highest, msm_list, true);

	//extract VMEs that should be excluded by Regions
	std::set< mafVME* > setExcludedVME;
	ExtractVMEsExcludedByRegions(msm_list, m_Regions_Bones, setExcludedVME);

	//now, we will exclude bones and bones groups without any bone
	for (medMSMGraph::MSMGraphNodeList::const_iterator 
		it = msm_list.begin(); it != msm_list.end(); it++)
	{			
		const medMSMGraph::MSMGraphNode* node = *it;
		if (node->m_NodeDescriptor.m_BoneInfo != medMSMGraph::BoneEnum::Invalid ||
			setExcludedVME.find(((mafVME*)node->m_Vme)) != setExcludedVME.end())
			continue;	//not a bone group or in excluded region

		//it is a bone group of highest resolution, so it contains bones (of various resolution) and its lower resolution as its children 		
		medMSMGraph::MSMGraphNodeList msm_bones_list;
		m_MSMGraph.GetBones(lod, msm_bones_list, true, node);

		//check, if the current bone group being tested has at least one bone that was not excluded
		bool bBonesGroupNotExcluded = false;
		for (medMSMGraph::MSMGraphNodeList::const_iterator 
			it2 = msm_bones_list.begin(); it2 != msm_bones_list.end(); it2++)
		{		
			const medMSMGraph::MSMGraphNode* node2 = *it2;
			if (node2->m_NodeDescriptor.m_BoneInfo == medMSMGraph::BoneEnum::Invalid)
				continue;	//this is bones group

			if (m_Items_Bones.find(node2->m_NodeDescriptor.m_BoneInfo) == m_Items_Bones.end()){
				bBonesGroupNotExcluded = true;	//we may proceed
				break;
			}			
		}		

		if (bBonesGroupNotExcluded)
		{
			//MSMGraph does not check the passed root, only its children, so we may need to add node
			bool bAdded = false;			
			for (medMSMGraph::MSMGraphNodeList::const_iterator 
				it2 = msm_bones_list.begin(); it2 != msm_bones_list.end(); it2++)
			{
				const medMSMGraph::MSMGraphNode* node2 = *it2;
				if (node2->m_NodeDescriptor.m_BoneInfo == medMSMGraph::BoneEnum::Invalid){
					output.push_back(node2->m_Vme);	bAdded = true;
				}			
			}			

			if (!bAdded || lod == LOD::Everything)
				output.push_back((node)->m_Vme);
		}
	}
}


//-------------------------------------------------------------------------
//Copy the contents of another musculoskeletal model VME into this one.
/*virtual*/ int medVMEMusculoSkeletalModel::DeepCopy(mafNode *a)
{
	medVMEMusculoSkeletalModel* mm = medVMEMusculoSkeletalModel::SafeDownCast(a);
	if (mm != NULL && Superclass::DeepCopy(a)==MAF_OK)
	{
		this->m_EnableFilteringGui = this->m_EnableFiltering = mm->m_EnableFiltering;
		this->m_Regions_Bones = mm->m_Regions_Bones;
		this->m_Regions_Muscles = mm->m_Regions_Muscles;
		this->m_Items_Bones = mm->m_Items_Bones;
		this->m_Items_Muscles = mm->m_Items_Muscles;

		if (m_Gui != NULL)		
			PopulateExcludeLists();		

		m_ConfirmTimestamp++;
		return MAF_OK;
	}  
	return MAF_ERROR;
}

//-------------------------------------------------------------------------
//Compare with another musculoskeletal model VME.
/*virtual*/ bool medVMEMusculoSkeletalModel::Equals(mafVME *vme)
{
	if (!Superclass::Equals(vme))   //checks also Links
		return false;

	medVMEMusculoSkeletalModel* mm = medVMEMusculoSkeletalModel::SafeDownCast(vme);
	if (mm == NULL || mm->m_EnableFiltering != this->m_EnableFiltering)
		return false;


	//compare sets
	return (
		this->m_Regions_Muscles == mm->m_Regions_Muscles &&
		this->m_Regions_Bones == mm->m_Regions_Bones &&
		this->m_Items_Muscles == mm->m_Items_Muscles &&
		this->m_Items_Bones == mm->m_Items_Bones
		);
}

//-------------------------------------------------------------------------
// Stores this object into the storage element passed as argument.
/*virtual*/ int medVMEMusculoSkeletalModel::InternalStore(mafStorageElement *parent)
{
	if (Superclass::InternalStore(parent) == MAF_OK)  //stores material + links to muscle and OI areas VMEs
	{
		parent->StoreInteger("DeformationMethod", m_DeformationMethod);
		parent->StoreInteger("InterpolationStepCount", m_InterpolateStepCount);
		parent->StoreInteger("UseMultiObj", m_UseMultipleObjects);
		parent->StoreInteger("UseProgHull", m_UseProgressiveHulls);
		parent->StoreInteger("UseFixedTimestep",m_UseFixedTimestepGui);
		parent->StoreInteger("EnableFiltering", m_EnableFiltering);
		parent->StoreDouble("ParticleIterStep", m_IterationStep);
		parent->StoreInteger("ParticleMaxIterNum", m_IterationNum);

		InternalStore< medRegionsType > (parent, "Regions_Bones", m_Regions_Bones);
		InternalStore< medRegionsType > (parent, "Regions_Muscles", m_Regions_Muscles);
		InternalStore< medBonesType > (parent, "Items_Bones", m_Items_Bones);
		InternalStore< medMusclesType > (parent, "Items_Muscles", m_Items_Muscles);

		return MAF_OK;
	}
	return MAF_ERROR;
}

//-------------------------------------------------------------------------
// Restores this object from the storage element passed as argument.
/*virtual*/ int medVMEMusculoSkeletalModel::InternalRestore(mafStorageElement *node)
{
	if (Superclass::InternalRestore(node) != MAF_OK)
		return MAF_ERROR;
	else
	{   			
		if (node->RestoreInteger("DeformationMethod", m_DeformationMethod) != MAF_OK)
			m_DeformationMethod = DEFAULT_USE_PKMETHOD;

		if (node->RestoreInteger("InterpolationStepCount", m_InterpolateStepCount) != MAF_OK)
			m_InterpolateStepCount = DEFAULT_INTERPOLATION_STEP_COUNT;

		if (node->RestoreInteger("UseMultiObj", m_UseMultipleObjects) != MAF_OK)
			m_UseMultipleObjects = DEFAULT_USE_MULTIPLEOBJECTS;

		if (node->RestoreInteger("UseProgHull", m_UseProgressiveHulls) != MAF_OK)
			m_UseProgressiveHulls = DEFAULT_USE_PROGRESSIVEHULL;
		
        if (node->RestoreInteger("UseFixedTimestep", m_UseFixedTimestep) != MAF_OK)
			m_UseFixedTimestep = DEFAULT_USE_FIXEDTIMESTEP;

		if (node->RestoreInteger("EnableFiltering", m_EnableFiltering) != MAF_OK)
			m_EnableFiltering = DEFAULT_ENABLE_FILTERING;

		if (node->RestoreDouble("ParticleIterStep", m_IterationStep) != MAF_OK)
			m_IterationStep = DEFAULT_PARTICLE_ITERATIONSTEP;

		if (node->RestoreInteger("ParticleMaxIterNum", m_IterationNum) != MAF_OK)
			m_IterationNum = DEFAULT_PARTICLE_ITERATIONNUM;

		if (InternalRestore< medRegionsType > (node, "Regions_Bones", m_Regions_Bones) != MAF_OK ||
			InternalRestore< medRegionsType > (node, "Regions_Muscles", m_Regions_Muscles)  != MAF_OK ||
			InternalRestore< medBonesType > (node, "Items_Bones", m_Items_Bones)  != MAF_OK ||
			InternalRestore< medMusclesType > (node, "Items_Muscles", m_Items_Muscles)  != MAF_OK)
			return MAF_ERROR;			

		m_ConfirmTimestamp++;
		return MAF_OK;
	}	
}

//-------------------------------------------------------------------------
//template method to restore the set (std::set) identified by its name.
//	returns MAF_OK or MAF_ERROR depending on, if it succeeds
template < typename T >
int medVMEMusculoSkeletalModel::InternalRestore(mafStorageElement *node, const char* name, std::set < T >& out_set)
{
	out_set.clear();	//empty the set
	int nCount;

	mafString strNum = "Num_Of_";

	if (node->RestoreInteger(strNum.Append(name).GetCStr(), nCount) == MAF_OK && nCount != 0)		
	{				
		T* tmp = new T[nCount];
		if (node->RestoreVectorN(name, (int*)tmp, nCount) != MAF_OK)
		{
			delete[] tmp;
			return MAF_ERROR;
		}

		for (int i = 0; i < nCount; i++) {
			out_set.insert(tmp[i]);
		}

		delete[] tmp;
	}

	return MAF_OK;
}

//-------------------------------------------------------------------------
//template method to store the set (std::set) identified by its name.
//	returns MAF_OK or MAF_ERROR depending on, if it succeeds
template < typename T >
int medVMEMusculoSkeletalModel::InternalStore(mafStorageElement *node, const char* name, const std::set < T >& input_set)
{	
	int nCount = (int)input_set.size();

	mafString strNum = "Num_Of_";
	node->StoreInteger(strNum.Append(name).GetCStr(), nCount);

	if (nCount == 0)
		return MAF_OK;

	T* tmp = new T[nCount]; 
	int index = 0;
	for (std::set < T >::const_iterator it = input_set.begin(); it != input_set.end(); it++){
		tmp[index++] = (*it);
	}

	node->StoreVectorN(name, (int*)tmp, nCount);

	delete[] tmp;
	return MAF_OK;
}

#pragma region GUI and Events Handling
#include <wx/tooltip.h>
//-------------------------------------------------------------------------
mafGUI* medVMEMusculoSkeletalModel::CreateGui()
	//-------------------------------------------------------------------------
{  
	m_Gui = mafNode::CreateGui(); // Called to show info about vmes' type and name
	m_Gui->SetListener(this);

#pragma region wxFormBuilder Code
	wxStaticBoxSizer* sBox1 = new wxStaticBoxSizer( new wxStaticBox( m_Gui, wxID_ANY, wxT("Model Configuration") ), wxVERTICAL );

	wxString choices[] = {wxT("Fast skinning"), wxT("PK Method"), wxT("Mass-spring system method"), wxT("Interpolated PK Method")};
	wxRadioBox* radioDeformMethod = new wxRadioBox(m_Gui, ID_DEFORM_METHOD, wxT("Deformation method"),
		wxDefaultPosition, wxDefaultSize, 4, choices, 0, wxRA_SPECIFY_ROWS); 
	//	wxCheckBox* checkUsePKMethod = new wxCheckBox( m_Gui, ID_DEFORM_METHOD, wxT("Use PK Method"), wxDefaultPosition, wxDefaultSize, 0 );	
	radioDeformMethod->SetToolTip( wxT("Fast skinning - muscles are wrapped independently using fast skinning technique that "
		"does no guarantee volume preservation. All remaining configuration options are then ignored.\n"
		"PK method - muscles are wrapped using slower energy minimization technique that better preserves volume of muscles "
		"(loss below 0.1%) but it is sensitive to the quality of input muscle surface. If the muscle is corrupted, the result may "
		"be completely unrecognizable from what was expected.\nMSS - muscles are wrapped using a mass spring system technique that takes into account "
		"other muscles as well as bones during deformation.") );	
	sBox1->Add( radioDeformMethod, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* interpolationStepCountBox1 = new wxBoxSizer( wxHORIZONTAL );
	interpolationStepCountBox1->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Interpol. Steps:"), wxDefaultPosition, wxSize( 100,-1 ), wxALIGN_RIGHT ), 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	wxTextCtrl* interpolationStepCount = new wxTextCtrl( m_Gui, ID_INTERPOLATE_STEP_COUNT, wxEmptyString, wxDefaultPosition, wxSize( 30,-1 ), 0 );
	interpolationStepCount->SetToolTip( wxT("Specifies number of steps used for the interpolated deformation PK method, the high is the number, the more time is needed for the computation.") );
	interpolationStepCountBox1->Add( interpolationStepCount, 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	sBox1->Add( interpolationStepCountBox1, 0, wxALL|wxEXPAND, 5 );


	wxCheckBox* checkUseProgHull = new wxCheckBox( m_Gui, ID_PROGRESSIVE_HULL, wxT("Use Progressive Hulls"), wxDefaultPosition, wxDefaultSize, 0 );	
	checkUseProgHull->SetToolTip( wxT("When checked the energy minimization method computes progressive hull to represent a coarse mesh of muscle, "
		"which is much slower but may improve the stability of the method, otherwise, quadric clustering is used. "
		"N.B. This option is ignored unless Use PK Method is checked. ") );	
	sBox1->Add( checkUseProgHull, 0, wxALL|wxEXPAND, 5 );

	wxCheckBox* checkUseMultipleObjs = new wxCheckBox( m_Gui, ID_MULTIPLE_OBJECTS, wxT("Use Multiple Objects"), wxDefaultPosition, wxDefaultSize, 0 );	
	checkUseMultipleObjs->SetToolTip( wxT("When unchecked, muscles are wrapped independently, otherwise the penetration is prevented (much slower). "
		"N.B. This option is ignored unless Use PK Method is checked. ") );	
	sBox1->Add( checkUseMultipleObjs, 0, wxALL|wxEXPAND, 5 );	

	wxCheckBox* checkEnableFiltering = new wxCheckBox( m_Gui, ID_ENABLE_FILTERING, wxT("Enable filtering"), wxDefaultPosition, wxDefaultSize, 0 );	
	checkEnableFiltering->SetToolTip( wxT("When checked, the following items (if present in the model) are ignored "
		"and will not be considered in muscle wrapping process. N.B. This option is valid only, if Use Multiple Objects is checked.") );	
	sBox1->Add( checkEnableFiltering, 0, wxALL|wxEXPAND, 5 );

	wxCheckBox* checkUseFixedTimestep = new wxCheckBox( m_Gui, ID_USE_FIXED_TIMESTEP, wxT("Enable constant time step"), wxDefaultPosition, wxDefaultSize, 0 );	
	checkEnableFiltering->SetToolTip( wxT("When checked, a constant time step (can be selected below) is used instead of adaptive time step "
		"for the mass-spring method.") );	
	sBox1->Add( checkUseFixedTimestep, 0, wxALL|wxEXPAND, 5 );

	// parameters for the particle method
	wxBoxSizer* particleBox1 = new wxBoxSizer( wxHORIZONTAL );
	particleBox1->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Iteration Time Step:"), wxDefaultPosition, wxSize( 100,-1 ), wxALIGN_RIGHT ), 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	wxTextCtrl* particleIterStep = new wxTextCtrl( m_Gui, ID_PARTICLE_ITERATION_STEP, wxEmptyString, wxDefaultPosition, wxSize( 30,-1 ), 0 );
	//  particleIterStep->SetToolTip( wxT("Specifies the iteration number between the penetration checking between particles in different muscles. If the step is too large, the simulation can be faulty.") );
	particleIterStep->SetToolTip( wxT("Specifies the time step that is integrated during one iteration, that is in between penetration checks. If the step is too large, the simulation can be faulty.") );
	particleBox1->Add( particleIterStep, 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	sBox1->Add( particleBox1, 0, wxALL|wxEXPAND, 5 );

	/* wxBoxSizer* particleBox2 = new wxBoxSizer( wxHORIZONTAL );
	particleBox2->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Starting PeneNum:"), wxDefaultPosition, wxSize( 100,-1 ), wxALIGN_RIGHT ), 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	wxTextCtrl* particlePeneNum = new wxTextCtrl( m_Gui, ID_PARTICLE_PENETRAION_NUMBER, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	particlePeneNum->SetToolTip( wxT("Specifies the start iteration number of the penetration checking between muscles and bones. ") );
	particleBox2->Add( particlePeneNum, 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	sBox1->Add( particleBox2, 0, wxALL|wxEXPAND, 5 );*/

	wxBoxSizer* particleBox3 = new wxBoxSizer( wxHORIZONTAL );
	particleBox3->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Maximum IterNum:"), wxDefaultPosition, wxSize( 100,-1 ), wxALIGN_RIGHT ), 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	wxTextCtrl* particleIterNum = new wxTextCtrl( m_Gui, ID_PARTICLE_ITERATION_NUMBER, wxEmptyString, wxDefaultPosition, wxSize( 30,-1 ), 0 );
	particleIterNum->SetToolTip( wxT("Specifies the maximum number of the mass spring iterations. ") );
	particleBox3->Add( particleIterNum, 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	sBox1->Add( particleBox3, 0, wxALL|wxEXPAND, 5 );

	wxNotebook* notebook1 = new wxNotebook( m_Gui, wxID_ANY, wxDefaultPosition, wxDefaultSize,  wxNB_TOP | wxNB_FLAT );	
	notebook1->SetMinSize(wxSize(10, 250));
	sBox1->Add( notebook1, 1, wxEXPAND | wxALL, 5 );

#pragma region Exclude Bodes
	mafGUI* newpage = new mafGUI(this);  	

	m_listExludedBones = new wxListBox( newpage, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SORT|wxLB_EXTENDED ); 
	m_listExludedBones->SetToolTip( wxT("If filtering is enabled, these items will be excluded from the deformation process.") );
	newpage->Add( m_listExludedBones, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );		
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	wxButton* bttnAddRegionBones = new wxButton( newpage, ID_ADD_REGIONS_BONES, wxT("Add regions"), wxDefaultPosition, wxDefaultSize, 0 );
	bttnAddRegionBones->SetToolTip( wxT("Adds all items present in the selected regions.") );	
	bSizer2->Add( bttnAddRegionBones, 0, wxALL, 5 );

	wxButton* bttnAddBones = new wxButton( newpage, ID_ADD_ITEMS_BONES, wxT("Add bones"), wxDefaultPosition, wxDefaultSize, 0 );
	bttnAddBones->SetToolTip( wxT("Adds individual bones") );	
	bSizer2->Add( bttnAddBones, 0, wxALL, 5 );

	wxButton* bttnDeleteBones = new wxButton( newpage, ID_REMOVE_BONES, wxT("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	bttnDeleteBones->SetToolTip( wxT("Removes selected items from the list.") );	
	bSizer2->Add( bttnDeleteBones, 0, wxALL, 5 );
	newpage->Add( bSizer2, 0, wxEXPAND, 5 );

	newpage->Reparent(notebook1);
	notebook1->AddPage(newpage, wxT("Bones"));  
#pragma endregion Exclude Bodes

#pragma region Exclude Muscles
	newpage = new mafGUI(this);  	

	m_listExludedMuscles = new wxListBox( newpage, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SORT|wxLB_EXTENDED ); 
	m_listExludedMuscles->SetToolTip( wxT("If filtering is enabled, these items will be excluded from the deformation process.") );
	newpage->Add( m_listExludedMuscles, 1, wxALL|wxEXPAND, 5 );

	bSizer2 = new wxBoxSizer( wxHORIZONTAL );		
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	wxButton* bttnAddRegionMuscles = new wxButton( newpage, ID_ADD_REGIONS_MUSCLES, wxT("Add regions"), wxDefaultPosition, wxDefaultSize, 0 );
	bttnAddRegionMuscles->SetToolTip( wxT("Adds all items present in the selected regions.") );	
	bSizer2->Add( bttnAddRegionMuscles, 0, wxALL, 5 );

	wxButton* bttnAddMuscles = new wxButton( newpage, ID_ADD_ITEMS_MUSCLES, wxT("Add muscles"), wxDefaultPosition, wxDefaultSize, 0 );
	bttnAddMuscles->SetToolTip( wxT("Adds individual muscles") );	
	bSizer2->Add( bttnAddMuscles, 0, wxALL, 5 );

	wxButton* bttnDeleteMuscles = new wxButton( newpage, ID_REMOVE_MUSCLES, wxT("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	bttnDeleteMuscles->SetToolTip( wxT("Removes selected items from the list.") );	
	bSizer2->Add( bttnDeleteMuscles, 0, wxALL, 5 );
	newpage->Add( bSizer2, 0, wxEXPAND, 5 );	

	newpage->Reparent(notebook1);
	notebook1->AddPage(newpage, wxT("Muscles"));  
#pragma endregion Exclude Muscles

	wxButton* bttnCommit = new wxButton( m_Gui, ID_COMMIT_CHANGES, wxT("Commit"), wxDefaultPosition, wxDefaultSize, 0 );
	bttnCommit->SetToolTip( wxT("Commits the changes that has been done.") );

	sBox1->Add( bttnCommit, 0, wxALL, 5 );

#pragma endregion wxFormBuilder Code

	//set check boxes
	m_DeformationMethodGui = m_DeformationMethod;
	m_UseMultipleObjectsGui = m_UseMultipleObjects;
	m_UseProgressiveHullsGui = m_UseProgressiveHulls;
	m_UseFixedTimestepGui = m_UseFixedTimestep;
	m_EnableFilteringGui = m_EnableFiltering;
	m_IterationStepGui = m_IterationStep;
	m_IterationNumGui = m_IterationNum;

	//populate lists
	PopulateExcludeLists();

	//validators
	radioDeformMethod->SetValidator(mafGUIValidator(this, ID_DEFORM_METHOD, radioDeformMethod, &m_DeformationMethodGui));
	interpolationStepCount->SetValidator(mafGUIValidator(this, ID_INTERPOLATE_STEP_COUNT, interpolationStepCount, &m_InterpolateStepCount, 1, 100));
	checkUseProgHull->SetValidator(mafGUIValidator(this, ID_PROGRESSIVE_HULL, checkUseProgHull, &m_UseProgressiveHullsGui));
	checkUseMultipleObjs->SetValidator(mafGUIValidator(this, ID_MULTIPLE_OBJECTS, checkUseMultipleObjs, &m_UseMultipleObjectsGui));
	checkEnableFiltering->SetValidator(mafGUIValidator(this, ID_ENABLE_FILTERING, checkEnableFiltering, &m_EnableFilteringGui));
	particleIterStep->SetValidator(mafGUIValidator(this, ID_PARTICLE_ITERATION_STEP, particleIterStep, &m_IterationStepGui, 0, 100));
	checkUseFixedTimestep->SetValidator(mafGUIValidator(this, ID_USE_FIXED_TIMESTEP, checkUseFixedTimestep, &m_UseFixedTimestepGui));
	particleIterNum->SetValidator(mafGUIValidator(this, ID_PARTICLE_ITERATION_NUMBER, particleIterNum, &m_IterationNumGui, 0, 10000));
	bttnAddBones->SetValidator(mafGUIValidator(this, ID_ADD_ITEMS_BONES, bttnAddBones));
	bttnAddMuscles->SetValidator(mafGUIValidator(this, ID_ADD_ITEMS_MUSCLES, bttnAddMuscles));
	bttnAddRegionBones->SetValidator(mafGUIValidator(this, ID_ADD_REGIONS_BONES, bttnAddRegionBones));
	bttnAddRegionMuscles->SetValidator(mafGUIValidator(this, ID_ADD_REGIONS_MUSCLES, bttnAddRegionMuscles));
	bttnDeleteBones->SetValidator(mafGUIValidator(this, ID_REMOVE_BONES, bttnDeleteBones));
	bttnDeleteMuscles->SetValidator(mafGUIValidator(this, ID_REMOVE_MUSCLES, bttnDeleteMuscles));
	bttnCommit->SetValidator(mafGUIValidator(this, ID_COMMIT_CHANGES, bttnCommit));

	//make sure our tooltips are not trimmed when showed
	checkEnableFiltering->GetToolTip()->GetWindow()->SetMaxSize(wxSize(1600, 75));

	m_Gui->Add(sBox1);
	m_Gui->FitGui();
	m_Gui->Layout();

	return m_Gui;
}

//-------------------------------------------------------------------------
char** medVMEMusculoSkeletalModel::GetIcon() 
	//-------------------------------------------------------------------------
{
#include "medVMEMusculoSkeletalModel.xpm"
	return medVMEMusculoSkeletalModel_xpm;
}

//-------------------------------------------------------------------------
//Process events coming from other objects 
/*virtual*/ void medVMEMusculoSkeletalModel::OnEvent(mafEventBase *maf_event)
{	
	// events to be sent up or down in the tree are simply forwarded
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{    
		bool bHandled = true;

		int nId = e->GetId();
		switch (nId)
		{
			//input mode has changed from simple to advanced or vice versa
		case ID_ENABLE_FILTERING:
		case ID_MULTIPLE_OBJECTS:
		case ID_DEFORM_METHOD:
		case ID_PROGRESSIVE_HULL:
		case ID_PARTICLE_ITERATION_STEP:
		case ID_PARTICLE_ITERATION_NUMBER:
			DisplayWarning();	//just display warning
			break;

		case ID_ADD_REGIONS_BONES:
			OnAddRegions(m_listExludedBones);
			DisplayWarning();
			break;

		case ID_ADD_REGIONS_MUSCLES:
			OnAddRegions(m_listExludedMuscles);
			DisplayWarning();
			break;

		case ID_ADD_ITEMS_BONES:
			OnAddBones(m_listExludedBones);
			DisplayWarning();
			break;

		case ID_ADD_ITEMS_MUSCLES:
			OnAddMuscles(m_listExludedMuscles);
			DisplayWarning();
			break;

		case ID_REMOVE_BONES:
			OnRemoveItems(m_listExludedBones);
			DisplayWarning();
			break;

		case ID_REMOVE_MUSCLES:
			OnRemoveItems(m_listExludedMuscles);
			DisplayWarning();
			break;

		case ID_COMMIT_CHANGES:
			OnCommitChanges();
			break;

		case ID_ADDGUI_OK:       
			m_AddDialog->EndModal(wxID_OK);      
			break;

		case ID_ADDGUI_CANCEL:
			m_AddDialog->EndModal(wxID_CANCEL);
			break;

		default:
			bHandled = false;
		}

		if (bHandled)
			return;
	}

	switch (maf_event->GetId())
	{
	case NODE_ATTACHED_TO_TREE:
	case NODE_DETACHED_FROM_TREE:
		m_MSMGraphValid = false;
		break;
	}

	Superclass::OnEvent(maf_event);	
}

//-------------------------------------------------------------------------
//populates the exclude list with regions or items (determined by isregion) 
template < typename T, class TC >
void medVMEMusculoSkeletalModel::PopulateList(wxListBox* list, 
	const std::set < T >& input_set, bool isregion)
{
	for (std::set < T >::const_iterator it = input_set.begin(); it != input_set.end(); it++)
	{
		wxString name = TC::GetName(*it);
		if (isregion)
			name.Append("\\*");

		list->Append(name, (void*)(*it));
	}
}

//-------------------------------------------------------------------------
//retrieves regions or items (determined by isregion)  from the given list
template < typename T, class TC >
void medVMEMusculoSkeletalModel::RetrieveList(const wxListBox* list, 
	std::set < T >& out_set, bool isregion)
{
	out_set.clear();

	int nCount = list->GetCount();
	for (int  i = 0; i < nCount; i++)
	{
		//if (ends with '*' and isregion == true) or (not ends with '*' and isregion == false)
		if (list->GetString(i).Last('*') >= 0 == isregion)
		{
			T tmp = (T)((int)list->GetClientData(i));
			out_set.insert(tmp);
		}
	}
}

//-------------------------------------------------------------------------
//Populates both exclude lists with the data
/*virtual*/ void medVMEMusculoSkeletalModel::PopulateExcludeLists()
{
	_VERIFY_RET(m_Gui != NULL);

	m_listExludedBones->Clear();	
	PopulateList< medRegionsType, medMSMGraph::RegionEnum >(m_listExludedBones, m_Regions_Bones, true);
	PopulateList< medBonesType, medMSMGraph::BoneEnum >(m_listExludedBones, m_Items_Bones, false);

	m_listExludedMuscles->Clear();
	PopulateList< medRegionsType, medMSMGraph::RegionEnum >(m_listExludedMuscles, m_Regions_Muscles, true);
	PopulateList< medMusclesType, medMSMGraph::MuscleEnum >(m_listExludedMuscles, m_Items_Muscles, false);
}

//-------------------------------------------------------------------------
// Gets the data from both exclude lists
/*virtual*/ void medVMEMusculoSkeletalModel::RetrieveExcludeLists()
{
	_VERIFY_RET(m_Gui != NULL);

	RetrieveList< medRegionsType, medMSMGraph::RegionEnum >(m_listExludedBones, m_Regions_Bones, true);
	RetrieveList< medBonesType, medMSMGraph::BoneEnum >(m_listExludedBones, m_Items_Bones, false);

	RetrieveList< medRegionsType, medMSMGraph::RegionEnum >(m_listExludedMuscles, m_Regions_Muscles, true);
	RetrieveList< medMusclesType, medMSMGraph::MuscleEnum >(m_listExludedMuscles, m_Items_Muscles, false);
}


//-------------------------------------------------------------------------
//Handles the commit changes event 
//It stores changes from lists into data structures
/*virtual*/ void medVMEMusculoSkeletalModel::OnCommitChanges()
{
	m_DeformationMethod = m_DeformationMethodGui;
	m_UseMultipleObjects = m_UseMultipleObjectsGui;
	m_UseProgressiveHulls = m_UseProgressiveHullsGui;
	m_EnableFiltering = m_EnableFilteringGui;
	m_IterationStep = m_IterationStepGui;
	m_IterationNum = m_IterationNumGui;
	m_UseFixedTimestep = m_UseFixedTimestepGui;
	RetrieveExcludeLists();

	m_bWarningDisplayed = false;

	//we need to refresh everything
	m_ConfirmTimestamp++;
	this->Modified();

	medVMEMusculoSkeletalModel::mafVMENodeList list;
	this->GetMuscleWrappers(list);
	for (medVMEMusculoSkeletalModel::mafVMENodeList::iterator it = list.begin(); 
		it != list.end(); it++) {
			(*it)->Modified();	//something has changed => we will need to rerun the method		
	}

	//make sure that everything is updated
	mafEvent e(this, CAMERA_UPDATE);
	this->ForwardUpEvent(&e);
}

//-------------------------------------------------------------------------
//Displays a warning that any change must be confirmed (committed)
/*virtual*/ void medVMEMusculoSkeletalModel::DisplayWarning(bool bOnlyIfNotDisplayedPreviously)
{
	if (!bOnlyIfNotDisplayedPreviously || !m_bWarningDisplayed)
	{
		wxMessageBox(_("BEWARE: changes are neither applied nor stored unless you press 'Commit changes' button."),
			_("Warning"), wxOK | wxICON_WARNING);
		m_bWarningDisplayed = true;
	}
}

//-------------------------------------------------------------------------
//Creates m_AddDialogdialog to be used for adding items
/*virtual*/	void medVMEMusculoSkeletalModel::CreateAddDialog()
{
	m_AddDialog = new mafGUIDialog(_("Select items to be excluded"), mafCLOSEWINDOW | mafRESIZABLE);  

	wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );	
	m_listAddDialog = new wxListBox(m_AddDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SORT|wxLB_EXTENDED ); 
	bSizer1->Add( m_listAddDialog, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer6 = new wxBoxSizer( wxHORIZONTAL );		
	bSizer6->Add( 0, 0, 1, wxEXPAND, 5 );
	wxButton* bttnOK = new wxButton( m_AddDialog, ID_ADDGUI_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( bttnOK, 0, wxALL|wxALIGN_BOTTOM, 5 );	
	wxButton* bttnCancel = new wxButton( m_AddDialog, ID_ADDGUI_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( bttnCancel, 0, wxALL|wxALIGN_BOTTOM, 5 );

	bSizer1->Add( bSizer6, 1, wxEXPAND, 1 );
	m_AddDialog->Add(bSizer1, 1, wxEXPAND);

	bttnOK->SetValidator(mafGUIValidator(this, ID_ADDGUI_OK, bttnOK));
	bttnCancel->SetValidator(mafGUIValidator(this, ID_ADDGUI_CANCEL, bttnCancel));

	m_AddDialog->SetMinSize(wxSize(460, 225));
}

//-------------------------------------------------------------------------
//Destroys m_AddDialog dialog 
/*virtual*/ void medVMEMusculoSkeletalModel::DeleteAddDialog()
{
	cppDEL(m_AddDialog);
	m_listAddDialog = NULL;
}

//-------------------------------------------------------------------------
//Handles the event of adding regions into the given list
/*virtual*/ void medVMEMusculoSkeletalModel::OnAddRegions(wxListBox* list)
{
	CreateAddDialog();

	//get current items from the list
	medRegionsSet tmpSet;
	RetrieveList< medRegionsType, medMSMGraph::RegionEnum >(list, tmpSet, true);

	PopulateListAddDialog< medRegionsType, medMSMGraph::RegionEnum >(tmpSet);
	if (wxID_OK == m_AddDialog->ShowModal())
	{
		RetrieveListAddDialog< medRegionsType, medMSMGraph::RegionEnum >(tmpSet);
		PopulateList< medRegionsType, medMSMGraph::RegionEnum >(list, tmpSet, true);
	}

	DeleteAddDialog();
}

//-------------------------------------------------------------------------
//Handles the event of adding bones into the given list
/*virtual*/ void medVMEMusculoSkeletalModel::OnAddBones(wxListBox* list)
{
	CreateAddDialog();

	//get current items from the list
	medBonesSet tmpSet;
	RetrieveList< medBonesType, medMSMGraph::BoneEnum >(list, tmpSet, false);

	PopulateListAddDialog< medBonesType, medMSMGraph::BoneEnum >(tmpSet);
	if (wxID_OK == m_AddDialog->ShowModal())
	{
		RetrieveListAddDialog< medBonesType, medMSMGraph::BoneEnum >(tmpSet);
		PopulateList< medBonesType, medMSMGraph::BoneEnum >(list, tmpSet, false);
	}

	DeleteAddDialog();
}

//-------------------------------------------------------------------------
//Handles the event of adding muscles into the given list
/*virtual*/ void medVMEMusculoSkeletalModel::OnAddMuscles(wxListBox* list)
{
	CreateAddDialog();

	//get current items from the list
	medMusclesSet tmpSet;
	RetrieveList< medMusclesType, medMSMGraph::MuscleEnum >(list, tmpSet, false);

	PopulateListAddDialog< medMusclesType, medMSMGraph::MuscleEnum >(tmpSet);
	if (wxID_OK == m_AddDialog->ShowModal())
	{
		RetrieveListAddDialog< medMusclesType, medMSMGraph::MuscleEnum >(tmpSet);
		PopulateList< medMusclesType, medMSMGraph::MuscleEnum >(list, tmpSet, false);
	}

	DeleteAddDialog();
}

//-------------------------------------------------------------------------
//Handles the event of removing selected items from the given list
/*virtual*/ void medVMEMusculoSkeletalModel::OnRemoveItems(wxListBox* list)
{
	int nCount = list->GetCount();
	for (int i = 0; i < nCount; )
	{
		if (!list->IsSelected(i))
			i++;
		else
		{
			list->Delete(i);
			nCount--;
		}		
	}	
}

//-------------------------------------------------------------------------
//populates m_listAddDialog list with all items in TConvert and checks those from input_items_set 
//TConvert is the class used to convert enums to strings
template < typename T, class TConvert >
void medVMEMusculoSkeletalModel::PopulateListAddDialog(const std::set < T >& input_set)
{
	for (TConvert en = (T)(TConvert::Null + 1); en < TConvert::Invalid; en = (T)((int)en + 1))
	{		
		if (input_set.find((T)en) == input_set.end()){
			m_listAddDialog->Append(en.GetName(), (void*)((int)en));
		}
	}
}
//-------------------------------------------------------------------------
//retrieves checked items from m_listAddDialog list
//TConvert is the class used to convert enums to strings
template < typename T, class TConvert >
void medVMEMusculoSkeletalModel::RetrieveListAddDialog(std::set < T >& out_set)
{
	out_set.clear();

	wxArrayInt selIds;
	m_listAddDialog->GetSelections(selIds);
	int nCount =  (int)selIds.size();

	for (int i = 0; i < nCount; i++){
		out_set.insert((T)((int)m_listAddDialog->GetClientData(selIds[i])));
	}
}
#pragma endregion