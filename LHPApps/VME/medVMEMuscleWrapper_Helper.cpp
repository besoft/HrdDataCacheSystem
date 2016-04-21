/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medVMEMuscleWrapper_Helper.cpp,v $
Language:  C++
Date:      $Date: 2012-04-30 14:52:43 $
Version:   $Revision: 1.1.2.16 $
Authors:   Josef Kohout
==========================================================================
Copyright (c) 2008-2012
University of Bedforshire, University of West Bohemia
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "medVMEMuscleWrapper.h"
#include "mafVMESurface.h"
#include "mafVMELandmarkCloud.h"
#include "mafVMELandmark.h"
#include "mafGUI.h"

#include "mmaMaterial.h"
#include "mafTransform.h"

#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkProperty.h"

#include <assert.h>
#include "mafDbg.h"
#include "mafMemDbg.h"
#include "mafDataChecksum.h"

#include "vtkDelaunay3D.h"
#include "vtkCellLocator.h"
#include "vtkMassSpringMuscle.h"
#include "vtkGenericCell.h"

#include <algorithm>

#pragma region CWrapperItem
medVMEMuscleWrapper::CWrapperItem::CWrapperItem() 
{
	memset(this, 0, sizeof(CWrapperItem));

	for (int i = 0; i < 2; i++) {
		pCurves[i] = vtkPolyData::New();		
	}
}

medVMEMuscleWrapper::CWrapperItem::CWrapperItem(const CWrapperItem& src)
{
	memset(this, 0, sizeof(CWrapperItem));
	for (int i = 0; i < 2; i++)
	{
		pVmeRP_CP[i] = src.pVmeRP_CP[i];
		pVmeRefSys_RP_CP[i] = src.pVmeRefSys_RP_CP[i];
		pCurves[i] = vtkPolyData::New();		
	}
}

medVMEMuscleWrapper::CWrapperItem::~CWrapperItem() 
{	
	//release cached data
	vtkDEL(pCurves[0]);
	vtkDEL(pCurves[1]);
}

#pragma endregion

//------------------------------------------------------------------------
//Removes all wrappers from the list (NOT FROM GUI!)
void medVMEMuscleWrapper::DeleteAllWrappers()
	//------------------------------------------------------------------------
{	
	int nCount = (int)m_Wrappers.size();
	for (int i = 0; i < nCount; i++){		
		delete m_Wrappers[i];
	}

	m_Wrappers.clear();
}

//------------------------------------------------------------------------
//Gets the abs matrix for the given VME that can be used to
//transform points from Local Coordinates to World and vice versa
const mafMatrix* medVMEMuscleWrapper::GetVMEAbsMatrix(mafVME* vme)
{
	vme->GetAbsMatrixPipe()->Update();
	return vme->GetAbsMatrixPipe()->GetMatrixPointer();
}


//------------------------------------------------------------------------
//Compute checksum for VTK polydata.
//N.B. pPolyData must be up to date!
unsigned long medVMEMuscleWrapper::ComputeCheckSum(vtkPolyData* pPoly)
	//------------------------------------------------------------------------
{	
	pPoly->Update();	//make sure we have correct data

	vtkDataArray* pDA = pPoly->GetPoints()->GetData();
	return mafDataChecksum::Adler32Checksum((unsigned char*)pDA->GetVoidPointer(0), pDA->GetSize());  
}

//------------------------------------------------------------------------
//Creates a new polydata of vme with the points transformed from
//vme local coordinate system into this coordinate system.
vtkPolyData* medVMEMuscleWrapper::CreateTransformedPolyDataFromVME(mafVME* vme
	//, bool bChangeTimeStamp = false
#ifdef _DEBUG_VIS_
	, const char* dbgtext
#endif	
	)
	//------------------------------------------------------------------------
{
	mafMatrix output_space;
	output_space.DeepCopy(GetVMEAbsMatrix(this));

	vme->GetOutput()->Update();

	vtkPolyData* pPoly = vtkPolyData::SafeDownCast(vme->GetOutput()->GetVTKData());
	pPoly->Update();

#ifdef _DEBUG_VIS_
	//Debug_Write(wxString::Format("%s(%s) - Local", dbgtext, vme->GetName()), pPoly, 100);
#endif

	//and transform it
	vtkPolyData* pTransformedPoly = pPoly->NewInstance();
	vtkPoints* pTrPoints = pPoly->GetPoints()->NewInstance();		
	TransformPoints(pPoly->GetPoints(), pTrPoints, GetVMEAbsMatrix(vme), &output_space);
	pTransformedPoly->ShallowCopy(pPoly);
	pTransformedPoly->SetPoints(pTrPoints);
	pTrPoints->Delete();

#ifdef _DEBUG_VIS_
	//Debug_Write(wxString::Format("%s(%s) - Transformed", dbgtext, vme->GetName()), pPoly, 100);
#endif
	return pTransformedPoly;
}

//------------------------------------------------------------------------
//Creates points form landmark cloud vme, landmark, etc.
//N.B. the caller is responsible for deleting the returned object.
vtkPoints* medVMEMuscleWrapper::CreateTransformedPointsFromVME(mafVME* vme)
	//------------------------------------------------------------------------
{
	vtkPoints* pRet = NULL;

	mafVMELandmarkCloud* cloud = mafVMELandmarkCloud::SafeDownCast(vme);
	if (cloud != NULL)
	{
		int N = cloud->GetNumberOfLandmarks();
		if (N != 0)
		{
			pRet = vtkPoints::New();
			pRet->SetNumberOfPoints(N);
			for (int i = 0; i < N; i++) 
			{
				double x[3];
				cloud->GetLandmarkPosition(i, x);

				pRet->SetPoint(i, x);
			}
		}
	}
	else
	{
		mafVMELandmark* landmark = mafVMELandmark::SafeDownCast(vme);
		if (landmark != NULL)
		{
			double x[3];
			landmark->GetPoint(x);

			pRet = vtkPoints::New();
			pRet->InsertNextPoint(x);
		}
		else if (vme != NULL)
		{
			//general data
			vtkDataSet* ds = vme->GetOutput()->GetVTKData();
			if (ds != NULL)
			{
				int N = ds->GetNumberOfPoints();
				if (N != 0)
				{
					pRet = vtkPoints::New();
					pRet->SetNumberOfPoints(N);
					for (int i = 0; i < N; i++) {
						pRet->SetPoint(i, ds->GetPoint(i));
					}
				}
			}      
		}
	}

	if (pRet != NULL)
	{
		//returned coordinates are local, so we will need to convert them to 
		//absolute (world coordinates) and from them to local coordinates 
		//of our output (corresponds to the coordinate system of input muscle)
		TransformPoints(pRet, GetVMEAbsMatrix(vme));   
	}

	return pRet;
}

//------------------------------------------------------------------------
//Creates points from the first (bStartPoints == true) or the last (bStartPoints == true) vertices of wrappers 
//If bUseCP is true (default) the current pose wrapper is used, otherwise, the rest pose wrapper is used.
//N.B. the caller is responsible for deleting the returned object. 
vtkPoints* medVMEMuscleWrapper::CreateTransformedPointsFromWrappers(CWrapperItemCollection& wrappers, bool bStartPoints, bool bUseCP)
	//------------------------------------------------------------------------
{
	int N = (int)wrappers.size();
	if (N == 0)
		return NULL;

	int iCurve = bUseCP ? 1 : 0;

	vtkPoints* pRet = vtkPoints::New();		
	for (int i = 0; i < N; i++) 
	{
		vtkPolyData* ds = wrappers[i]->pCurves[iCurve];
		if (ds != NULL)
		{
			if (bStartPoints)
				pRet->InsertNextPoint(ds->GetPoint(0));
			else
				pRet->InsertNextPoint(ds->GetPoint(ds->GetNumberOfPoints() - 1));		
		}
	}

	//Curves contain already transformed coordinates, so we are finished
	return pRet;
}

//------------------------------------------------------------------------
//Creates a new polydata without duplicate vertices and edges that might be in the input.
void medVMEMuscleWrapper::FixPolyline(vtkPolyData* input, vtkPolyData* output)
	//------------------------------------------------------------------------
{
	int nInputPoints = input->GetNumberOfPoints();
	int nOutputPoints = 0;

	typedef double VCoord[3];
	VCoord* pPtCoords = new VCoord[nInputPoints];
	vtkIdType* pPtIdMap = new vtkIdType[nInputPoints];

	//we need the same data type
	vtkPoints* out_points = vtkPoints::New(input->GetPoints()->GetDataType());

	//Remove duplicated points (different Id but the same coordinates)
	for (int i = 0; i < nInputPoints; i++)
	{
		input->GetPoint(i, pPtCoords[i]);

		bool bFound = false;
		for (int j = 0; j < i; j++)
		{
			if (
				pPtCoords[j][0] == pPtCoords[i][0] &&
				pPtCoords[j][1] == pPtCoords[i][1] &&
				pPtCoords[j][2] == pPtCoords[i][2]
			)
			{
				pPtIdMap[i] = pPtIdMap[j];
				bFound = true;
				break;
			}
		} //end for

		if (!bFound)
		{
			out_points->InsertNextPoint(pPtCoords[i]);
			pPtIdMap[i] = nOutputPoints;
			nOutputPoints++;
		}
	} //end for i

	delete[] pPtCoords;   //no longer needed

	//Process edges, removing those that are of zero-length
	vtkCellArray* out_lines = vtkCellArray::New();

	input->BuildCells();
	int nInEdges = input->GetNumberOfCells();
	int nOutEdges = 0;

	typedef int Edge[2];	
	for (int i = 0; i < nInEdges; i++)
	{
		vtkIdType nPts, *pPtIds;
		input->GetCellPoints(i, nPts, pPtIds);

		Edge currEdge;
		currEdge[0] = pPtIdMap[pPtIds[0]];
		for (int k = 1; k < nPts; k++)
		{    
			currEdge[1] = pPtIdMap[pPtIds[k]];
			if (currEdge[0] == currEdge[1])
				continue; //invalid edge (of zero length), so skip the point

			//check, if this edge does not already exist

			bool bFound = false;			
			vtkIdType* pPtrCells = out_lines->GetPointer();
			for (int j = 0; j < nOutEdges; j++)
			{
				pPtrCells++;	//skip number of points
				if (
					(currEdge[0] == pPtrCells[0] && currEdge[1] == pPtrCells[1]) ||
					(currEdge[0] == pPtrCells[1] && currEdge[1] == pPtrCells[0])
					)
				{
					bFound = true;
					break;
				}

				pPtrCells += 2;
			}

			if (!bFound) {
				out_lines->InsertNextCell(2, currEdge);				
				nOutEdges++;
			}

			currEdge[0] = currEdge[1];
		} //end for k
	}

	delete[] pPtIdMap;  //no longer needed

	output->SetPoints(out_points);
	output->SetLines(out_lines);

	out_points->Delete();  //no longer needed
	out_lines->Delete();  //no longer needed  
}

//------------------------------------------------------------------------
//Gets the origin of the specified vme.
//Returns false, if the vme is invalid.
bool medVMEMuscleWrapper::GetRefSysVMEOrigin(mafVME* vme, double* origin)
	//------------------------------------------------------------------------
{
	if (vme == NULL || vme->GetOutput() == NULL)
		return false;

	vtkDataSet* ds = vme->GetOutput()->GetVTKData();
	if (ds == NULL)
		return false;

	ds->Update();
	ds->GetCenter(origin);

	//returned coordinates are local, so we will need to convert them to 
	//absolute (world coordinates) and from them to local coordinates 
	//of our output (corresponds to the coordinate system of input muscle)  
	mafTransform transform;  
	transform.SetMatrix(*GetVMEAbsMatrix(vme));  
	transform.TransformPoint(origin, origin);

	transform.SetMatrix(*GetVMEAbsMatrix(this));
	transform.Invert();
	transform.TransformPoint(origin, origin);  
	return true;
}

//------------------------------------------------------------------------
//Transform the given inPoints having inTransform matrix into
//outPoints that have outTransform matrix (i.e., transforms coordinates
//from one reference system into another one.
void medVMEMuscleWrapper::TransformPoints(
	vtkPoints* inPoints, vtkPoints* outPoints, 
	const mafMatrix* inTransform, const mafMatrix* outTransform)
	//------------------------------------------------------------------------
{
	mafTransform transform;
	transform.SetMatrix(*outTransform);  
	transform.Invert();

	transform.Concatenate(*inTransform, 0);	//y = (out-1)*(in*x)

	//BES: 15.11.2012 - check, if it the transformation is identity
	if (IsIdentityTransform(&transform)) {
		if (outPoints != inPoints) {
			outPoints->ShallowCopy(inPoints);
		}
	}
	else
	{		
		double x[3];
		int N = inPoints->GetNumberOfPoints();
		if (inPoints != outPoints) {	//if it is not inplace
			outPoints->SetNumberOfPoints(N);
		}

		for (int i = 0; i < N; i++)
		{      
			transform.TransformPoint(inPoints->GetPoint(i), x);
			outPoints->SetPoint(i, x);
		}
	}
}

//------------------------------------------------------------------------
//Transforms the coordinates of particles inParticles having inTransform matrix 
//into outParticles that  have outTransform matrix (i.e., transforms coordinates
//from one reference system into another one.
void medVMEMuscleWrapper::TransformParticles(const std::vector< CParticle >& inParticles, std::vector< CParticle >& outParticles, 
	const mafMatrix* inTransform, const mafMatrix* outTransform)
{
	_ASSERTE(inParticles.size() == outParticles.size());

	mafTransform transform;
	transform.SetMatrix(*outTransform);  
	transform.Invert();

	transform.Concatenate(*inTransform, 0);	//y = (out-1)*(in*x)	

	//BES: 15.11.2012 - check, if it the transformation is identity
	if (IsIdentityTransform(&transform)) 
	{
		if (&inParticles.front() != &outParticles.front()) 
		{
			//it is not inplace, so we will need to copy it
			int N = (int)inParticles.size();
			for (int i = 0; i < N; i++) {      
				for (int k = 0; k < 3; k++) {
					outParticles[i].position[k] = inParticles[i].position[k];
				}
			}			
		}
	}
	else
	{		
		int N = (int)inParticles.size();				
		for (int i = 0; i < N; i++) {      
			transform.TransformPoint(inParticles[i].position, outParticles[i].position);
		}
	}
}

//------------------------------------------------------------------------
//Returns the instance of medVMEMusculoSkeletalModel under which this VME belongs. 
//May return NULL, if there is no ancestor of medVMEMusculoSkeletalModel kind. 
medVMEMusculoSkeletalModel* medVMEMuscleWrapper::GetMusculoSkeletalModel()
{
	medVMEMusculoSkeletalModel* retPar = NULL;
	mafNode* parent = this->GetParent();

	while (parent != NULL && (retPar = medVMEMusculoSkeletalModel::SafeDownCast(parent)) == NULL)
	{
		parent = parent->GetParent();
	}

	return retPar;
}

//------------------------------------------------------------------------
//Returns the instance of medVMEMusculoSkeletalModel in the rest-pose under which this VME belongs. 
//May return NULL, if there is no ancestor of medVMEMusculoSkeletalModel kind or RP is currently unavailable. 
medVMEMusculoSkeletalModel* medVMEMuscleWrapper::GetMusculoSkeletalModel_RP()
{
	medVMEMusculoSkeletalModel* retPar = NULL;
	mafNode* parent = GetMuscleVME_RP();

	while (parent != NULL && (retPar = medVMEMusculoSkeletalModel::SafeDownCast(parent)) == NULL)
	{
		parent = parent->GetParent();
	}

	return retPar;
}

//------------------------------------------------------------------------
//Returns the instance of lower resolution of the given VME.
//Returns NULL, if there is no lower resolution for the given VME. 
mafVME* medVMEMuscleWrapper::GetLowerResolutionVME(mafVME* vme)
	//------------------------------------------------------------------------
{
	return mafVME::SafeDownCast(vme->GetLink("LowerResVME"));
}

//------------------------------------------------------------------------
//Returns the instance of er resolution of the given VME.
//Returns NULL, if there is no lower resolution for the given VME.
mafVME* medVMEMuscleWrapper::GetHullVME(mafVME* vme)
	//------------------------------------------------------------------------
{
	return mafVME::SafeDownCast(vme->GetLink("HullVME"));
}

//------------------------------------------------------------------------
//Creates a new surface VME to store hull of the given vme. 
//Hull are automatically transformed from this coordinate system
//into the local coordinate system of the target VME. 
void medVMEMuscleWrapper::StoreHull(mafVME* vme, vtkPolyData* hull)
	//------------------------------------------------------------------------
{	
	const char* groupName = "Hulls";

	mafVME* vmeParent = vme->GetParent();	
	int idx = vmeParent->FindNodeIdx(groupName);
	if (idx >= 0) {
		vmeParent = mafVME::SafeDownCast(vmeParent->GetChild(idx));
	}
	else
	{
		//create a new group
		mafVMEGroup* group;
		mafNEW(group);
		group->SetName(groupName);
		group->ReparentTo(vmeParent);
		vmeParent = group;
		mafDEL(group);	//already referenced, so we may decrease our reference
	}

	//vmeParent denotes the group into which we will create a new vme
	mafString outname = vme->GetName();
	outname.Append(" Hull");	
	mafVMESurface* out;
	mafNEW(out);
	out->SetName(outname);
	out->ReparentTo(vmeParent);
	out->GetMaterial()->m_Prop->SetRepresentationToWireframe();

	//hull must be transformed into the local coordinates of vmeParent	
	vtkPolyData* pTransformedPoly = hull->NewInstance();
	vtkPoints* pTrPoints = hull->GetPoints()->NewInstance();		
	TransformPoints(hull->GetPoints(), pTrPoints, GetVMEAbsMatrix(this), GetVMEAbsMatrix(vmeParent));
	pTransformedPoly->ShallowCopy(hull);
	pTransformedPoly->SetPoints(pTrPoints);
	pTrPoints->Delete();

	out->SetDataByReference(pTransformedPoly, 0);
	pTransformedPoly->Delete();

	//and set the link
	vme->SetLink("HullVME", out);
	mafDEL(out);	//we no longer need to store reference to out
}

/** Returns the object containing particles, springs and other data needed for processing of the
mass spring system associated with the given VME.
Returns NULL, if there is none. */
mafVME* medVMEMuscleWrapper::GetMSSParticlesVME(mafVME* vme)
{
	return mafVME::SafeDownCast(vme->GetLink("MSSPartsVME"));
}

/** Creates a new VME containing particles, springs and other data needed for processing of the
mass spring system  of the given vme (muscle). */
void medVMEMuscleWrapper::StoreMSSParticles(mafVME* vme, vtkPolyData* data)
{
	const char* groupName = "MSSData";

	mafVME* vmeParent = vme->GetParent();	
	int idx = vmeParent->FindNodeIdx(groupName);
	if (idx >= 0) {
		vmeParent = mafVME::SafeDownCast(vmeParent->GetChild(idx));
	}
	else
	{
		//create a new group
		mafVMEGroup* group;
		mafNEW(group);
		group->SetName(groupName);
		group->ReparentTo(vmeParent);
		vmeParent = group;
		mafDEL(group);	//already referenced, so we may decrease our reference
	}

	//vmeParent denotes the group into which we will create a new vme
	mafString outname = vme->GetName();
	outname.Append(" MSSParts");	

	mafVMEGeneric* out;
	mafNEW(out);
	out->SetName(outname);
	out->ReparentTo(vmeParent);

	out->SetDataByReference(data, 0);

	//and set the link
	vme->SetLink("MSSPartsVME", out);
	mafDEL(out);	//we no longer need to store reference to out
}

/** Returns the object containing bone as a set of balls.
Returns NULL, if there is none. */
mafVME* medVMEMuscleWrapper::GetMSSBoneVME(mafVME* vme)
{
	return mafVME::SafeDownCast(vme->GetLink("MSSBoneVME"));
}

/** Creates a new VME containing a bone as a set of balls. */
void medVMEMuscleWrapper::StoreMSSBone(mafVME* vme, vtkPolyData* data)
{
	const char* groupName = "MSSBones";

	mafVME* vmeParent = vme->GetParent();	
	int idx = vmeParent->FindNodeIdx(groupName);
	if (idx >= 0) {
		vmeParent = mafVME::SafeDownCast(vmeParent->GetChild(idx));
	}
	else
	{
		//create a new group
		mafVMEGroup* group;
		mafNEW(group);
		group->SetName(groupName);
		group->ReparentTo(vmeParent);
		vmeParent = group;
		mafDEL(group);	//already referenced, so we may decrease our reference
	}

	//vmeParent denotes the group into which we will create a new vme
	mafString outname = vme->GetName();
	outname.Append(" MSSBone");	

	mafVMEGeneric* out;
	mafNEW(out);
	out->SetName(outname);
	out->ReparentTo(vmeParent);

	out->SetDataByReference(data, 0);

	//and set the link
	vme->SetLink("MSSBoneVME", out);
	mafDEL(out);	//we no longer need to store reference to out
}

//------------------------------------------------------------------------
//Gets the transformation that transforms rest-pose pelvis onto current-pose pelvis and
//that can be used to minimize the differences between both poses and improve the stability of method.
//Returns false, if the matrix could not be calculated (e.g., because pelvis was not found in atlas)	
bool medVMEMuscleWrapper::GetInitialTransform( mafTransform* transform )	
{
	//Find both CURRENT-POSE and REST-POSE model
	medVMEMusculoSkeletalModel *modelCP, *modelRP;
	if (NULL == (modelRP = GetMusculoSkeletalModel_RP()))
		return false;

	if (NULL == (modelCP = GetMusculoSkeletalModel()))
		return false;

	//get both pelvis regions
	medMSMGraph::mafVMENodeList listRegionRP, listRegionCP;
	modelRP->GetMSMGraph()->GetRegion(medMSMGraph::RegionType::Pelvis, listRegionRP, true);
	if (listRegionRP.size() == 0)
		return false;

	modelCP->GetMSMGraph()->GetRegion(medMSMGraph::RegionType::Pelvis, listRegionCP, true);
	if (listRegionCP.size() == 0)
		return false;

	//extract VMEs for both pelvis'
	mafVME* pelvisRP = mafVME::SafeDownCast(listRegionRP[0]);
	mafVME* pelvisCP = mafVME::SafeDownCast(listRegionCP[0]);			

	//get transformation matrices for both regions
	const mafMatrix* matRP = this->GetVMEAbsMatrix(pelvisRP);
	const mafMatrix* matCP = this->GetVMEAbsMatrix(pelvisCP);			

	double posRP[3], posCP[3];
	mafTransform::GetPosition(*matRP, posRP);
	mafTransform::GetPosition(*matCP, posCP);

	double oriRP[3], oriCP[3];
	mafTransform::GetOrientation(*matRP, oriRP);
	mafTransform::GetOrientation(*matCP, oriCP);

	//OK, we must build a transformation
	double delta_pos[3], delta_ori[3];
	for (int i = 0; i < 3; i++){
		delta_pos[i] = posCP[i] - posRP[i];
		delta_ori[i] = oriCP[i] - oriRP[i];
	}

	transform->SetPosition(delta_pos);
	transform->SetOrientation(delta_ori);	

	return true;
}

//------------------------------------------------------------------------
//Returns true, if the given transformation is identity, i.e., it does not change position of vertices being transformed by this transformation.
bool medVMEMuscleWrapper::IsIdentityTransform(const mafTransform* transform)
	//------------------------------------------------------------------------
{
	double x_in[3] = {1.0, 2.0, 4.0}, x_out[3];
	(const_cast< mafTransform* >(transform))->TransformPoint(x_in, x_out);

	for (int k = 0; k < 3; k++) {
		if (x_in[k] != x_out[k]) {
			return false;
		}
	}

	return true;
}

//------------------------------------------------------------------------
//Creates a new polydata with points transformed by the given transformation.
//N.B., the caller is responsible for deleting the returned object.
vtkPolyData* medVMEMuscleWrapper::TransformPolyData(vtkPolyData* poly, const mafTransform* transform)
{
	vtkPolyData* pRet = poly->NewInstance();
	pRet->ShallowCopy(poly);

	//BES: 15.11.2012 - check, if it the transformation is identity
	if (IsIdentityTransform(transform))
		return pRet;	//we are ready
	

	vtkPoints* inPoints = poly->GetPoints();
	vtkPoints* outPoints = inPoints->NewInstance();
	pRet->SetPoints(outPoints);
	outPoints->Delete();	//BES: 3.5.2012 - delete it to avoid memory leaks

	double x[3];
	int N = inPoints->GetNumberOfPoints();
	outPoints->SetNumberOfPoints(N);
	for (int i = 0; i < N; i++)
	{      
		(const_cast< mafTransform* >(transform))->TransformPoint(inPoints->GetPoint(i), x);
		outPoints->SetPoint(i, x);
	}

	return pRet;
}

//------------------------------------------------------------------------
//Calculates distance between two particles
double medVMEMuscleWrapper::CalculateDistance(double posA[3], double posB[3])
{
	double dx = posB[0] - posA[0];
	double dy = posB[1] - posA[1];
	double dz = posB[2] - posA[2];
	return sqrt(dx * dx + dy * dy + dz * dz);
}

//------------------------------------------------------------------------
//Adds particles and remove duplicated particles
int medVMEMuscleWrapper::AddParticle(int fiberID, double position[3], bool checkDuplicated)
{
	// check whether duplicated particles
	if(checkDuplicated) {
		for(size_t i = 0; i < m_Particles.size(); ++i) {
			if(position[0] == m_Particles[i].position[0] && position[1] == m_Particles[i].position[1] && position[2] == m_Particles[i].position[2])
				return i;
		}
	}
	m_Particles.push_back(CParticle(fiberID, position));
	return m_Particles.size() - 1;
}

//------------------------------------------------------------------------
//Finds all vertices connected with a given (seed) vertex
void medVMEMuscleWrapper::GetConnectedVertices(vtkUnstructuredGrid * mesh, int seed, vtkIdList *connectedVertices)
{  
	//get all cells that vertex 'seed' is a part of
	vtkIdList * cellIdList = vtkIdList::New();
	mesh->GetPointCells(seed, cellIdList);  

	//loop through all the cells that use the seed point
	for(vtkIdType i = 0; i < cellIdList->GetNumberOfIds(); i++)
	{
		vtkCell* cell = mesh->GetCell(cellIdList->GetId(i));

		//if we get to here, the cell is a polygon, so extract its border edges
		for(int e = 0; e < cell->GetNumberOfEdges(); e++)
		{
			vtkCell* edge = cell->GetEdge(e);

			vtkIdList* pointIdList = edge->GetPointIds();

			if(pointIdList->GetId(0) == seed)
			{
				connectedVertices->InsertUniqueId(pointIdList->GetId(1));
			}
			else if(pointIdList->GetId(1) == seed)
			{
				connectedVertices->InsertUniqueId(pointIdList->GetId(0));
			}
		}

	}
	cellIdList->Delete();
}

//------------------------------------------------------------------------
//Creates neighbourhood between individual particles (to be springs) as 
//edges of delaunay tetrahedronization of the particles.
void medVMEMuscleWrapper::GenerateNeighbourParticlesDelaunay()
{
	vtkDelaunay3D *del = vtkDelaunay3D::New();
	vtkPolyData *partsData = vtkPolyData::New();
	partsData->Initialize();
	vtkPoints *partsPoints = vtkPoints::New();
	partsPoints->Initialize();

	// copy the particle centers into the Del3D class as vertices of the future tetrahedronization
	double point[3];
	for (int i = 0; i < m_Particles.size(); i++)
	{
		point[0] = m_Particles[i].position[0];
		point[1] = m_Particles[i].position[1];
		point[2] = m_Particles[i].position[2];
		partsPoints->InsertNextPoint(point);
	}
	partsData->SetPoints(partsPoints);
	partsData->Update();
	del->SetInput(partsData);
	del->Update();

	vtkUnstructuredGrid *result = del->GetOutput();
	vtkIdList *connectedVertices = vtkIdList::New();
	// set the neighbors
	for (int i = 0; i <  m_Particles.size(); i++)
	{
		GetConnectedVertices(result, i, connectedVertices); // feth edges of the given particle
		for(vtkIdType id = 0; id < connectedVertices->GetNumberOfIds(); id++)
			m_Particles[i].AddNeighbor(connectedVertices->GetId(id));
		connectedVertices->Reset();
	}

	del->Delete();
	partsData->Delete();
	partsPoints->Delete();
	connectedVertices->Delete();
}

//------------------------------------------------------------------------
//Creates neighbourhood between individual particles (to be springs) by 
//connecting each particle with m_NumClosestParticles closest other particles.
void medVMEMuscleWrapper::GenerateNeighbourParticlesClosest()
{
	int *connections = new int[m_PClosestParticleNum]; // stores the indices of the closest particles
	float *distances = new float[m_PClosestParticleNum]; // stores the distances to the closest particles
	int *valences = new int[m_Particles.size()]; // stores how many connections a given particle have
	for (int i = 0; i < m_Particles.size(); i++)
		valences[i] = 0;

	for (int i = 0; i < m_Particles.size(); i++) // for each particle find m_NumClosestParticles closest particles
	{
		if (valences[i] == m_PClosestParticleNum) continue; // the particle could have already been the closest to m_NumClosestParticles particles
		for (int k = valences[i]; k < m_PClosestParticleNum; k++)
		{
			distances[k] = FLT_MAX;
			connections[k] = -1;
		}
		for (int j = i + 1; j < m_Particles.size(); j++) // first create temporary connections, i.e. do not increment valence
		{
			if (valences[j] == m_PClosestParticleNum) continue; // if the j-th particle is already at full valence, skip it, it coudl not be used anyway

			// find the so far worst (longest) temporary connection
			float maxDistance = FLT_MIN;
			int maxDistIndex = valences[i];
			for (int k = valences[i]; k < m_PClosestParticleNum; k++) // starts from valences[i] - the previously made connections are permanent, so do not rewrite them
			{
				if (distances[k] > maxDistance)
				{
					maxDistance = distances[k];
					maxDistIndex = k;
				}
			}
			double distance = (m_Particles[i].position[0] - m_Particles[j].position[0]) * (m_Particles[i].position[0] - m_Particles[j].position[0])
				+ (m_Particles[i].position[1] - m_Particles[j].position[1]) * (m_Particles[i].position[1] - m_Particles[j].position[1])
				+ (m_Particles[i].position[2] - m_Particles[j].position[2]) * (m_Particles[i].position[2] - m_Particles[j].position[2]);
			// if the longest connection is longer than the possible new one, replace it
			if (distance < maxDistance) // if some previous temporary connection was more distant, it was now replaced
			{
				connections[maxDistIndex] = j;
				distances[maxDistIndex] = distance;
			}
		}

		for (int k = valences[i]; k < m_PClosestParticleNum; k++) // write the neighbourhood information
		{
			// if a temporary connection was made at this index (if not, the particle will end up with less than m_NumClosestParticles neighbours - possible in some cases)
			if (connections[k] >= 0)
			{
				valences[connections[k]]++; // increase the other particle's valence in order to mark that a neighbour has been added to it
				// write each other as neighbours
				m_Particles[i].AddNeighbor(connections[k]);
				m_Particles[connections[k]].AddNeighbor(i);
			}
			else break;
		}
		valences[i] = m_PClosestParticleNum; // mark it as at full valence. even if it is not, it surely will not be filled in next iterations - if it could, it would already
	}

	delete []valences;
	delete []distances;
	delete []connections;
}

//------------------------------------------------------------------------
//Creates neighbourhood between individual particles (to be springs).
//Assumes the particls form a cubical grid. The neighbours are set as edges (and diagonals) of the grid. 
//Setting of m_SpringLayoutType decides how many edges/diagonals are used.
void medVMEMuscleWrapper::GenerateNeighbourParticlesCubicLattice()
{
	int sqrtN = (int)sqrt((double)m_FbNumFib);
	for(int i = 0; i < sqrtN; ++i) {
		for(int j = 0; j < sqrtN; ++j) {
			int offset = (m_FbResolution + 1) * (i * sqrtN + j);
			for(int k = 0; k <= m_FbResolution; ++k) {
				// 6-model
				int index = offset + k;
				int mapID = m_ParticleMap[index];
				CParticle& particle = m_Particles[mapID];
				// up
				int up = index + 1;
				if(k < m_FbResolution && m_ParticleMap[up] != mapID) particle.AddNeighbor(m_ParticleMap[up]);
				// down
				int down = index - 1;
				if(k > 0 && m_ParticleMap[down] != mapID) particle.AddNeighbor(m_ParticleMap[down]);
				// left
				int left = (m_FbResolution + 1) * ((i - 1) * sqrtN + j) + k;
				if(i > 0 && m_ParticleMap[left] != mapID) particle.AddNeighbor(m_ParticleMap[left]);
				// right
				int right = (m_FbResolution + 1) * ((i + 1) * sqrtN + j) + k;
				if(i + 1 < sqrtN && m_ParticleMap[right] != mapID) particle.AddNeighbor(m_ParticleMap[right]);
				// forward
				int forward = (m_FbResolution + 1) * (i * sqrtN + j + 1) + k;
				if(j + 1 < sqrtN && m_ParticleMap[forward] != mapID) particle.AddNeighbor(m_ParticleMap[forward]);
				// backward
				int backward = (m_FbResolution + 1) * (i * sqrtN + j - 1) + k;
				if(j > 0 && m_ParticleMap[backward] != mapID) particle.AddNeighbor(m_ParticleMap[backward]);
				if (m_PSpringLayoutType == 1) // 26-model
				{
					if (j + 1 < sqrtN) // forward => + (m_FbResolution + 1) everywhere
					{
						if (i > 0) // left
						{
							int forwardUpLeft = left + 1 + (m_FbResolution + 1); // go to left particle, one particle up, and one fibre forward
							if (k < m_FbResolution && m_ParticleMap[forwardUpLeft] != mapID) particle.AddNeighbor(m_ParticleMap[forwardUpLeft]);
							int forwardLeft = left + (m_FbResolution + 1);
							if (m_ParticleMap[forwardLeft] != mapID) particle.AddNeighbor(m_ParticleMap[forwardLeft]);
							int forwardDownLeft = left - 1 + (m_FbResolution + 1);
							if (k > 0 && m_ParticleMap[forwardDownLeft] != mapID) particle.AddNeighbor(m_ParticleMap[forwardDownLeft]);
						}
						if (i + 1 < sqrtN) // right
						{
							int forwardUpRight = right + 1 + (m_FbResolution + 1);
							if (k < m_FbResolution && m_ParticleMap[forwardUpRight] != mapID) particle.AddNeighbor(m_ParticleMap[forwardUpRight]);
							int forwardRight = right + (m_FbResolution + 1);
							if (m_ParticleMap[forwardRight] != mapID) particle.AddNeighbor(m_ParticleMap[forwardRight]);
							int forwardDownRight = right - 1 + (m_FbResolution + 1);
							if (k > 0 && m_ParticleMap[forwardDownRight] != mapID) particle.AddNeighbor(m_ParticleMap[forwardDownRight]);
						}				
						// middle
						int forwardUp = forward + 1;// one particle up, and one fibre forward
						if (k < m_FbResolution && m_ParticleMap[forwardUp] != mapID) particle.AddNeighbor(m_ParticleMap[forwardUp]);						
						int forwardDown = forward - 1;// one particle up, and one fibre forward
						if (k > 0 && m_ParticleMap[forwardDown] != mapID) particle.AddNeighbor(m_ParticleMap[forwardDown]);
					}
					if (j > 0) // backward => -(m_FbResolution + 1) everywhere
					{
						if (i > 0) // left
						{
							int backwardUpLeft = left + 1 - (m_FbResolution + 1); // go to left particle, one particle up, and one fibre backward
							if (k < m_FbResolution && m_ParticleMap[backwardUpLeft] != mapID) particle.AddNeighbor(m_ParticleMap[backwardUpLeft]);
							int backwardLeft = left - (m_FbResolution + 1);
							if (m_ParticleMap[backwardLeft] != mapID) particle.AddNeighbor(m_ParticleMap[backwardLeft]);
							int backwardDownLeft = left - 1 - (m_FbResolution + 1);
							if (k > 0 && m_ParticleMap[backwardDownLeft] != mapID) particle.AddNeighbor(m_ParticleMap[backwardDownLeft]);
						}
						if (i + 1 < sqrtN) // right
						{
							int backwardUpRight = right + 1 - (m_FbResolution + 1);
							if (k < m_FbResolution && m_ParticleMap[backwardUpRight] != mapID) particle.AddNeighbor(m_ParticleMap[backwardUpRight]);
							int backwardRight = right - (m_FbResolution + 1);
							if (m_ParticleMap[backwardRight] != mapID) particle.AddNeighbor(m_ParticleMap[backwardRight]);
							int backwardDownRight = right - 1 - (m_FbResolution + 1);
							if (k > 0 && m_ParticleMap[backwardDownRight] != mapID) particle.AddNeighbor(m_ParticleMap[backwardDownRight]);
						}				
						// middle
						int backwardUp = backward + 1;// one particle up, and one fibre backward
						if (k < m_FbResolution && m_ParticleMap[backwardUp] != mapID) particle.AddNeighbor(m_ParticleMap[backwardUp]);						
						int backwardDown = backward - 1;// one particle up, and one fibre backward
						if (k > 0 && m_ParticleMap[backwardDown] != mapID) particle.AddNeighbor(m_ParticleMap[backwardDown]);
					}
					// remaining left and right
					if (i > 0) // left
					{
						int upLeft = left + 1; // go to left particle, one particle up, and one fibre backward
						if (k < m_FbResolution && m_ParticleMap[upLeft] != mapID) particle.AddNeighbor(m_ParticleMap[upLeft]);
						int downLeft = left - 1;
						if (k > 0 && m_ParticleMap[downLeft] != mapID) particle.AddNeighbor(m_ParticleMap[downLeft]);
					}
					if (i + 1 < sqrtN) // right
					{
						int upRight = right + 1;
						if (k < m_FbResolution && m_ParticleMap[upRight] != mapID) particle.AddNeighbor(m_ParticleMap[upRight]);
						int downRight = right - 1;
						if (k > 0 && m_ParticleMap[downRight] != mapID) particle.AddNeighbor(m_ParticleMap[downRight]);
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------
//Initializes the particle system for the rest-pose according to the current settings of the wrapper. 
//The system is created for the muscle being wrapped by this wrapper using the passed
//parameters: boneGroupsList_RP contains an array of all bones in the model, idxBones 
//contains indices (zero-based) into boneGroupsList_RP determining the bones being affected
//by the muscle being wrapped.
//N.B. This method is supposed to be called from a rest-pose wrapper only!
void medVMEMuscleWrapper::InitializeParticleSystem_RP(
	const medMSMGraph::mafVMENodeList& boneGroupsList_RP, const std::vector< int>& idxBones)
//------------------------------------------------------------------------
{
	//BES 30.6.2012 - make sure that we have the correct settings because otherwise the method will crash
	//correct settings are: uniform lattice and number of fibers equal to m*m, where m is an integer
	bool bNeedGenerateFibres = false;
	if (m_FbUniformSampling == 0) 
	{
		mafLogMessage("Uniform sampling enabled for rest-pose muscle wrapper #%d (%s)", this->GetId(), this->GetName());
		this->m_FbUniformSampling = 1;
		bNeedGenerateFibres = true;
	}

	int sqrtN = (int)sqrt((double)this->m_FbNumFib);
	sqrtN = sqrtN*sqrtN;

	if (sqrtN != this->m_FbNumFib)
	{
		mafLogMessage("Number of fibres adjusted for rest-pose muscle wrapper #%d (%s)", this->GetId(), this->GetName());
		this->m_FbNumFib = sqrtN;
		bNeedGenerateFibres = true;
	}

	if (bNeedGenerateFibres || (this->m_pDecomposedMuscle == NULL || m_pDecomposedMuscle->GetNumberOfLines() != m_FbNumFib))
	{
		//update fibres
		m_Modified |= FIBERS_OPTIONS_MODIFIED;
		this->GetOutput()->Update();		

		//if fibres are not to be displayed, they may be still not generated
		if ((m_Modified & FIBERS_OPTIONS_MODIFIED) != 0) 
		{	
			//thus, generate it internally
			this->GenerateFibers();
			m_Modified &= ~FIBERS_OPTIONS_MODIFIED;
		}		
		
		//decomposition method could change the  number of fibres, do the last checkings
		sqrtN = (int)sqrt((double)this->m_pDecomposedMuscle->GetNumberOfLines());
		sqrtN = sqrtN*sqrtN;

		if (sqrtN != this->m_FbNumFib && sqrtN == this->m_pDecomposedMuscle->GetNumberOfLines()) {
			this->m_FbNumFib = sqrtN; bNeedGenerateFibres = true;
		}		

		//if any settings has changed, we should update GUI
		if (bNeedGenerateFibres) {
			if (m_Gui != NULL) {
				m_Gui->TransferDataToWindow();
			}
		}
	}
	
	//generate particles from the fibres
	this->GenerateParticles();

	// generate neighborhood information for particles
	this->GenerateParticleNeighbors();

	// generate constraints for particles on the boundary of fibers
	this->GenerateParticleConstraints(boneGroupsList_RP, idxBones);
	
	// regenerate fibers if the vertex number is not the same as the particle number
	this->RegenerateFiberFromParticle();
}

//------------------------------------------------------------------------
//Generates particles from the current fibers
void medVMEMuscleWrapper::GenerateParticles()
{
	int sqrtN = (int)sqrt((double)m_FbNumFib);
	if (!m_FbUniformSampling || sqrtN * sqrtN != m_FbNumFib || m_pDecomposedMuscle->GetNumberOfLines() != m_FbNumFib) {
		mafLogMessage("Unexpected Error in medVMEMuscleWrapper::GenerateParticles()");
		return;
	}

	int npts = 0;
	int *pts = 0;
	int fiberID = 0;
	m_Particles.clear();
	m_ParticleMap.clear();    // from the sampled particles on the fiber to the particles in m_Particles (removed duplicated particles)
	double pos[3], posA[3], posB[3];

	vtkCellArray *lineArray = m_pDecomposedMuscle->GetLines();
	lineArray->InitTraversal();
	while(lineArray->GetNextCell(npts, pts)) {
		// calculate the length of the fiber
		double totalLength = 0;
		m_pDecomposedMuscle->GetPoint(pts[0], posA);
		for(int i = 1; i < npts; ++i) {
			m_pDecomposedMuscle->GetPoint(pts[i], posB);
			totalLength += CalculateDistance(posA, posB);
			posA[0] = posB[0];
			posA[1] = posB[1];
			posA[2] = posB[2];
		}

		// generate particles along the fiber with equal distances
		int particleID = 1;
		double dL = totalLength / (m_FbResolution - 1); // assume length of the first line segment can be 0 (if rand generates 0) - count as if there was one less segment
		double nextLength = ((double)rand() / RAND_MAX) * dL; // randomize particle positions to scatter them - more precise collision detection
		double accuLength = 0;
		m_pDecomposedMuscle->GetPoint(pts[0], posA);
		m_ParticleMap.push_back(AddParticle(fiberID, posA, true));
		for(int i = 1; i < npts; ++i) {
			m_pDecomposedMuscle->GetPoint(pts[i], posB);
			double length = CalculateDistance(posA, posB);
			while(accuLength + length > nextLength && particleID < m_FbResolution) {
				double lamda = (nextLength - accuLength) / length;
				pos[0] = posA[0] * (1 - lamda) + posB[0] * lamda;
				pos[1] = posA[1] * (1 - lamda) + posB[1] * lamda;
				pos[2] = posA[2] * (1 - lamda) + posB[2] * lamda;
				m_ParticleMap.push_back(AddParticle(fiberID, pos));
				particleID++;
				nextLength += dL;
			}
			accuLength += length;
			posA[0] = posB[0];
			posA[1] = posB[1];
			posA[2] = posB[2];
		}
		m_ParticleMap.push_back(AddParticle(fiberID, posB, true));

		// set up links between particles of the same fiber
		int index = (m_FbResolution + 1) * fiberID;
		m_Particles[m_ParticleMap[index]].nextParticle = m_ParticleMap[index + 1];
		for(int i = 1; i < m_FbResolution; ++i) {
			m_Particles[m_ParticleMap[index + i]].prevParticle = m_ParticleMap[index + i - 1];
			m_Particles[m_ParticleMap[index + i]].nextParticle = m_ParticleMap[index + i + 1];
		}
		m_Particles[m_ParticleMap[index + m_FbResolution]].prevParticle = m_ParticleMap[index + m_FbResolution - 1];
		// set the first and last particle as fixed
		m_Particles[m_ParticleMap[index]].fixed = true;
		m_Particles[m_ParticleMap[index + m_FbResolution]].fixed = true;

		// next fiber
		++fiberID;
	}

	//????? what these two lines do ????
	std::vector<CParticle>(m_Particles).swap(m_Particles);
	std::vector<int>(m_ParticleMap).swap(m_ParticleMap);
}

//------------------------------------------------------------------------
// Generates neighbors of particles
void medVMEMuscleWrapper::GenerateParticleNeighbors()
{
	if (m_PSpringLayoutType == 2) // delaunay tetrahedronization
		GenerateNeighbourParticlesDelaunay();
	else if (m_PSpringLayoutType == 3) // N nearest neighbours
		GenerateNeighbourParticlesClosest();
	else
		GenerateNeighbourParticlesCubicLattice(); // cube lattice N-models, where N is number of neighbours per particle

}

//------------------------------------------------------------------------
// Generates contraints for particles
//Parameters: boneGroupsList_RP contains an array of all bones in the model, idxBones 
//contains indices (zero-based) into boneGroupsList_RP determining the bones being affected
//by the muscle being wraped, noOfBones specifies the number of these bones, i.e., the number
//of entries in the array idxBones.
void medVMEMuscleWrapper::GenerateParticleConstraints(
	const medMSMGraph::mafVMENodeList& boneGroupsList_RP, const std::vector< int >& idxBones)
{	
	// get all bone polydata surfaces
	int nBones = (int)idxBones.size();
	std::vector<vtkPolyData*> bones;
	
	for (int i = 0; i < nBones; ++i) 
	{	
		//extract the polydata surface meshes from the given bones 
		//and transforms their vertices into the common coordinate system (of this wrapper)
		//in which we have already constructed muscle fibres and thus also particles
		mafVMESurface* pSurface = mafVMESurface::SafeDownCast(boneGroupsList_RP[idxBones[i]]);			
		vtkPolyData *pPoly = CreateTransformedPolyDataFromVME(pSurface);
		bones.push_back(pPoly);	
	}

	//create locators to fast detection of mutual positions between particles and surfaces
	vtkGenericCell* dummyCell = vtkGenericCell::New();
	vtkCellLocator** locators = new vtkCellLocator*[nBones];
	for (int i = 0; i < nBones; i++)
	{		
		locators[i] = vtkCellLocator::New();		
		locators[i]->SetDataSet(bones[i]);
		locators[i]->Update();
	}

	// check each particle on the boundary with bones
	int sqrtN = (int)sqrtf(m_FbNumFib);
	for(int i = 0; i < sqrtN; ++i) {
		for(int j = 0; j < sqrtN; ++j) {
			int offset = (m_FbResolution + 1) * (i * sqrtN + j);
			for(int k = 0; k <= m_FbResolution; ++k) {
				int index = offset + k;
				int mapID = m_ParticleMap[index];
				CParticle& particle = m_Particles[mapID];

				double minDistance1 = DBL_MAX, minDistance2 = DBL_MAX, dist;
				double closestPoint[3];
				int cellId, subId, boneId1 = -1, boneId2 = -1;
				for (int q = 0; q < nBones; q++)
				{
					locators[q]->FindClosestPoint(particle.position, closestPoint, dummyCell, cellId, subId, dist);
					if (dist < minDistance1)
					{
						minDistance2 = minDistance1;
						minDistance1 = dist;
						boneId2 = boneId1;
						boneId1 = q;
					}
					else if (dist < minDistance2)
					{
						minDistance2 = dist;
						boneId2 = q;
					}
				}

				//boneId1 and boneId2 contains indices to bones array
				//convert it to global indices to boneGroupsList_RP
				particle.boneID[0] = idxBones[boneId1];
				if (boneId2 != -1) 
				{
					particle.boneID[1]  = idxBones[boneId2];

					//calculate non-linear weight (dist is square distance)
					particle.boneWeight = minDistance2 / (minDistance1 + minDistance2);
				} 
				else 
				{
					particle.boneID[1]  = -1;
					particle.boneWeight = 1;
				}

				if(k == 0 || k == m_FbResolution || i == 0 || i == sqrtN - 1 || j == 0 || j == sqrtN - 1) {// particles on the boundary of fibers
					particle.boundary = true;
					// particle.fixed = (distance < m_PConstraintThreshold); // fix only first and last now
				}
				else {
					particle.boundary = false;
					particle.fixed = false;
				}
			}
		}
	}

	if (m_ParticleMap[0] == m_ParticleMap[m_FbResolution + 1])
		m_Particles[m_ParticleMap[0]].fixed = true;
	if (m_ParticleMap[m_FbResolution] == m_ParticleMap[m_FbResolution * 2 + 1])
		m_Particles[m_ParticleMap[m_FbResolution]].fixed = true;

	//clean up	
	for (int i = 0; i < nBones; i++)
	{		
		locators[i]->Delete();
		bones[i]->Delete();
	}

	delete[] locators;
	dummyCell->Delete();

#if defined(_MSC_VER) && defined(_DEBUG)
	//debug info
	std::set<int> boneIDs;
	for(int i = 0; i < m_Particles.size(); ++i) {
		boneIDs.insert(m_Particles[i].boneID[0]);
		boneIDs.insert(m_Particles[i].boneID[1]);
	}

	_RPT0(_CRT_WARN, "Associated Bones: ");
	for(std::set<int>::iterator iter = boneIDs.begin(); iter != boneIDs.end(); ++iter)
		_RPT1(_CRT_WARN, "%d ", *iter);
	_RPT0(_CRT_WARN, "\n");
#endif
}

//------------------------------------------------------------------------
// Get the muscle wrapper VME at the rest position
medVMEMuscleWrapper* medVMEMuscleWrapper::GetMuscleWrapper_RP()
{
	medVMEMusculoSkeletalModel* modelRP = GetMusculoSkeletalModel_RP();
	if (modelRP == NULL)	//if the wrapper is not a part of MM, RP does not exist
		return NULL;

	medVMEMusculoSkeletalModel::mafVMENodeList wrappersRP;
	modelRP->GetMSMGraph()->GetMuscleWrappers(wrappersRP, true); // load the rest pose of active wrappers
	medVMEMuscleWrapper *wrapperRP = NULL;
	for (int j = 0; j < wrappersRP.size(); j++) // find the rest pose muscle wrapper of current wrapper
	{
		medVMEMuscleWrapper *wrapper = medVMEMuscleWrapper::SafeDownCast(wrappersRP[j]);
		if (wrapper->GetMuscleVME_RP() == GetMuscleVME_RP()) {
			wrapperRP = wrapper;	
			break;
		}
	}
	return wrapperRP;
}

//------------------------------------------------------------------------
//Gets the absolute transform matrices of all bones.	
//The method returns the transformation that would transform the rest-pose into the current-pose (in absolute coordinates).
//N.B. the caller is responsible for calling Delete method upon every returned item
void medVMEMuscleWrapper::GetAllBoneCurrentTransforms(
		const medMSMGraph::mafVMENodeList& bonesCP, std::vector< mafTransform* >& transforms)
//------------------------------------------------------------------------
{
	int nBones = (int)bonesCP.size();
	for (int i = 0; i < nBones; ++i) 
	{		
		mafVMESurface* pSurface = mafVMESurface::SafeDownCast(bonesCP[i]);
		pSurface->GetOutput()->Update();

		mafTransform* pTransform = mafTransform::New();
		pTransform->SetMatrix(GetOutput()->GetAbsTransform()->GetMatrix());
		pTransform->Invert();

		pTransform->Concatenate(pSurface->GetOutput()->GetAbsTransform()->GetMatrix(), 0);	//y = (out-1)*(in*x)		
		transforms.push_back(pTransform);		
	}
}

//------------------------------------------------------------------------
// Get the bounding box of particles
void medVMEMuscleWrapper::GetBoundingBoxOfParticles(double bounds[6])
{
	bounds[0] = bounds[1] = m_Particles[0].position[0];
	bounds[2] = bounds[3] = m_Particles[0].position[1];
	bounds[4] = bounds[5] = m_Particles[0].position[2];

	for(size_t i = 1; i < m_Particles.size(); ++i) {
		if(m_Particles[i].position[0] < bounds[0])
			bounds[0] = m_Particles[i].position[0];
		if(m_Particles[i].position[0] > bounds[1])
			bounds[1] = m_Particles[i].position[0];

		if(m_Particles[i].position[1] < bounds[2])
			bounds[2] = m_Particles[i].position[1];
		if(m_Particles[i].position[1] > bounds[3])
			bounds[3] = m_Particles[i].position[1];

		if(m_Particles[i].position[2] < bounds[4])
			bounds[4] = m_Particles[i].position[2];
		if(m_Particles[i].position[2] > bounds[5])
			bounds[5] = m_Particles[i].position[2];
	}
}

//------------------------------------------------------------------------
//Regenerate fibers from the sampled particles */
void medVMEMuscleWrapper::RegenerateFiberFromParticle()
{
	int numPoint = m_pDecomposedMuscle->GetNumberOfPoints();
	if(numPoint != m_ParticleMap.size()) {
		int numFib = m_FbNumFib;
		int numRes = m_FbResolution + 1;
		vtkCellArray* pCells = vtkCellArray::New();
		vtkIdType*    pIds   = new vtkIdType[numRes];
		vtkPoints*    pPoints = m_pDecomposedMuscle->GetPoints();
		pPoints->SetNumberOfPoints(numFib * numRes);
		for(int i = 0; i < numFib; ++i) {
			for(int j = 0; j < numRes; ++j) {
				int index = i * numRes + j;
				pPoints->SetPoint(index, m_Particles[m_ParticleMap[index]].position);
				pIds[j] = index;
			}
			pCells->InsertNextCell(numRes, pIds);
		}
		m_pDecomposedMuscle->SetLines(pCells);
		delete []pIds;
		pCells->Delete();
	}
}

//------------------------------------------------------------------------
// Updates the decomposed fibers using the deformed particles
void medVMEMuscleWrapper::UpdateFiberFromParticle()
{
	medVMEMuscleWrapper *wrapperRP = GetMuscleWrapper_RP();
	int numFib = wrapperRP->m_FbNumFib;
	int numRes = wrapperRP->m_FbResolution + 1;
	if (m_DeformationMethod == 2 && (m_Particles.empty() || m_ParticleMap.size() != numFib * numRes)) {
		// the particles are not the latest, update them first
		// haven't deformed or the particles parameters at the rest pose are changed (the particle numbers are different)
		DeformMuscle();
	}

	if(!m_Particles.empty() && m_ParticleMap.size() == numFib * numRes) {
		vtkPoints*    pPoints = vtkPoints::New();
		vtkCellArray* pCells  = vtkCellArray::New();
		vtkIdType*    pIds    = new vtkIdType[numRes];
		pPoints->SetNumberOfPoints(numFib * numRes);
		for(int i = 0; i < numFib; ++i) {
			for(int j = 0; j < numRes; ++j) {
				int index = i * numRes + j;
				pPoints->SetPoint(index, m_Particles[m_ParticleMap[index]].position);
				pIds[j] = index;
			}
			pCells->InsertNextCell(numRes, pIds);
		}
		m_pDecomposedMuscle->SetPoints(pPoints);
		m_pDecomposedMuscle->SetLines(pCells);
		delete []pIds;
		pPoints->Delete();
		pCells->Delete();
	}
	else {
		_RPT0(_CRT_WARN, "The particles need to update first, as the particle numbers at the rest and current pose are not the same.\n");
	}
}

//------------------------------------------------------------------------
// Updates the showed particles in the children node
void medVMEMuscleWrapper::UpdateChildrenParticleNode()
{
	unsigned long number = GetNumberOfChildren();
	for(unsigned long i = 0; i < number; ++i) {
		mafNode *node = GetChild(i);
		if (node->IsMAFType(mafVMELandmarkCloud)) {
			mafVMELandmarkCloud *cloud = dynamic_cast<mafVMELandmarkCloud*>(node);
			int particleNum = cloud->GetNumberOfLandmarks();
			for(int j = 0; j < particleNum; ++j)
				cloud->RemoveLandmark(0);

			int count = 0;
			std::stringstream ss;
			for(size_t j = 0; j < m_Particles.size(); ++j) {
				if(!m_FbDebugShowConstraints || m_Particles[j].fixed) {
					ss.clear();
					ss.str("");
					ss<<count++;
					cloud->AppendLandmark(m_Particles[j].position[0], m_Particles[j].position[1], m_Particles[j].position[2], ss.str().c_str());
				}
			}
			_RPT2(_CRT_WARN, "Number of %s Particles: %d \n", (m_FbDebugShowConstraints ? "Constraint " : ""), cloud->GetNumberOfLandmarks());
		}
	}
}

//------------------------------------------------------------------------
medVMEMuscleWrapper::CParticleSystem::~CParticleSystem()
//------------------------------------------------------------------------
{
	int nMuscles = (int)this->m_MSSmuscles.size();
	for (int i = 0; i < nMuscles; i++)
	{
		m_MSSmuscles[i]->SetOutput(NULL); // detach m_pDeformedMuscle from the filter
		m_MSSmuscles[i]->Delete();
	}

	m_MSSmuscles.clear();
	
	int nBones = (int)this->m_MSSbones.size();
	for (int i = 0; i < nBones; i++) {
		if (m_MSSbones[i] != NULL) {
			m_MSSbones[i]->Delete();
		}
	}

	m_MSSbones.clear();
}

//------------------------------------------------------------------------
//Initializes the particle system.
//Creates or updates m_pParticleSystem according to the current state.
//N.B. This method can be called from MASTER or SINGLE current-pose wrapper only. 
void medVMEMuscleWrapper::InitializeParticleSystem()
//------------------------------------------------------------------------
{
	_ASSERTE(m_MasterVMEMuscleWrapper == NULL);	//must be MASTER or SINGLE

	medVMEMusculoSkeletalModel* model_RP = this->GetMusculoSkeletalModel_RP();
	medVMEMusculoSkeletalModel* model_CP = this->GetMusculoSkeletalModel();	

	_ASSERTE(model_RP != model_CP);	//must be called from current-pose

	if (m_pParticleSystem == NULL)
	{
		//creates a new particle system
		m_pParticleSystem = new CParticleSystem();

		//get all lowest resolution bone groups
		//RELEASE NOTE: it is assumed that both atlases are compatible, i.e., motion fused atlas contain the same bone groups as its rest-pose counterpart.
		model_RP->GetMSMGraph()->GetBone(medMSMGraph::BoneType::Invalid, 
			medMSMGraph::LOD::Lowest, m_pParticleSystem->m_BoneGroupsList_RP, true);

		model_CP->GetMSMGraph()->GetBone(medMSMGraph::BoneType::Invalid, 
			medMSMGraph::LOD::Lowest, m_pParticleSystem->m_BoneGroupsList_CP, true);

		_ASSERTE(m_pParticleSystem->m_BoneGroupsList_RP.size() == m_pParticleSystem->m_BoneGroupsList_CP.size());

		//initialize an empty m_MSSbones (we will create the system only for bones that must be processed)
		m_pParticleSystem->m_MSSbones.resize(m_pParticleSystem->m_BoneGroupsList_RP.size(), NULL);
	}

	//let us update VME wrappers considered in the particle system
	//temporarily add this object (master) into m_VMEMuscleWrappers
	m_VMEMuscleWrappers.push_back(this);
	int nMuscles = m_VMEMuscleWrappers.size();

	for (int i = 0; i < nMuscles; i++)
	{
		medVMEMuscleWrapper* wrapper_CP = medVMEMuscleWrapper::SafeDownCast(m_VMEMuscleWrappers[i]);

		//check, if we have already an entry for this wrapper in our particle system
		std::vector< medVMEMuscleWrapper* >::iterator it = std::find(
			m_pParticleSystem->m_MuscleWrappers_CP.begin(), m_pParticleSystem->m_MuscleWrappers_CP.end(), wrapper_CP);

		int idxMuscle; //index to the entry for this wrapper
		if (it != m_pParticleSystem->m_MuscleWrappers_CP.end()) 
		{
			//OK, the entry is already present
			idxMuscle = std::distance(m_pParticleSystem->m_MuscleWrappers_CP.begin(), it);
		} 
		else 
		{
			//the entry does not exist => create it
			idxMuscle = (int)m_pParticleSystem->m_MuscleWrappers_CP.size();
			m_pParticleSystem->m_MuscleWrappers_CP.push_back(wrapper_CP);
			m_pParticleSystem->m_MuscleWrappers_RP.push_back(wrapper_CP->GetMuscleWrapper_RP());			
			m_pParticleSystem->m_MSSmuscles.push_back(NULL);

			std::vector< int > boneIdx;
			GetMuscleBoneAssociation(wrapper_CP->GetMuscleVME_RP(), m_pParticleSystem->m_BoneGroupsList_RP, boneIdx);
			m_pParticleSystem->m_MuscleBoneAssociation.push_back(boneIdx);
		} 		
		
		//check, if the particles for the rest-pose system are available
		medVMEMuscleWrapper* wrapper_RP = m_pParticleSystem->m_MuscleWrappers_RP[idxMuscle];
		if (!wrapper_RP->m_Particles.empty())
		{
			if (wrapper_CP->m_Particles.empty()) 
			{
				wrapper_CP->m_Particles = wrapper_RP->m_Particles;
				wrapper_CP->m_ParticleMap = wrapper_RP->m_ParticleMap;
			}
		}
		else
		{
			//particles are not available => we will need to generate particles and update the mass-spring system data for it
			//RP particle system is in coordinate space of RP wrapper (which might or might not be the same as our)
			wrapper_RP->InitializeParticleSystem_RP(m_pParticleSystem->m_BoneGroupsList_RP, 
				m_pParticleSystem->m_MuscleBoneAssociation[idxMuscle]);

			//copy the particles to the current-pose system
			wrapper_CP->m_Particles = wrapper_RP->m_Particles;
			wrapper_CP->m_ParticleMap = wrapper_RP->m_ParticleMap;

			//do we have already existing mass-spring system for the muscle?
			vtkMassSpringMuscle* mssMuscle = m_pParticleSystem->m_MSSmuscles[idxMuscle];		
			if (mssMuscle == NULL){
				m_pParticleSystem->m_MSSmuscles[idxMuscle] = mssMuscle = vtkMassSpringMuscle::New();						
			}
			
			//RELEASE NOTE: vtkMassSpringMuscle is ill-written requiring the Output to be specified before setting Input
			mssMuscle->SetOutput(wrapper_CP->m_pDeformedMuscle);			

			//create the input mass-spring data in our (this) coordinate space			
			mafVME* muscle_RP = wrapper_CP->GetMuscleVME_RP();			
			vtkPolyData *RPMesh = this->CreateTransformedPolyDataFromVME(muscle_RP);
			this->TransformParticles(wrapper_CP->m_Particles, &wrapper_RP->GetOutput()->GetAbsTransform()->GetMatrix());

			vtkMSSDataSet *inputData = vtkMassSpringMuscle::CreateData(wrapper_CP->m_Particles, RPMesh);
			mssMuscle->SetInput(inputData, RPMesh);
			inputData->Delete();	//no longer needed
			RPMesh->Delete();	//no longer needed
		} //end if [wrapper_RP->m_Particles.empty()]

		//check, if we have particles for the rest-pose bones
		int nBones = (int)m_pParticleSystem->m_MuscleBoneAssociation[idxMuscle].size();
		for (int j = 0; j < nBones; j++)
		{
			int idxBone = m_pParticleSystem->m_MuscleBoneAssociation[idxMuscle][j];

			//do we have already existing mass-spring system for the muscle?
			vtkMassSpringBone* mssBone = m_pParticleSystem->m_MSSbones[idxBone];		
			if (mssBone == NULL)
			{
				m_pParticleSystem->m_MSSbones[idxBone] = mssBone = vtkMassSpringBone::New();	

				//create the input mass-spring data (we need to do it only once (unlike in the case of muscles))
				//but although LOWEST resolutions are sufficient for everything else, for particles construction of bones,
				//we need the highest resolution -> find it
				mafVME* bone_RP = mafVME::SafeDownCast(m_pParticleSystem->m_BoneGroupsList_RP[idxBone]);
				const medMSMGraph::MSMGraphNode* bone_RP_Node = model_RP->GetMSMGraph()->FindMSMNode(bone_RP, true);
				_ASSERTE(bone_RP_Node != NULL);
				
				while (bone_RP_Node->m_HigherResNode != NULL) {
					bone_RP_Node = bone_RP_Node->m_HigherResNode;
				}
				bone_RP = mafVME::SafeDownCast(bone_RP_Node->m_Vme);

				//we need to convert the mesh into our coordinate space										
				vtkPolyData *RPMesh = CreateTransformedPolyDataFromVME(bone_RP);
				vtkMSSDataSet *inputData = vtkMassSpringMuscle::GenerateSolidObject(RPMesh);
				mssBone->SetInput(inputData);
				inputData->Delete();	//no longer needed
				RPMesh->Delete();	//no longer needed
			}
		}
	}

	//remove all muscle wrappers that are not available in the current context
	int nMusclesOld = (int)m_pParticleSystem->m_MuscleWrappers_CP.size();
	if (nMusclesOld != m_VMEMuscleWrappers.size())
	{		
		for (int i = 0, index = 0; i < nMusclesOld; i++)
		{
			if (m_VMEMuscleWrappers.end() != std::find(m_VMEMuscleWrappers.begin(), 
				m_VMEMuscleWrappers.end(), m_pParticleSystem->m_MuscleWrappers_CP[index])) {
					index++;	//this is valid wrapper
			}
			else
			{
				m_pParticleSystem->m_MuscleWrappers_CP.erase(m_pParticleSystem->m_MuscleWrappers_CP.begin() + index);
				m_pParticleSystem->m_MuscleWrappers_RP.erase(m_pParticleSystem->m_MuscleWrappers_RP.begin() + index);
				m_pParticleSystem->m_MuscleBoneAssociation.erase(m_pParticleSystem->m_MuscleBoneAssociation.begin() + index);
				m_pParticleSystem->m_MSSmuscles[index]->SetOutput(NULL);
				m_pParticleSystem->m_MSSmuscles[index]->Delete();
				m_pParticleSystem->m_MSSmuscles.erase(m_pParticleSystem->m_MSSmuscles.begin() + index);
			}			
		}
	}

	m_VMEMuscleWrappers.pop_back();
}

//------------------------------------------------------------------------
//Returns indices to bones (given in the array 'bones') affected by the given muscle (in the rest-pose).
//	N.B. bones array may contain a bone VME or a bone group VME in any resolution in the rest-pose musculoskeletal model.
void medVMEMuscleWrapper::GetMuscleBoneAssociation(const mafVME* muscle, const medMSMGraph::mafVMENodeList& bones, std::vector< int >& out_idx)
//------------------------------------------------------------------------
{
	medVMEMusculoSkeletalModel* model_RP = this->GetMusculoSkeletalModel_RP();

	//get the MSM node for the wrapped muscle		
	const medMSMGraph::MSMGraphNode* muscleNode_RP = model_RP->GetMSMGraph()->FindMSMNode(muscle, true);
	if (muscleNode_RP != NULL)
	{
		//retrieve information about bones whose movement influence this muscle
		medMSMGraph::BoneEnumList muscleBoneInfos;
		medMSMGraph::MSMGraphRelations::GetBonesByMuscle(muscleNode_RP->m_NodeDescriptor.m_MuscleInfo, muscleBoneInfos);

		int boneTypes = (int)muscleBoneInfos.size();
		if (boneTypes != 0) 
		{
			//we have at least one bone 
			bool bOK = true;
			for (int j = 0; j < boneTypes; j++)
			{	
				int idxBone = FindBone(bones, muscleBoneInfos[j]);
				if (idxBone < 0) {
					bOK = false; break;
				}

				//avoid duplicities
				if (std::find(out_idx.begin(), out_idx.end(), idxBone) == out_idx.end()) {
					out_idx.push_back(idxBone);	//this is a unique bone
				}
			}

			if (bOK) {
				return;	//we are ready
			}

			out_idx.clear();	//clear the out buffer and fix it
		}
	} //end if muscleNode_RP != NULL

	//there is some error => we need to associate this muscle with every bone
	int nBones = (int)bones.size();
	for (int i = 0; i < nBones; i++) {
		out_idx.push_back(i);
	}
}

//------------------------------------------------------------------------
//Returns index of the bone VME (given in the array 'bones') that contains the model of bone identified by 'bonetype' tag (specified as BoneType enum).
//	N.B.  bones array may contain a bone VME or a bone group VME in any resolution in the rest-pose musculoskeletal model.		
//	If the bone could not be found (in any of its resolutions and bone groups) in 'bones' array, the method returns -1
int medVMEMuscleWrapper::FindBone(const medMSMGraph::mafVMENodeList& bones, int bonetype)
//------------------------------------------------------------------------
{
	medVMEMusculoSkeletalModel* model_RP = this->GetMusculoSkeletalModel_RP();

	//first, get all bone groups available
	medMSMGraph::MSMGraphNodeList boneGroupsList, boneNode; 
	model_RP->GetMSMGraph()->GetBone(medMSMGraph::BoneType::Invalid, medMSMGraph::LOD::Highest, boneGroupsList, true);

	int nGroups = (int)boneGroupsList.size();
	for (int i = 0; i < nGroups; i++)
	{		
		//try to find the requested bone in the current group
		medMSMGraph::MSMGraphNode* pRet[2] = {NULL, boneGroupsList[i]};
		model_RP->GetMSMGraph()->GetBone(bonetype, medMSMGraph::LOD::Highest, boneNode, true, pRet[1]);
		if (boneNode.size() != 0)
		{
			//the requested bone is present in the current bone group
			pRet[0] = boneNode[0];
			for (int j = 0; j < 2; j++)
			{
				//try to find the bone (or its bone group) of any resolution in 'bones'
				const medMSMGraph::MSMGraphNode* pCur = pRet[j];						
				while (pCur != NULL) 
				{
					//check, if we have already an entry for this wrapper in our particle system					
					medMSMGraph::mafVMENodeList::const_iterator it = std::find(bones.begin(), bones.end(), pCur->m_Vme);		
					if (it != bones.end()) 
					{
						//we have found it, so we are ready
						return std::distance(bones.begin(), it);
					}

					//try to find the next resolution
					pCur = pCur->m_LowerResNode;
				} //end while
			}			
		}
	}

	return -1;	//not found
}