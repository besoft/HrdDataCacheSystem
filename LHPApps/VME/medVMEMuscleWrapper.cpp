/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medVMEMuscleWrapper.cpp,v $
Language:  C++
Date:      $Date: 2012-04-30 14:52:43 $
Version:   $Revision: 1.1.1.1.2.22 $
Authors:   Josef Kohout
==========================================================================
Copyright (c) 2008-2012
University of Bedforshire, University of West Bohemia
=========================================================================
History: 

1.2.2012 - the implementation was split into several functional modules, each in one file
with the prefix medVMEMuscleWrapper_ + m_InputMode has been removed
*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "medVMEMuscleWrapper.h"
#include "mafVMEOutputSurface.h"
#include "mafDataPipeCustom.h"
#include "mafStorageElement.h"
#include "mmaMaterial.h"
#include "mafTransform.h"

#include "vtkPolyData.h"
#include "vtkTubeFilter.h"

#include <assert.h>
#include "mafDbg.h"
#include "mafMemDbg.h"
#include "DeformationCache.h"	//PK method deformation cache



//-------------------------------------------------------------------------
mafCxxTypeMacro(medVMEMuscleWrapper)
	//-------------------------------------------------------------------------


	const /*static*/ char* medVMEMuscleWrapper::MUSCLEWRAPPER_LINK_NAMES[] = {
		"MuscleVME_RP", "WrapperVME_RP_", "WrapperVME_CP_", 
		"RefSysVME_RP_", "RefSysVME_CP_",
		"OAreaVME", "IAreaVME", 
};

#pragma warning(once: 4995)

#define DEFAULT_OUTPUT_MODE   0   //1 = generates fibers, 0 - deformed surface
#define DEFAULT_USEREFSYSVAL  1
#define DEFAULT_FASTCHECKS		1
#define DEFAULT_DECOMPOSITION_METHOD	0
#define DEFAULT_FIBERS_TYPE   FT_PENNATE
#define DEFAULT_FIBERS_NUM    64
#define DEFAULT_FIBERS_RES    40
#define DEFAULT_FIBERS_SMOOTH	1
#define DEFAULT_FIBERS_SMOOTHSTEPS	5
#define DEFAULT_FIBERS_SMOOTHWEIGHT	4
#define DEFAULT_FIBERS_THICKNESS 1.25
#define DEFAULT_FIBERS_UNIFORM_SAMPLING 1
#define DEFAULT_PARTICLE_CONTAINT 3.0
#define DEFAULT_PARTICLE_BONEDISTANCE 0
#define DEFAULT_PARTICLE_ITERATIONSTEP 0.4
#define DEFAULT_PARTICLE_PENEITERNUM  0
#define DEFAULT_PARTICLE_ITERATIONNUM 70
#define DEFAULT_SPRING_LAYOUT_TYPE   2
#define DEFAULT_CLOSEST_PARTICLE_NUM 15

//-------------------------------------------------------------------------
medVMEMuscleWrapper::medVMEMuscleWrapper()
	//-------------------------------------------------------------------------
{  
	m_PolyData = vtkPolyData::New();
	m_pTransformedMuscle = vtkPolyData::New();
	m_pDeformedMuscle = vtkPolyData::New();
	m_pDecomposedMuscle = vtkPolyData::New();
	m_pTubeFilter = NULL;
	m_pParticleSystem = NULL;

	m_MuscleVme = NULL;
	m_nMuscleChecksum = 0;
	for (int i = 0; i < 2; i++){
		m_RefSysVme[i] = m_WrappersVme[i] = m_OIVME[i] = NULL;    
	}

	m_VisMode = DEFAULT_OUTPUT_MODE;
	m_UseRefSys = DEFAULT_USEREFSYSVAL;
	m_UseFastChecks = DEFAULT_FASTCHECKS;
	m_DecompositionMethod = DEFAULT_DECOMPOSITION_METHOD;
	m_FbResolution = DEFAULT_FIBERS_RES;
	m_FbNumFib = DEFAULT_FIBERS_NUM;
	m_FbTemplate = DEFAULT_FIBERS_TYPE; //FT_PENNATE
	m_FbThickness = DEFAULT_FIBERS_THICKNESS;
	m_FbSmooth = DEFAULT_FIBERS_SMOOTH;
	m_FbSmoothSteps = DEFAULT_FIBERS_SMOOTHSTEPS;
	m_FbSmoothWeight = DEFAULT_FIBERS_SMOOTHWEIGHT;
	m_FbUniformSampling = DEFAULT_FIBERS_UNIFORM_SAMPLING;
	m_PConstraintThreshold = DEFAULT_PARTICLE_CONTAINT;
	m_PBoneDistance = DEFAULT_PARTICLE_BONEDISTANCE;  
	m_PSpringLayoutType = DEFAULT_SPRING_LAYOUT_TYPE;
	m_PClosestParticleNum = DEFAULT_CLOSEST_PARTICLE_NUM;

#ifdef _DEBUG_VIS_
	m_DefDebugShow = 0;
	m_DefDebugShowSteps = 0;
	m_FbDebugShowTemplate = 0;
	m_FbDebugShowFitting = 0;
	m_FbDebugShowFittingRes = 0;
	m_FbDebugShowSlicing = 0;
	m_FbDebugShowSlicingRes = 0;
	m_FbDebugFiberID = 1;
	m_FbDebugShowConstraints = 0;	
	m_FbDebugShowOIConstruction = 0;
#ifdef ADV_KUKACKA
	m_FbDebugShowHarmFunc = 0;
#endif
#endif
	m_LastUpdateTimeStamp= -1.0;
	m_LastMMConfirmTimeStamp = -1;

	m_bLinksRestored = false;  

	m_pVMEMusculoSkeletalModel = NULL;
	m_MasterVMEMuscleWrapper = NULL;

	m_DeformationMethod = 0;
	m_UseProgressiveHulls = m_UsePenetrationPrevention = m_UseFixedTimestep = false;
	m_IterationStep = DEFAULT_PARTICLE_ITERATIONSTEP;
	m_IterationNum = DEFAULT_PARTICLE_ITERATIONNUM;
	m_Modified = NOTHING_MODIFIED;	

	mafNEW(m_Transform);
	mafVMEOutputSurface *output = mafVMEOutputSurface::New(); // an output with no data  
	output->SetTransform(m_Transform); // force my transform in the output
	SetOutput(output);  
	m_nOutputSpaceChecksum = 0;

	DependsOnLinkedNodeOn();

	// attach a data pipe which creates a bridge between VTK and MAF
	mafDataPipeCustom *dpipe = mafDataPipeCustom::New();
	dpipe->SetDependOnAbsPose(true);
	SetDataPipe(dpipe);
	dpipe->SetInput(m_PolyData); 
}
//-------------------------------------------------------------------------
medVMEMuscleWrapper::~medVMEMuscleWrapper()
	//-------------------------------------------------------------------------
{  
	DeleteAllWrappers();  

	vtkDEL(m_pTubeFilter);
	vtkDEL(m_pTransformedMuscle);
	vtkDEL(m_pDecomposedMuscle);
	vtkDEL(m_pDeformedMuscle);
	vtkDEL(m_PolyData);
	mafDEL(m_Transform);  
	SetOutput(NULL);  

	delete m_pParticleSystem;
	m_pParticleSystem = NULL;

	//BES: 3.5.2012 - dispose PK method deformation cache to avoid memory leaks
	DeformationCache::GetInstance()->Dispose();
}

//-------------------------------------------------------------------------
int medVMEMuscleWrapper::DeepCopy(mafNode *a)
	//-------------------------------------------------------------------------
{ 
	if (Superclass::DeepCopy(a)==MAF_OK)
	{
		medVMEMuscleWrapper *wrapper = medVMEMuscleWrapper::SafeDownCast(a);
		m_Transform->SetMatrix(wrapper->m_Transform->GetMatrix());

		for (int i = 0; i < 2; i++){    
			m_OIVMEName[i] = wrapper->m_OIVMEName[i];
			m_RefSysVmeName[i] = wrapper->m_RefSysVmeName[i];
		}

		m_VisMode = wrapper->m_VisMode;
		m_UseRefSys = wrapper->m_UseRefSys;
		m_UseFastChecks = wrapper->m_UseFastChecks;
		m_DecompositionMethod = wrapper->m_DecompositionMethod;
		m_FbTemplate = wrapper->m_FbTemplate;
		m_FbNumFib = wrapper->m_FbNumFib;
		m_FbResolution = wrapper->m_FbResolution;
		m_FbSmooth = wrapper->m_FbSmooth;
		m_FbSmoothSteps = wrapper->m_FbSmoothSteps;
		m_FbSmoothWeight = wrapper->m_FbSmoothWeight;
		m_FbUniformSampling = wrapper->m_FbUniformSampling;
		m_PConstraintThreshold = wrapper->m_PConstraintThreshold;
		m_PBoneDistance = wrapper->m_PBoneDistance;
		m_PSpringLayoutType = wrapper->m_PSpringLayoutType;
		m_PClosestParticleNum = wrapper->m_PClosestParticleNum;

#ifdef _DEBUG_VIS_
		m_DefDebugShow = wrapper->m_DefDebugShow;
		m_DefDebugShowSteps = wrapper->m_DefDebugShowSteps;
		m_FbDebugShowTemplate = wrapper->m_FbDebugShowTemplate;
		m_FbDebugShowFitting = wrapper->m_FbDebugShowFitting;
		m_FbDebugShowFittingRes = wrapper->m_FbDebugShowFittingRes;
		m_FbDebugShowSlicing = wrapper->m_FbDebugShowSlicing;
		m_FbDebugShowSlicingRes = wrapper->m_FbDebugShowSlicingRes;	
		m_FbDebugFiberID = wrapper->m_FbDebugFiberID;
		m_FbDebugShowConstraints = wrapper->m_FbDebugShowConstraints;
#endif

		DeleteAllWrappers();				
		m_Wrappers.resize(wrapper->m_Wrappers.size());		//reserve links

		m_Modified = EVERYTHING_MODIFIED;

		//DeepCopy copied links => restore internal data
		RestoreMeterLinks();

		//BES: 12.1.2009 - DataPipe has NULL input now (although it was set in ctor)
		//=> we need to reassign input for the data pipe
		mafDataPipeCustom *dpipe = mafDataPipeCustom::SafeDownCast(GetDataPipe());
		if (dpipe != NULL){
			dpipe->SetInput(m_PolyData);
		}    

		return MAF_OK;
	}  
	return MAF_ERROR;
}
//-------------------------------------------------------------------------
bool medVMEMuscleWrapper::Equals(mafVME *vme)
	//-------------------------------------------------------------------------
{
	if (!Superclass::Equals(vme))   //checks also Links
		return false;

	medVMEMuscleWrapper *wrapper = medVMEMuscleWrapper::SafeDownCast(vme);
	if (wrapper == NULL ||
		m_MuscleVme != wrapper->m_MuscleVme  ||  
		m_OIVME[0] != wrapper->m_OIVME[0] ||
		m_OIVME[1] != wrapper->m_OIVME[1] ||
		m_UseRefSys != wrapper->m_UseRefSys ||
		m_UseFastChecks != wrapper->m_UseFastChecks ||
		m_VisMode != wrapper->m_VisMode ||
		m_DecompositionMethod != wrapper->m_DecompositionMethod ||
		m_FbTemplate != wrapper->m_FbTemplate ||
		m_FbNumFib != wrapper->m_FbNumFib ||
		m_FbResolution != wrapper->m_FbResolution ||
		m_FbSmooth != wrapper->m_FbSmooth ||
		m_FbSmoothSteps != wrapper->m_FbSmoothSteps ||
		m_FbSmoothWeight != wrapper->m_FbSmoothWeight ||
		m_FbUniformSampling != wrapper->m_FbUniformSampling ||
		m_PConstraintThreshold != wrapper->m_PConstraintThreshold ||
		m_PBoneDistance != wrapper->m_PBoneDistance ||
		m_PSpringLayoutType != wrapper->m_PSpringLayoutType ||
		m_PClosestParticleNum != wrapper->m_PClosestParticleNum ||

		!(m_Transform->GetMatrix() == wrapper->m_Transform->GetMatrix())
		)
		return false;

	int nWrappers = (int)m_Wrappers.size();
	if (nWrappers != (int)wrapper->m_Wrappers.size())
		return false;

	for (int j = 0; j < nWrappers; j++)
	{
		const CWrapperItem* pItem = m_Wrappers[j];
		const CWrapperItem* pSrcItem = wrapper->m_Wrappers[j];
		for (int i = 0; i < 2; i++)
		{
			if (pSrcItem->pVmeRP_CP[i] != pItem->pVmeRP_CP[i])
				return false;

			if (pSrcItem->pVmeRefSys_RP_CP[i] != pItem->pVmeRefSys_RP_CP[i])
				return false;
		}    
	}

	return true;
}

//-------------------------------------------------------------------------
void medVMEMuscleWrapper::SetMatrix(const mafMatrix &mat)
	//-------------------------------------------------------------------------
{    
	m_Transform->SetMatrix(mat);
	m_Modified |= OUTPUT_SPACE_MODIFIED;
	Modified();
}

//-------------------------------------------------------------------------
int medVMEMuscleWrapper::InternalInitialize()
	//-------------------------------------------------------------------------
{
	if (Superclass::InternalInitialize()==MAF_OK)
	{
		// force material allocation
		GetMaterial();
		return MAF_OK;
	}

	return MAF_ERROR;
}

//-------------------------------------------------------------------------
mmaMaterial *medVMEMuscleWrapper::GetMaterial()
	//-------------------------------------------------------------------------
{
	mmaMaterial *material = (mmaMaterial *)GetAttribute("MaterialAttributes");
	if (material == NULL)
	{
		material = mmaMaterial::New();
		SetAttribute("MaterialAttributes", material);
	}
	return material;
}

//-------------------------------------------------------------------------
bool medVMEMuscleWrapper::IsAnimated()
	//-------------------------------------------------------------------------
{
	return false;
}
//-------------------------------------------------------------------------
void medVMEMuscleWrapper::GetLocalTimeStamps(std::vector<mafTimeStamp> &kframes)
	//-------------------------------------------------------------------------
{
	kframes.clear(); // no timestamps
}

//------------------------------------------------------------------------
//Set a new link for the given vme. 
//The link have name with prefix from the link table at index nLinkNameId (see LNK_ enums)
//and suffix nPosId. If nPosId is -1, no suffix is specified
void medVMEMuscleWrapper::StoreMeterLink(mafVME* vme, int nLinkNameId, int nPosId)
	//------------------------------------------------------------------------
{
	if (vme != NULL)
	{
		mafString szName;
		if (nPosId >= 0)
			szName = wxString::Format(wxT("%s%d"), MUSCLEWRAPPER_LINK_NAMES[nLinkNameId], nPosId);
		else
			szName = MUSCLEWRAPPER_LINK_NAMES[nLinkNameId];

		SetLink(szName, vme);
	}
}

//------------------------------------------------------------------------
//Stores all meter links into Links list.
//N.B. This method is supposed to be call from InternalStore
//------------------------------------------------------------------------
void medVMEMuscleWrapper::StoreMeterLinks()
{
	if (!m_bLinksRestored)
		return; //no link to be stored

	//we need to remove all existing links first
	mafLinksMap* pLinks = GetLinks(); 
	bool bNeedRestart;   

	do
	{
		bNeedRestart = false;
		for (mafLinksMap::iterator i = pLinks->begin(); i != pLinks->end(); i++)
		{
			if (
				i->first.Equals(MUSCLEWRAPPER_LINK_NAMES[LNK_RESTPOSE_MUSCLE]) ||
				i->first.Equals(MUSCLEWRAPPER_LINK_NAMES[LNK_FIBERS_ORIGIN]) ||
				i->first.Equals(MUSCLEWRAPPER_LINK_NAMES[LNK_FIBERS_INSERTION]) ||        
				i->first.StartsWith(MUSCLEWRAPPER_LINK_NAMES[LNK_RESTPOSE_WRAPPERx]) ||
				i->first.StartsWith(MUSCLEWRAPPER_LINK_NAMES[LNK_DYNPOSE_WRAPPERx]) ||
				i->first.StartsWith(MUSCLEWRAPPER_LINK_NAMES[LNK_RESTPOSE_REFSYSx]) ||
				i->first.StartsWith(MUSCLEWRAPPER_LINK_NAMES[LNK_DYNPOSE_REFSYSx])
				)
			{
				if (i->second.m_NodeId != -1)
					RemoveLink(i->first);	//use a standard MAF stuff
				else
					pLinks->erase(i);			//MAF crashes, if the link does not exist
				bNeedRestart = true;
				break;
			}
		}
	}while (bNeedRestart);

	//create links, so they can be stored
	StoreMeterLink(m_MuscleVme, LNK_RESTPOSE_MUSCLE);
	StoreMeterLink(m_OIVME[0], LNK_FIBERS_ORIGIN);
	StoreMeterLink(m_OIVME[1], LNK_FIBERS_INSERTION);  

	int nId = 0;
	for (CWrapperItemCollectionIterator pItem = m_Wrappers.begin(); 
		pItem != m_Wrappers.end(); pItem++)
	{
		StoreMeterLink((*pItem)->pVmeRP_CP[0], LNK_RESTPOSE_WRAPPERx, nId);
		StoreMeterLink((*pItem)->pVmeRP_CP[1], LNK_DYNPOSE_WRAPPERx, nId);  
		StoreMeterLink((*pItem)->pVmeRefSys_RP_CP[0], LNK_RESTPOSE_REFSYSx, nId);
		StoreMeterLink((*pItem)->pVmeRefSys_RP_CP[1], LNK_DYNPOSE_REFSYSx, nId);
		nId++;
	}
}

//------------------------------------------------------------------------
//Restore vme with the specified link. 
//Returns NULL, if there is no VME. IMPORTANT: removes vme from the Links list
mafVME* medVMEMuscleWrapper::RestoreMeterLink(int nLinkNameId, int nPosId)
	//------------------------------------------------------------------------
{
	mafString szName;
	if (nPosId >= 0)
		szName = wxString::Format(wxT("%s%d"), MUSCLEWRAPPER_LINK_NAMES[nLinkNameId], nPosId);
	else
		szName = MUSCLEWRAPPER_LINK_NAMES[nLinkNameId];

	return mafVME::SafeDownCast(GetLink(szName));  
}

//------------------------------------------------------------------------
//Restore all meter links from Links list. 
//N.B. this method is supposed to be called only after all VMEs were restored,
//i.e, it cannot be called from InternalRestore
void medVMEMuscleWrapper::RestoreMeterLinks()
	//------------------------------------------------------------------------
{
	//restore links (and remove them as well)  
	m_MuscleVme = RestoreMeterLink(LNK_RESTPOSE_MUSCLE);
	m_OIVME[0] = RestoreMeterLink(LNK_FIBERS_ORIGIN);
	m_OIVME[1] = RestoreMeterLink(LNK_FIBERS_INSERTION);

	int nId = 0;
	for (CWrapperItemCollectionIterator pItem = m_Wrappers.begin(); 
		pItem != m_Wrappers.end(); pItem++, nId++)
	{ 
		if (*pItem == NULL)
			(*pItem) = new CWrapperItem();
		(*pItem)->Modified = CWrapperItem::EVERYTHING_MODIFIED;

		(*pItem)->pVmeRP_CP[0] = RestoreMeterLink(LNK_RESTPOSE_WRAPPERx, nId);
		(*pItem)->pVmeRP_CP[1] = RestoreMeterLink(LNK_DYNPOSE_WRAPPERx, nId);      
		(*pItem)->pVmeRefSys_RP_CP[0] = RestoreMeterLink(LNK_RESTPOSE_REFSYSx, nId);
		(*pItem)->pVmeRefSys_RP_CP[1] = RestoreMeterLink(LNK_DYNPOSE_REFSYSx, nId);
	} //end for	
}

//------------------------------------------------------------------------
/*virtual*/ int medVMEMuscleWrapper::InternalStore(mafStorageElement *parent)
	//------------------------------------------------------------------------
{
	//store Links, so they can be saved by base clase  
	StoreMeterLinks();

	if (Superclass::InternalStore(parent) == MAF_OK)  //stores material + links to muscle and OI areas VMEs
	{
		parent->StoreInteger("Wrappers_Num", (int)m_Wrappers.size());
		parent->StoreInteger("VisualMode", m_VisMode);   		
		parent->StoreInteger("UseRefSys", m_UseRefSys);
		parent->StoreInteger("UseFastChecks", m_UseFastChecks);
		parent->StoreInteger("DecompositionMethod", m_DecompositionMethod);
		parent->StoreInteger("Fibers_Type", m_FbTemplate);
		parent->StoreInteger("Fibers_Num", m_FbNumFib);
		parent->StoreInteger("Fibers_Res", m_FbResolution);
		parent->StoreDouble("Fibers_Thickness", m_FbThickness);
		parent->StoreInteger("Fibers_Smooth", m_FbSmooth);
		parent->StoreInteger("Smooth_Steps", m_FbSmoothSteps);    
		parent->StoreDouble("Smooth_Weight", m_FbSmoothWeight); 

		parent->StoreInteger("Uniform_Sampling", m_FbUniformSampling);
		parent->StoreDouble("Particle_Constraint", m_PConstraintThreshold);
		parent->StoreDouble("Particle_BoneDistance", m_PBoneDistance);
		parent->StoreInteger("Spring_Layout_Type", m_PSpringLayoutType);
		parent->StoreInteger("Closest_Particle_Num", m_PClosestParticleNum);

		parent->StoreMatrix("Transform",&m_Transform->GetMatrix());
		return MAF_OK;
	}
	return MAF_ERROR;
}

//------------------------------------------------------------------------
/*virtual*/ int medVMEMuscleWrapper::InternalRestore(mafStorageElement *node)
	//------------------------------------------------------------------------
{
	if (Superclass::InternalRestore(node)==MAF_OK)
	{   
		int nWrappers;
		if (node->RestoreInteger("Wrappers_Num", nWrappers) != MAF_OK)
			nWrappers = 0;  //no wrapper available

		m_Wrappers.resize(nWrappers);		

		if (node->RestoreInteger("VisualMode", m_VisMode) != MAF_OK)
			m_VisMode = DEFAULT_VISUAL_MODE; 

		if (node->RestoreInteger("UseRefSys", m_UseRefSys) != MAF_OK)
			m_UseRefSys = DEFAULT_USEREFSYSVAL;

		if (node->RestoreInteger("UseFastChecks", m_UseFastChecks) != MAF_OK)
			m_UseFastChecks = DEFAULT_FASTCHECKS;

		if (node->RestoreInteger("DecompositionMethod", m_DecompositionMethod) != MAF_OK)
			m_DecompositionMethod = DEFAULT_DECOMPOSITION_METHOD;

		if (node->RestoreInteger("Fibers_Type", m_FbTemplate) != MAF_OK)
			m_FbTemplate = DEFAULT_FIBERS_TYPE;

		if (node->RestoreInteger("Fibers_Num", m_FbNumFib) != MAF_OK)
			m_FbNumFib = DEFAULT_FIBERS_NUM;

		if (node->RestoreInteger("Fibers_Res", m_FbResolution) != MAF_OK)
			m_FbResolution = DEFAULT_FIBERS_RES;

		if (node->RestoreDouble("Fibers_Thickness", m_FbThickness) != MAF_OK)
			m_FbThickness = DEFAULT_FIBERS_THICKNESS;

		if (node->RestoreInteger("Fibers_Smooth", m_FbSmooth) != MAF_OK)
			m_FbSmooth = DEFAULT_FIBERS_SMOOTH;

		if (node->RestoreInteger("Smooth_Steps", m_FbSmoothSteps) != MAF_OK)
			m_FbSmoothSteps = DEFAULT_FIBERS_SMOOTHSTEPS;

		if (node->RestoreDouble("Smooth_Weight", m_FbSmoothWeight) != MAF_OK)
			m_FbSmoothWeight = DEFAULT_FIBERS_SMOOTHWEIGHT;

		if (node->RestoreInteger("Uniform_Sampling", m_FbUniformSampling) != MAF_OK)
			m_FbUniformSampling = DEFAULT_FIBERS_UNIFORM_SAMPLING;

		if (node->RestoreDouble("Particle_Constraint", m_PConstraintThreshold) != MAF_OK)
			m_PConstraintThreshold = DEFAULT_PARTICLE_CONTAINT;

		if (node->RestoreDouble("Particle_BoneDistance", m_PBoneDistance) != MAF_OK)
			m_PBoneDistance = DEFAULT_PARTICLE_BONEDISTANCE;

		if (node->RestoreInteger("Spring_Layout_Type", m_PSpringLayoutType) != MAF_OK)
			m_PSpringLayoutType = DEFAULT_SPRING_LAYOUT_TYPE;

		if (node->RestoreInteger("Spring_Layout_Type", m_PSpringLayoutType) != MAF_OK)
			m_PSpringLayoutType = DEFAULT_SPRING_LAYOUT_TYPE;

		if (node->RestoreInteger("Closest_Particle_Num", m_PClosestParticleNum) != MAF_OK)
			m_PClosestParticleNum = DEFAULT_CLOSEST_PARTICLE_NUM;

		mafMatrix matrix;
		if (node->RestoreMatrix("Transform",&matrix)==MAF_OK) {    
			m_Transform->SetMatrix(matrix);
		}

		//everything has modified and should be updated
		m_Modified = EVERYTHING_MODIFIED;
		return MAF_OK;
	}

	return MAF_ERROR;  
}

#pragma region GetVMEs
//-------------------------------------------------------------------------
mafVME *medVMEMuscleWrapper::GetMuscleVME_RP()
	//-------------------------------------------------------------------------
{
	if (!m_bLinksRestored)
	{
		RestoreMeterLinks();  //so we restore links      
		m_bLinksRestored = true;
	}

	return m_MuscleVme;
}
//-------------------------------------------------------------------------
mafVME *medVMEMuscleWrapper::GetWrapperVME_RP(int nIndex)
	//-------------------------------------------------------------------------
{
	if (!m_bLinksRestored)
	{
		RestoreMeterLinks();  //so we restore links      
		m_bLinksRestored = true;
	}

	_VERIFY_CMD(nIndex >= 0 && nIndex < (int)m_Wrappers.size(), return NULL);
	return m_Wrappers[nIndex]->pVmeRP_CP[0];
}

//-------------------------------------------------------------------------
mafVME *medVMEMuscleWrapper::GetWrapperVME(int nIndex)
	//-------------------------------------------------------------------------
{
	if (!m_bLinksRestored)
	{
		RestoreMeterLinks();  //so we restore links      
		m_bLinksRestored = true;
	}

	_VERIFY_CMD(nIndex >= 0 && nIndex < (int)m_Wrappers.size(), return NULL);
	return m_Wrappers[nIndex]->pVmeRP_CP[1];
}

//-------------------------------------------------------------------------
mafVME *medVMEMuscleWrapper::GetWrapperRefSysVME_RP(int nIndex)
	//-------------------------------------------------------------------------
{
	if (!m_bLinksRestored)
	{
		RestoreMeterLinks();  //so we restore links      
		m_bLinksRestored = true;
	}

	_VERIFY_CMD(nIndex >= 0 && nIndex < (int)m_Wrappers.size(), return NULL);
	return m_Wrappers[nIndex]->pVmeRefSys_RP_CP[0];
}

//-------------------------------------------------------------------------
mafVME *medVMEMuscleWrapper::GetWrapperRefSysVME(int nIndex)
	//-------------------------------------------------------------------------
{
	if (!m_bLinksRestored)
	{
		RestoreMeterLinks();  //so we restore links      
		m_bLinksRestored = true;
	}

	_VERIFY_CMD(nIndex >= 0 && nIndex < (int)m_Wrappers.size(), return NULL);
	return m_Wrappers[nIndex]->pVmeRefSys_RP_CP[1];
}

//-------------------------------------------------------------------------
//Gets the origin area VME in its current pose
mafVME* medVMEMuscleWrapper::GetFibersOriginVME()
	//-------------------------------------------------------------------------
{
	if (!m_bLinksRestored)
	{
		RestoreMeterLinks();  //so we restore links      
		m_bLinksRestored = true;
	}

	return m_OIVME[0];
}

//-------------------------------------------------------------------------
//Gets the insertion area VME in its current pose
mafVME* medVMEMuscleWrapper::GetFibersInsertionVME()
	//-------------------------------------------------------------------------
{
	if (!m_bLinksRestored)
	{
		RestoreMeterLinks();  //so we restore links      
		m_bLinksRestored = true;
	}

	return m_OIVME[1];
}

#pragma endregion