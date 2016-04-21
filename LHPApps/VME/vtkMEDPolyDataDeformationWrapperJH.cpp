#include "vtkMEDPolyDataDeformationWrapperJH.h"
#include "PKUtils.h"


/*static*/ void WrapperInterpoler::MatchCurves(vtkPolyData* startWrapper, vtkPolyData* endWrapper) { 
	WrapperInterpoler* def = (WrapperInterpoler*)vtkMEDPolyDataDeformation::New();
	def->MyMatchCurves(startWrapper, endWrapper);
	def->Delete();
}

void WrapperInterpoler::MyMatchCurves(vtkPolyData* startWrapper, vtkPolyData* endWrapper) {

	vtkPoints* startPoints = startWrapper->GetPoints();
	int nStartPoints = startPoints->GetNumberOfPoints();

	vtkPoints* endPoints = endWrapper->GetPoints();
	int nEndPoints = endPoints->GetNumberOfPoints();
	
	int nPointsTotal = nStartPoints + nEndPoints - 2; // all points, starting and ending point only once

	CSkeletonVertex** startSkeletonArray = new CSkeletonVertex*[nPointsTotal];
	CSkeletonVertex** startSkeletonArrayRef = new CSkeletonVertex*[nPointsTotal];
	for (int i = 0; i < nPointsTotal; i++) {
		if (i < nStartPoints) {
			double p[3] = {0,0,0};
			startPoints->GetPoint(i, p);
			startSkeletonArrayRef[i] = startSkeletonArray[i] = new CSkeletonVertex(p);
		}
		else {
			startSkeletonArrayRef[i] = startSkeletonArray[i] = new CSkeletonVertex();
		}
	}

	CSkeletonVertex** endSkeletonArray = new CSkeletonVertex*[nPointsTotal];
	CSkeletonVertex** endSkeletonArrayRef = new CSkeletonVertex*[nPointsTotal];
	for (int i = 0; i < nPointsTotal; i++) {
		if (i < nEndPoints) {
			double p[3] = {0,0,0};
			endPoints->GetPoint(i, p);
			endSkeletonArrayRef[i] = endSkeletonArray[i] = new CSkeletonVertex(p);
		}
		else {
			endSkeletonArrayRef[i] = endSkeletonArray[i] = new CSkeletonVertex();
		}
	}

	int nPoints = vtkMEDPolyDataDeformation::MatchCurves(startSkeletonArray, nStartPoints, endSkeletonArray, nEndPoints);

	vtkPoints* nsp = vtkPoints::New();
	startWrapper->SetPoints(nsp);
	nsp->Delete();
	vtkPoints* nep = vtkPoints::New();
	endWrapper->SetPoints(nep);
	nep->Delete();
/*
	// reductions of points which are too close to each other
	nsp->InsertPoint(0,startSkeletonArray[0]->Coords); // starting points
	nep->InsertPoint(0,endSkeletonArray[0]->Coords);
	int inserted = 1;
	int last = 0;
	
	for(int i = 1; i < nPoints-1; i++) {
		double *p1 = startSkeletonArray[i]->Coords;
		double *p2 = startSkeletonArray[last]->Coords;
		double p[3] = {0,0,0};
		PKUtils::SubtractVertex(p1, p2, p);
		double distStart = PKUtils::CalculateVertexLength(p);
		
		p1 = endSkeletonArray[i]->Coords;
		p2 = endSkeletonArray[last]->Coords;
		PKUtils::SubtractVertex(p1, p2, p);
		double distEnd = PKUtils::CalculateVertexLength(p);

		if (distStart > DIST_LIMIT && distEnd > DIST_LIMIT) {
			nsp->InsertPoint(inserted,startSkeletonArray[i]->Coords);
			nep->InsertPoint(inserted,endSkeletonArray[i]->Coords);
			inserted++;
			last = i;
		}
	}

	nsp->InsertPoint(inserted,startSkeletonArray[nPoints-1]->Coords); // ending points
	nep->InsertPoint(inserted,endSkeletonArray[nPoints-1]->Coords);
	
	nPoints = inserted + 1;*/

	for(int i = 0; i < nPoints; i++) {
		nsp->InsertPoint(i,startSkeletonArray[i]->Coords);
		nep->InsertPoint(i,endSkeletonArray[i]->Coords);
	}

	vtkCellArray* linesStart = vtkCellArray::New();
	startWrapper->SetLines(linesStart);
	vtkCellArray* linesEnd = vtkCellArray::New();
	endWrapper->SetLines(linesEnd);

	for(int k = 0; k < nPoints-1; k++) {
		vtkIdType ps[2] = {k,k+1};
		linesStart->InsertNextCell(2, ps);
		linesEnd->InsertNextCell(2, ps);
	}
	linesStart->Delete();
	linesEnd->Delete();

	for (int i = 0; i < nPoints; i++)
	{
		// edges, which are created automatically (and not used for anything) has to be deleted
		int size = startSkeletonArray[i]->OneRingEdges.size();
		if (size > 0 && i < nPoints-1) {
			delete startSkeletonArray[i]->OneRingEdges[0];
			delete endSkeletonArray[i]->OneRingEdges[0];
		}
		
		delete startSkeletonArray[i];
		delete endSkeletonArray[i];
	}

	for(int i = 0; i < nPointsTotal; i++) 
	{		
		delete startSkeletonArrayRef[i];
		delete endSkeletonArrayRef[i];
	}
	delete[] startSkeletonArray;
	delete[] endSkeletonArray;
	delete[] startSkeletonArrayRef;
	delete[] endSkeletonArrayRef;
}

