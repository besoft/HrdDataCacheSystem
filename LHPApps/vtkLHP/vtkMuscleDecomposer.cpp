/*=========================================================================
Module:    $RCSfile: vtkMuscleDecomposer.cxx $
Language:  C++
Date:      $Date: 2011-05-03 21:10 $
Version:   $Revision: 0.1.0.0 $
Author:    David Cholt
Notes:
=========================================================================*/

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "vtkMuscleDecomposer.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkMuscleDecomposer, "$Revision: 0.2.0.0 $");
vtkStandardNewMacro(vtkMuscleDecomposer);

//// =========================================================================
//// Nested classes
//// =========================================================================

//// FIBER

//----------------------------------------------------------------------------
vtkMuscleDecomposer::Fiber::Fiber()
	//----------------------------------------------------------------------------
{
	BBoundary = false;
}

//----------------------------------------------------------------------------
vtkMuscleDecomposer::Fiber::~Fiber()
	//----------------------------------------------------------------------------
{
	PointIDs.clear();
}

//// POINT

int vtkMuscleDecomposer::Point::count = 0;

//----------------------------------------------------------------------------
vtkMuscleDecomposer::Point::Point(double *pCoord)
	//----------------------------------------------------------------------------
{
	DCoord[0] = pCoord[0]; //x
	DCoord[1] = pCoord[1]; //y
	DCoord[2] = pCoord[2]; //z

	BBoundary = false;
	BMuscle = false;
	TendonConnectionID = -1;
	interpolated = false;
	distToNext = 0;
	count++;
}

//----------------------------------------------------------------------------
vtkMuscleDecomposer::Point::~Point()
	//----------------------------------------------------------------------------
{
	count--;
}

//----------------------------------------------------------------------------
// Computes coordinates of point on interpolated curve, using given x-coordinate
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::Point::ComputeInterpolatedCurveCoord(double t, double * result, bool substract)
{
	if (!this->interpolated)
	{
		result[0] = result[1] = result[2] = 0;
		return;
	}
	double h = t;
	if (substract)
	{
		h = (t - this->t);
	}
	double u = 0;

	for (int axis = 0; axis < 3; axis++)
	{
		u = a[axis] * h * h * h + b[axis] * h * h + c[axis] * h + d[axis];
		result[axis] = u;
	}
}

//----------------------------------------------------------------------------
// Assigns normal to this Point
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::Point::SetNormal(double* normal)
{
	this->DNormal[0] = normal[0];
	this->DNormal[1] = normal[1];
	this->DNormal[2] = normal[2];
}

//// =========================================================================
//// Interpolator
//// =========================================================================

//----------------------------------------------------------------------------
vtkMuscleDecomposer::SplineInterpolator::SplineInterpolator(int splineType)
	//----------------------------------------------------------------------------
{
	this->splineType = splineType;
}

//----------------------------------------------------------------------------
vtkMuscleDecomposer::SplineInterpolator::~SplineInterpolator()
	//----------------------------------------------------------------------------
{
	derivations->clear();
	diagonal->clear();
	rightHand->clear();
	result->clear();
	subdiagonal.clear();
	superdiagonal.clear();
}

//----------------------------------------------------------------------------
// Interpolates a single fiber
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::SplineInterpolator::Interpolate(Fiber* fiber, vtkstd::vector<Point*> & points)
{
	this->fiber = fiber;
	this->pointCloud = points;
	n = fiber->PointIDs.size() - 2;

	BuildParameter();
	if (this->splineType != SPLINE_CONSTRAINED)
	{
		BuildEquation();
		SolveEquation();
		ComputeWeights();				// compute and assign a,b,c,d weights to input points
	}
	else
	{
		ComputeFibreDerivations();
		ComputeConstrainedWeights();
	}
}

//----------------------------------------------------------------------------
// Builds Spline interpolation parameter for all the points in the fibre, and
// associates them to the points in cloud.
// The parameter is not normalised to t=<0,1> and not squarerooted to avoid
// Division inperfection and unnecessery operations
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::SplineInterpolator::BuildParameter()
{
	double currDistance = 0;
	double segmentLength = 0;
	double prevDistance = 0;

	int currID = this->fiber->PointIDs[0];
	int prevID = 0;

	this->pointCloud[currID]->t = 0;

	for(int i = 1; i < (int)this->fiber->PointIDs.size(); i++)
	{
		prevID = currID;
		currID = this->fiber->PointIDs[i];

		segmentLength = sqrt(vtkMath::Distance2BetweenPoints(this->pointCloud[currID]->DCoord, this->pointCloud[prevID]->DCoord));
		currDistance = prevDistance + segmentLength;
		pointCloud[currID]->t = currDistance;
		prevDistance = currDistance;
	}
}

//----------------------------------------------------------------------------
// Builds Spline interpolation equations described in
// "Cubic Spline Interpolation, MAE 5093, Charles O’Neill, 28 May 2002"
// modified to be parametric as At = b (tridiagonal matrix equation set)
// for all three axes (X, Y, Z) in linear time.
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::SplineInterpolator::BuildEquation()
{
	// prev, this, next refer to values in matrix/vector, not curve point - not anymore
	double t_prev, t_this, t_next = 0;  // independent parameter (t)
	double u_prev, u_this, u_next = 0;  // dependant value (x, y or z value)
	double h_prev = 0;
	double h_this = 0;

	// Matrix A is the same for all 3 axes (calculation depends only on t)
	superdiagonal.resize(n); // the last one is actually unused
	subdiagonal.resize(n); // the first one is actually unused

	for (int axis = 0; axis < 3; axis++) // we have 3 axes, 3 right sides, and 3 diagonals (modified by solver)
	{
		diagonal[axis].resize(n);
		rightHand[axis].resize(n);
	}

	for (int i = 0; i < n; i++)
	{
		t_prev = pointCloud[fiber->PointIDs[i]]->t;
		t_this = pointCloud[fiber->PointIDs[i+1]]->t;
		t_next = pointCloud[fiber->PointIDs[i+2]]->t;
		h_prev = t_this - t_prev;
		h_this = t_next - t_this;

		// matrices are the same for both axes, we can do them in one pass
		// refer to paper above for details
		// because we modify diagonal in solving process, it is copied for every axis
		for (int axis = 0; axis < 3; axis++)
		{
			diagonal[axis][i] = 2 * (h_prev + h_this) ;
			// different spline types have different border conditions
			if ((i == 0 || i == n - 1))
			{
				switch (this->splineType)
				{
					case SPLINE_CUBIC_RUNOUT:

						if (i == 0)
						{
							diagonal[axis][i] = 2 * (h_prev + h_this) + 2 * h_prev;
							superdiagonal[0] = 0;
						}
						else
						{
							diagonal[axis][i] = 2 * (h_prev + h_this) + 2 * h_this;
							subdiagonal[subdiagonal.size() - 1] = 0;
						}
						break;
					case SPLINE_PARABOLIC_RUNOUT:
						if (i == 0)
						{
							diagonal[axis][i] = 2 * (h_prev + h_this)+ h_prev;
						}
						else
						{
							diagonal[axis][i] = 2 * (h_prev + h_this) + h_this;
						}
						break;
					case SPLINE_NATURAL:
					default:
						break;
				}
			}
		}
		subdiagonal[i] = h_prev;
		superdiagonal[i] = h_this;

		// right hand vectors differ for each axis, we need three separate passes
		for (int axis = 0; axis < 3; axis++)
		{
			u_prev = pointCloud[fiber->PointIDs[i]]->DCoord[0 + axis];
			u_this = pointCloud[fiber->PointIDs[i+1]]->DCoord[0 + axis];
			u_next = pointCloud[fiber->PointIDs[i+2]]->DCoord[0 + axis];
			rightHand[axis][i] = ((u_next - u_this) / h_this - (u_this - u_prev) / h_prev) * 6;
		}
		// equation built for 3 axes and with low memory usage
	}
}

// Solves equation for all axes using TDMA algorithm
void vtkMuscleDecomposer::SplineInterpolator::SolveEquation()
{
	for (int axis = 0; axis < 3; axis++) // we have 3 axes
	{
		result[axis].resize(n);

		// super: accessed on indices 0..n-2 (n-1 is max index)
		// sub: accessed on indices 1..n-1 (n-1 is max index)
		double m;

		for (int i = 1; i < n; i++) // modify coefficients, forward pass
		{
			m = subdiagonal[i]/diagonal[axis][i-1];
			diagonal[axis][i] = diagonal[axis][i] - m * superdiagonal[i-1];
			rightHand[axis][i] = rightHand[axis][i] - m * rightHand[axis][i-1];
		}

		result[axis][n-1] = rightHand[axis][n-1]/diagonal[axis][n-1];

		for (int i = n - 2; i >= 0; i--) // indexed from zero, solve equations, backward pass
			result[axis][i]=(rightHand[axis][i] - superdiagonal[i] * result[axis][i+1]) / diagonal[axis][i];

		// result[axis] now holds S_i .. S_n
		diagonal[axis].clear();
		rightHand[axis].clear();
	}
	superdiagonal.clear();
	subdiagonal.clear();
}

// Computes weights for all 3 axes
void vtkMuscleDecomposer::SplineInterpolator::ComputeWeights()
{
	int ID;
	// next rows now refer to position in curve
	double S_this, S_next;	// computed by TDMA solver
	double t_this, t_next;  // computed in BuildParameter
	double u_this, u_next;  // coords of interpolated points
	double h;

	double S2, S3, SNm1, SNm2;	// for boundary conditions

	// First we add end point variables to the result
	for(int axis = 0; axis < 3; axis++)
	{
		switch (this->splineType)
		{
		case SPLINE_NATURAL: // natural splines have S=0 for end points
			result[axis].insert(result[axis].begin(), 0);
			result[axis].push_back(0);
			break;

		case SPLINE_PARABOLIC_RUNOUT: // parabolic runout splines have S equal to next(previous) S on end points
			S2 = result[axis][0];
			result[axis].insert(result[axis].begin(), S2); // first point copy
			SNm1 = result[axis][result[axis].size()-1];
			result[axis].push_back(SNm1); // last point copy
			break;
		case SPLINE_CUBIC_RUNOUT: // cubic runout splines have S1 = 2*S2 - S3 and SN = 2*SN-1 - SN-2
			S2 = result[axis][0];
			S3 = result[axis][1];
			result[axis].insert(result[axis].begin(), 2*S2 - S3); // first point calc
			SNm1 = result[axis][result[axis].size()-1];
			SNm2 = result[axis][result[axis].size()-2];
			result[axis].push_back(2*SNm1 - SNm2); // last point calc
			break;
		}
	}

	// Next, we compute weights for all points in fibre
	for(int i = 0; i < (int)fiber->PointIDs.size() - 1; i++) // last point doesnt have it's segment
	{
		ID = fiber->PointIDs[i];
		for(int axis = 0; axis < 3; axis++)
		{
			// again refer to paper above for details
			S_this = result[axis][i];
			S_next = result[axis][i+1];

			t_this = pointCloud[fiber->PointIDs[i]]->t;
			t_next = pointCloud[fiber->PointIDs[i+1]]->t;
			h = t_next - t_this;

			u_this = pointCloud[fiber->PointIDs[i]]->DCoord[axis];
			u_next = pointCloud[fiber->PointIDs[i+1]]->DCoord[axis];

			pointCloud[ID]->a[axis] = (S_next - S_this) / (6 * h);
			pointCloud[ID]->b[axis] = S_this / 2;
			pointCloud[ID]->c[axis] = ((u_next - u_this) / h) - ((2 * h * S_this + h * S_next) / 6);
			pointCloud[ID]->d[axis] = u_this;
			pointCloud[ID]->distToNext = h;
		}
		pointCloud[ID]->interpolated = true;
	}
}

// Calculates derivation aproximation of fibre data derivatives in all axes
// with respect to parameter t
void vtkMuscleDecomposer::SplineInterpolator::ComputeFibreDerivations()
{
	int size = this->fiber->PointIDs.size();
	double t_prev, t_this, t_next = 0;  // independent parameter (t)
	double u_prev, u_this, u_next = 0;  // dependant value (x, y or z value)
	double h_prev, h_this = 0;
	double h_u_prev, h_u_this = 0;
	double derivation;

	for (int axis = 0; axis < 3; axis++)
	{
		derivations[axis].resize(size);
		for (int i = 1; i < size - 1; i++)  // border derivations will be calculated later
		{
			t_prev = pointCloud[fiber->PointIDs[i-1]]->t;
			t_this = pointCloud[fiber->PointIDs[i]]->t;
			t_next = pointCloud[fiber->PointIDs[i+1]]->t;
			h_prev = t_this - t_prev;
			h_this = t_next - t_this;

			u_prev = pointCloud[fiber->PointIDs[i-1]]->DCoord[axis];
			u_this = pointCloud[fiber->PointIDs[i]]->DCoord[axis];
			u_next = pointCloud[fiber->PointIDs[i+1]]->DCoord[axis];
			h_u_prev = u_this - u_prev;
			h_u_this = u_next - u_this;

			double temp = (h_this/h_u_this) * (h_prev/h_u_prev);
			// if sope sign changes at the point, the derivation should be = 0
			if (temp > 0)
			{
				derivation = 2 / ((h_this/h_u_this) + (h_prev/h_u_prev));
			} else
			{
				derivation = 0;
			}

			derivations[axis][i] = derivation;
		}
		// border derivation calc on fibre start
		t_this = pointCloud[fiber->PointIDs[0]]->t;
		t_next = pointCloud[fiber->PointIDs[1]]->t;
		h_this = t_next - t_this;
		u_this = pointCloud[fiber->PointIDs[0]]->DCoord[axis];
		u_next = pointCloud[fiber->PointIDs[1]]->DCoord[axis];
		h_u_this = u_next - u_this;

		derivations[axis][0] = ((3 * h_u_this) / (2 * h_this)) - (derivations[axis][1] / 2);
		// border derivation calc on fibre end
		t_this = pointCloud[fiber->PointIDs[size-1]]->t;
		t_prev = pointCloud[fiber->PointIDs[size-2]]->t;
		h_prev = t_this - t_prev;
		u_this = pointCloud[fiber->PointIDs[size-1]]->DCoord[axis];
		u_prev = pointCloud[fiber->PointIDs[size-2]]->DCoord[axis];
		h_u_prev = u_this - u_prev;

		derivations[axis][size-1] = ((3 * h_u_prev) / (2 * h_prev)) - (derivations[axis][size - 2] / 2);
	}
	vtkstd::vector<double> x = derivations[0];;
	//x = //.begin(), derivation[0].end());
	vtkstd::vector<double> y = derivations[1];
}

// Calculates parameters a, b, c, d for constrained splines
// as described in "Constrained Cubic Spline Interpolation
// for Chemical Engineering Applications" by CJC Kruger
// As the first derivatives are known, we dont need to solve any equations.
// (second derivatives are computed from first derivatives)
void vtkMuscleDecomposer::SplineInterpolator::ComputeConstrainedWeights()
{
	int ID;
	double t_this, t_next = 0;  // independent parameter (t)
	double u_this, u_next = 0;  // dependant value (x, y or z value)
	double h_this = 0;
	double h_u_this = 0;
	double this_2ndDerivation;
	double next_2ndDerivation;
	double a, b, c, d;

	for(int i = 0; i < (int)fiber->PointIDs.size() - 1; i++) // last point doesnt have it's segment
	{
		ID = fiber->PointIDs[i];

		for(int axis = 0; axis < 3; axis++)
		{
			// refer to paper above for details
			t_next = pointCloud[fiber->PointIDs[i+1]]->t;
			t_this = pointCloud[fiber->PointIDs[i]]->t;

			h_this = t_next - t_this;

			u_next = pointCloud[fiber->PointIDs[i+1]]->DCoord[axis];
			u_this = pointCloud[fiber->PointIDs[i]]->DCoord[axis];

			h_u_this = u_next - u_this;

			this_2ndDerivation = (-2 * (derivations[axis][i+1] + 2*derivations[axis][i]) / (h_this)) + (6*(h_u_this) / (h_this * h_this));
			next_2ndDerivation = (2 * (2 * derivations[axis][i+1] + derivations[axis][i]) / (h_this)) - (6*(h_u_this) / (h_this * h_this));

			d = (next_2ndDerivation - this_2ndDerivation) / (6 * h_this);
			c = (t_next * this_2ndDerivation - t_this * next_2ndDerivation ) / (2 * h_this);
			b = (h_u_this - c*(t_next*t_next - t_this*t_this) - d*(t_next*t_next*t_next - t_this*t_this*t_this))/(h_this);
			a = u_this - (b*t_this) - (c * t_this * t_this) - (d * t_this * t_this * t_this);

			pointCloud[ID]->a[axis] = d;
			pointCloud[ID]->b[axis] = c;
			pointCloud[ID]->c[axis] = b;
			pointCloud[ID]->d[axis] = a;
			pointCloud[ID]->distToNext = h_this;
		}
		pointCloud[ID]->interpolated = true;
	}
}

int vtkMuscleDecomposer::SplineInterpolator::GetInterpolationType()
{
	return this->splineType;
}

//// =========================================================================
//// Support methods
//// =========================================================================

//----------------------------------------------------------------------------
// vtkMuscleDecomposer Constructor
//----------------------------------------------------------------------------
vtkMuscleDecomposer::vtkMuscleDecomposer()
{
	 fiberCount = 0;
	 tendonCount = 0;
	 TendonConnectionMethod = CONNECT_INTENDED_TENDON_POINT;
	 InterpolationMethod = SPLINE_CONSTRAINED;
	 InterpolationSubdivision = 10;
	 SurfaceSubdivision = 10;
	 ArtifactEpsilon = 10.0;
}

//----------------------------------------------------------------------------
// vtkMuscleDecomposer Destructor
//----------------------------------------------------------------------------
vtkMuscleDecomposer::~vtkMuscleDecomposer()
{
	ClearMesh();
}

//----------------------------------------------------------------------------
// Add Fibers input. Fiber data has to be sorted (from one end to another)
// and adding of whole fibers has to be in order from "left to right" also
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::AddFiberData(vtkPolyData *Data)
{
	AddGenericData(Fibers, Data);
	fiberCount++;
}

//----------------------------------------------------------------------------
// Add Tendons input. Tendon data has to be sorted (probably, for future creation methods :D)
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::AddTendonData(vtkPolyData *Data)
{
	AddGenericData(Tendons, Data);
	tendonCount++;
}

//----------------------------------------------------------------------------
// Adds data to correct list
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::AddGenericData( vtkstd::vector<Fiber*> & Where, vtkPolyData * Data )
{
	// TODO: fibreID is incorrect?
	double pCoord[3];		// Point coords
	Point* pt;				// Point in internal structure
	int pointCount = Data->GetNumberOfPoints(); // Number of added points
	int PointID;			// ID of currently processed point

	int WhereIndex = Where.size();
	Where.push_back(new Fiber);

	PointCloud.reserve(PointCloud.size() + pointCount);

	// For all points in Fiber
	for (int i = 0; i < pointCount; i++)
	{
		Data->GetPoint(i, pCoord); // get point from data
		pt = new Point(pCoord);

		PointCloud.push_back(pt);
		PointID = PointCloud.size() - 1; // get its global id (in cloud)

		pt->Id = PointID;
		pt->BBoundary = (i == 0 || i == pointCount - 1);

		Where[WhereIndex]->PointIDs.push_back(PointID); // add point id to specified fiber vector
		Where[WhereIndex]->BBoundary = true;

		if (WhereIndex > 2)
		{
			Where[fiberCount - 1]->BBoundary = false; // if fiber vector contains more than 3 fibers, the fibers in middle are not boundary
		}
	}
}

void vtkMuscleDecomposer::AddMeshData(vtkPolyData *Mesh)
{
	this->MuscleMesh = vtkPolyData::New();
	this->MuscleMesh->DeepCopy(Mesh);
}

//----------------------------------------------------------------------------
// Splits polydata to polylines in internal structures
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::LoadFiber(vtkPolyData *data, vtkstd::vector<Fiber*> & Polyline)
{
	// TODO: For muscle area determining - coloring
}

//----------------------------------------------------------------------------
// Computes distance between two PointIDs
//----------------------------------------------------------------------------
double vtkMuscleDecomposer::ComputePointDistance( int ID1, int ID2 )
{
	Point* First = PointCloud[ID1];
	Point* Second = PointCloud[ID2];

	return vtkMath::Distance2BetweenPoints(First->DCoord, Second->DCoord);
}

//----------------------------------------------------------------------------
// Connects Fibers to closest tendron PointIDs
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::ConnectClosestTendons()
{
	Log("  Connecting closest tendons...");
	int firstID, lastID;
	double MinDistanceFirst = VTK_DOUBLE_MAX;
	double MinDistanceLast = VTK_DOUBLE_MAX;
	double currentDistance;

	// Go trough all the fibers
	for (int fiber = 0; fiber < (int)Fibers.size(); fiber++)
	{
		// Find out the start and end points
		firstID = Fibers[fiber]->PointIDs[0];
		lastID = Fibers[fiber]->PointIDs[Fibers[fiber]->PointIDs.size() - 1];
		MinDistanceFirst = VTK_DOUBLE_MAX;
		MinDistanceLast = VTK_DOUBLE_MAX;

		// go trough all tendons fibers
		for (int j = 0; j < (int)Tendons.size(); j++)
		{
			// and all their points
			for (int k = 0; k < (int)Tendons[j]->PointIDs.size(); k++)
			{
				// measure distance to first point
				currentDistance = ComputePointDistance(firstID, Tendons[j]->PointIDs[k]);
				if (currentDistance < MinDistanceFirst)
				{
					MinDistanceFirst = currentDistance; // if smaller, connect
					PointCloud[firstID]->TendonConnectionID = Tendons[j]->PointIDs[k];
				}

				// and last point
				currentDistance = ComputePointDistance(lastID, Tendons[j]->PointIDs[k]);
				if (currentDistance < MinDistanceLast)
				{
					MinDistanceLast = currentDistance; // if smaller, connect
					PointCloud[lastID]->TendonConnectionID = Tendons[j]->PointIDs[k];
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
// Connects Fibers to closest tendron PointIDs
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::ConnectSortedTendons()
{
	Log("  Connecting tendons by order in data structure...");
	int firstID, lastID;
	int startTendonPointID, endPointTendonID;
	// First, we have to figure out, which tendon is on start and which on end.
	// I use proximity of the line conecting first and last points (outer points) of first
	// tendon to the line connecting first/last points of outer! fibres. Lesser distance
	// from fibre start means it first points of fibers belong to the first tendon, otherwise
	// last points of fibre belong to first tendon.

	// Tendon outer points
	double* tendonPoint1 = PointCloud[this->Tendons[0]->PointIDs[0]]->DCoord;
	double* tendonPoint2 = PointCloud[this->Tendons[0]->PointIDs[this->Tendons[0]->PointIDs.size()-1]]->DCoord;
	// Outer fibres start points
	double* fibrePoint11 = PointCloud[this->Fibers[0]->PointIDs[0]]->DCoord;
	double* fibrePoint12 = PointCloud[this->Fibers[this->Fibers.size()-1]->PointIDs[0]]->DCoord;
	// Outer fibres end points
	double* fibrePoint21 = PointCloud[this->Fibers[0]->PointIDs[this->Fibers[0]->PointIDs.size()-1]]->DCoord;
	double* fibrePoint22 = PointCloud[this->Fibers[this->Fibers.size()-1]->PointIDs[this->Fibers[0]->PointIDs.size()-1]]->DCoord;

	double distance1 = LineLineDistance(tendonPoint1, tendonPoint2, fibrePoint11, fibrePoint12);
	double distance2 = LineLineDistance(tendonPoint1, tendonPoint2, fibrePoint21, fibrePoint22);

	int startTendon = 0;
	if (distance1 > distance2) // tendon is closer to end of fibres
	{
		startTendon++;
	}

	int tendonPointCountStart = this->Tendons[startTendon]->PointIDs.size();
	int tendonPointCountEnd = this->Tendons[(startTendon + 1) % 2]->PointIDs.size();

	float ratioStart = (float)tendonPointCountStart / Fibers.size();
	float ratioEnd = (float)tendonPointCountEnd / Fibers.size();

	// Since the fibres and tendon points are sorted, we can assign them directly
	for (int fiber = 0; fiber < (int)Fibers.size(); fiber++)
	{
		firstID = Fibers[fiber]->PointIDs[0];
		lastID = Fibers[fiber]->PointIDs[Fibers[fiber]->PointIDs.size() - 1];
		startTendonPointID = vtkMath::Round(fiber * ratioStart);
		endPointTendonID = vtkMath::Round(fiber * ratioEnd);

		PointCloud[firstID]->TendonConnectionID = this->Tendons[startTendon]->PointIDs[startTendonPointID];
		PointCloud[lastID]->TendonConnectionID = this->Tendons[(startTendon + 1) % 2]->PointIDs[endPointTendonID];
	}
}

//----------------------------------------------------------------------------
// Outputs a Fiber vector
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::OutputFibersData( vtkstd::vector<Fiber*> & Data, vtkPoints * PointIDs, vtkCellArray * lines )
{
	vtkIdType * lineIDs = new vtkIdType[2]; // delete in here
	vtkIdType oldpoint, newpoint;
	oldpoint = 0;

	// go trough all data ready for output

	for (int i = 0; i < (int)Data.size(); i++)
	{
		for (int j = 0; j < (int)Data[i]->PointIDs.size(); j++)
		{
			// insert point to output
			newpoint = PointIDs->InsertNextPoint(PointCloud[Data[i]->PointIDs[j]]->DCoord);
			if (j > 0)
			{
				// if at point > 0, create polyline
				lineIDs[0] = oldpoint;
				lineIDs[1] = newpoint;
				lines->InsertNextCell(2, lineIDs);
			}
			oldpoint = newpoint;
		}
	}
	delete[] lineIDs;
	lineIDs = NULL;
}

//// =========================================================================
//// Interpolation methods
//// =========================================================================

void vtkMuscleDecomposer::InterpolateSurface(int subdivision, int interpolationType, vtkstd::vector<Fiber*> & Data)
{
	LogFormated("  Interpolating surface points using %s method...", GetSplineType(interpolationType));

	Fiber* crossPoints; // we are interpolating surface across all fibers - these are across all fibers
	vtkstd::vector<Fiber*> NewFibers;
	int fiberCount = Data.size();
	int pointCount = Data.at(0)->PointIDs.size();
	int interpolatedIndex;

	NewFibers.reserve(fiberCount * subdivision);

	SplineInterpolator* interpolator = new SplineInterpolator(interpolationType); // delete in here

	for (int j = 0; j < (int)Data.at(0)->PointIDs.size(); j++)
	{
		crossPoints = new Fiber(); // delete in here
		crossPoints->PointIDs.reserve(Data.size());
		for (int i = 0; i < fiberCount; i++)
		{
			crossPoints->PointIDs.push_back(Data.at(i)->PointIDs[j]);
		}

		InterpolateFiber(crossPoints, subdivision, interpolator, 0, false);

		for (int i = 0; i < (int)crossPoints->PointIDs.size(); i++)
		{
			if ((int)NewFibers.size() <= i)
			{
				NewFibers.push_back(new Fiber()); // delete later in ClearMesh()
			}
			NewFibers[i]->PointIDs.push_back(crossPoints->PointIDs[i]);
		}

		crossPoints->PointIDs.clear();
		delete crossPoints;
		crossPoints = NULL;
	}
	NewFibers[0]->BBoundary = true;
	NewFibers[NewFibers.size()-1]->BBoundary = true;
	// we need to interpolate the tendon connection settings also
	for(int i = 0; i < (int)NewFibers.size(); i++)
	{
		interpolatedIndex = i/subdivision;
		PointCloud[NewFibers[i]->PointIDs[0]]->TendonConnectionID = PointCloud[Data.at(interpolatedIndex)->PointIDs[0]]->TendonConnectionID;
		PointCloud[NewFibers[i]->PointIDs[pointCount-1]]->TendonConnectionID = PointCloud[Data.at(interpolatedIndex)->PointIDs[pointCount-1]]->TendonConnectionID;
	}

	Data.clear();
	Data.assign(NewFibers.begin(), NewFibers.end());
	delete interpolator;
	interpolator = NULL;
}

void vtkMuscleDecomposer::RemoveArtifacts(double epsilon, vtkstd::vector<Fiber*> & data)
{
	Log("  Removing possible data artiffacts...");
	int prevID = 0;
	int currID = 0;
	double segmentLength = 0;
	for(int i = 0; i < (int)Fibers.size(); i++)
	{
		int j = 0;
		while (++j < (int)data[i]->PointIDs.size())
		{
			prevID = data[i]->PointIDs[j-1];
			currID = data[i]->PointIDs[j];

			segmentLength = PointPointDistance(this->PointCloud[currID]->DCoord, this->PointCloud[prevID]->DCoord);
			if (segmentLength < epsilon)
				data[i]->PointIDs.erase(data[i]->PointIDs.begin() + j );
		}
	}
}

//----------------------------------------------------------------------------
// Interpolates all the fibers in Outputfibers
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::InterpolateFibers(int subdivision, int interpolationType, vtkstd::vector<Fiber*> & Data, bool tendons)
{
	if(tendons)
	{
		LogFormated("  Interpolating tendons using %s method...", GetSplineType(interpolationType));
	} else
	{
		LogFormated("\n  Interpolating all individual fibres using %s method...", GetSplineType(interpolationType));
	}
	SplineInterpolator* interpolator = new SplineInterpolator(interpolationType); // delete in here
	for (int i = 0; i < (int)Data.size(); i++)
	{
		InterpolateFiber(Data[i], subdivision, interpolator, 0, false);
	}
	delete interpolator;
	interpolator = NULL;
}

void vtkMuscleDecomposer::InterpolateFiber(Fiber* fiber, int subdivision, SplineInterpolator *interpolator, int skipOnEnds, bool insertSingularity)
{
	double* pCoord = new double[3]; // delete in here
	Point* newPoint;
	int ID, nextID, newID;
	double distance;
	double step;
	double oldT;
	// we call interpolator to calculate the weights
	interpolator->Interpolate(fiber, PointCloud);

	// reserve memory for faster insertion
	int increase = (int)(fiber->PointIDs.size() * subdivision);
	PointCloud.reserve(PointCloud.size() + increase);
	fiber->PointIDs.reserve(fiber->PointIDs.size() + increase);

	// now we can call point->ComputeInterpolatedCurveCoord(x) for desired x
	for (int j = skipOnEnds; j < (int)fiber->PointIDs.size() - 1 - skipOnEnds; j++)
	{
		ID = fiber->PointIDs[j];
		nextID = fiber->PointIDs[j+1];
		distance = PointCloud[ID]->distToNext;
		step = distance / subdivision;
		//distance *= 0.99;
		oldT = PointCloud[ID]->t;

		if (distance == 0 && insertSingularity) // we need to maintain point count, so we insert copies of the start point even when distance == 0
		{
			for(int k = 0; k < subdivision - 1;k++)
			{
				//PointCloud[ID]->ComputeInterpolatedCurveCoord(oldT, pCoord, interpolator->GetInterpolationType() != SPLINE_CONSTRAINED);
				pCoord = PointCloud[ID]->DCoord;
				newPoint = new Point(pCoord); //delete in ClearMesh()

				// insert it to cloud
				PointCloud.push_back(newPoint);
				newID = PointCloud.size() - 1;
				newPoint->Id = newID;
				// and to the list of IDs for this fibre
				j++;
				fiber->PointIDs.insert(fiber->PointIDs.begin() + j, newID);
			}
		}
		else
		{
			double newT = oldT;
			for (int l = 0; l < subdivision - 1; l++) // more stable
			//for (double newT = oldT + step; abs(newT - oldT) < abs(distance); newT += step)
			{
				newT += step;
				// compute point position

				PointCloud[ID]->ComputeInterpolatedCurveCoord(newT, pCoord, interpolator->GetInterpolationType() != SPLINE_CONSTRAINED);

				newPoint = new Point(pCoord); //delete in ClearMesh()

				// insert it to cloud
				PointCloud.push_back(newPoint);
				newID = PointCloud.size() - 1;
				newPoint->Id = newID;
				// and to the list of IDs for this fibre
				j++;
				fiber->PointIDs.insert(fiber->PointIDs.begin() + j, newID);
			}
		}
	}
	delete[] pCoord;
	pCoord = NULL;
}

//// =========================================================================
//// Filter execution methods
//// =========================================================================

//----------------------------------------------------------------------------
// Initializes mesh data, determines what part of data is muscle part of it
// (opposed to tendon data). This is for later rendering/export purposes
//----------------------------------------------------------------------------
bool vtkMuscleDecomposer::InitData()
{
	Log("Initializing precise data...");
	OutputMesh = this->GetOutput(); // Link Output

	Log("  Fiber polylines: ", (int)fiberCount);
	Log("  Tendon polylines: ", (int)tendonCount);

	Log("  All point count: ", (int)(PointCloud.size()));

	// TODO: Determine muscle data
	Log("Data initialized.\n");
	return true;
}

//----------------------------------------------------------------------------
// Executes muscle decompositor
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::Execute()
{
	clock_t whole; // benchmark
	whole = clock();

	Log("Source executed.\n");
	if (!InitData()) {
		Log("Execution halted! Missing some data!");
		return;
	}
	BuildMesh();
	DoneMesh();
	ClearMesh();
	Log("Execution successful.");
	Log("Execution time [ms]: ", (int)(clock() - whole));
}

//----------------------------------------------------------------------------
// Builds the internal relationship of fibers and tendons, builds output data
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::BuildMesh()
{
	Log("Decomposing muscle...");

	RemoveArtifacts(ArtifactEpsilon, Fibers);
	AlignMesh();

	InterpolateFibers(SurfaceSubdivision, InterpolationMethod, Tendons, true);
	InterpolateSurface(SurfaceSubdivision, InterpolationMethod, Fibers);
	switch (TendonConnectionMethod)
	{
	case CONNECT_CLOSEST_TENDON_POINT: // OBSOLETE??

		ConnectClosestTendons();
		break;

	case CONNECT_INTENDED_TENDON_POINT:
		ConnectSortedTendons();
		break;
	}

	ComputeNormals();
	ComputeThickness();

	BuildBottomFibers(1);

	BuildSideLayers(SurfaceSubdivision / 3, InterpolationMethod);
	Log("");
	BuildAndOutputConnectedData(Fibers, "top fiber");
	BuildAndOutputConnectedData(LeftFibers, "left side fiber");
	BuildAndOutputConnectedData(RightFibers, "right side fiber");
	BuildAndOutputConnectedData(BottomFibers, "bottom fiber");

	for (float i = 1.0f/(SurfaceSubdivision); i < 1; i+=1.0f/(SurfaceSubdivision))
	{
		BuildBottomFibers(i);
		BuildAndOutputConnectedData(BottomFibers, "interior layer fiber");
	}

	InterpolateFibers(InterpolationSubdivision, InterpolationMethod, OutputFibers, false);

	Log("Muscle built.\n");
}

//----------------------------------------------------------------------------
// Builds OutputFibers. Physically connects the tendon points.
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::BuildAndOutputConnectedData(vtkstd::vector<Fiber*> & Data, char* what)
{
	LogFormated("  Building connected %s data...", what);
	int firstID;
	int lastID;

	int outputSize = OutputFibers.size();
	for (int i = 0; i < (int)Data.size(); i++)
	{
		// create output fiber
		OutputFibers.push_back(new Fiber); //delete in ClearMesh()
		// get ID of connections
		firstID = PointCloud[Data[i]->PointIDs[0]]->TendonConnectionID;
		lastID = PointCloud[Data[i]->PointIDs[Data[i]->PointIDs.size() - 1]]->TendonConnectionID;

		if (firstID > -1)
			 // push first connected tendon point
			OutputFibers[outputSize+i]->PointIDs.push_back(PointCloud[firstID]->Id);

		// then Fiber data
		OutputFibers[outputSize+i]->PointIDs.insert(OutputFibers[outputSize+i]->PointIDs.end(), Data[i]->PointIDs.begin(), Data[i]->PointIDs.end());

		if (lastID > -1)
			// then push second connected tendon point
			OutputFibers[outputSize+i]->PointIDs.push_back(PointCloud[lastID]->Id);
	}
}

//----------------------------------------------------------------------------
// Extract data from internal structure and put them to output in vtkPolyData
// format
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::DoneMesh()

{
	Log("Creating data on output...");
	vtkPoints * PointIDs = vtkPoints::New(); //delete in here
	vtkCellArray * lines = vtkCellArray::New(); //delete in here
	OutputFibersData(OutputFibers, PointIDs, lines);

	OutputMesh->SetPoints(PointIDs);
	OutputMesh->SetLines(lines);

	PointIDs->Delete();
	PointIDs = NULL;
	lines->Delete();
	lines = NULL;
#ifdef debug
	OutputNormals();
#endif

	Log("Mesh done.\n");
}

//----------------------------------------------------------------------------
// Clear data from memory
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::ClearMesh()
{
	Log("Clearing data...");
	for (int i = 0; i < (int)Fibers.size(); i++)
	{
		Fibers[i]->PointIDs.clear();
		delete Fibers[i];
		Fibers[i] = NULL;
	}
	Fibers.clear();

	for (int i = 0; i < (int)Tendons.size(); i++)
	{
		Tendons[i]->PointIDs.clear();
		delete Tendons[i];
		Tendons[i] = NULL;
	}
	Tendons.clear();

	for (int i = 0; i < (int)OutputFibers.size(); i++)
	{
		OutputFibers[i]->PointIDs.clear();
		delete OutputFibers[i];
		OutputFibers[i] = NULL;
	}
	OutputFibers.clear();

	for (int i = 0; i < (int)BottomFibers.size(); i++)
	{
		BottomFibers[i]->PointIDs.clear();
		delete BottomFibers[i];
		BottomFibers[i] = NULL;
	}
	BottomFibers.clear();

	for (int i = 0; i < (int)LeftFibers.size(); i++)
	{
		LeftFibers[i]->PointIDs.clear();
		delete LeftFibers[i];
		LeftFibers[i] = NULL;
	}

	LeftFibers.clear();

	for (int i = 0; i < (int)RightFibers.size(); i++)
	{
		RightFibers[i]->PointIDs.clear();
		delete RightFibers[i];
		RightFibers[i] = NULL;
	}
	RightFibers.clear();

	for (int i = 0; i < (int)PointCloud.size(); i++)
	{
		delete PointCloud[i]; // everything inside PointCloud[i] is static...
		PointCloud[i] = NULL;
	}
	PointCloud.clear();

	#ifndef debug
	if (MuscleMesh)
	{
		MuscleMesh->Delete();
		MuscleMesh = NULL;
	}
	#endif

	Log("Mesh cleared.\n");
}

// Moves the muscle mesh so its centered in the fibre-defined muscle hull
void vtkMuscleDecomposer::AlignMesh()
{
	Log("  Aligning muscle mesh to precise data...");
	double pCoord[3];		// Triangle vertices coords
	double result[3];

	double MaxDistance = DBL_MIN;
	double MinDistance = DBL_MAX;
	int MaxDistanceIndex = 0;
	int MinDistanceIndex = 0;

	if (MuscleMesh == NULL)
		return;
	Log("    Computing direction...");
	int fiberPointCount = 0;

	for (int i = 0; i < (int)Fibers.size(); i++)
	{
		fiberPointCount += Fibers[i]->PointIDs.size();
	}

	int* closestPoints = new int [fiberPointCount]; //delete in here
	double* distances = new double [fiberPointCount]; //delete in here
	vtkstd::vector<int> fiberPointIDS; //delete in here

	for (int i = 0; i < fiberPointCount; i++)
	{
		distances[i] = DBL_MAX;
	}

	fiberPointIDS.reserve(fiberPointCount);
	for (int j = 0; j < (int)Fibers.size(); j++)
	{
		for (int k = 0; k < (int)Fibers[j]->PointIDs.size(); k++)
		{
			fiberPointIDS.push_back(Fibers[j]->PointIDs[k]);
		}
	}

	int NumOfPoints = MuscleMesh->GetNumberOfPoints();
	for (int i = 0; i < NumOfPoints; i++)
	{
		MuscleMesh->GetPoint(i, pCoord);
		for (int j = 0; j < fiberPointCount; j++)
		{
			double distance = PointPointDistance(pCoord, PointCloud[fiberPointIDS[j]]->DCoord);
			if (distance < distances[j])
			{
				distances[j] = distance;
				closestPoints[j] = i;
			}
		}
	}
	/*
	result[0] = result[1] = result[2] = 0;
	for (int i = 0; i < fiberPointCount; i++)
	{
		DCUtils::Substract(PointCloud[fiberPointIDS[i]]->DCoord, MuscleMesh->GetPoint(closestPoints[i]), substract);
		result[0] += substract[0];
		result[1] += substract[1];
		result[2] += substract[2];
		if (distances[i] > MaxDistance)
		{
			MaxDistance = distances[i];
			MaxDistanceIndex = i;
		}
		if (distances[i] < MinDistance)
		{
			MinDistance = distances[i];
			MinDistanceIndex = i;
		}
	}
	result[0] /= fiberPointCount;
	result[1] /= fiberPointCount;
	result[2] /= fiberPointCount;
	*/
	Substract(PointCloud[fiberPointIDS[MaxDistanceIndex]]->DCoord, MuscleMesh->GetPoint(closestPoints[MaxDistanceIndex]), result);

	Log("    Moving mesh points...");
	vtkPoints* outputPoints = vtkPoints::New(); //delete in ClearMesh() in MuscleMesh destructor
	outputPoints->SetNumberOfPoints(NumOfPoints);
	for (int i = 0; i < NumOfPoints; i++)
	{
		MuscleMesh->GetPoint(i, pCoord);
		outputPoints->SetPoint(i, pCoord[0] - result[0], pCoord[1] -result[1],pCoord[2]- result[2]);
	}
	//MuscleMesh->GetPoints()->Delete(); // clear the old -- Unnecesary, SetPoints destructs the old object
	MuscleMesh->SetPoints(outputPoints);

	delete[] distances;
	distances = NULL;
	fiberPointIDS.clear();
	delete[] closestPoints;
	closestPoints = NULL;
}

// Computes normals for all fiber points
//		|\2|6/| k
//		|1\|/5| k
//		|--*--| k
//		|3/|\7| k
//		|/4|8\|  jjjjjjj
void vtkMuscleDecomposer::ComputeNormals()
{
	Log("  Computing normals...");
	vtkstd::vector<double*> normals;
	normals.reserve(8);
	double* normal;
	double* point1;
	double* point2;
	double* point3;
	int TC1, TC2;

	for (int k = 0; k < (int)Fibers[0]->PointIDs.size(); k++)
	{
		for (int j = 0; j < (int)Fibers.size(); j++)
		{
			point1 = PointCloud[Fibers[j]->PointIDs[k]]->DCoord;

			if (j > 0) // do the left normals;
			{
				if (k < (int)Fibers[j-1]->PointIDs.size()-1)
				{
					normal = new double[3]; //delete in here
					point2 = PointCloud[Fibers[j-1]->PointIDs[k]]->DCoord;
					point3 = PointCloud[Fibers[j-1]->PointIDs[k+1]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //1

					normal = new double[3]; //delete in here
					point2 = PointCloud[Fibers[j-1]->PointIDs[k+1]]->DCoord;
					point3 = PointCloud[Fibers[j]->PointIDs[k+1]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //2
				}else
				{
					normal = new double[3];//delete in here

					point2 = PointCloud[Fibers[j-1]->PointIDs[k]]->DCoord;
					TC1 = PointCloud[Fibers[j - 1]->PointIDs[k]]->TendonConnectionID;
					point3 = PointCloud[TC1]->DCoord;
					//point3 = PointCloud[Fibers[j-1]->PointIDs[k+1]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //1

					normal = new double[3];//delete in here
					TC1 = PointCloud[Fibers[j - 1]->PointIDs[k]]->TendonConnectionID;
					TC2 = PointCloud[Fibers[j]->PointIDs[k]]->TendonConnectionID;
					point2 = PointCloud[TC1]->DCoord;
					point3 = PointCloud[TC2]->DCoord;
					//point2 = PointCloud[Fibers[j-1]->PointIDs[k+1]]->DCoord;
					//point3 = PointCloud[Fibers[j]->PointIDs[k+1]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //1
				}

				if (k > 0)
				{
					normal = new double[3];//delete in here
					point2 = PointCloud[Fibers[j - 1]->PointIDs[k - 1]]->DCoord;
					point3 = PointCloud[Fibers[j - 1]->PointIDs[k]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //3

					normal = new double[3];//delete in here
					point2 = PointCloud[Fibers[j]->PointIDs[k-1]]->DCoord;
					point3 = PointCloud[Fibers[j-1]->PointIDs[k-1]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //4
				}else // consider tendon points
				{
					normal = new double[3];//delete in here
					TC1 = PointCloud[Fibers[j - 1]->PointIDs[k]]->TendonConnectionID;
					point2 = PointCloud[TC1]->DCoord;
					//point2 = PointCloud[Fibers[j - 1]->PointIDs[k - 1]]->DCoord;
					point3 = PointCloud[Fibers[j - 1]->PointIDs[k]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //1

					normal = new double[3];//delete in here
					TC1 = PointCloud[Fibers[j]->PointIDs[k]]->TendonConnectionID;
					TC2 = PointCloud[Fibers[j - 1]->PointIDs[k]]->TendonConnectionID;
					point2 = PointCloud[TC1]->DCoord;
					point3 = PointCloud[TC2]->DCoord;
					//point2 = PointCloud[Fibers[j]->PointIDs[k-1]]->DCoord;
					//point3 = PointCloud[Fibers[j-1]->PointIDs[k-1]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //1
				}
			}
			if (j < (int)Fibers.size() - 1) // do the right normals;
			{
				if (k < (int)Fibers[j+1]->PointIDs.size()-1)
				{
					normal = new double[3];//delete in here
					point2 = PointCloud[Fibers[j+1]->PointIDs[k+1]]->DCoord;
					point3 = PointCloud[Fibers[j+1]->PointIDs[k]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //5

					normal = new double[3];//delete in here
					point2 = PointCloud[Fibers[j]->PointIDs[k+1]]->DCoord;
					point3 = PointCloud[Fibers[j+1]->PointIDs[k+1]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //6
				}
				else
				{
					normal = new double[3];//delete in here
					TC1 = PointCloud[Fibers[j + 1]->PointIDs[k]]->TendonConnectionID;
					point2 = PointCloud[TC1]->DCoord;
					//point2 = PointCloud[Fibers[j+1]->PointIDs[k+1]]->DCoord;
					point3 = PointCloud[Fibers[j+1]->PointIDs[k]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //1

					normal = new double[3];//delete in here
					TC1 = PointCloud[Fibers[j]->PointIDs[k]]->TendonConnectionID;
					TC2 = PointCloud[Fibers[j+1]->PointIDs[k]]->TendonConnectionID;
					point2 = PointCloud[TC1]->DCoord;
					point3 = PointCloud[TC2]->DCoord;
					//point2 = PointCloud[Fibers[j]->PointIDs[k+1]]->DCoord;
					//point3 = PointCloud[Fibers[j+1]->PointIDs[k+1]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //1
				}

				if (k > 0)
				{
					normal = new double[3];//delete in here
					point2 = PointCloud[Fibers[j + 1]->PointIDs[k]]->DCoord;
					point3 = PointCloud[Fibers[j + 1]->PointIDs[k - 1]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //7

					normal = new double[3];//delete in here
					point2 = PointCloud[Fibers[j+1]->PointIDs[k-1]]->DCoord;
					point3 = PointCloud[Fibers[j]->PointIDs[k-1]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //8
				} else // consider tendon points
				{
					normal = new double[3];//delete in here

					point2 = PointCloud[Fibers[j + 1]->PointIDs[k]]->DCoord;
					TC1 = PointCloud[Fibers[j + 1]->PointIDs[k]]->TendonConnectionID;
					point3 = PointCloud[TC1]->DCoord;
					//point3 = PointCloud[Fibers[j + 1]->PointIDs[k - 1]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //1

					normal = new double[3];//delete in here
					TC1 = PointCloud[Fibers[j+1]->PointIDs[k]]->TendonConnectionID;
					TC2 = PointCloud[Fibers[j]->PointIDs[k]]->TendonConnectionID;
					point2 = PointCloud[TC1]->DCoord;
					point3 = PointCloud[TC2]->DCoord;
					//point2 = PointCloud[Fibers[j+1]->PointIDs[k-1]]->DCoord;
					//point3 = PointCloud[Fibers[j]->PointIDs[k-1]]->DCoord;
					ComputeNormal(point1, point2, point3, normal);
					normals.push_back(normal); //1
				}
			}
		} // j
		// average normals and assign as normal
		// the normal is same for the whole tier of points (to avoid normal intersections)
		normal = new double[3]; //delete in here
		normal[0] = normal[1] = normal[2] = 0;
		for (int i = 0; i < (int)normals.size(); i++)
		{
			normal[0] += normals[i][0];
			normal[1] += normals[i][1];
			normal[2] += normals[i][2];
			delete[] normals[i];
			normals[i] = NULL;
		}
		vtkMath::Normalize(normal);
		for	(int l = 0; l < (int)Fibers.size(); l++)
		{
			PointCloud[Fibers[l]->PointIDs[k]]->SetNormal(normal);
		}
		normals.clear();
		delete[] normal;
		normal = NULL;
	} // k
}

//----------------------------------------------------------------------------
// Outputs normals - debug purposes only. Currently disabled in DoneMesh();
//----------------------------------------------------------------------------
void vtkMuscleDecomposer::OutputNormals()
{
	vtkPoints * PointIDs = vtkPoints::New(); //delete in here
	vtkCellArray * lines = vtkCellArray::New(); //delete in here
	vtkIdType * lineIDs = new vtkIdType[2]; //delete in here
	double moved[3];
	double* original;
	double* normal;

	for (int i = 0; i < (int)Fibers.size(); i++)
	{
		for (int j = 0; j < (int)Fibers[i]->PointIDs.size(); j++)
		{
			/*if (PointCloud[Fibers[i]->PointIDs[j]]->thickness < MIN_THICKNESS)
				continue;*/
			// insert point to output
			original = PointCloud[Fibers[i]->PointIDs[j]]->DCoord;
			lineIDs[0] = PointIDs->InsertNextPoint(original);
			normal =  PointCloud[Fibers[i]->PointIDs[j]]->DNormal;
			Add(original, normal, moved,  -PointCloud[Fibers[i]->PointIDs[j]]->thickness);
			lineIDs[1] = PointIDs->InsertNextPoint(moved);
			lines->InsertNextCell(2, lineIDs);
		}
	}

	NormalMesh = vtkPolyData::New(); //not deleted (used for debugging only)
	NormalMesh->SetPoints(PointIDs);
	NormalMesh->SetLines(lines);

	delete[] lineIDs;
	lineIDs = NULL;
	PointIDs->Delete();
	lines->Delete();
}

// Computes muscle thinckness for fiber points
void vtkMuscleDecomposer::ComputeThickness(void)
{
	Log("  Computing muscle thickness...");
	double* rayPoint1;
	double* rayPoint2;
	double intersectionPoint1[3];
	double intersectionPoint2[3];
	double* original;
	double* normal;
	double thickness;

	double areaThickness;
	int valueCount = 0;
	double neighThickness;
	bool needsAnotherIteration = true;
	int iterationCount = 0;

	// first we compute muscle thicknes from mesh data where possible
	Log("    Sampling muscle thickness from mesh data...");

	// TODO look into the build in tree version problem
	vtkOBBTree* tree = vtkOBBTree::New(); //delete in here
	tree->SetDataSet(this->MuscleMesh);
	tree->BuildLocator();
	for (int i = 0; i < (int)Fibers.size(); i++)
	{
		for (int j = 0; j < (int)Fibers[i]->PointIDs.size(); j++)
		{
			rayPoint1 = new double[3]; //delete in here
			rayPoint2 = new double[3]; //delete in here

			original = PointCloud[Fibers[i]->PointIDs[j]]->DCoord;
			normal =  PointCloud[Fibers[i]->PointIDs[j]]->DNormal;

			Add(original, normal, rayPoint1, -RAY_POINT_DISTANCE);
			Add(original, normal, rayPoint2, RAY_POINT_DISTANCE);

			vtkPoints* intersections = vtkPoints::New(); //delete in here
			tree->IntersectWithLine(rayPoint1, rayPoint2, intersections, NULL);

			delete[] rayPoint1; rayPoint1 = NULL;
			delete[] rayPoint2; rayPoint2 = NULL;

			thickness = 0;
			if (intersections->GetNumberOfPoints() > 1)
			{
				for (int i = 0; i < intersections->GetNumberOfPoints() - 1; i++)
				{
					intersections->GetPoint(i, intersectionPoint1);
					intersections->GetPoint(i+1, intersectionPoint2);
					thickness += PointPointDistance(intersectionPoint1, intersectionPoint2);
				}
			}
			PointCloud[Fibers[i]->PointIDs[j]]->thickness = thickness;

			intersections->Delete();
			intersections = NULL;
		}
	}
	tree->Delete();
	tree = NULL;

	// now we interpolate thickness to points where the thickness could not be computed
	Log("    Interpolating unknown thickness...");
	while (needsAnotherIteration)
	{
		iterationCount++;
		if (iterationCount >10000)
			break;
		needsAnotherIteration = false;
		for (int j = 0; j < (int)Fibers.size(); j++)
		{
			for (int k = 0; k < (int)Fibers[j]->PointIDs.size(); k++) // we dont alter the tickness on the end of the muscles
			{
				thickness = PointCloud[Fibers[j]->PointIDs[k]]->thickness;
				if (thickness < MIN_THICKNESS)
				{
					areaThickness = 0;
					for (int m = j - 1; m <= j + 1; m++)
					{
						int n = k;
						//for (int n = k; n <= k; n++)
						{
							if (m >= 0 && n >= 0 && m < (int)Fibers.size() && n < (int)Fibers[m]->PointIDs.size())
							{
								neighThickness = PointCloud[Fibers[m]->PointIDs[n]]->thickness;
								if (neighThickness > MIN_THICKNESS)
								{
								areaThickness+=neighThickness;
								valueCount++;
								}
							}
						}
					}
					if (valueCount > 0)
					{
						thickness+=areaThickness / valueCount;
						PointCloud[Fibers[j]->PointIDs[k]]->thickness = thickness;
					}
					if (thickness < MIN_THICKNESS)
						needsAnotherIteration = true;
					valueCount = 0;
				}
			}
		}
	}
	Log("      Iterations done: " , iterationCount);
}

// Builds bottom layer of fibers from normals and thickness
void vtkMuscleDecomposer::BuildBottomFibers(float t)
{
	Log("  Building bottom fibers...");
	int ID = 0;
	int newID = 0;
	Fiber* newFiber;
	Point* newPoint;
	// copy all fibers
	BottomFibers.reserve(Fibers.size());
	double pCoord[3];

	for (int i = 0; i < (int)Fibers.size(); i++)
	{
		newFiber = new Fiber(); //delete in ClearMesh()
		newFiber->PointIDs.reserve(Fibers[i]->PointIDs.size());
		for (int j = 0; j < (int)Fibers[i]->PointIDs.size(); j++)
		{
			ID = Fibers[i]->PointIDs[j];
			// compute point position from normal and thickness
			Add(PointCloud[ID]->DCoord, PointCloud[ID]->DNormal, pCoord, -PointCloud[ID]->thickness * t);
			// create point
			newPoint = new Point(pCoord); //delete in ClearMesh()

			// insert it to cloud
			PointCloud.push_back(newPoint);
			newID = PointCloud.size() - 1;
			newPoint->Id = newID;
			// adjust position
			newPoint->BBoundary = PointCloud[ID]->BBoundary;
			newPoint->BMuscle = PointCloud[ID]->BMuscle;
			newPoint->TendonConnectionID = PointCloud[ID]->TendonConnectionID;
			newPoint->interpolated = PointCloud[ID]->interpolated;
			// and to the list of IDs for this fibre
			newFiber->PointIDs.push_back(newID);
		}
		BottomFibers.push_back(newFiber);
	}
}

// Builds side layers after bottom layer has been built
void vtkMuscleDecomposer::BuildSideLayers(int subdivision, int interpolationType)
{
	LogFormated("  Interpolating muscle side points using %s method...", GetSplineType(interpolationType));

	Fiber* crossPoints;
	int fiberCount = 4;
	int pointCount = Fibers[0]->PointIDs.size();

	LeftFibers.reserve(fiberCount * subdivision);
	RightFibers.reserve(fiberCount * subdivision);

	SplineInterpolator* interpolator = new SplineInterpolator(interpolationType); //delete in here

	for (int j = 0; j < pointCount; j++)
	{
		crossPoints = new Fiber(); //delete in here
		crossPoints->PointIDs.reserve(fiberCount);
		for (int i = 1; i >= 0; i--)
		{
			crossPoints->PointIDs.push_back(Fibers[i]->PointIDs[j]);
		}

		for (int i = 0; i < 2; i++)
		{
			crossPoints->PointIDs.push_back(BottomFibers[i]->PointIDs[j]);
		}

		InterpolateFiber(crossPoints, subdivision, interpolator, 1, true);

		for (int i = 1; i < (int)crossPoints->PointIDs.size() - 1; i++)
		{
			if ((int)LeftFibers.size() <= i - 1)
			{
				LeftFibers.push_back(new Fiber()); //delete in ClearMesh()
			}
			LeftFibers[i-1]->PointIDs.push_back(crossPoints->PointIDs[i]);
		}

		crossPoints->PointIDs.clear();

		for (int i = (int)Fibers.size() - 2; i < (int)Fibers.size(); i++)
		{
			crossPoints->PointIDs.push_back(Fibers[i]->PointIDs[j]);
		}

		for (int i = (int)BottomFibers.size() - 1; i >= (int)BottomFibers.size() - 3; i--)
		{
			crossPoints->PointIDs.push_back(BottomFibers[i]->PointIDs[j]);
		}

		InterpolateFiber(crossPoints, subdivision, interpolator, 1, true);

		for (int i = 1; i < (int)crossPoints->PointIDs.size() - 1; i++)
		{
			if ((int)RightFibers.size() <= i - 1)
			{
				RightFibers.push_back(new Fiber()); //delete in ClearMesh()
			}
			RightFibers[i-1]->PointIDs.push_back(crossPoints->PointIDs[i]);
		}

		delete crossPoints;
		crossPoints = NULL;
	}
	LeftFibers[0]->BBoundary = false;
	LeftFibers[LeftFibers.size()-1]->BBoundary = false;
	RightFibers[0]->BBoundary = false;
	RightFibers[RightFibers.size()-1]->BBoundary = false;
	// we need to interpolate the tendon connection settings also

	for(int i = 0; i < (int)LeftFibers.size(); i++)
	{
		//interpolatedIndex = vtkMath::Round(i/subdivision);
		PointCloud[LeftFibers[i]->PointIDs[0]]->TendonConnectionID = PointCloud[Fibers[0]->PointIDs[0]]->TendonConnectionID;
		PointCloud[LeftFibers[i]->PointIDs[pointCount-1]]->TendonConnectionID = PointCloud[Fibers[0]->PointIDs[pointCount-1]]->TendonConnectionID;
	}

	for(int i = 0; i < (int)RightFibers.size(); i++)
	{
		//interpolatedIndex = vtkMath::Round(i/subdivision);
		PointCloud[RightFibers[i]->PointIDs[0]]->TendonConnectionID = PointCloud[Fibers[Fibers.size()-1]->PointIDs[0]]->TendonConnectionID;
		PointCloud[RightFibers[i]->PointIDs[pointCount-1]]->TendonConnectionID = PointCloud[Fibers[Fibers.size()-1]->PointIDs[pointCount-1]]->TendonConnectionID;
	}

	/*Data->clear();
	Data->assign(NewFibers.begin(), NewFibers.end());*/
	delete interpolator;
	interpolator = NULL;
}

//// =========================================================================
//// Utils
//// =========================================================================

int vtkMuscleDecomposer::PointIsWithinBounds(double point[3], double bounds[6], double delta[3])
{
  if(!point || !bounds || !delta)
	{
	return 0;
	}
  for(int i=0;i<3;i++)
	{
	if(point[i]+delta[i] < bounds[2*i] || point[i]-delta[i] > bounds[2*i+1])
	  {
	  return 0;
	  }
	}
  return 1;
}

// Computes Line-Line distance between lines defined by (p11, p12) and (p21, p22)
double vtkMuscleDecomposer::LineLineDistance(double* p11, double* p12, double* p21, double* p22)
{
	/*
	The distance between two skew lines with equations
	x	=	x_1+(x_2-x_1)s
	x	=	x_3+(x_4-x_3)t

	D=(|c·(axb)|)/(|axb|)

	by defining
	a	=	x_2-x_1
	b	=	x_4-x_3
	c   =	x_3-x_1
	*/
	double a[3];
	double b[3];
	double c[3];
	double aCrossB[3];

	Substract(p12, p11, a);
	Substract(p22, p21, b);
	Substract(p21, p11, c);

	vtkMath::Cross(a, b, aCrossB);

	double result = abs(vtkMath::Dot(c, aCrossB) / sqrt(aCrossB[0]*aCrossB[0] + aCrossB[1]*aCrossB[1] + aCrossB[2]*aCrossB[2]));
	return result;
}

// Substracts point1 from point2 to result
void vtkMuscleDecomposer::Substract(double* point1, double* point2, double* result)
{
	for (int i = 0; i < 3; i++)
	{
		result[i] = point2[i] - point1[i];
	}
}

void vtkMuscleDecomposer::Add(double* point1, double* point2, double* result, double multiplication)
{
	for (int i = 0; i < 3; i++)
	{
		result[i] = point2[i] * multiplication + point1[i];
	}
}

double vtkMuscleDecomposer::PointPointDistance(double* point1, double* point2)
{
	return sqrt(vtkMath::Distance2BetweenPoints(point1, point2));
}

// Computes normal of from 3 points (point1 being the normal origin)
void vtkMuscleDecomposer::ComputeNormal(double* point1, double* point2, double* point3, double* normal)
{
	double u[3];
	double v[3];
	double result[3];
	Substract(point2, point1, u);
	Substract(point3, point1, v);
	vtkMath::Cross(u, v, result);
	vtkMath::Normalize(result);
	normal[0] = result[0];
	normal[1] = result[1];
	normal[2] = result[2];
}