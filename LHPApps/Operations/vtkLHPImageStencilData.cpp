/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: vtkLHPImageStencilData.cpp,v $
Language:  C++
Date:      $Date: 2010-04-14 16:27:45 $
Version:   $Revision: 1.1.2.4 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2009
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#include "mafDefines.h" 

//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "vtkLHPImageStencilData.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkCubeSource.h"
#include "vtkSphereSource.h"

vtkCxxRevisionMacro(vtkLHPImageStencilData, "$Revision: 1.1.2.4 $");

vtkStandardNewMacro(vtkLHPImageStencilData);

//----------------------------------------------------------------------------
vtkLHPImageStencilData::vtkLHPImageStencilData()
//----------------------------------------------------------------------------
{
	for (int i = 0; i < 3; i++)
	{
		Origin[i] = 0;
		Spacing[i] = 1.0;
		//m_Translation[i] = 0;
		m_RecordedTranslation[0] = 0;
	}

	m_bRecordTranslation = false;
}

//----------------------------------------------------------------------------
vtkLHPImageStencilData::~vtkLHPImageStencilData()
//----------------------------------------------------------------------------
{
	// release memory
	Clear();
}

//////////////////////////////////////////////////////////////////////////////
// VTK5 vtkImageStencilData functions
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
void vtkLHPImageStencilData::InternalAdd(vtkImageStencilData * stencil1 )
//----------------------------------------------------------------------------
{
	int extent[6], extent1[6], extent2[6], r1, r2, idy, idz, iter=0;
	stencil1->GetExtent(extent1);
	this->GetExtent(extent2);

	extent[0] = (extent1[0] < extent2[0]) ? extent2[0] : extent1[0];
	extent[1] = (extent1[1] > extent2[1]) ? extent2[1] : extent1[1];
	extent[2] = (extent1[2] < extent2[2]) ? extent2[2] : extent1[2];
	extent[3] = (extent1[3] > extent2[3]) ? extent2[3] : extent1[3];
	extent[4] = (extent1[4] < extent2[4]) ? extent2[4] : extent1[4];
	extent[5] = (extent1[5] > extent2[5]) ? extent2[5] : extent1[5];

	bool modified = false;
	for (idz=extent[4]; idz<=extent[5]; idz++, iter=0)
	{
		for (idy = extent[2]; idy <= extent[3]; idy++, iter=0)
		{
			int moreSubExtents = 1;
			while( moreSubExtents )
			{
				moreSubExtents = stencil1->GetNextExtent( 
					r1, r2, extent[0], extent[1], idy, idz, iter);

				if (r1 <= r2 ) // sanity check 
				{ 
					this->InsertAndMergeExtent(r1, r2, idy, idz); 
					modified = true;
				}
			}
		}
	}

	if (modified)
	{
		this->Modified();
	}
}

//----------------------------------------------------------------------------
// core function for stencil handling, but invovled in memory allocation
void vtkLHPImageStencilData::Add( vtkImageStencilData * stencil1 )
//----------------------------------------------------------------------------
{
	int extent[6], extent1[6], extent2[6], r1, r2, idy, idz, iter=0;
	stencil1->GetExtent(extent1);
	this->GetExtent(extent2);

	if (extent1[0] > extent1[1] || 
		extent1[2] > extent1[3] || 
		extent1[4] > extent1[5])
	{
		return;
	}

	// not in vtkMath 4
	//if (vtkMath::ExtentIsWithinOtherExtent(extent1,extent2))
	if (ExtentIsWithinOtherExtent(extent1,extent2))
	{

		// Extents of stencil1 are entirely within the Self's extents. There
		// is no need to re-allocate the extent lists.

		this->InternalAdd(stencil1);
		return;
	}

	// Need to reallocate extent lists. 
	// 1. We will create a temporary stencil data.
	// 2. Copy Self into this temporary stencil. 
	// 3. Reallocate Self's extents to match the resized stencil. 
	// 4. Merge stencil data from both into the Self.

	// Find the smallest bounding box large enough to hold both stencils.
	extent[0] = (extent1[0] > extent2[0]) ? extent2[0] : extent1[0];
	extent[1] = (extent1[1] < extent2[1]) ? extent2[1] : extent1[1];
	extent[2] = (extent1[2] > extent2[2]) ? extent2[2] : extent1[2];
	extent[3] = (extent1[3] < extent2[3]) ? extent2[3] : extent1[3];
	extent[4] = (extent1[4] > extent2[4]) ? extent2[4] : extent1[4];
	extent[5] = (extent1[5] < extent2[5]) ? extent2[5] : extent1[5];

	vtkImageStencilData *tmp = vtkImageStencilData::New();
	tmp->DeepCopy(this);

	this->SetExtent(extent);
	this->AllocateExtents(); // Reallocate extents.

	for (idz=extent2[4]; idz<=extent2[5]; idz++, iter=0)
	{
		for (idy = extent2[2]; idy <= extent2[3]; idy++, iter=0)
		{
			int moreSubExtents = 1;
			while( moreSubExtents )
			{
				moreSubExtents = tmp->GetNextExtent( 
					r1, r2, extent[0], extent[1], idy, idz, iter);

				if (r1 <= r2 ) // sanity check 
				{ 
					this->InsertAndMergeExtent(r1, r2, idy, idz); 
				}
			}
		}
	}

	tmp->Delete();

	for (idz=extent1[4]; idz<=extent1[5]; idz++, iter=0)
	{
		for (idy = extent1[2]; idy <= extent1[3]; idy++, iter=0)
		{
			int moreSubExtents = 1;
			while( moreSubExtents )
			{
				moreSubExtents = stencil1->GetNextExtent( 
					r1, r2, extent[0], extent[1], idy, idz, iter);

				if (r1 <= r2 ) // sanity check 
				{ 
					this->InsertAndMergeExtent(r1, r2, idy, idz); 
				}
			}
		}
	}

	this->Modified();
} 

//----------------------------------------------------------------------------
void vtkLHPImageStencilData::Subtract( vtkImageStencilData * stencil1 )
//----------------------------------------------------------------------------
{
	int extent[6], extent1[6], extent2[6], r1, r2, idy, idz, iter=0;
	stencil1->GetExtent(extent1);
	this->GetExtent(extent2);

	if ((extent1[0] > extent2[1]) || (extent1[1] < extent2[0]) || 
		(extent1[2] > extent2[3]) || (extent1[3] < extent2[2]) || 
		(extent1[4] > extent2[5]) || (extent1[5] < extent2[4]))
	{
		// The extents don't intersect.. No subraction needed
		return;
	}

	// Find the smallest box intersection of the extents
	extent[0] = (extent1[0] < extent2[0]) ? extent2[0] : extent1[0];
	extent[1] = (extent1[1] > extent2[1]) ? extent2[1] : extent1[1];
	extent[2] = (extent1[2] < extent2[2]) ? extent2[2] : extent1[2];
	extent[3] = (extent1[3] > extent2[3]) ? extent2[3] : extent1[3];
	extent[4] = (extent1[4] < extent2[4]) ? extent2[4] : extent1[4];
	extent[5] = (extent1[5] > extent2[5]) ? extent2[5] : extent1[5];

	for (idz=extent[4]; idz<=extent[5]; idz++, iter=0)
	{
		for (idy = extent[2]; idy <= extent[3]; idy++, iter=0)
		{
			int moreSubExtents = 1;
			while( moreSubExtents )
			{
				moreSubExtents = stencil1->GetNextExtent( 
					r1, r2, extent[0], extent[1], idy, idz, iter);

				if (r1 <= r2 ) // sanity check 
				{ 
					this->RemoveExtent(r1, r2, idy, idz); 
				}
			}
		}
	}

	this->Modified();
} 


//----------------------------------------------------------------------------
void vtkLHPImageStencilData::Replace( vtkImageStencilData * stencil1 )
//----------------------------------------------------------------------------
{
	int extent[6], extent1[6], extent2[6], r1, r2, idy, idz, iter=0;
	stencil1->GetExtent(extent1);
	this->GetExtent(extent2);

	if ((extent1[0] > extent2[1]) || (extent1[1] < extent2[0]) || 
		(extent1[2] > extent2[3]) || (extent1[3] < extent2[2]) || 
		(extent1[4] > extent2[5]) || (extent1[5] < extent2[4]))
	{
		// The extents don't intersect.. No subraction needed
		return;
	}

	// Find the smallest box intersection of the extents
	extent[0] = (extent1[0] < extent2[0]) ? extent2[0] : extent1[0];
	extent[1] = (extent1[1] > extent2[1]) ? extent2[1] : extent1[1];
	extent[2] = (extent1[2] < extent2[2]) ? extent2[2] : extent1[2];
	extent[3] = (extent1[3] > extent2[3]) ? extent2[3] : extent1[3];
	extent[4] = (extent1[4] < extent2[4]) ? extent2[4] : extent1[4];
	extent[5] = (extent1[5] > extent2[5]) ? extent2[5] : extent1[5];

	for (idz=extent[4]; idz<=extent[5]; idz++, iter=0)
	{
		for (idy = extent[2]; idy <= extent[3]; idy++, iter=0)
		{
			this->RemoveExtent(extent[0], extent[1], idy, idz); 

			int moreSubExtents = 1;
			while( moreSubExtents )
			{
				moreSubExtents = stencil1->GetNextExtent( 
					r1, r2, extent[0], extent[1], idy, idz, iter);

				if (r1 <= r2 ) // sanity check 
				{ 
					this->InsertAndMergeExtent(r1, r2, idy, idz);
				}
			}
		}
	}

	this->Modified();
} 


//----------------------------------------------------------------------------
void vtkLHPImageStencilData::InsertAndMergeExtent(int r1, int r2,
											   int yIdx, int zIdx)
//----------------------------------------------------------------------------
{
	// calculate the index into the extent array
	int extent[6];
	this->GetExtent(extent);
	int yExt = extent[3] - extent[2] + 1;
	int incr = (zIdx - extent[4])*yExt + (yIdx - extent[2]);

	int &clistlen = this->ExtentListLengths[incr];
	int *&clist = this->ExtentLists[incr];

	if (clistlen == 0)
	{ // no space has been allocated yet
		clist = new int[2];
		clist[clistlen++] = r1;
		clist[clistlen++] = r2 + 1;
		return;
	}

	for (int k = 0; k < clistlen; k+=2)
	{
		if ((r1 >= clist[k] && r1 < clist[k+1]) || 
			(r2 >= clist[k] && r2 < clist[k+1]))
		{
			// An intersecting extent is already present. Merge with that one.
			if (r1 < clist[k])
			{
				clist[k]   = r1;
			}
			else if (r2 >= clist[k+1])
			{
				clist[k+1] = r2+1;
				this->CollapseAdditionalIntersections(r2, k+2, clist, clistlen);
			}
			return;
		}
		else if (r1 < clist[k] && r2 >= clist[k+1])
		{
			clist[k]   = r1;
			clist[k+1] = r2+1;
			this->CollapseAdditionalIntersections(r2, k+2, clist, clistlen);
			return;
		}
	}

	// We will be inserting a unique extent...

	// check whether more space is needed
	// the allocated space is always the smallest power of two
	// that is not less than the number of stored items, therefore
	// we need to allocate space when clistlen is a power of two
	int clistmaxlen = 2;
	while (clistlen > clistmaxlen)
	{
		clistmaxlen *= 2;
	}
	int insertIndex = clistlen, offset = 0;
	if (clistmaxlen == clistlen || r1 < clist[clistlen-1])
	{ // need to allocate more space or rearrange
		if (clistmaxlen == clistlen)
		{
			clistmaxlen *= 2;
		}
		int *newclist = new int[clistmaxlen];
		for (int k = 0; k < clistlen; k+=2)
		{
			if (offset == 0 && r1 < clist[k])
			{
				insertIndex = k;
				offset = 2;
			}
			newclist[k+offset] = clist[k];
			newclist[k+1+offset] = clist[k+1];
		}
		delete [] clist;
		clist = newclist;
	}

	clist[insertIndex] = r1;
	clist[insertIndex+1] = r2 + 1;
	clistlen += 2;
}


//----------------------------------------------------------------------------
void vtkLHPImageStencilData::CollapseAdditionalIntersections(int r2, int idx,
														  int *clist,
														  int &clistlen)
//----------------------------------------------------------------------------
{
	if (idx >= clistlen)
	{
		return;
	}

	int removeExtentStart = idx, removeExtentEnd = idx;
	// overlap with any of the remainder of the list?
	for (; idx < clistlen; idx+=2, removeExtentEnd+=2)
	{
		if (r2 < clist[idx])
		{
			if (idx == removeExtentStart)
			{
				// no additional overlap... thus no collapse
				return;
			}
			break;
		}
		else if (r2 < clist[idx+1])
		{
			clist[removeExtentStart - 1] = clist[idx+1];
		}
	}

	// collapse the list?
	int i;
	for (i = removeExtentEnd, idx = removeExtentStart; i < clistlen; i++, idx++)
	{
		clist[idx] = clist[i];
	}
	clistlen = idx;
}

// copied from vtkMath
//----------------------------------------------------------------------------
int vtkLHPImageStencilData::ExtentIsWithinOtherExtent(int extent1[6], int extent2[6])
//----------------------------------------------------------------------------
{
	if (!extent1 || !extent2)
	{
		return 0;
	}

	int i;
	for (i = 0; i < 6; i += 2)
	{
		if (extent1[i]     < extent2[i] || extent1[i]     > extent2[i + 1] ||
			extent1[i + 1] < extent2[i] || extent1[i + 1] > extent2[i + 1])
		{
			return 0;
		}
	}

	return 1;
}


//----------------------------------------------------------------------------
void vtkLHPImageStencilData::RemoveExtent(int r1, int r2, int yIdx, int zIdx)
//----------------------------------------------------------------------------
{
	if (zIdx < this->Extent[4] || zIdx > this->Extent[5] || 
		yIdx < this->Extent[2] || yIdx > this->Extent[3] )
	{
		return;
	}

	// calculate the index into the extent array
	int extent[6];
	this->GetExtent(extent);
	int yExt = extent[3] - extent[2] + 1;
	int incr = (zIdx - extent[4])*yExt + (yIdx - extent[2]);

	int &clistlen = this->ExtentListLengths[incr];
	int *&clist = this->ExtentLists[incr];

	if (clistlen == 0)
	{ // nothing here.. nothing to remove
		return;
	}

	if (r1 <= extent[0] && r2 >= extent[1])
	{
		// remove the whole row.
		clistlen = 0;
		delete [] clist;
		clist = NULL;
		return;
	}

	int length = clistlen;
	for (int k = 0; k < length; k += 2)
	{
		if (r1 <=  clist[k] && r2 >= (clist[k+1]-1))
		{  
			// Remove this entry;
			clistlen -= 2;

			if (clistlen == 0)
			{
				delete [] clist;
				clist = NULL;
				return;
			}

			int clistmaxlen = 2;
			while (clistlen > clistmaxlen)
			{
				clistmaxlen *= 2;
			}

			if (clistmaxlen == clistlen)
			{
				int *newclist = new int[clistmaxlen];
				for (int m = 0;   m < k;    m++)   { newclist[m]   = clist[m]; }
				for (int m = k+2; m < length; m++) { newclist[m-2] = clist[m]; }
				delete [] clist;
				clist = newclist;
			}
			else
			{
				for (int m = k+2; m < length; m++) { clist[m-2] = clist[m]; }
			}

			length = clistlen;
			if (k >= length)
			{
				return;
			}
		}

		if ((r1 >= clist[k] && r1 < clist[k+1]) || 
			(r2 >= clist[k] && r2 < clist[k+1]))
		{

			bool split = false;  
			int tmp = -1;    

			// An intersecting extent is already present. Merge with that one.
			if (r1 > clist[k])
			{
				tmp = clist[k+1];
				clist[k+1] = r1;
				split    = true;
			}
			if (split)
			{
				if (r2 < tmp-1)
				{
					// check whether more space is needed
					// the allocated space is always the smallest power of two
					// that is not less than the number of stored items, therefore
					// we need to allocate space when clistlen is a power of two
					int clistmaxlen = 2;
					while (clistlen > clistmaxlen)
					{
						clistmaxlen *= 2;
					}
					if (clistmaxlen == clistlen)
					{ // need to allocate more space
						clistmaxlen *= 2;
						int *newclist = new int[clistmaxlen];
						for (int m = 0; m < clistlen; m++)
						{
							newclist[m] = clist[m];
						}
						delete [] clist;
						clist = newclist;
					}
					clist[clistlen++] = r2+1;
					clist[clistlen++] = tmp;
				}
			}
			else
			{
				if (r2 < clist[k+1]-1)
				{
					clist[k] = r2+1;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////
// end of VTK5 vtkImageStencilData functions
///////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------------
// custom functions
// allocate an extent in advance to avoid frequent memory operation
// adapted from Add()
void vtkLHPImageStencilData::AddAndResizeExtent( int extent1[6])
//----------------------------------------------------------------------------
{
	int extent[6], extent2[6], r1, r2, idz, idy, iter=0;
	//stencil1->GetExtent(extent1);
	this->GetExtent(extent2);

	if (extent1[0] > extent1[1] || 
		extent1[2] > extent1[3] || 
		extent1[4] > extent1[5])
	{
		return;
	}

	// not in vtkMath 4
	//if (vtkMath::ExtentIsWithinOtherExtent(extent1,extent2))
	if (ExtentIsWithinOtherExtent(extent1,extent2))
	{

		// Extents of stencil1 are entirely within the Self's extents. There
		// is no need to re-allocate the extent lists.

		//this->InternalAdd(stencil1);
		return;
	}

	// Need to reallocate extent lists. 
	// 1. We will create a temporary stencil data.
	// 2. Copy Self into this temporary stencil. 
	// 3. Reallocate Self's extents to match the resized stencil. 
	// 4. Merge stencil data from both into the Self.

	// Find the smallest bounding box large enough to hold both stencils.
	bool empty = false;;
	if (extent2[0] > extent2[1])
	{
		empty = true;
	}

	if (empty)
	{
		for (int i = 0; i < 6; i++)
			extent[i] = extent1[i];
	}
	else
	{
		extent[0] = (extent1[0] > extent2[0]) ? extent2[0] : extent1[0];
		extent[1] = (extent1[1] < extent2[1]) ? extent2[1] : extent1[1];
		extent[2] = (extent1[2] > extent2[2]) ? extent2[2] : extent1[2];
		extent[3] = (extent1[3] < extent2[3]) ? extent2[3] : extent1[3];
		extent[4] = (extent1[4] > extent2[4]) ? extent2[4] : extent1[4];
		extent[5] = (extent1[5] < extent2[5]) ? extent2[5] : extent1[5];
	}

	vtkImageStencilData *tmp = vtkImageStencilData::New();
	tmp->DeepCopy(this);

	this->SetExtent(extent);
	this->AllocateExtents(); // Reallocate extents.

	if (!empty)
	{
		for (idz=extent2[4]; idz<=extent2[5]; idz++, iter=0)
		{
			for (idy = extent2[2]; idy <= extent2[3]; idy++, iter=0)
			{
				int moreSubExtents = 1;
				while( moreSubExtents )
				{
					moreSubExtents = tmp->GetNextExtent( 
						r1, r2, extent[0], extent[1], idy, idz, iter);

					if (r1 <= r2 ) // sanity check 
					{ 
						this->InsertAndMergeExtent(r1, r2, idy, idz); 
					}
				}
			}
		}
	}


	tmp->Delete();
}

//----------------------------------------------------------------------------
// this function essentially calls InternalAdd, adapted from Add()
void vtkLHPImageStencilData::AddWithExtentAllocated( vtkImageStencilData * stencil1 )
//----------------------------------------------------------------------------
{
	int extent1[6], extent2[6], iter=0;
	stencil1->GetExtent(extent1);
	this->GetExtent(extent2);

	if (extent1[0] > extent1[1] || 
		extent1[2] > extent1[3] || 
		extent1[4] > extent1[5])
	{
		return;
	}

	// not in vtkMath 4
	//if (vtkMath::ExtentIsWithinOtherExtent(extent1,extent2))
	if (ExtentIsWithinOtherExtent(extent1,extent2))
	{

		// Extents of stencil1 are entirely within the Self's extents. There
		// is no need to re-allocate the extent lists.

		this->InternalAdd(stencil1);
	}

} 

//----------------------------------------------------------------------------
void vtkLHPImageStencilData::Clear()
//----------------------------------------------------------------------------
{
	// adapted from vtkImageStencilData::AllocateExtents()
	if (this->NumberOfExtentEntries != 0)
	{
		int n = this->NumberOfExtentEntries;
		for (int i = 0; i < n; i++)
		{
			delete [] this->ExtentLists[i];
		}
		delete [] this->ExtentLists;
		delete [] this->ExtentListLengths;
	}

	this->SetExtent(0, -1, 0, -1, 0, -1);
	this->NumberOfExtentEntries = 0;
	this->ExtentLists = NULL;
	this->ExtentListLengths = NULL;
}

//----------------------------------------------------------------------------
void vtkLHPImageStencilData::GenBoxStencil(vtkCubeSource * cube)
//----------------------------------------------------------------------------
{
	Clear();

	// calculate bounding box (in float) and extents ( in integer)
	double center[3];
	cube->GetCenter(center);
	double size[3];
	size[0] = cube->GetXLength();
	size[1] = cube->GetYLength();
	size[2] = cube->GetZLength();

	double bbox[6];

	bbox[0] = bbox[2] = bbox[4] = 0;
	bbox[1] = size[0];
	bbox[3] = size[1];
	bbox[5] = size[2];


	// extents should be larger than 0
	Extent[0] = int (bbox[0] / Spacing[0]);
	Extent[1] = int (bbox[1] / Spacing[0]);
	Extent[2] = int (bbox[2] / Spacing[1]);
	Extent[3] = int (bbox[3] / Spacing[1]);
	Extent[4] = int (bbox[4] / Spacing[2]);
	Extent[5] = int (bbox[5] / Spacing[2]);

	AllocateExtents();


	// generate range lists in a simple way
	int idxX, xStart, xEnd;
	xStart = Extent[0];
	xEnd = Extent[1];
	for (int idxZ = 0; idxZ <= Extent[5] - Extent[4]; idxZ++)
	{
		for (int idxY = 0; idxY <= Extent[3] - Extent[2]; idxY++)
		{
			// the start and end are included so we need to add 1 to the length
			idxX = idxZ * (Extent[3] - Extent[2] + 1) + idxY;
			this->ExtentListLengths[idxX] = 2;
			this->ExtentLists[idxX] = new int[2];
			this->ExtentLists[idxX][0] = xStart;
			this->ExtentLists[idxX][1] = xEnd;
		}
	 }
}

//----------------------------------------------------------------------------
// Generate sphere stencil from the center and radius
void vtkLHPImageStencilData::GenSphereStencil(vtkSphereSource * sphere)
//----------------------------------------------------------------------------
{
	Clear();

	// calculate bounding box (in float) and extents ( in integer)
	double center[3];

	sphere->GetCenter(center);
	double radius = sphere->GetRadius();
	
	double bbox[6];
	
	bbox[0] = bbox[2] = bbox[4] = 0;
	bbox[1] = bbox[3] = bbox[5] = 2 * radius;


	// extents should be larger than 0
	Extent[0] = int (bbox[0] / Spacing[0]);
	Extent[1] = int (bbox[1] / Spacing[0]);
	Extent[2] = int (bbox[2] / Spacing[1]);
	Extent[3] = int (bbox[3] / Spacing[1]);
	Extent[4] = int (bbox[4] / Spacing[2]);
	Extent[5] = int (bbox[5] / Spacing[2]);


	AllocateExtents();


	// generate range lists in a simple way
	int idxX, xStart, xEnd;
	// coordinates relative to the sphere center
	double xr, yr, zr;

	for (int idxZ = 0; idxZ <= Extent[5] - Extent[4]; idxZ++)
	{
		// z relative to the sphere center
		zr = idxZ * Spacing[2] - radius;
		
		for (int idxY = 0; idxY <= Extent[3] - Extent[2]; idxY++)
		{
			// y relative to the sphere center
			yr = idxY * Spacing[1]- radius;

			// xr*xr + yr*yr + zr*zr = r*r
			double xr_square = radius * radius - yr * yr - zr * zr;
			
			if (xr_square >=0 )
			{
				xr = sqrt(xr_square);
				
				xStart = int ((radius - xr)/Spacing[0] +0.5);
				xEnd = int ((radius + xr)/Spacing[0] + 0.5);

				idxX = idxZ * (Extent[3] - Extent[2] + 1) + idxY;
				this->ExtentListLengths[idxX] = 2;
				this->ExtentLists[idxX] = new int[2];
				this->ExtentLists[idxX][0] = xStart;
				this->ExtentLists[idxX][1] = xEnd;
			}
		}
	}
}
//----------------------------------------------------------------------------
void vtkLHPImageStencilData::StartRecordTranslation()
//----------------------------------------------------------------------------
{
	m_RecordedTranslation[0] = m_RecordedTranslation[1] = m_RecordedTranslation[2] = 0;
	m_bRecordTranslation = true;
}

//----------------------------------------------------------------------------
void vtkLHPImageStencilData::RollbackRecordedTranslation()
//----------------------------------------------------------------------------
{
	ApplyTranslation(-m_RecordedTranslation[0], -m_RecordedTranslation[1], -m_RecordedTranslation[2]);
	// redundant ??
	m_RecordedTranslation[0] =  m_RecordedTranslation[1] = m_RecordedTranslation[2] = 0;
	//m_bRecordTranslation = false;
}

//----------------------------------------------------------------------------
void vtkLHPImageStencilData::StopRecordTranslation()
//----------------------------------------------------------------------------
{
	//ApplyTranslation(-m_RecordedTranslation[0], -m_RecordedTranslation[1], -m_RecordedTranslation[2]);
	// redundant ??
	m_RecordedTranslation[0] =  m_RecordedTranslation[1] = m_RecordedTranslation[2] = 0;
	m_bRecordTranslation = false;
}


//----------------------------------------------------------------------------
// translate the stencil, avoiding regeneration of stencils
void vtkLHPImageStencilData::ApplyTranslation(int dx, int dy, int dz)
//----------------------------------------------------------------------------
{

	this->Extent[0] += dx;
	this->Extent[1] += dx;
	this->Extent[2] += dy;
	this->Extent[3] += dy;
	this->Extent[4] += dz;
	this->Extent[5] += dz;

	if (m_bRecordTranslation)
	{
		m_RecordedTranslation[0] += dx;
		m_RecordedTranslation[1] += dy;
		m_RecordedTranslation[2] += dz;
	}


	if (dx != 0)
	{
		// translate x
		for (int idx = 0; idx < this->NumberOfExtentEntries; idx++)
		{
			for (int len = 0; len < this->ExtentListLengths[idx]; len++)
			{
				this->ExtentLists[idx][len] += dx;
			}
		}
	}

	//m_Translation[0] = m_Translation[1] = m_Translation[2] = 0;
}

//----------------------------------------------------------------------------
void vtkLHPImageStencilData::Fill(unsigned char * ptr, int incr[3], unsigned char label, int boundExtent[6])
//----------------------------------------------------------------------------
{
	unsigned char * pVoxel;
	int idx, xStart, xEnd;

	// to remove extents out side the volume
	int fillExtent[6];
	for (int i = 0; i < 3; i++)
	{
		if (this->Extent[2 * i] < boundExtent[2 * i])
			fillExtent[ 2 * i] = boundExtent[2 * i];
		else 
			fillExtent[2 * i] = this->Extent[2 * i];

		if (this->Extent[2 * i + 1] > boundExtent[2 * i + 1])
			fillExtent[ 2 * i + 1] = boundExtent[2 * i + 1];
		else
			fillExtent[ 2 * i + 1] = this->Extent[2 * i + 1];
	}
	
	// left bottom corner
	unsigned char * pBaseVoxel = ptr + fillExtent[4] * incr[2] + fillExtent[2] * incr[1];

	for (int idZ = fillExtent[4] ; idZ <= fillExtent[5] ; idZ++)
	{
		pVoxel = pBaseVoxel;

		for (int idY = fillExtent[2]; idY <= fillExtent[3] ; idY++)
		{

			idx = ( idZ - Extent[4]) * (Extent[3] - Extent[2] + 1) + idY - Extent[2];

			for (int len = 0; len < this->ExtentListLengths[idx]; len+=2)
			{
				xStart = this->ExtentLists[idx][len] /*+ m_Translation[0]*/;
				xEnd = this->ExtentLists[idx][len + 1] /* + m_Translation[0]*/;

				// todo temporary handling
				if ((xEnd < boundExtent[0]) || (xStart > boundExtent[1]))
					continue;

				if (xStart < boundExtent[0])
					xStart = boundExtent[0];
				else if (xEnd > boundExtent[1])
					xEnd = boundExtent[1];
				
				lhpMemSet(pVoxel + xStart, label, xEnd - xStart + 1);
			}

			pVoxel += incr[1];

		}

		pBaseVoxel += incr[2];
	}

}