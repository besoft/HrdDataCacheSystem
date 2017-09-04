/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medVMEMuscleWrapper_Core.cpp,v $
Language:  C++
Date:      $Date: 2012-04-30 14:52:43 $
Version:   $Revision: 1.1.2.22 $
Authors:   Josef Kohout
==========================================================================
Copyright (c) 2008-2012
University of Bedforshire, University of West Bohemia
=========================================================================*/
//#include <vld.h>

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "medVMEMuscleWrapper.h"
#include "mafTransform.h"

#include "vtkMAFSmartPointer.h"

#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkTubeFilter.h"

#include "vtkMAFMuscleDecomposition.h"
#include "vtkMAFMuscleDecompositionMMSS.h"
#include "vtkMEDPolyDataDeformation.h"
#include "vtkMEDPolyDataDeformationPK.h"

#include <assert.h>
#include "mafDbg.h"
#include "mafMemDbg.h"
#include "mafDataChecksum.h"

#include "mafVMEGeneric.h"
#include "mafVMEItemVTK.h"
#include "mafDataVector.h"
#include "mafVMESurface.h"

#include "vtkMassSpringMuscle.h"


#ifdef _PROFILING_
#include "vtkMassProperties.h"
#endif

#include "vtkMEDPolyDataDeformationPKCaching.h"

/*#include <Windows.h>
#include <Psapi.h>
#pragma comment( lib, "psapi.lib" )*/

#define USE_CACHE

//-----------------------------------------------------------------------
//Sets m_Modified attribute according to what should be updated since it has changed 
//N.B. called from 
/*virtual*/ void medVMEMuscleWrapper::MustUpdateCheck()
{
#pragma region FAST CHECKS
	if (m_UseFastChecks != 0)		
	{
		//faster check assumes that the change may come only 
		//1) from the GUI of this VME => m_Modified flags are already set from the OnEvent handling
		//2) from the timebar => in that case we need to check, if timestamp for wrappers is different

		//timebar check
		double mt = GetTimeStamp();
		if (mt != m_LastUpdateTimeStamp)
		{
			m_LastUpdateTimeStamp = mt;

			//sets modified to all CP
			for (CWrapperItemCollectionIterator pItem = m_Wrappers.begin(); pItem != m_Wrappers.end(); pItem++)
			{
				(*pItem)->Modified |= CWrapperItem::CP_WRAPPER_MODIFIED;

				if ((*pItem)->pVmeRP_CP[1] != NULL && (*pItem)->pVmeRP_CP[0] != NULL)	//check, if the wrapper as the whole is valid and may have an impact on the result
				{				
					m_Modified |= INPUT_WRAPPER_MODIFIED;

					_RPT1(_CRT_WARN, "INPUT_WRAPPER_MODIFIED(flgs = %d)\n", (*pItem)->Modified);
				}
			}
		} //end if [mt != m_LastUpdateTimeStamp]		
	}
#pragma endregion FAST CHECKS

#pragma region SLOW CHECKS
	else
	{		
		//check, if the output space matrix has not changed			
		unsigned long checksum = mafDataChecksum::Adler32Checksum((unsigned char*)m_Transform->GetMatrix().GetElements(), 16*sizeof(double));
		if (checksum != m_nOutputSpaceChecksum)
		{
			m_nOutputSpaceChecksum = checksum;
			m_Modified |= OUTPUT_SPACE_MODIFIED;

			_RPT0(_CRT_WARN, "OUTPUT_SPACE_MODIFIED\n");
		}

		//check, if the input muscle has not changed
		if (m_MuscleVme == NULL)
		{
			if (m_nMuscleChecksum != 0)
			{
				m_nMuscleChecksum = 0;
				m_Modified |= INPUT_MUSCLE_MODIFIED;

				_RPT0(_CRT_WARN, "INPUT_MUSCLE_MODIFIED #2\n");
			}
		}
		else
		{
			//muscle must be always tested at time 0, but we suppose it is not unchanged during the time	
			m_MuscleVme->GetOutput()->Update();
			checksum = 1 | mafDataChecksum::CombineAdler32Checksums(			
				mafDataChecksum::Adler32Checksum((unsigned char*)
					medVMEMuscleWrappingHelper::GetVMEAbsMatrix(m_MuscleVme)->GetElements(), 16*sizeof(double)),
				ComputeCheckSum(vtkPolyData::SafeDownCast(m_MuscleVme->GetOutput()->GetVTKData()))
				); 	

			if (checksum != m_nMuscleChecksum)
			{
				m_nMuscleChecksum = checksum;
				m_Modified |= INPUT_MUSCLE_MODIFIED;

				_RPT0(_CRT_WARN, "INPUT_MUSCLE_MODIFIED\n");
			}			
		}


		//check all REST and CURRENT POSITION wrappers and their RefSys, if they have not changed
		for (CWrapperItemCollectionIterator pItem = m_Wrappers.begin(); pItem != m_Wrappers.end(); pItem++)
		{		
			for (int i = 0; i < 2; i++)
			{										
				if ((*pItem)->pVmeRP_CP[i] == NULL)
				{
					//wrapper is invalid
					if ((*pItem)->nWrapperCheckSums[i] != 0)
					{
						//but we know about it => there must have been a change
						(*pItem)->nWrapperCheckSums[i] = 0;
						(*pItem)->Modified |= (i == 0 ? CWrapperItem::RP_WRAPPER_MODIFIED : CWrapperItem::CP_WRAPPER_MODIFIED);										
					}							
				}
				else
				{
					//wrapper is valid					
					(*pItem)->pVmeRP_CP[i]->GetOutput()->Update();
					checksum = 1 | mafDataChecksum::CombineAdler32Checksums(
						mafDataChecksum::Adler32Checksum((unsigned char*)
							medVMEMuscleWrappingHelper::GetVMEAbsMatrix((*pItem)->pVmeRP_CP[i])->GetElements(), 16*sizeof(double)),
						ComputeCheckSum(vtkPolyData::SafeDownCast((*pItem)->pVmeRP_CP[i]->GetOutput()->GetVTKData()))
						);

					if (checksum != (*pItem)->nWrapperCheckSums[i])
					{
						(*pItem)->nWrapperCheckSums[i] = checksum;
						(*pItem)->Modified |= (i == 0 ? CWrapperItem::RP_WRAPPER_MODIFIED : CWrapperItem::CP_WRAPPER_MODIFIED);				
					}					
				} //end else [Wrapper]

				//check if the RefSys hasn't changed		
				if ((*pItem)->pVmeRefSys_RP_CP[i] == NULL)
				{
					//RefSys is not valid
					if ((*pItem)->nRefSysChecksSums[i] != 0)
					{
						//but we know about it => there must have been a change
						(*pItem)->nRefSysChecksSums[i] = 0;
						(*pItem)->Modified |= (i == 0 ? CWrapperItem::RP_REFSYS_MODIFIED : CWrapperItem::CP_REFSYS_MODIFIED);
					}	
				}
				else
				{
					//RefSys is valid
					double RSO[3];
					unsigned long checksum = GetRefSysVMEOrigin((*pItem)->pVmeRefSys_RP_CP[i], RSO) ?
						(1 | mafDataChecksum::Adler32Checksum((unsigned char*)&RSO[0], sizeof(RSO))) : 0;

					if (checksum != (*pItem)->nRefSysChecksSums[i])			
					{            
						(*pItem)->nRefSysChecksSums[i] = checksum;
						(*pItem)->RefSysOrigin[i][0] = RSO[0];
						(*pItem)->RefSysOrigin[i][1] = RSO[1];
						(*pItem)->RefSysOrigin[i][2] = RSO[2];					
					}					
				} //end else [RefSys]			
			} //end for i		

			if ((*pItem)->pVmeRP_CP[1] != NULL && (*pItem)->pVmeRP_CP[0] != NULL)	//check, if the wrapper as the whole is valid and may have an impact on the result
			{
				if ((*pItem)->Modified != CWrapperItem::NOTHING_MODIFIED)
				{
					m_Modified |= INPUT_WRAPPER_MODIFIED;
					_RPT1(_CRT_WARN, "INPUT_WRAPPER_MODIFIED(flgs = %d)\n", (*pItem)->Modified);
				}
			}			
		} //end for [all wrappers]	
	} //end if m_UseFastCheck = 1	
#pragma endregion SLOW CHECKS

	//if output space has changed, we will need to transform everything once again
	if ((m_Modified & OUTPUT_SPACE_MODIFIED) != 0)
		m_Modified |= INPUT_MUSCLE_MODIFIED | INPUT_WRAPPER_MODIFIED;

	//if input has changed, we will need to run deformation process once again
	if ((m_Modified & (INPUT_MUSCLE_MODIFIED | INPUT_WRAPPER_MODIFIED)) != 0)
		m_Modified |= DEFORMATION_OPTIONS_MODIFIED;

	//musculoskeletal model check
	m_pVMEMusculoSkeletalModel = GetMusculoSkeletalModel();
	if (m_pVMEMusculoSkeletalModel != NULL)
	{
		//this medVMEMuscleWrapper is a part of complex musculoskeletal model
		int lu = m_pVMEMusculoSkeletalModel->GetConfirmTimeStamp();
		if (lu != m_LastMMConfirmTimeStamp)
		{
			m_LastMMConfirmTimeStamp = lu;		//something has changed

			m_DeformationMethod = m_pVMEMusculoSkeletalModel->GetDeformationMethod();
			m_InterpolateStepCount = m_pVMEMusculoSkeletalModel->GetInterpolateStepCount();
			m_UseProgressiveHulls = m_pVMEMusculoSkeletalModel->UseProgressiveHulls();
			m_UsePenetrationPrevention = m_pVMEMusculoSkeletalModel->UsePenetrationPrevention();
			m_IterationStep = m_pVMEMusculoSkeletalModel->GetParticleIterStep();
			m_IterationNum = m_pVMEMusculoSkeletalModel->GetParticleMaxIterNum();
			m_UseFixedTimestep = m_pVMEMusculoSkeletalModel->UseFixedTimestep();
			m_Modified |= DEFORMATION_OPTIONS_MODIFIED;
		}
	}

	//if the muscle should deform, its decomposition must run once again
	if ((m_Modified & DEFORMATION_OPTIONS_MODIFIED) != 0)
		m_Modified |= FIBERS_OPTIONS_MODIFIED | POLYDATA_MUST_UPDATE;	//if deformed, then polydata is to be updated

	//check, if there is anything new to be visualized
	int nMask = m_VisMode == 0 ? FIBERS_OPTIONS_MODIFIED : 0;
	if ((m_Modified & ~nMask) != NOTHING_MODIFIED) 
		m_Modified |= POLYDATA_MUST_UPDATE;	//output polydata should be displayed
}

//-----------------------------------------------------------------------
//It ensures that we have the transformed muscle surface and wrappers
//on the input, so the deformation may proceed. m_Modified flags are 
//set in this method to allow determination, if input has changed.
//N.B. Called from InternalPreUpdate 
/*virtual*/ void medVMEMuscleWrapper::PrepareInput()
{	
	//this happens when the user just checks VME without its selection  
	//or deletes some linked VME from VME tree
	if (!m_bLinksRestored)
	{
		RestoreMeterLinks();  //so we restore links  => everything must be updated   
		m_bLinksRestored = true;
	}

	//check what must be update
	MustUpdateCheck();	

	if ((m_Modified & (INPUT_MUSCLE_MODIFIED | INPUT_WRAPPER_MODIFIED)) != 0)
	{
		//something has changed
		//check, if it is not simple switch from deformed muscle to decomposed one

		//something important has changed => need to update transformed input
#ifdef _PROFILING_
		LARGE_INTEGER liStart;
		QueryPerformanceCounter(&liStart);
#endif
		//but we will perform the deformation only, if we have valid muscle and at least one valid wrapper
		//nevertheless cached data must be prepared			
		PrepareInputWrappers();
		PrepareInputMuscle();

#ifdef _PROFILING_
		LARGE_INTEGER liEnd, liFreq;
		QueryPerformanceCounter(&liEnd);
		QueryPerformanceFrequency(&liFreq);	

		FILE* fProf;
		if (
#if _MSC_VER >= 1400
			0 == (fopen_s(&fProf, 
#else
			NULL != (fProf = fopen(
#endif
			"muscle_wrapping_profiling_transformation.txt", "at")))
		{
			_ftprintf(fProf, "%s\t%.3f\t%.2f ms\n", this->GetName(), this->GetTimeStamp(),
				(1000.0*(liEnd.QuadPart - liStart.QuadPart)) / liFreq.QuadPart);
			fclose(fProf);
		}
#endif	

		m_Modified &= ~(INPUT_MUSCLE_MODIFIED | INPUT_WRAPPER_MODIFIED | OUTPUT_SPACE_MODIFIED);
	}
}

//-----------------------------------------------------------------------
//It ensures that we have the transformed wrappers on the input. 	
//N.B. Must be called after MustUpdateCheck() + called from PrepareInput
/*virtual*/ void medVMEMuscleWrapper::PrepareInputWrappers()
{			
	if ((m_Modified & INPUT_WRAPPER_MODIFIED) != 0)
	{
		mafMatrix output_space;
		output_space.DeepCopy(medVMEMuscleWrappingHelper::GetVMEAbsMatrix(this));

		for (CWrapperItemCollectionIterator pItem = m_Wrappers.begin(); pItem != m_Wrappers.end(); pItem++)
		{
			for (int i = 0; i < 2; i++)
			{
				if (((*pItem)->Modified & (i + 1)) != 0 || (m_Modified & OUTPUT_SPACE_MODIFIED) != 0)
				{
					//this curve needs to be updated					
					if ((*pItem)->pVmeRP_CP[i] != NULL)
					{
						//the source is valid						
						//retrieve polydata curve, fix it, transform it and store it
						//first, get control curve from the associated VME
						vtkPolyData* pPoly = vtkPolyData::SafeDownCast((*pItem)->pVmeRP_CP[i]->GetOutput()->GetVTKData());
						pPoly->Update();    //force update        

						//mafVMEMeter (version 20.10.2008) generates corrupted polylines, 
						//they contain duplicated coordinates and edges, e.g.
						//Pts: 0(318,305,-467), 1(379,292,-824), 2(388,310, -871), 3(379,292,-824)
						//Edges: 0-1,1-2,2-3 => vertex 1 and 3 are redundant
						FixPolyline(pPoly, (*pItem)->pCurves[i]);

						//Debug_Write(i == 0 ? "OC - Local" : "DC - Local", (*pItem)->pCurves[i]);

						//transform coordinates into output reference system
						TransformPoints((*pItem)->pCurves[i]->GetPoints(), (*pItem)->pCurves[i]->GetPoints(), 
							medVMEMuscleWrappingHelper::GetVMEAbsMatrix((*pItem)->pVmeRP_CP[i]), &output_space);

						//Debug_Write(i == 0 ? "OC - Transformed" : "DC - Transformed", (*pItem)->pCurves[i]);

						//if we do not use m_UseFastChecks, we have already transformed reference VME points, otherwise do it now
						if (m_UseFastChecks != 0 && (*pItem)->pVmeRefSys_RP_CP[i] != NULL)
						{
							double RSO[3];
							unsigned long checksum = GetRefSysVMEOrigin((*pItem)->pVmeRefSys_RP_CP[i], RSO) ?
								(1 | mafDataChecksum::Adler32Checksum((unsigned char*)&RSO[0], sizeof(RSO))) : 0;

							if (checksum != (*pItem)->nRefSysChecksSums[i])			
							{            
								(*pItem)->nRefSysChecksSums[i] = checksum;
								(*pItem)->RefSysOrigin[i][0] = RSO[0];
								(*pItem)->RefSysOrigin[i][1] = RSO[1];
								(*pItem)->RefSysOrigin[i][2] = RSO[2];					
							}
						}						
					}
				}
			} //end for i

			(*pItem)->Modified = CWrapperItem::NOTHING_MODIFIED;	//everything is cached now
		} //end for [all wrappers]
	}
}

//-----------------------------------------------------------------------
//It ensures that we have the transformed muscle surface on the input.	
//N.B. Must be called after MustUpdateCheck() + called from PrepareInput
/*virtual*/ void medVMEMuscleWrapper::PrepareInputMuscle()
{
	//next, process input muscle
	if ((m_Modified & (INPUT_MUSCLE_MODIFIED | OUTPUT_SPACE_MODIFIED)) != 0)
	{
		//the input muscle has changed or we have different output space		
		if (m_MuscleVme == NULL)
		{
			//if there is no valid associated muscle, output is empty
			m_pTransformedMuscle->SetPoints(NULL);
			m_pTransformedMuscle->SetPolys(NULL);
			m_pDeformedMuscle->SetPoints(NULL);
			m_pDeformedMuscle->SetPolys(NULL);
			m_pDecomposedMuscle->SetPoints(NULL);
			m_pDecomposedMuscle->SetPolys(NULL);
			return;	//there is nothing to be done
		}

		//there is a valid associated muscle, so transform it
		vtkPolyData* pPoly = CreateTransformedPolyDataFromVME(m_MuscleVme
#ifdef _DEBUG_VIS_
			,"Muscle"
#endif
			);

		m_pTransformedMuscle->ShallowCopy(pPoly);
		pPoly->Delete();	//release pPoly
	} //end if [muscle processed]

}


//-----------------------------------------------------------------------
//called to prepare the update of output
//it ensures that we have transformed input muscle and wrappers, so
//InternalUpdate may deform x deform + create fibers as needed.
//This method also changes m_Modified status.
/*virtual*/ void medVMEMuscleWrapper::InternalPreUpdate() 
	//-----------------------------------------------------------------------
{	
	//this is called by the kernel when a musclewrapper should prepare its data
	//there are five various scenarios:
	//
	//1) this musclewrapper is not a part of any musculoskeletalmodel =>
	//		no penetration check will be performed and the muscle will be deformed by
	//		the default method (it is stored in m_DeformationMethod)
	//		the musclewrapper is processed as single
	//
	// 2) this musclewrapper is a part of some musculoskeletal model but it is
	//		 excluded from the processing =>
	//		no penetration check will be performed and the muscle will be deformed by
	//		the default method (it is stored in m_DeformationMethod)
	//		the musclewrapper is processed as single
	//		if the musclewrapper is MASTER, it needs to leave its MASTERSHIP
	//
	//	3) this musclewrapper is a part of some musculoskeletal model, it is not
	//		excluded from the processing and it is the first musclewrapper being called =>
	//		this musclewrapper becomes MASTER of all other musclewrappers not excluded
	//		from this process that belong to the same musculoskeletal model.
	//		MASTER must detect, if any of its musclewrappers is not outdated
	//		If it is, the deformation process must rerun (called at this)
	//		and fibres recreated (called at all musclewrappers)
	//
	//4) this musclewrapper has its MASTER => theoretically, its outdate check was
	//		already performed by its MASTER, so it does not need to continue
	//		practically, it checks, if something is new and if it is, it will call MASTER

	//check our input and if it is outdated, transform the input so it could be deformed later
	PrepareInput();

	//check, if we do not have any master above us	
	if (m_MasterVMEMuscleWrapper != NULL)
	{
		//if we does, let the master to govern InternalPreUpdate
		//but only, if there is any request to run the deformation
		if ((m_Modified & DEFORMATION_OPTIONS_MODIFIED) != 0) {
			m_MasterVMEMuscleWrapper->InternalPreUpdate();
			//master may revoke its mastership => we may be new masters
		}
	}

	//original might revoke its mastership => we may have no longer a master
	if (m_MasterVMEMuscleWrapper == NULL)
	{
		//there is no MASTER for us => we are either new MASTER or SINGLE node
		//first, revoke our mastership from all wrappers we know about, so they could become MASTERS
		for (medVMEMusculoSkeletalModel::mafVMENodeList::iterator // const_iterator 
			it = m_VMEMuscleWrappers.begin();	it != m_VMEMuscleWrappers.end(); it++)
		{
			medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(*it);
			delete wrapper->m_pParticleSystem;
			wrapper->m_pParticleSystem = NULL;

			wrapper->m_MasterVMEMuscleWrapper = NULL;
		}


		//WE might be MASTER only, if we have MusculoskeletalModel here, deformation is set to be PkMode
		//and we are not excluded from the process
		bool bThisFound = false;
		if (m_pVMEMusculoSkeletalModel != NULL)
		{
			//this medVMEMuscleWrapper is a part of complex musculoskeletal model
			//if PK method is supposed to be used and penetration should be prevented ...										
			if (m_DeformationMethod && m_UsePenetrationPrevention)
			{
				//... get all medVMEMuscleWrappers coeexisting in this model 
				//and call InternalPreUpdate upon then, if they signalized a change
				//set DEFORMATION_OPTIONS_MODIFIED to this muscle wrappers						
				m_pVMEMusculoSkeletalModel->GetMuscleWrappers(m_VMEMuscleWrappers);

				//first, check, if we are a member of enabled VME muscle wrappers,
				//if not, let us behave in SINGLE mode
				for (medVMEMusculoSkeletalModel::mafVMENodeList::iterator // const_iterator 
					it = m_VMEMuscleWrappers.begin();	it != m_VMEMuscleWrappers.end(); it++)
				{
					medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(*it);
					if (wrapper == this) 
					{
						//removes this from list => list contains only SLAVES
						m_VMEMuscleWrappers.erase(it);
						bThisFound = true;
						break;
					}
				}
			}
		} //end if [(m_pVMEMusculoSkeletalModel != NULL)]

		if (!bThisFound)
		{
			//if penetrations preservation is not needed or this object is filtered out				
			//get the configuration and behave like SINGLE (not a part of MM model)				
			m_VMEMuscleWrappers.clear();
		}
		else if (m_VMEMuscleWrappers.size() != 0)
		{									
			//OK, we are MASTER of other muscle wrappers with which we will need to cooperate
			//notify them that we are their master + check if any of them signal that their muscle 
			//needs to be deformed (it is out of date)
			for (medVMEMusculoSkeletalModel::mafVMENodeList::const_iterator 
				it = m_VMEMuscleWrappers.begin();	it != m_VMEMuscleWrappers.end(); it++)
			{
				medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(*it);

				_ASSERTE((wrapper->m_MasterVMEMuscleWrapper == NULL|| 
					wrapper->m_MasterVMEMuscleWrapper == this) &&
					wrapper->m_VMEMuscleWrappers.size() == 0);

				//set us to be MASTER 
				wrapper->m_MasterVMEMuscleWrapper = this;

				//check the wrapper input and if it is outdated, transform the input so it could be deformed later				
				wrapper->PrepareInput();

				//if the wrapper is out of date, we will need to execute deformation
				if ((wrapper->m_Modified & DEFORMATION_OPTIONS_MODIFIED) != 0)
					m_Modified |= DEFORMATION_OPTIONS_MODIFIED | FIBERS_OPTIONS_MODIFIED | POLYDATA_MUST_UPDATE;	
			}

			//if this MASTER muscle wrapper is to be updated, we will need to update SLAVES as well
			if ((m_Modified & DEFORMATION_OPTIONS_MODIFIED) != 0)
			{				
				for (medVMEMusculoSkeletalModel::mafVMENodeList::const_iterator 
					it = m_VMEMuscleWrappers.begin();	it != m_VMEMuscleWrappers.end(); it++)
				{
					medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(*it);
					wrapper->m_Modified |= DEFORMATION_OPTIONS_MODIFIED | FIBERS_OPTIONS_MODIFIED | POLYDATA_MUST_UPDATE;	
				}
			}
		}
	} //end if (m_MasterVMEMuscleWrapper == NULL)	
}

//-----------------------------------------------------------------------
//NB. mafVMEMeter and MAF is really stupid, it generates new data even if
//nothing has changed (especially lot of data is generated during the animation)
//we will need to avoid redundant deformation as much as possible
void medVMEMuscleWrapper::InternalUpdate()
	//-----------------------------------------------------------------------
{
#pragma region DEFORMATION PART
	//check, if we do not have any master above us
	if (m_MasterVMEMuscleWrapper != NULL)
	{
		//if we does, let the master to govern InternalUpdate
		m_MasterVMEMuscleWrapper->InternalUpdate();
	}
	else
	{
		//we are either MASTER or we are SINGLE (not a part of any model)
		//if needed, deform the muscle		
		if ((m_Modified & DEFORMATION_OPTIONS_MODIFIED) != 0) 
		{
#ifdef _PROFILING_
			LARGE_INTEGER liStart;
			QueryPerformanceCounter(&liStart);
#endif
			//OK, we will need deform muscle
			wxBusyCursor busy;
			DeformMuscle();

			//now, everything is deformed, so we may change m_Modified
			m_Modified &= ~DEFORMATION_OPTIONS_MODIFIED;

			//change m_Modified for our slaves
			for (medVMEMusculoSkeletalModel::mafVMENodeList::const_iterator 
				it = m_VMEMuscleWrappers.begin();	it != m_VMEMuscleWrappers.end(); it++)
			{
				medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(*it);
				wrapper->m_Modified &= ~DEFORMATION_OPTIONS_MODIFIED;							
			}

#ifdef _PROFILING_
			LARGE_INTEGER liEnd, liFreq;
			QueryPerformanceCounter(&liEnd);
			QueryPerformanceFrequency(&liFreq);

			FILE* fProf;
			if (
#if _MSC_VER >= 1400
				0 == (fopen_s(&fProf, 
#else
				NULL != (fProf = fopen(
#endif
				"muscle_wrapping_profiling_deformation.txt", "at")))
			{	
				vtkMassProperties* mp = vtkMassProperties::New();
				mp->SetInput(m_pTransformedMuscle);
				double dblVol1 = mp->GetVolume();

				mp->SetInput(m_pDeformedMuscle);
				double dblVol2 = mp->GetVolume();

				/****************************/
				/*PROCESS_MEMORY_COUNTERS memCounterAfter;
				bool result = GetProcessMemoryInfo(GetCurrentProcess(), &memCounterAfter, sizeof(memCounterAfter));*/
				/****************************/

				_ftprintf(fProf, "MASTER\t%s\t%.3f\t%.2f ms\t%.2f%%\n", this->GetName(), this->GetTimeStamp(),
					(1000.0*(liEnd.QuadPart - liStart.QuadPart)) / liFreq.QuadPart,
					100.0*dblVol2 / dblVol1);

				for (medVMEMusculoSkeletalModel::mafVMENodeList::const_iterator 
					it = m_VMEMuscleWrappers.begin();	it != m_VMEMuscleWrappers.end(); it++)
				{
					medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(*it);
					mp->SetInput(wrapper->m_pTransformedMuscle);
					dblVol1 = mp->GetVolume();
					mp->SetInput(wrapper->m_pDeformedMuscle);
					dblVol2 = mp->GetVolume();

					_ftprintf(fProf, "SLAVE\t%s\t%.3f\t\t%.2f%%\n", wrapper->GetName(), 
						this->GetTimeStamp(),	100.0*dblVol2 / dblVol1);
				}

				fclose(fProf);
				/*****************************************/
				/*
				FILE* myMeasurementData = fopen("myMeasurementData.csv", "at");
				_ftprintf(fProf, ";%s;%.3f;%d;\n", this->GetName(), (1000.0*(liEnd.QuadPart - liStart.QuadPart)) / liFreq.QuadPart, memCounterAfter.WorkingSetSize);
				fclose(myMeasurementData);*/
				/*****************************************/
			}
#endif
		}			
	} //end [we are MASTER]
#pragma endregion DEFORMATION PART

#pragma region DECOMPOSITION PART
	//from now on, the MASTER, SLAVE, SINGLE work is identical
	//SLAVES that are visualized will receive their own InternalUpdate, 
	//and those not visualized at present have only DeformMuscle present
	//and the rest is done 

	//m_VisMode says, if we are going to display fibers or only deformed muscles
	//if we do not intend to display fibers, skip any change to fibers options
	//at present, it will be reflected when m_VisMode is changed	
	if (m_VisMode != 0 && (m_Modified & FIBERS_OPTIONS_MODIFIED) != 0) 
	{	
		//fibers must be recreated (typically, it there is any immediate need for it)

#ifdef _PROFILING_
		LARGE_INTEGER liStart;
		QueryPerformanceCounter(&liStart);
#endif
		//OK, we will need to regenerate fibers		
		wxBusyCursor busy;
		GenerateFibers();
		if (GetMusculoSkeletalModel() == GetMusculoSkeletalModel_RP()) {
			m_Particles.clear();
			m_ParticleMap.clear();
		}

		m_Modified &= ~FIBERS_OPTIONS_MODIFIED;	//remove m_Modified flag

#ifdef _PROFILING_
		LARGE_INTEGER liEnd, liFreq;
		QueryPerformanceCounter(&liEnd);
		QueryPerformanceFrequency(&liFreq);

		FILE* fProf;
		if (
#if _MSC_VER >= 1400
			0 == (fopen_s(&fProf, 
#else
			NULL != (fProf = fopen(
#endif
			"muscle_wrapping_profiling_decomposition.txt", "at")))
		{
			_ftprintf(fProf, "%s\t%.3f\t%.2f ms\n", this->GetName(), this->GetTimeStamp(),
				(1000.0*(liEnd.QuadPart - liStart.QuadPart)) / liFreq.QuadPart);
			fclose(fProf);
		}
#endif							
	}	

#pragma endregion DECOMPOSITION PART

#pragma region VISUALIZATION PART
	if ((m_Modified & POLYDATA_MUST_UPDATE) != 0)
	{
		if (m_VisMode == 0)
			m_PolyData->ShallowCopy(m_pDeformedMuscle);
		else
		{
			if (m_FbThickness == 0.0)// || m_DeformationMethod == 2) // method 2 = mass spring system - generates balls instead of tubes
				m_PolyData->ShallowCopy(m_pDecomposedMuscle);    
			else
			{
				if (m_pTubeFilter == NULL)
					m_pTubeFilter = vtkTubeFilter::New();
				m_pTubeFilter->SetInput(m_pDecomposedMuscle);
				//pTube->UseDefaultNormalOff();        
				//pTube->SetCapping(true);
				m_pTubeFilter->SetNumberOfSides(8);
				m_pTubeFilter->SetRadius(m_FbThickness); //0.01);				
				m_pTubeFilter->Update();    
				m_PolyData->ShallowCopy(m_pTubeFilter->GetOutput());    
			}


#ifdef _DEBUG_SAVE_VME
			//this code saves the output as a new VME
			mafVMESurface* VME;
			mafNEW(VME);
			VME->ReparentTo(this);
			VME->SetData(m_PolyData, 0);
			VME->SetName(wxString::Format("CONTOURS_%d", m_FbResolution));  

			mafEvent ev(this, VME_ADD, VME);
			this->ForwardUpEvent(ev);
#endif			
		}

		m_Modified &= ~POLYDATA_MUST_UPDATE;
	}
#pragma endregion VISUALIZATION PART
}


void recomputePoint(double* point, const double* p1, const double* p2, double t) {
	double s[3] = {0,0,0};
	PKUtils::SubtractVertex(p2, p1, s);
	PKUtils::MultiplyVertex(s, t);
	PKUtils::AddVertex(s, p1, point);
}

/*Recursively processes collisions in parallel. Used in the MSS deformation method*/
void ProcessCollisions(int startIndex, int noOfItems, vtkMassSpringMuscle **MSSmuscles, int numberOfIteration)
{
	if (noOfItems == 1) return;
	int group1End = noOfItems / 2;

#pragma omp parallel for num_threads(group1End)
	for (int c = 0; c < group1End; c++)
		MSSmuscles[c + startIndex]->CollideWithAnotherObject(MSSmuscles[group1End + c], numberOfIteration);

	int isOdd = noOfItems % 2;
	if (isOdd == 1)
	{
		for (int i = startIndex; i <= group1End; i++)
			MSSmuscles[i]->CollideWithAnotherObject(MSSmuscles[startIndex + (noOfItems - 1)], numberOfIteration);
	}
#pragma omp parallel num_threads(2)
	{
#pragma omp single nowait
		ProcessCollisions(startIndex, group1End, MSSmuscles, numberOfIteration);
#pragma omp single
		ProcessCollisions(group1End + startIndex, group1End + isOdd, MSSmuscles, numberOfIteration);
	}
}

//------------------------------------------------------------------------
//Deforms the transformed muscle according to existing wrappers.
//m_pTransformedMuscle -> m_pDeformedMuscle.
void medVMEMuscleWrapper::DeformMuscle()
	//------------------------------------------------------------------------
{	
	//VLDEnable();

	//this method cannot be called from SLAVES!
	//use m_MasterVMEMuscleWrapper->InternalUpdate() to do the change
	_ASSERTE(m_MasterVMEMuscleWrapper == NULL);

	if (m_DeformationMethod == 2)
	{
		//Mass spring system method
		DeformMuscleMMSS();		
	}
	else
	{

		//BES: 13.2.2012 - rest-pose position may be completely different from current-pose,
		//e.g., while rest-pose is a standing skeleton with pelvis centroid at 0,0,0 in world coordinates
		//facing positive x-axis, current pose could be a walking skeleton with pelvis centroid at 1000, -100, 0
		//facing negative x-axis. This may cause troubles since trajectories between the rest-pose muscle 
		//and its current-pose clone intersect bones => let us transform rest-pose in order to align pelvis
		//in order to minimize the differences between both poses and improve the stability of method

		mafTransform transform;

		bool bTransform = GetInitialTransform(&transform);	

		mafTransform transform_1(transform);
		transform_1.Invert();

		if (m_DeformationMethod == 1)	//1 == PK method,SHOULD we use the new method?
		{
			//BES: 14.1.2009 - added correspondence to avoid problems when rest pose is 
			//very different from the current pose
			vtkIdList* pCorrespondence = vtkIdList::New();
			pCorrespondence->InsertNextId(0);
			pCorrespondence->InsertNextId(0);

#ifdef USE_CACHE
			vtkMAFSmartPointer< vtkMEDPolyDataDeformationPKCaching > pDeformer;
#else
			vtkMAFSmartPointer< vtkMEDPolyDataDeformationPK > pDeformer;
#endif
			IMultiMeshDeformer* pIface = (IMultiMeshDeformer*)pDeformer;

#ifdef _DEBUG_VIS_
			if (m_DefDebugShowSteps != 0) {
				pIface->SetDebugMode(1);
			}
#endif

			//temporarily add this object (master) into m_VMEMuscleWrappers
			m_VMEMuscleWrappers.push_back(this);

			//set muscle meshes
			int nMuscles = (int)m_VMEMuscleWrappers.size();
			pIface->SetNumberOfMeshes(nMuscles);

			//HULLS buffer
			bool* CoarseMeshPresent = new bool[nMuscles];		
			for (int i = 0; i < nMuscles; i++)		
			{
				medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(m_VMEMuscleWrappers[i]);
				if (!bTransform)
					pIface->SetInputMesh(i, wrapper->m_pTransformedMuscle);	//set the input mesh
				else
				{
					vtkPolyData* pTransformedPoly = TransformPolyData(wrapper->m_pTransformedMuscle, &transform);
					pIface->SetInputMesh(i, pTransformedPoly);	//set the input mesh
					pTransformedPoly->Delete();
				}

				//check, if there is any hull already prepared for the input VME
				mafVME* hull = GetHullVME(wrapper->GetMuscleVME_RP());
				if (CoarseMeshPresent[i] = (hull != NULL))
				{	//and set it as a coarse mesh for the input VME
					vtkPolyData* pTransformedPoly = CreateTransformedPolyDataFromVME(hull);				
					if (!bTransform)
						pIface->SetCoarseMesh(i, pTransformedPoly);
					else
					{
						vtkPolyData* pTransformedPoly2 = TransformPolyData(pTransformedPoly, &transform);
						pIface->SetCoarseMesh(i, pTransformedPoly2);	//set the input mesh
						pTransformedPoly2->Delete();
					}
					pTransformedPoly->Delete();
				}	

				pIface->SetOutputMesh(i, wrapper->m_pDeformedMuscle);		//set the output mesh

				//set wrappers (skeletons, action lines) for the current muscle surface
				int nWrappers = (int)wrapper->m_Wrappers.size();
				pIface->SetNumberOfMeshSkeletons(i, nWrappers);
				for (int j = 0; j < nWrappers; j++)
				{
					//N.B. wrapper->m_Wrappers[j]->pCurves[x] MAY NOT BE NULL!
					if (!bTransform) {
						pIface->SetMeshSkeleton(i, j, 
							wrapper->m_Wrappers[j]->pCurves[0], wrapper->m_Wrappers[j]->pCurves[1], pCorrespondence, 
							(wrapper->m_Wrappers[j]->nRefSysChecksSums[0] != 0 ? wrapper->m_Wrappers[j]->RefSysOrigin[0] : NULL),
							(wrapper->m_Wrappers[j]->nRefSysChecksSums[1] != 0 ? wrapper->m_Wrappers[j]->RefSysOrigin[1] : NULL)
							); 
					}
					else
					{
						double trRefSys[3];
						vtkPolyData* pTransformedPoly = TransformPolyData(wrapper->m_Wrappers[j]->pCurves[0], &transform);
						if (wrapper->m_Wrappers[j]->nRefSysChecksSums[0] != 0)
							transform.TransformPoint(wrapper->m_Wrappers[j]->RefSysOrigin[0], trRefSys);

						pIface->SetMeshSkeleton(i, j, 
							pTransformedPoly, wrapper->m_Wrappers[j]->pCurves[1], pCorrespondence, 
							(wrapper->m_Wrappers[j]->nRefSysChecksSums[0] != 0 ? trRefSys : NULL),
							(wrapper->m_Wrappers[j]->nRefSysChecksSums[1] != 0 ? wrapper->m_Wrappers[j]->RefSysOrigin[1] : NULL)
							); 

						pTransformedPoly->Delete();
					}
				}
			}

			//if we have some hard constraints, such as bones, use them now	
			int nObstacles = 0;
			bool* CoarseObstaclePresent = NULL;
			medVMEMusculoSkeletalModel::mafVMENodeList list;

			if (m_pVMEMusculoSkeletalModel != NULL && m_UsePenetrationPrevention)
			{						
				m_pVMEMusculoSkeletalModel->GetBoneGroups(list, medVMEMusculoSkeletalModel::LOD::Lowest);

				nObstacles = (int)list.size();
				pIface->SetNumberOfObstacles(nObstacles);
				CoarseObstaclePresent = new bool[nObstacles];
				for (int i = 0; i < nObstacles; i++)
				{
					//get VTK polydata with bones (of the lowest resolution available - to speed up the processing)
					mafVME* vme = mafVME::SafeDownCast(list[i]);
					vtkPolyData* pTransformedPoly = CreateTransformedPolyDataFromVME(vme);				
					pIface->SetObstacle(i, pTransformedPoly);
					pTransformedPoly->Delete();

					//check, if there is any hull already prepared for the vme
					mafVME* hull = GetHullVME(vme);
					if (CoarseObstaclePresent[i] = (hull != NULL))
					{
						pTransformedPoly = CreateTransformedPolyDataFromVME(hull);				
						pIface->SetCoarseObstacle(i, pTransformedPoly);
						pTransformedPoly->Delete();					
					}				
				}
			}

			//AND DO IT :-)
			pIface->SetUseProgressiveHulls(m_UseProgressiveHulls);
			_VERIFY(pIface->ExecuteMultiData());

#ifdef USE_CACHE
			for (unsigned int i = 0; i < nMuscles; i++)
			{
				medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(m_VMEMuscleWrappers[i]);
				wrapper->m_pDeformedMuscle->ShallowCopy(pIface->GetOutputMesh(i));
				pIface->GetOutputMesh(i)->Delete();
			}
#endif

			//store hulls, if they are available
			for (int i = 0; i < nMuscles; i++) 
			{
				if (!CoarseMeshPresent[i])
				{
					//we may have a new coarse mesh here computed as a by-product
					vtkPolyData* hullData = pIface->GetCoarseMesh(i);
					if (hullData != NULL)
					{
						medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(m_VMEMuscleWrappers[i]);
						if (!bTransform)
							StoreHull(wrapper->GetMuscleVME_RP(), hullData);
						else
						{
							//Hull was computed in RP' position => transform it it RP
							vtkPolyData* pTransformedPoly = TransformPolyData(hullData, &transform_1);
							StoreHull(wrapper->GetMuscleVME_RP(), pTransformedPoly);
							pTransformedPoly->Delete();
						}
					}
					//hullData is destroyed by pIface automatically
				}
			}

			for (int i = 0; i < nObstacles; i++) 
			{
				if (!CoarseObstaclePresent[i])
				{
					//we may have a new coarse mesh here computed as a by-product
					vtkPolyData* hullData = pIface->GetCoarseObstacle(i);
					if (hullData != NULL) {					
						StoreHull(GetLowestResolutionVME(mafVME::SafeDownCast(list[i])), hullData);
					}
					//hullData is destroyed by pIface automatically
				}
			}

			delete[] CoarseMeshPresent;
			delete[] CoarseObstaclePresent;


			//let us detach the from output
			for (int i = 0; i < nMuscles; i++) {
				pIface->SetOutputMesh(i, NULL);		//set the output mesh
			}

			//remove ourself
			m_VMEMuscleWrappers.pop_back();
			pCorrespondence->Delete();
		}

		//	================================================================================	
		else if (m_DeformationMethod == 3)	//1 == HAJ modification - interpolation using PK method
		{
			int stepCount = m_InterpolateStepCount; // number of steps for the interpolation of deformation
			//BES: 14.1.2009 - added correspondence to avoid problems when rest pose is 
			//very different from the current pose
			vtkIdList* pCorrespondence = vtkIdList::New();
			pCorrespondence->InsertNextId(0);
			pCorrespondence->InsertNextId(0);

			vtkMAFSmartPointer< vtkMEDPolyDataDeformationPK > pDeformer;
			IMultiMeshDeformer* pIface = (IMultiMeshDeformer*)pDeformer;

#ifdef _DEBUG_VIS_
			if (m_DefDebugShowSteps != 0) {
				pIface->SetDebugMode(1);
			}
#endif

			//temporarily add this object (master) into m_VMEMuscleWrappers
			m_VMEMuscleWrappers.push_back(this);

			//set muscle meshes
			int nMuscles = (int)m_VMEMuscleWrappers.size();
			pIface->SetNumberOfMeshes(nMuscles);


			//HULLS buffer
			bool* CoarseMeshPresent = new bool[nMuscles];	
			vtkPolyData ***semiWrappers = new vtkPolyData**[nMuscles]; // all wrappers for all muscles in all time steps
			int *wrappersCount = new int[nMuscles];
			double ***wrapperPoints = new double**[nMuscles];

			//vtkPolyData * coarseDeformed = vtkPolyData::New();
			for (int i = 0; i < nMuscles; i++)		
			{
				medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(m_VMEMuscleWrappers[i]);		
				if (!bTransform)
					pIface->SetInputMesh(i, wrapper->m_pTransformedMuscle);	//set the input mesh
				else
				{
					vtkPolyData* pTransformedPoly = TransformPolyData(wrapper->m_pTransformedMuscle, &transform);
					pIface->SetInputMesh(i, pTransformedPoly);	//set the input mesh
					pTransformedPoly->Delete();
				}

				//check, if there is any hull already prepared for the input VME
				mafVME* hull = GetHullVME(wrapper->GetMuscleVME_RP());
				if (CoarseMeshPresent[i] = (hull != NULL))
				{	//and set it as a coarse mesh for the input VME
					vtkPolyData *pTransformedPoly = CreateTransformedPolyDataFromVME(hull);				
					if (!bTransform)
						pIface->SetCoarseMesh(i, pTransformedPoly);
					else
						if (bTransform)
						{
							vtkPolyData *pTransformedPoly2 = TransformPolyData(pTransformedPoly, &transform);
							pIface->SetCoarseMesh(i, pTransformedPoly2);	//set the input mesh
							pTransformedPoly2->Delete();
						}
						pTransformedPoly->Delete();
						hull->Delete();
				}	

				pIface->SetOutputMesh(i, wrapper->m_pDeformedMuscle);		//set the output mesh
				//pIface->SetOutputMeshCoarse(i, coarseDeformed);		//set the output mesh

				//set wrappers (skeletons, action lines) for the current muscle surface
				int nWrappers = (int)wrapper->m_Wrappers.size();
				pIface->SetNumberOfMeshSkeletons(i, nWrappers);

				wrappersCount[i] = nWrappers;
				vtkPolyData **semiMuscleWrappers;			
				semiMuscleWrappers = new vtkPolyData*[nWrappers * (stepCount+1)]; // +1 also the ending wrappers have to be saved
				for(int y = 0; y < stepCount+1; y++) { //H
					for (int x= 0; x < nWrappers; x++) { //W
						semiMuscleWrappers[x+y*nWrappers] = vtkPolyData::New();
					}
				}
				semiWrappers[i] = semiMuscleWrappers;

				//typedef double VCoord[3];
				//VCoord* muscleWrapperPoint = new VCoord[nWrappers * (stepCount+1)];			

				double ** muscleWrapperPoint = new double*[nWrappers * (stepCount+1)];
				wrapperPoints[i] = muscleWrapperPoint;

				for (int j = 0; j < nWrappers; j++)
				{
					double pw0[3];
					double pw1[3];
					if (wrapper->m_Wrappers[j]->nRefSysChecksSums[0] != 0 ) {
						pw0[0] = wrapper->m_Wrappers[j]->RefSysOrigin[0][0];
						pw0[1] = wrapper->m_Wrappers[j]->RefSysOrigin[0][1];
						pw0[2] = wrapper->m_Wrappers[j]->RefSysOrigin[0][2];
					}

					if (wrapper->m_Wrappers[j]->nRefSysChecksSums[1] != 0)	{
						pw1[0] = wrapper->m_Wrappers[j]->RefSysOrigin[1][0];
						pw1[1] = wrapper->m_Wrappers[j]->RefSysOrigin[1][1];
						pw1[2] = wrapper->m_Wrappers[j]->RefSysOrigin[1][2];
					}


					//N.B. wrapper->m_Wrappers[j]->pCurves[x] MAY NOT BE NULL!
					if (!bTransform) {
						pIface->SetMeshSkeleton(i, j, 
							wrapper->m_Wrappers[j]->pCurves[0], wrapper->m_Wrappers[j]->pCurves[1], pCorrespondence, 
							(wrapper->m_Wrappers[j]->nRefSysChecksSums[0] != 0 ? wrapper->m_Wrappers[j]->RefSysOrigin[0] : NULL),
							(wrapper->m_Wrappers[j]->nRefSysChecksSums[1] != 0 ? wrapper->m_Wrappers[j]->RefSysOrigin[1] : NULL)
							); 
					}
					else
					{
						vtkPolyData* pTransformedPoly = TransformPolyData(wrapper->m_Wrappers[j]->pCurves[0], &transform); // wrapper transform
						if (wrapper->m_Wrappers[j]->nRefSysChecksSums[0] != 0)
							transform.TransformPoint(wrapper->m_Wrappers[j]->RefSysOrigin[0], pw0); // point transformed to the moved RP


						//vtkPolyData *newStart = semiMuscleWrappers[j];
						semiMuscleWrappers[j]->DeepCopy(pTransformedPoly);

						vtkPolyData *newEnd = vtkPolyData::New();
						newEnd->DeepCopy(wrapper->m_Wrappers[j]->pCurves[1]);

						WrapperInterpoler::MatchCurves(semiMuscleWrappers[j], newEnd);
						double pStart[3];
						transform.TransformPoint(wrapper->m_Wrappers[j]->RefSysOrigin[0], pStart);
						double *pEnd = wrapper->m_Wrappers[j]->RefSysOrigin[1];


						int fn = 0;
						muscleWrapperPoint[j] = new double[3];
						muscleWrapperPoint[j][0] = pw0[0];
						muscleWrapperPoint[j][1] = pw0[1];
						muscleWrapperPoint[j][2] = pw0[2];


						for (fn = 1; fn <= stepCount; fn++) {
							muscleWrapperPoint[fn*nWrappers+j] = new double[3];

							vtkPoints* startPoints = semiMuscleWrappers[j]->GetPoints();
							vtkPoints* endPoints = newEnd->GetPoints();
							int nPoints = startPoints->GetNumberOfPoints();
							vtkPoints* outPoints = vtkPoints::New();
							outPoints->SetNumberOfPoints(nPoints);



							double *sp, *ep, *pw;
							pw = muscleWrapperPoint[fn*nWrappers+j];
							for(int k = 0; k < 3; k++) {
								pw[k] = (pw1[k]-pw0[k])/stepCount + pw1[k];
							}
							for (int l = 0; l < nPoints; l++) // for all points of the mesh surface
							{      
								sp = startPoints->GetPoint(l);
								ep = endPoints->GetPoint(l);

								double x[3];
								// the same point in the moved RP and CP are selected, the distance between them divided into several (= stepCount)
								// parts and a new point position is computed in each step
								for(int k = 0; k < 3; k++) {
									x[k] = sp[k] + (fn)*(ep[k] - sp[k])/stepCount;
								}

								outPoints->SetPoint(l, x); // create a new point position
							}

							vtkPolyData* pNew = semiMuscleWrappers[fn*nWrappers+j];
							pNew->SetPoints(outPoints);
							outPoints->Delete();

							vtkCellArray* outLines = vtkCellArray::New();
							pNew->SetLines(outLines);

							for(int k = 0; k < nPoints-1; k++) {
								vtkIdType ps[2] = {k,k+1};
								outLines->InsertNextCell(2, ps);
							}
							outLines->Delete();
							// interpolated wrappers are precomputed into array (all wrappers of one step in the row, each row for the next interpolation step)
						}
						pTransformedPoly->Delete();
						newEnd->Delete();
					}
				}
			}

			//if we have some hard constraints, such as bones, use them now	
			int nObstacles = 0;
			int nObstacles_RP = 0;
			bool* CoarseObstaclePresent = NULL;
			medVMEMusculoSkeletalModel::mafVMENodeList list_CP;
			medVMEMusculoSkeletalModel::mafVMENodeList list_RP;/*HAJ*/
			vtkPolyData **semiresults;

			if (m_pVMEMusculoSkeletalModel != NULL && m_UsePenetrationPrevention)
			{					
				m_pVMEMusculoSkeletalModel->GetBoneGroups(list_CP, medVMEMusculoSkeletalModel::LOD::Lowest);

				nObstacles = (int)list_CP.size();
				pIface->SetNumberOfObstacles(nObstacles);
				CoarseObstaclePresent = new bool[nObstacles];

				medVMEMusculoSkeletalModel *m_pVMEMusculoSkeletalModel_RP = GetMusculoSkeletalModel_RP();/*HAJ*/
				m_pVMEMusculoSkeletalModel_RP->GetMSMGraph()->GetRegions(list_RP);/*HAJ*/

				m_pVMEMusculoSkeletalModel_RP->GetBoneGroups(list_RP, medVMEMusculoSkeletalModel::LOD::Lowest);
				nObstacles_RP = (int)list_RP.size();

				semiresults = new vtkPolyData*[nObstacles * stepCount];
				for(int y = 0; y < stepCount; y++) { //H
					for (int x= 0; x < nObstacles; x++) { //W
						semiresults[x+y*nObstacles] = vtkPolyData::New();
					}
				}

				for (int i = 0; i < nObstacles; i++)
				{
					//get VTK polydata with bones (of the lowest resolution available - to speed up the processing)

					mafVME* vme_CP = mafVME::SafeDownCast(list_CP[i]); //*
					mafVME* vme_RP;

					for (int j = 0; j < nObstacles_RP; j++) {
						const char* n1 = list_CP[i]->GetName();
						const char* n2 = list_RP[j]->GetName();
						if (strcmp(n1,n2) == 0) {
							vme_RP = mafVME::SafeDownCast(list_RP[j]);
							break;
						}
					}

					vtkPolyData* pTransformedPolyCP = CreateTransformedPolyDataFromVME(vme_CP);	// CP surface	
					vtkPolyData* pTransformedPolyRP1 = CreateTransformedPolyDataFromVME(vme_RP);  // RP surface
					vtkPolyData* pTransformedPolyRP = TransformPolyData(pTransformedPolyRP1, &transform);   // RC moved to the CP pelvis position

					pIface->SetObstacle(i, pTransformedPolyCP);


					vtkPoints* pointsCP = pTransformedPolyCP->GetPoints();
					vtkPoints* pointsRP = pTransformedPolyRP->GetPoints();

					int nCP = pointsCP->GetNumberOfPoints();
					int nRP = pointsRP->GetNumberOfPoints();

					for (int fn=0; fn < stepCount; fn++) { // in each step

						double *cp, *rp;
						vtkPolyData* pNew = semiresults[fn*nObstacles+i];//pTransformedPolyRP->NewInstance();
						pNew->DeepCopy(pTransformedPolyRP);
						vtkPoints* outPoints = pointsRP->NewInstance();
						pNew->SetPoints(outPoints);
						outPoints->Delete();

						outPoints->SetNumberOfPoints(nRP);
						for (int l = 0; l < nRP; l++) // for all points of the mesh surface
						{      
							cp = pointsCP->GetPoint(l);
							rp = pointsRP->GetPoint(l);

							double x[3];
							// the same point in the moved RP and CP are selected, the distance between them divided into several (= stepCount)
							// parts and a new point position is computed in each step
							for(int k = 0; k < 3; k++) {
								x[k] = rp[k] + (fn+1)*(cp[k] - rp[k])/stepCount;
							}

							outPoints->SetPoint(l, x); // create a new point position
						}
						// interpolated mesh surfaces are precomputed into array (all bones of one step in the row, each row for the next interpolation step)
					}

					pTransformedPolyCP->Delete();
					pTransformedPolyRP->Delete();
					pTransformedPolyRP1->Delete();
				}
			}
			/*OTESTUJ!!!*/
			for (int i = 0; i < stepCount; i++) {
				pIface->FreeWrappers();
				for (int j = 0; j < nObstacles; j++) {
					vtkPolyData* pNew = semiresults[i*nObstacles+j];
					pIface->SetObstacle(j, pNew);
				}

				for (int j = 0; j < nMuscles; j++) {

					if (i > 0) {
						medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(m_VMEMuscleWrappers[j]);
						pIface->SetInputMesh(j, wrapper->m_pDeformedMuscle);	//set the input mesh

						pIface->SetCoarseMesh(j, NULL /*coarseDeformed*/);
					}

					vtkPolyData** semiMuscleWrapper = semiWrappers[j];
					int nWrappers = wrappersCount[j];

					for (int k = 0; k < nWrappers; k++) {
						vtkPolyData* newStart = semiWrappers[j][i*nWrappers+k];
						vtkPolyData* newEnd = semiWrappers[j][(i+1)*nWrappers+k];

						pIface->AddWrapper(newStart);
						pIface->AddWrapper(newEnd);

						double *pw0 = wrapperPoints[j][i*nWrappers+k];
						double *pw1 = wrapperPoints[j][(i+1)*nWrappers+k];
						pIface->SetMeshSkeleton(j, k, newStart, newEnd, pCorrespondence, pw0, pw1); 
					}
				}


				pIface->SetUseProgressiveHulls(m_UseProgressiveHulls);
				_VERIFY(pIface->ExecuteMultiData());
			}

			for (int i = 0; i < stepCount; i++) {
				for (int j = 0; j < nObstacles; j++) {
					semiresults[i*nObstacles+j]->Delete();
				}
			}
			for (int j = 0; j < nMuscles; j++) {
				double** muscleWrapperPoint = wrapperPoints[j];
				int nWrappers = wrappersCount[j];
				for (int i = 0; i < stepCount+1; i++) {
					for (int k = 0; k < nWrappers; k++) {
						delete[] muscleWrapperPoint[i*nWrappers+k];
						semiWrappers[j][i*nWrappers+k]->Delete();
					}
				}
				delete[] muscleWrapperPoint;
				delete[] semiWrappers[j];
			}
			delete[] wrapperPoints;
			delete[] semiresults;
			delete[] semiWrappers;
			delete[] wrappersCount;



			//store hulls, if they are available
			for (int i = 0; i < nMuscles; i++) 
			{
				if (!CoarseMeshPresent[i])
				{
					//we may have a new coarse mesh here computed as a by-product
					vtkPolyData* hullData = pIface->GetCoarseMesh(i);
					if (hullData != NULL)
					{
						medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(m_VMEMuscleWrappers[i]);
						if (!bTransform)
							StoreHull(wrapper->GetMuscleVME_RP(), hullData);
						else
						{
							//Hull was computed in RP' position => transform it it RP
							vtkPolyData* pTransformedPoly = TransformPolyData(hullData, &transform_1);
							StoreHull(wrapper->GetMuscleVME_RP(), pTransformedPoly);
							pTransformedPoly->Delete();
						}
					}
					//hullData is destroyed by pIface automatically
				}
			}

			for (int i = 0; i < nObstacles; i++) 
			{
				if (!CoarseObstaclePresent[i])
				{
					//we may have a new coarse mesh here computed as a by-product
					vtkPolyData* hullData = pIface->GetCoarseObstacle(i);
					if (hullData != NULL) {					
						StoreHull(GetLowestResolutionVME(mafVME::SafeDownCast(list_CP[i])), hullData);
					}
					//hullData is destroyed by pIface automatically
				}
			}

			delete[] CoarseMeshPresent;
			delete[] CoarseObstaclePresent;

			pIface->FreeWrappers();
			//let us detach the from output
			for (int i = 0; i < nMuscles; i++) {
				pIface->SetOutputMesh(i, NULL);		//set the output mesh
			}
			for (int i = 0; i < nMuscles; i++) {
				pIface->SetOutputMeshCoarse(i, NULL);		//set the output mesh
			}

			//remove ourself
			m_VMEMuscleWrappers.pop_back();
			pCorrespondence->Delete();
		}
		//	================================================================================
		else
		{
			//m_DeformationMethod == 0, the original mesh skinning technique
			vtkMAFSmartPointer< vtkMEDPolyDataDeformation > pDeformer;

			int nCurves = 0;
			pDeformer->SetNumberOfSkeletons(0);

			//BES: 14.1.2009 - added correspondence to avoid problems when rest pose is 
			//very different from the current pose
			vtkIdList* pCorrespondence = vtkIdList::New();
			pCorrespondence->InsertNextId(0);
			pCorrespondence->InsertNextId(0);

			for (CWrapperItemCollectionIterator pItem = m_Wrappers.begin();
				pItem != m_Wrappers.end(); pItem++)
			{    
				if ((*pItem)->pCurves[0] != NULL && (*pItem)->pCurves[1] != NULL) {
					pDeformer->SetNthSkeleton(nCurves++, 
						(*pItem)->pCurves[0], (*pItem)->pCurves[1], pCorrespondence, 
						((*pItem)->nRefSysChecksSums[0] != 0 ? (*pItem)->RefSysOrigin[0] : NULL),
						((*pItem)->nRefSysChecksSums[1] != 0 ? (*pItem)->RefSysOrigin[1] : NULL)
						);          
				}       
			}

			pCorrespondence->Delete();

			// perform the deformation
			pDeformer->SetInput(m_pTransformedMuscle);
			pDeformer->SetOutput(m_pDeformedMuscle);

			long t = clock();

			pDeformer->Update();

			t = clock() - t;
			std::ofstream file("FILE.txt", ios::out | ios::app);
			file << "Time: " << t << std::endl;
			file.close();

			//disconnect PolyData from its source
			pDeformer->SetOutput(NULL);		
		}

	}

#ifdef _DEBUG_VIS_
	if (m_DefDebugShow != 0)
	{
		//If Debug, visualize data to confirm that it works as it should
		//NB. this takes just one wrapper into account!
		Debug_Visualize_DeformationData();	 
	}
#endif
}

//------------------------------------------------------------------------
//Deforms the transformed muscle using Mass-spring system
//m_pTransformedMuscle -> m_pDeformedMuscle.
void medVMEMuscleWrapper::DeformMuscleMMSS()
	//------------------------------------------------------------------------
{	
	//THIS IS CALLED BY MASTER OR SINGLE ONLY

	//update  or creates the particle system according to the current settings
	InitializeParticleSystem();	//updates m_pParticleSystem


	// get all transformations of bones at the current time stamp 
	// these transformations transforms everything into the common coordinate space ot this wrapper
	std::vector<mafTransform *> transforms;
	this->GetAllBoneCurrentTransforms(m_pParticleSystem->m_BoneGroupsList_CP, transforms);

	//performs initial transformation
	int nBones = (int)m_pParticleSystem->m_MSSbones.size();
#pragma omp for
	for (int i = 0; i < nBones; ++i)
	{
		if (m_pParticleSystem->m_MSSbones[i] != NULL) {	//some bones might be ignored
			m_pParticleSystem->m_MSSbones[i]->Preprocess(transforms[i]);
		}
	}
			
	int nMuscles = (int)m_pParticleSystem->m_MSSmuscles.size();
#pragma omp for
	for (int i = 0; i < nMuscles; i++)
	{
		m_pParticleSystem->m_MSSmuscles[i]->Preprocess(transforms);
	}


#pragma region ITERATIVE MMSS SIMULATION
	float timeWindow = m_IterationStep; // one MSS iterations per step
	// m_IterationNum: the number of iterations (mss iteration + collision detection) needed for stabilization of the system without bones
	if (m_UseFixedTimestep)
	{
		for (int i = 0; i < nMuscles; i++)
			m_pParticleSystem->m_MSSmuscles[i]->GetMSS()->setDt(m_IterationStep);
	}

	for (int i = 0; i < m_IterationNum; i++) // first ignore the bones
	{
		if (m_UseFixedTimestep)
		{
			for (int j = 0; j < nMuscles; j++) // for all muscles, compute the next step of the mass spring system
				m_pParticleSystem->m_MSSmuscles[j]->GetMSS()->NextStep(timeWindow);
		}
		else
		{
			float minDt = FLT_MAX;
			for (int j = 0; j < nMuscles; j++)
			{
				m_pParticleSystem->m_MSSmuscles[j]->GetMSS()->NextStepForces();
				if (m_pParticleSystem->m_MSSmuscles[j]->GetMSS()->getDt() < minDt)
					minDt = m_pParticleSystem->m_MSSmuscles[j]->GetMSS()->getDt();
			}

			for (int j = 0; j < nMuscles; j++) 
			{
				m_pParticleSystem->m_MSSmuscles[j]->GetMSS()->setDt(minDt);
				m_pParticleSystem->m_MSSmuscles[j]->GetMSS()->NextStepPositions();
			}
		}

		for (int j = 0; j < nMuscles; j++) {
			m_pParticleSystem->m_MSSmuscles[j]->ResetCollisions(); // prepare for collision processing
		}

		ProcessCollisions(0, nMuscles, 
#if defined(_MSC_VER) && _MSC_VER >= 1600 
			m_pParticleSystem->m_MSSmuscles.data()
#else
			&m_pParticleSystem->m_MSSmuscles.front()
#endif
			, i);

		for (int j = 0; j < nMuscles; j++)
		{
			// detect and resolve collisions between muscles and bones
			for (int k = 0; k < nBones; k++) {
				if (m_pParticleSystem->m_MSSbones[k] != NULL) {	//some bones might be ignored
					m_pParticleSystem->m_MSSmuscles[j]->CollideWithAnotherObject(m_pParticleSystem->m_MSSbones[k], i);
				}
			}
		}

		for (int j = 0; j < nMuscles; j++)
			m_pParticleSystem->m_MSSmuscles[j]->ProcessCollisions(); // invalidate the collisions
	}	
#pragma endregion

	//store the result into internal structures
	for (int i = 0; i < nMuscles; i++)
	{
		m_pParticleSystem->m_MSSmuscles[i]->Update();	//this is to obtain the surface
		m_pParticleSystem->m_MSSmuscles[i]->SetOutput(NULL);	//wrapper->detach m_pDeformedMuscle

		//update positions
		medVMEMuscleWrapper* wrapper = m_pParticleSystem->m_MuscleWrappers_CP[i];							
		m_pParticleSystem->m_MSSmuscles[i]->GetParticles(wrapper->m_Particles);

		//Now, we need to transform m_pDeformedMuscle mesh and m_Particles from the absolute (common)
		//coordinate space into the local space of the wrapper (unless the wrapper's space is the same)		
		const mafMatrix* inMatrix = medVMEMuscleWrappingHelper::GetVMEAbsMatrix(wrapper);
		wrapper->TransformPoints(wrapper->m_pDeformedMuscle->GetPoints(), inMatrix);
		wrapper->TransformParticles(wrapper->m_Particles, inMatrix);
	}	

	for (int i = 0; i < nBones; i++){
		transforms[i]->Delete();
	}
}

//------------------------------------------------------------------------
//Generates fibers for the given muscle
//m_pDeformedMuscle -> m_pDecomposedMuscle
void medVMEMuscleWrapper::GenerateFibers()
	//------------------------------------------------------------------------
{
	if (m_DecompositionMethod == GF_UFP)
	{
		//Update fibers from the deformed particles directly (Tomas and Yubo)
		UpdateFiberFromParticle();
		UpdateChildrenParticleNode();
		if (m_FbSmooth)		// smooth the fibres if required
		{
			vtkMAFMuscleDecomposition* pMD = vtkMAFMuscleDecomposition::New();
			vtkCellArray *lines = m_pDecomposedMuscle->GetLines();
			vtkPoints *points = m_pDecomposedMuscle->GetPoints();
			vtkIdType npts;
			vtkIdType *pointsIdx;
			lines->InitTraversal();
			vtkMAFMuscleDecomposition::VCoord *pointArray = new vtkMAFMuscleDecomposition::VCoord[m_FbResolution * 2]; // allocate big enough array
			for (int i = 0; i < lines->GetNumberOfCells(); i++) // for each fiber
			{
				lines->GetNextCell(npts, pointsIdx);
				for (int j = 0; j < npts; j++) // store points coordinates of the fiber 
					points->GetPoint(pointsIdx[j], pointArray[j]);
				pMD->SmoothFiber(pointArray, npts, m_FbSmoothWeight, m_FbSmoothSteps);
				for (int j = 0; j < npts; j++) // store new coordinates
					points->SetPoint(pointsIdx[j], pointArray[j]);
			}
			m_pDecomposedMuscle->Update();
			delete[] pointArray;
			pMD->Delete();
		}
#ifdef _FIBRES_LENGTS_OUTPUT_		
		goto Fibres_Lengths;
#endif
		return;
	}

	//make sure that m_pDeformedMuscle is valid
	if (m_pDeformedMuscle->GetPoints() == NULL) // if the muscle was not deformed yet
	{
		if (m_pTransformedMuscle->GetPoints() == NULL)
			PrepareInput();

		m_pDeformedMuscle->DeepCopy(m_pTransformedMuscle); // assume it does not need to be deformed (true for rest pose muscles) => deformed == undeformed
	}


	vtkMAFMuscleFibers* pFibres = NULL;
	switch (m_FbTemplate)
	{
	case FT_PARALLEL: pFibres = vtkMAFParallelMuscleFibers::New(); break;
	case FT_PENNATE: pFibres = vtkMAFPennateMuscleFibers::New(); break;
	case FT_CURVED: pFibres = vtkMAFCurvedMuscleFibers::New(); break;
	case FT_FANNED: pFibres = vtkMAFFannedMuscleFibers::New(); break;
	case FT_RECTUS: pFibres = vtkMAFRectusMuscleFibers::New(); break;
	}

	if (pFibres == NULL)
		return;

	//get the rest-pose muscle
	medVMEMuscleWrapper* pThis = this->GetMuscleWrapper_RP();
	if (pThis == NULL) {
		pThis = this;	//RP not available
	}
	else {
		pThis->PrepareInput();	//make sure, we have transformed RP muscle
	}

	vtkPoints* ori_points = CreateTransformedPointsFromVME(pThis->m_OIVME[0]);
	vtkPoints* ins_points = CreateTransformedPointsFromVME(pThis->m_OIVME[1]);

	//if OI is not specified, ori_points and ins_points must be derived from action llines end-points
	if (ori_points == NULL)
		ori_points = CreateTransformedPointsFromWrappers(pThis->m_Wrappers, true);

	if (ins_points == NULL)
		ori_points = CreateTransformedPointsFromWrappers(pThis->m_Wrappers, false);

#ifdef _DEBUG_VIS_
	int DebugMode = m_FbDebugShowTemplate*vtkMAFMuscleDecomposition::dbgDoNotProjectFibres |
		m_FbDebugShowFitting*vtkMAFMuscleDecomposition::dbgVisualizeFitting |
		m_FbDebugShowFittingRes*vtkMAFMuscleDecomposition::dbgVisualizeFittingResult | 
		m_FbDebugShowSlicing*vtkMAFMuscleDecomposition::dbgVisualizeSlicing |
		m_FbDebugShowSlicingRes*vtkMAFMuscleDecomposition::dbgVisualizeSlicingResult |
		m_FbDebugShowOIConstruction*vtkMAFMuscleDecomposition::dbgVisualizeAttachmentConstruction
#ifdef ADV_KUKACKA
		| m_FbDebugShowHarmFunc*vtkMAFMuscleDecomposition::dbgVisualizeHarmonicField
#endif
		;
#endif

	if (m_DecompositionMethod == GF_MMSS)
	{
		//Mass Spring Method by I. Zeleny (VPHOP)
		vtkMAFMuscleDecompositionMMSS* pMD = vtkMAFMuscleDecompositionMMSS::New();
		pMD->SetInput(m_pDeformedMuscle);
		pMD->SetFibersTemplate(pFibres);
		pMD->SetNumberOfFibres(m_FbNumFib);
		pMD->SetResolution(m_FbResolution);
		pMD->SetOriginArea(ori_points);
		pMD->SetInsertionArea(ins_points);
		pMD->SetSmoothFibers(m_FbSmooth);
		pMD->SetSmoothSteps(m_FbSmoothSteps);
		pMD->SetSmoothFactor(m_FbSmoothWeight);
#ifdef _DEBUG_VIS_
		pMD->SetDebugMode(DebugMode);
#endif

		pMD->SetOutput(m_pDecomposedMuscle);
		pMD->Update();

		pMD->SetOutput(NULL); //disconnect output
		pMD->Delete();
	}
	else
	{
		//Slicing Method (by J. Kohout)
		vtkMAFMuscleDecomposition* pMD = vtkMAFMuscleDecomposition::New();
		pMD->SetInput(m_pDeformedMuscle);
		pMD->SetInputTemplate(pThis->m_pTransformedMuscle);
		pMD->SetFibersTemplate(pFibres);
		pMD->SetNumberOfFibres(m_FbNumFib);
		pMD->SetResolution(m_FbResolution);
		pMD->SetOriginArea(ori_points);
		pMD->SetInsertionArea(ins_points);
		pMD->SetSmoothFibers(m_FbSmooth);
		pMD->SetSmoothSteps(m_FbSmoothSteps);
		pMD->SetSmoothFactor(m_FbSmoothWeight);
		pMD->SetUniformSampling(m_FbUniformSampling);
#ifdef _DEBUG_VIS_
		pMD->SetDebugMode(DebugMode);
#endif

		pMD->SetAdvancedSlicing(m_DecompositionMethod == GF_SLICE_ADV ? 1 : 0);
#ifdef ADV_KUKACKA
		pMD->SetAdvancedKukacka(m_DecompositionMethod == GF_KUKACKA ? 1 : 0);
#endif
		pMD->SetOutput(m_pDecomposedMuscle);
		pMD->Update();

		pMD->SetOutput(NULL); //disconnect output
		pMD->Delete();
	}


	vtkDEL(ori_points);
	vtkDEL(ins_points); 

	pFibres->Delete();

#ifdef _FIBRES_LENGTS_OUTPUT_	
Fibres_Lengths:
	FILE* fProf;
	if (
#if _MSC_VER >= 1400
		0 == (fopen_s(&fProf, 
#else
		NULL != (fProf = fopen(
#endif
		"muscle_wrapping_fibres_legths.txt", "at")))
	{
		m_pDecomposedMuscle->BuildCells();

		int nFibres = m_pDecomposedMuscle->GetNumberOfCells();
		for (int i = 0; i < nFibres; i++)
		{
			vtkIdType nPts, *pPts;
			m_pDecomposedMuscle->GetCellPoints(i, nPts, pPts);

			double dblLen = 0.0;
			for (int j = 1; j < nPts; j++) 
			{
				double A[3], B[3];
				m_pDecomposedMuscle->GetPoint(pPts[j - 1], A);
				m_pDecomposedMuscle->GetPoint(pPts[j], B);
				dblLen += sqrt(vtkMath::Distance2BetweenPoints(A, B));
			}

			_ftprintf(fProf, "%s\t%.3f\t#%d\t%.2f mm\n", 
				this->GetName(), this->GetTimeStamp(), i, dblLen);

		}

		fclose(fProf);
	}
#endif						
}