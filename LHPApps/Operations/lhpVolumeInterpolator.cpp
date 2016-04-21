/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpVolumeInterpolator.cpp,v $
  Language:  C++
  Date:      $Date: 2011-12-12 12:33:41 $
  Version:   $Revision: 1.1.1.1.2.2 $
  Authors:   Gianluigi Crimi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "lhpVolumeInterpolator.h"
#include "vtkDataSet.h"
#include "vtkPointData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredPoints.h"


#define r m_Weights[0]
#define s m_Weights[1]
#define t m_Weights[2]

#define cr (1 - m_Weights[0])
#define cs (1 - m_Weights[1])
#define ct (1 - m_Weights[2])

mafCxxTypeMacro(lhpVolumeInterpolator);

//----------------------------------------------------------------------------
double lhpVolumeInterpolator::InternalInterpolate(double (*valueFunction)(double)) 
//----------------------------------------------------------------------------
{
	double w[8];
	double retValue;
	int i,j,k;
	unsigned int id;

	w[0] = cr * cs * ct;
	w[1] = r * cs * ct;  
	w[2] = cr * s * ct;
	w[3] = r * s * ct;
	w[4] = cr * cs * t;
	w[5] = r * cs * t;
	w[6] = cr * s * t;
	w[7] = r * s * t;


	retValue = 0;
	for (k=0; k < 2; k++)
		for (j=0; j < 2; j++)
			for (i=0; i < 2; i++) 
			{
				id = m_Ids[i] + m_Ids[2 + j]*m_Dims[0] + m_Ids[4 + k]* m_Dims[0]*m_Dims[1];

				double pointValue = valueFunction ? valueFunction(m_Scalars->GetTuple1(id)) : m_Scalars->GetTuple1(id);

				retValue += w[i + 2*j + 4*k] * pointValue;
			}

	return retValue; 
};


void lhpVolumeInterpolator::SetVolume(vtkDataSet *volume)
{
	m_Volume=volume;
	m_IsRectiliearGrid = vtkRectilinearGrid::SafeDownCast(volume) ? true : false;
	m_Scalars = volume->GetPointData()->GetScalars();
	if (m_IsRectiliearGrid)
		vtkRectilinearGrid::SafeDownCast(volume)->GetDimensions(m_Dims);
	else
		vtkStructuredPoints::SafeDownCast(volume)->GetDimensions(m_Dims);
}

//----------------------------------------------------------------------------
double  lhpVolumeInterpolator::InterpolateSP(double pos[3] , double (*valueFunction)(double))
//----------------------------------------------------------------------------
{ 
	double idf[3];
	int i; 

	vtkStructuredPoints *volume=vtkStructuredPoints::SafeDownCast(m_Volume);

	double *spacing=volume->GetSpacing();
	double *origin=volume->GetOrigin();
	

	for (i=0; i<3; i++)
	{
		idf[i] = (pos[i] - origin[i]) / spacing[i];
		if (idf[i] < 0 || idf[i] > m_Dims[i] - 1)
			return 0;

		m_Ids[i*2+0] = (unsigned int) floor(idf[i]);
		m_Ids[i*2+1] = (unsigned int) ceil (idf[i]);

		m_Weights[i] = (idf[i] - m_Ids[i*2+0]); 
	}

	return InternalInterpolate(valueFunction); 
}


//----------------------------------------------------------------------------
int SearchIds(vtkDataArray *coords, unsigned int dim, double x, unsigned int *ids) 
//----------------------------------------------------------------------------
{
	unsigned int id;

	if (x < coords->GetTuple1(0) || x > coords->GetTuple1(dim - 1))
		return 1;

	ids[0] = 0;
	ids[1] = dim - 1;

	do 
	{
		id = (ids[0] + ids[1]) / 2;

		if ( x < coords->GetTuple1(id)) 
		{
			if (ids[1] == id) break;
			else ids[1] = id;
		}
		else
		{
			if ( x > coords->GetTuple1(id)) 
			{
				if (ids[0] == id) break;
				else ids[0] = id;
			}
			else 
			{
				// x == coords[id]
				ids[0] = ids[1] = id;
				break;
			}
		}
	} while (1);

	return 0;
}

//----------------------------------------------------------------------------
double lhpVolumeInterpolator::InterpolateRG(double pos[3], double (*valueFunction)(double)) 
//----------------------------------------------------------------------------
{ 

	vtkRectilinearGrid *volume = vtkRectilinearGrid::SafeDownCast(m_Volume);
	
	vtkDataArray *coords[3];

	coords[0]=volume->GetXCoordinates();
	coords[1]=volume->GetYCoordinates();
	coords[2]=volume->GetZCoordinates();

	int i; 
	
	for (i=0; i < 3; i++) 
	{
		if (SearchIds(coords[i], m_Dims[i], pos[i], m_Ids+i*2))
			return 0;

		if (m_Ids[i*2+0] == m_Ids[i*2+1])
			m_Weights[i] = 0;
		else
			m_Weights[i] = (pos[i] - coords[i]->GetTuple1(m_Ids[i*2+0])) / (coords[i]->GetTuple1(m_Ids[i*2+1]) - coords[i]->GetTuple1(m_Ids[i*2+0]));

	}

	return InternalInterpolate(valueFunction);
}

double lhpVolumeInterpolator::Interpolate(double pos[3], double (*valueFunction)(double) /*= NULL*/)
{
	if (m_IsRectiliearGrid)
		return InterpolateRG(pos,valueFunction);
	else
		return InterpolateSP(pos,valueFunction);
}
