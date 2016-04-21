/*========================================================================= 
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: MMSSMorphing.cpp,v $ 
  Language: C++ 
  Date: $Date: 2011-11-01 09:45:53 $ 
  Version: $Revision: 1.1.2.1 $ 
  Authors: Ivo Zelený
  ========================================================================== 
  Copyright (c) 2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

////////////////////////////////////////////////////////////////  
// This file contains:
// Morphing class 
// Morph points from source mesh to points on target mesh
////////////////////////////////////////////////////////////////

#include "MMSSMorphing.h"
#include <stdlib.h>
#include <float.h>
#include <time.h>

/*
Constructor, define regular tetrahedra and init areas.
*/
MMSSMorphing::MMSSMorphing()
{
	maxDiference = 0.1f;

	srand((unsigned int)time(0));//init random generator

	//define regular tetrahedron
	tetrahedraP1.x = 0;
	tetrahedraP1.y = 1;
	tetrahedraP1.z = 0;

	float m[16];
	GLdouble t[16];

	glGetFloatv(GL_MODELVIEW_MATRIX,m);
	glLoadIdentity();
	glRotatef(109.5f,0,0,-1);
	glGetDoublev(GL_MODELVIEW_MATRIX,t);
	
	tetrahedraP2 = MMSSVector3d::Transform(tetrahedraP1,t);
	
	glLoadIdentity();
	glRotatef(109.5f,0,1,0);
	glRotatef(109.5f,0,0,-1);
	glGetDoublev(GL_MODELVIEW_MATRIX,t);
	
	tetrahedraP3 = MMSSVector3d::Transform(tetrahedraP1,t);
	
	glLoadIdentity();
	glRotatef(109.5f,0,1,0);
	glRotatef(109.5f,0,1,0);
	glRotatef(109.5f,0,0,-1);
	glGetDoublev(GL_MODELVIEW_MATRIX,t);
	
	tetrahedraP4 = MMSSVector3d::Transform(tetrahedraP1,t);

	glLoadMatrixf(m);//load back original matrix

	sourceMesh = NULL;
	originalTargetMesh = NULL;
	targetMesh = NULL;

	//default values for areas
	sourceOriginArea = NULL;
	sourceOriginAreaCount = 0;
	sourceInsertArea = NULL;
	sourceInsertAreaCount = 0;
	targetOriginArea = NULL;
	targetOriginAreaCount = 0;
	targetInsertArea = NULL;
	targetInsertAreaCount = 0;
	
	sourceMeshEdges = NULL;
	

	setTetrahedra = false;
}

/*
release alocated memory
*/
MMSSMorphing::~MMSSMorphing(void)
{
	if(sourceMesh)free(sourceMesh);
	if(targetMesh)free(targetMesh);
	if(originalTargetMesh)free(originalTargetMesh);
	
	if(sourceOriginArea)free(sourceOriginArea);
	if(sourceInsertArea)free(sourceInsertArea);

	if(targetOriginArea)free(targetOriginArea);
	if(targetInsertArea)free(targetInsertArea);

	if(sourceMeshEdges)free(sourceMeshEdges);
	
}

/*
Define source points and edges.
*/
void MMSSMorphing::SetSourceMesh(Vector* sourceMeshPoints,GLuint pointsCount,Edge* edges,GLuint edgesCount)
{
	if(sourceMesh)free(sourceMesh);
	if(sourceMeshEdges)free(sourceMeshEdges);
	sourceMesh = (Vector*)malloc(pointsCount*sizeof(Vector));
	sourceMeshEdges = (Edge*)malloc(edgesCount*sizeof(Edge));
	sourceMeshCount = pointsCount;
	sourceMeshEdgesCount = edgesCount;

	for(unsigned int i = 0; i < pointsCount; i++)
	{
		sourceMesh[i] = sourceMeshPoints[i];
	}

	for(unsigned int i = 0; i < edgesCount; i++)
	{
		sourceMeshEdges[i] = edges[i];
	}

	//get center and radius of circumsphere
	Vector center;
	GLfloat radius;
	GetCircumSphere(sourceMesh,pointsCount,&center,&radius);

	//transform sphere and points to origin and scale to unit
	float scale = 1.0f/radius;
	for(unsigned int i = 0; i < pointsCount; i++)
	{
		sourceMesh[i] = scale*(sourceMesh[i]-center);
	}
}

/*
Set areas on source mesh.
*/
void MMSSMorphing::SetSourceMeshAreas(GLuint* originArea, GLuint originAreaCount, GLuint* insertArea, GLuint insertAreaCount)
{
	if(sourceOriginArea)free(sourceOriginArea);
	if(sourceInsertArea)free(sourceInsertArea);

	sourceOriginAreaCount = originAreaCount;
	sourceInsertAreaCount = insertAreaCount;

	sourceOriginArea = (GLuint*)malloc(sourceOriginAreaCount*sizeof(GLuint));
	sourceInsertArea = (GLuint*)malloc(sourceInsertAreaCount*sizeof(GLuint));
	//copy areas
	for(unsigned int i = 0; i < sourceOriginAreaCount; i++)
	{
		sourceOriginArea[i] = originArea[i];
	}

	for(unsigned int i = 0; i < sourceInsertAreaCount; i++)
	{
		sourceInsertArea[i] = insertArea[i];
	}
}

/*
Set areas on target mesh.
*/
void MMSSMorphing::SetTargetMeshAreas(GLuint* originArea, GLuint originAreaCount, GLuint* insertArea, GLuint insertAreaCount)
{
	if(targetOriginArea)free(targetOriginArea);
	if(targetInsertArea)free(targetInsertArea);
	
	targetOriginAreaCount = originAreaCount;
	targetInsertAreaCount = insertAreaCount;
	
	targetOriginArea = (GLuint*)malloc(targetOriginAreaCount*sizeof(GLuint));
	targetInsertArea = (GLuint*)malloc(targetInsertAreaCount*sizeof(GLuint));
	
	//copy areas
	for(unsigned int i = 0; i < targetOriginAreaCount; i++)
	{
		targetOriginArea[i] = originArea[i];
	}

	for(unsigned int i = 0; i < targetInsertAreaCount; i++)
	{
		targetInsertArea[i] = insertArea[i];
	}
}

/*
Define target mesh points and edges.
*/
void MMSSMorphing::SetTargetMesh(Vector* targetMeshPoints,GLuint pointsCount,Triangle* triangles,GLuint trianglesCount)
{
	if(sourceMesh==NULL)return;
	if(targetMesh)free(targetMesh);
	if(originalTargetMesh)free(originalTargetMesh);
	
	Plane* planesTriangle = (Plane*)malloc(trianglesCount*sizeof(Plane));
	targetMesh = (Vector*)malloc(pointsCount*sizeof(Vector));
	
	for(unsigned int i = 0; i < pointsCount; i++)
	{
		targetMesh[i] = targetMeshPoints[i];
	}

	//decompose triangles to edges
	GLuint edgesCount = (trianglesCount*3)/2;
	Edge* edges = (Edge*)malloc(edgesCount * sizeof(Edge));
	Edge edge;
	bool found;
	unsigned int index = 0;
	for(unsigned int i = 0; i < trianglesCount; i++)
	{
		//first edge of triangle
		edge.p1_index = triangles[i].p1_index;
		edge.p2_index = triangles[i].p2_index;
		found = false;
		for(unsigned int j = 0; j < index; j++)
		{
			if(edges[j].p1_index == edge.p1_index && edges[j].p2_index == edge.p2_index || edges[j].p1_index == edge.p2_index && edges[j].p2_index == edge.p1_index)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			edges[index] = edge;
			index++;
		}

		//second edge of triangle
		edge.p1_index = triangles[i].p1_index;
		edge.p2_index = triangles[i].p3_index;
		found = false;
		for(unsigned int j = 0; j < index; j++)
		{
			if(edges[j].p1_index == edge.p1_index && edges[j].p2_index == edge.p2_index || edges[j].p1_index == edge.p2_index && edges[j].p2_index == edge.p1_index)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			edges[index] = edge;
			index++;
		}

		//third edge of triangle
		edge.p1_index = triangles[i].p2_index;
		edge.p2_index = triangles[i].p3_index;
		found = false;
		for(unsigned int j = 0; j < index; j++)
		{
			if(edges[j].p1_index == edge.p1_index && edges[j].p2_index == edge.p2_index || edges[j].p1_index == edge.p2_index && edges[j].p2_index == edge.p1_index)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			edges[index] = edge;
			index++;
		}
	}

	setTetrahedra = true;

	//get center and radius of circumsphere
	Vector center;
	GLfloat radius;
	GetCircumSphere(targetMesh,pointsCount,&center,&radius);

	//transform sphere and points to origin and scale to unit
	//center will be [0,0,0]
	float scale = 1.0f/radius;
	for(unsigned int i = 0; i < pointsCount; i++)
	{
		targetMesh[i] = scale*(targetMesh[i]-center);
	}

	Vector n1,n2,n3;
	int intersectionCount = 0;
	//if center is not inside the polyhedron -> move to inside
	//detect if center(center is [0,0,0] -> easier for testing) outside
	//test vector [1,0,0] and count intersect, odd->inside, even->outside
	for(unsigned int i = 0; i < trianglesCount; i++)
	{
		//test only one half of object -> right side
		if(targetMesh[triangles[i].p1_index].x > 0 || targetMesh[triangles[i].p2_index].x > 0 || targetMesh[triangles[i].p3_index].x > 0)
		{
			//if all points lies up or down of plane a=0,b=1,c=0,d=0 -> cannot be intersected
			if(targetMesh[triangles[i].p1_index].y > 0 && targetMesh[triangles[i].p2_index].y > 0 && targetMesh[triangles[i].p3_index].y > 0)continue;
			else if(targetMesh[triangles[i].p1_index].y < 0 && targetMesh[triangles[i].p2_index].y < 0 && targetMesh[triangles[i].p3_index].y < 0)continue;
			
			//if all points lies up or down of plane a=0,b=0,c=1,d=0 -> cannot be intersected
			if(targetMesh[triangles[i].p1_index].z > 0 && targetMesh[triangles[i].p2_index].z > 0 && targetMesh[triangles[i].p3_index].z > 0)continue;
			else if(targetMesh[triangles[i].p1_index].z < 0 && targetMesh[triangles[i].p2_index].z < 0 && targetMesh[triangles[i].p3_index].z < 0)continue;
			
			//intersect minmaxbox of triangle -> count if intersect triangle

			n1 = MMSSVector3d::VectorCross(targetMesh[triangles[i].p1_index],targetMesh[triangles[i].p2_index]);
			if(n1*targetMesh[triangles[i].p3_index]<0)
			{
				n1 = MMSSVector3d::VectorCross(targetMesh[triangles[i].p2_index],targetMesh[triangles[i].p1_index]);
			}

			n2 = MMSSVector3d::VectorCross(targetMesh[triangles[i].p2_index],targetMesh[triangles[i].p3_index]);
			if(n2*targetMesh[triangles[i].p1_index]<0)
			{
				n2 = MMSSVector3d::VectorCross(targetMesh[triangles[i].p3_index],targetMesh[triangles[i].p2_index]);
			}

			n3 = MMSSVector3d::VectorCross(targetMesh[triangles[i].p3_index],targetMesh[triangles[i].p1_index]);
			if(n3*targetMesh[triangles[i].p2_index]<0)
			{
				n3 = MMSSVector3d::VectorCross(targetMesh[triangles[i].p1_index],targetMesh[triangles[i].p3_index]);
			}

			if(n1.x > 0 && n2.x > 0 && n3.x > 0)
			{
				intersectionCount++;
			}
		}
	}

	//if intersectionCount is even -> point is outside -> move center behind nearest point
	Vector nearest;
	float len = FLT_MAX;
	float actLen;
	if((intersectionCount & 1) == 0)
	{
		for(unsigned int i = 0; i < pointsCount; i++)	
		{
			actLen = MMSSVector3d::VectorLength(targetMesh[i]);
			if(actLen < len)
			{
				len = actLen;
				nearest = targetMesh[i];
			}
		}

		center = 1.001f * nearest;//behind nearest point


		for(unsigned int i = 0; i < pointsCount; i++)
		{
			targetMesh[i] = targetMesh[i]-center;
		}
	}

	//get embeding of mesh on sphere
	targetMesh = TransformToUnitSphere(targetMesh,pointsCount,edges,edgesCount,NULL,0);//NULL and 0 last parameters are no added fixing
	
	//-------------------------------------------------------------------------------------
	//Feature alignment
	//-------------------------------------------------------------------------------------
	if(sourceOriginAreaCount > 0 && targetOriginAreaCount > 0)
	{
		//rotate sphere to align important areas
		//count center of source origin area
		Vector sourceCenterOrigin;
		sourceCenterOrigin.x = 0;
		sourceCenterOrigin.y = 0;
		sourceCenterOrigin.z = 0;
		for(unsigned int i = 0; i < sourceOriginAreaCount; i++)
		{
			sourceCenterOrigin = sourceCenterOrigin + sourceMesh[sourceOriginArea[i]];
		}
		sourceCenterOrigin = (1.0f/sourceOriginAreaCount)*sourceCenterOrigin;
		sourceCenterOrigin = MMSSVector3d::Normalize(sourceCenterOrigin);

		//count center of target origin area
		Vector targetCenterOrigin;
		targetCenterOrigin.x = 0;
		targetCenterOrigin.y = 0;
		targetCenterOrigin.z = 0;
		for(unsigned int i = 0; i < targetOriginAreaCount; i++)
		{
			targetCenterOrigin = targetCenterOrigin + targetMesh[targetOriginArea[i]];
		}
		targetCenterOrigin = (1.0f/targetOriginAreaCount)*targetCenterOrigin;
		targetCenterOrigin = MMSSVector3d::Normalize(targetCenterOrigin);

		Vector normal = MMSSVector3d::VectorCross(sourceCenterOrigin,targetCenterOrigin);

		GLfloat m[16];
		GLdouble t[16];

		glGetFloatv(GL_MODELVIEW_MATRIX,m);//get orig transform
		glLoadIdentity();
		glRotatef(180*(acos(sourceCenterOrigin*targetCenterOrigin)/3.14159265f),normal.x,normal.y,normal.z);
		glGetDoublev(GL_MODELVIEW_MATRIX,t);

		//rotate all points of source mesh, align centers of origin areas 
		for(unsigned int i = 0; i < sourceMeshCount; i++)
		{
			sourceMesh[i] = MMSSVector3d::Transform(sourceMesh[i],t);
		}

		//count center of source insertion area
		Vector sourceCenterInsert;
		sourceCenterInsert.x = 0;
		sourceCenterInsert.y = 0;
		sourceCenterInsert.z = 0;
		for(unsigned int i = 0; i < sourceInsertAreaCount; i++)
		{
			sourceCenterInsert = sourceCenterInsert + sourceMesh[sourceInsertArea[i]];
		}
		sourceCenterInsert = (1.0f/sourceInsertAreaCount)*sourceCenterInsert;
		sourceCenterInsert = MMSSVector3d::Normalize(sourceCenterInsert);

		//count center of target insertion area
		Vector targetCenterInsert;
		targetCenterInsert.x = 0;
		targetCenterInsert.y = 0;
		targetCenterInsert.z = 0;
		for(unsigned int i = 0; i < targetInsertAreaCount; i++)
		{
			targetCenterInsert = targetCenterInsert + targetMesh[targetInsertArea[i]];
		}
		targetCenterInsert = (1.0f/targetInsertAreaCount)*targetCenterInsert;
		targetCenterInsert = MMSSVector3d::Normalize(targetCenterInsert);


		//rotate around source center and found best fit rotation of source insert center to target insert center
		glLoadIdentity();
		glRotatef(360.0f/FEATURE_ALIGN_ROTATION_ITERATION,targetCenterOrigin.x,targetCenterOrigin.y,targetCenterOrigin.z);
		glGetDoublev(GL_MODELVIEW_MATRIX,t);

		float actLen;
		Vector actRotationSourceCenterInsert = sourceCenterInsert;
		float min_length = MMSSVector3d::VectorLength(targetCenterInsert - actRotationSourceCenterInsert);
		int min_index = 0;
		for(unsigned int i = 1; i < FEATURE_ALIGN_ROTATION_ITERATION; i++)
		{
			actRotationSourceCenterInsert = MMSSVector3d::Transform(actRotationSourceCenterInsert,t);
			actLen = MMSSVector3d::VectorLength(targetCenterInsert - actRotationSourceCenterInsert);
			if(actLen < min_length)
			{
				min_length = actLen;
				min_index = i;
			}
		}

		//if found better fit with some rotation -> rotate all points of sphere 
		if(min_index > 0)
		{
			glLoadIdentity();
			glRotatef(360.0f*(((float)min_index)/FEATURE_ALIGN_ROTATION_ITERATION),targetCenterOrigin.x,targetCenterOrigin.y,targetCenterOrigin.z);
			glGetDoublev(GL_MODELVIEW_MATRIX,t);

			//rotate all points of source mesh, align centers of origin areas 
			for(unsigned int i = 0; i < sourceMeshCount; i++)
			{
				sourceMesh[i] = MMSSVector3d::Transform(sourceMesh[i],t);
			}
		}
		
		glLoadMatrixf(m);//load back original matrix
		//end of rotating
		
		/*
		//reorder area
		//found nearest point of source area to first point in target area
		min_length = MMSSVector3d::VectorLength(sourceMesh[sourceOriginArea[0]] - targetMesh[targetOriginArea[0]]);
		min_index = 0;
		for(unsigned int i = 1; i < sourceOriginAreaCount; i++)
		{
			actLen = MMSSVector3d::VectorLength(sourceMesh[sourceOriginArea[i]] - targetMesh[targetOriginArea[0]]);
			if(actLen < min_length)
			{
				min_length = actLen;
				min_index = i;
			}
		}
		
		//reorder indices of areas
		if(min_index > 0)
		{
			unsigned int act_index = min_index;
			unsigned int temp;
			unsigned int index, index_prev;

			while(act_index != 0)
			{
				index = act_index;
				index_prev = act_index-1;
				for(unsigned int i = 0; i < sourceOriginAreaCount-1; i++)
				{
					temp = sourceOriginArea[index_prev];
					sourceOriginArea[index_prev] = sourceOriginArea[index];
					sourceOriginArea[index] = temp;

					index_prev = index;
					index = (index+1)%sourceOriginAreaCount;
				}
				
				act_index--;
			}
		}
		*/
		
		//count lengths of origin areas
		Vector* savedOriginAreaSourcePositions = (Vector*)malloc(sourceOriginAreaCount*sizeof(Vector));
		Vector* originAreaSourcePositions = (Vector*)malloc(sourceOriginAreaCount*sizeof(Vector));
		
		float sourceOriginAreaLength = 0;
		for(unsigned int i = 0; i < sourceOriginAreaCount-1; i++)
		{
			sourceOriginAreaLength += MMSSVector3d::VectorLength(sourceMesh[sourceOriginArea[i+1]]-sourceMesh[sourceOriginArea[i]]);
			savedOriginAreaSourcePositions[i] = sourceMesh[sourceOriginArea[i]];
		}
		sourceOriginAreaLength += MMSSVector3d::VectorLength(sourceMesh[sourceOriginArea[0]]-sourceMesh[sourceOriginArea[sourceOriginAreaCount-1]]);//from last to first
		savedOriginAreaSourcePositions[sourceOriginAreaCount-1] = sourceMesh[sourceOriginArea[sourceOriginAreaCount-1]];
			
		float targetOriginAreaLength = 0;
		for(unsigned int i = 0; i < targetOriginAreaCount-1; i++)
		{
			targetOriginAreaLength += MMSSVector3d::VectorLength(targetMesh[targetOriginArea[i+1]]-targetMesh[targetOriginArea[i]]);
		}
		targetOriginAreaLength += MMSSVector3d::VectorLength(targetMesh[targetOriginArea[0]]-targetMesh[targetOriginArea[targetOriginAreaCount-1]]);//from last to first
		

		//count lengths of insertion areas
		Vector* savedInsertAreaSourcePositions = (Vector*)malloc(sourceInsertAreaCount*sizeof(Vector));
		Vector* insertAreaSourcePositions = (Vector*)malloc(sourceInsertAreaCount*sizeof(Vector));
		
		float sourceInsertAreaLength = 0;
		for(unsigned int i = 0; i < sourceInsertAreaCount-1; i++)
		{
			sourceInsertAreaLength += MMSSVector3d::VectorLength(sourceMesh[sourceInsertArea[i+1]]-sourceMesh[sourceInsertArea[i]]);
			savedInsertAreaSourcePositions[i] = sourceMesh[sourceInsertArea[i]];
		}
		sourceInsertAreaLength += MMSSVector3d::VectorLength(sourceMesh[sourceInsertArea[0]]-sourceMesh[sourceInsertArea[sourceInsertAreaCount-1]]);//from last to first
		savedInsertAreaSourcePositions[sourceInsertAreaCount-1] = sourceMesh[sourceInsertArea[sourceInsertAreaCount-1]];
			
		float targetInsertAreaLength = 0;
		for(unsigned int i = 0; i < targetInsertAreaCount-1; i++)
		{
			targetInsertAreaLength += MMSSVector3d::VectorLength(targetMesh[targetInsertArea[i+1]]-targetMesh[targetInsertArea[i]]);
		}
		targetInsertAreaLength += MMSSVector3d::VectorLength(targetMesh[targetInsertArea[0]]-targetMesh[targetInsertArea[targetInsertAreaCount-1]]);//from last to first
		

		//set positions of areas points around target areas
		//origin area
		float acumulatedLength = 0;
		float ratio, lengthInTargetOriginArea,tempLength;
		float actLength = 0;
		originAreaSourcePositions[0] = targetMesh[targetOriginArea[0]];//set position to first
		
		for(unsigned int i = 1; i < sourceOriginAreaCount; i++)
		{
			acumulatedLength += MMSSVector3d::VectorLength(sourceMesh[sourceOriginArea[i]]-sourceMesh[sourceOriginArea[i-1]]);
			ratio = acumulatedLength / sourceOriginAreaLength;
			lengthInTargetOriginArea = ratio * targetOriginAreaLength;

			actLength = 0;
			unsigned int index = 0;
			unsigned int index_next = 1;

			bool found = false;
			//found edge of target area where new position will be
			while(index < (targetOriginAreaCount-1))
			{
				tempLength = MMSSVector3d::VectorLength(targetMesh[targetOriginArea[index_next]]-targetMesh[targetOriginArea[index]]);
				actLength += tempLength;
				if(actLength > lengthInTargetOriginArea)
				{
					actLength -= tempLength;
					found = true;
					break;
				}
				index++;
				index_next++;
			}
			if(!found)
			{
				index_next = 0;
				tempLength = MMSSVector3d::VectorLength(targetMesh[targetOriginArea[index_next]]-targetMesh[targetOriginArea[index]]);
			}
			if(tempLength!=0)ratio = (lengthInTargetOriginArea-actLength)/tempLength;
			else ratio = 0;
			originAreaSourcePositions[i] = targetMesh[targetOriginArea[index]] + (ratio * (targetMesh[targetOriginArea[index_next]]-targetMesh[targetOriginArea[index]]));
		}

		//insert area
		acumulatedLength = 0;
		actLength = 0;
		float lengthInTargetInsertArea;
		
		insertAreaSourcePositions[0] = targetMesh[targetInsertArea[0]];//set position near to first

		for(unsigned int i = 1; i < sourceInsertAreaCount; i++)
		{
			acumulatedLength += MMSSVector3d::VectorLength(sourceMesh[sourceInsertArea[i]]-sourceMesh[sourceInsertArea[i-1]]);
			ratio = acumulatedLength / sourceInsertAreaLength;
			lengthInTargetInsertArea = ratio * targetInsertAreaLength;

			actLength = 0;
			unsigned int index = 0;
			unsigned int index_next = 1;

			bool found = false;
			//found edge of target area where new position will be
			while(index < (targetInsertAreaCount-1))
			{
				tempLength = MMSSVector3d::VectorLength(targetMesh[targetInsertArea[index_next]]-targetMesh[targetInsertArea[index]]);
				actLength += tempLength;
				if(actLength > lengthInTargetInsertArea)
				{
					actLength -= tempLength;
					found = true;
					break;
				}
				index++;
				index_next++;
			}
			if(!found)
			{
				index_next = 0;
				tempLength = MMSSVector3d::VectorLength(targetMesh[targetInsertArea[index_next]]-targetMesh[targetInsertArea[index]]);
			}
			if(tempLength!=0)ratio = (lengthInTargetInsertArea-actLength)/tempLength;
			else ratio = 0;
			
			insertAreaSourcePositions[i] = targetMesh[targetInsertArea[index]] + (ratio * (targetMesh[targetInsertArea[index_next]]-targetMesh[targetInsertArea[index]]));
		}
		
		//join indices of areas to one array like fixing indices
		GLuint fixedIndicesCount = sourceOriginAreaCount + sourceInsertAreaCount;
		GLuint* fixedIndices = (GLuint*)malloc(fixedIndicesCount*sizeof(GLuint));
		int index = 0;
		for(unsigned int i = 0; i < sourceOriginAreaCount; i++)
		{
			fixedIndices[index] = sourceOriginArea[i];
			index++;
		}
		for(unsigned int i = 0; i < sourceInsertAreaCount; i++)
		{
			fixedIndices[index] = sourceInsertArea[i];
			index++;
		}

		//smooth move points to target positions
		float iterationAlpha = 1.0f / FEATURE_ALIGN_ITERATION;
		float alpha = 0;
		for(int i = 0; i < FEATURE_ALIGN_ITERATION; i++)
		{
			alpha += iterationAlpha;
			
			for(unsigned int j = 0; j < sourceOriginAreaCount; j++)
			{
				sourceMesh[sourceOriginArea[j]] = (1-alpha) * savedOriginAreaSourcePositions[j] + alpha * originAreaSourcePositions[j];
			}

			for(unsigned int j = 0; j < sourceInsertAreaCount; j++)
			{
				sourceMesh[sourceInsertArea[j]] = (1-alpha) * savedInsertAreaSourcePositions[j] + alpha * insertAreaSourcePositions[j];
			}

			sourceMesh = TransformToUnitSphere(sourceMesh,sourceMeshCount,sourceMeshEdges,sourceMeshEdgesCount,fixedIndices,fixedIndicesCount);//NULL and 0 last parameters are no added fixing
			
		}

		free(originAreaSourcePositions);
		free(savedOriginAreaSourcePositions);

		free(insertAreaSourcePositions);
		free(savedInsertAreaSourcePositions);

		free(fixedIndices);
	}

	#pragma region Transform points to target mesh
	//-------------------------------------------------------------------------------------
	//Find barycentric coords of source mesh embeding in triangles of target mesh embeding
	//Transform points to target mesh
	//-------------------------------------------------------------------------------------
	free(edges);
	Plane p;
	unsigned int tmpIndex;
	//prepare planes from triangles
	for(unsigned int i = 0; i < trianglesCount;i++)
	{
		p.n = MMSSVector3d::VectorCross(targetMesh[triangles[i].p3_index]-targetMesh[triangles[i].p1_index],
													targetMesh[triangles[i].p2_index]-targetMesh[triangles[i].p1_index]);
		
		p.d = p.n * targetMesh[triangles[i].p1_index];//dot product
		p.d = -p.d;
			
		if(p.d>0)
		{
			//if d>0 -> center(0,0,0) is up of plane, we need down of plane
			tmpIndex = triangles[i].p3_index;
			triangles[i].p3_index = triangles[i].p2_index;
			triangles[i].p2_index = tmpIndex;

			p.n = MMSSVector3d::VectorCross(targetMesh[triangles[i].p3_index]-targetMesh[triangles[i].p1_index],
													targetMesh[triangles[i].p2_index]-targetMesh[triangles[i].p1_index]);
			
			p.d = p.n * targetMesh[triangles[i].p1_index];//dot product
			p.d = -p.d;
		}
		planesTriangle[i] = p;
	}

	//find barycentric coord
	float t;
	float dotProduct;
	Vector intersection;
	float u,v,w;//barycentric coord
	for(unsigned int i = 0; i < sourceMeshCount;i++)
	{
		for(unsigned int j = 0; j < trianglesCount;j++)
		{
			dotProduct = sourceMesh[i]*planesTriangle[j].n;//dot product
			if((dotProduct + planesTriangle[j].d)>=0)
			{
				//point is up of plane -> may be lay in triangle
				
				//count intersect
				t = (-planesTriangle[j].d) / (dotProduct);
				intersection = t * sourceMesh[i];
				//sourceMesh[i] = intersection;
				
				n1 = MMSSVector3d::VectorCross(targetMesh[triangles[j].p2_index]-intersection,targetMesh[triangles[j].p1_index]-intersection);
				n2 = MMSSVector3d::VectorCross(targetMesh[triangles[j].p3_index]-intersection,targetMesh[triangles[j].p2_index]-intersection);
				n3 = MMSSVector3d::VectorCross(targetMesh[triangles[j].p1_index]-intersection,targetMesh[triangles[j].p3_index]-intersection);

				u = MMSSVector3d::VectorLength(n1);
				v = MMSSVector3d::VectorLength(n2);
				w = MMSSVector3d::VectorLength(n3);
				
				//we need oriented 
				if(n1*planesTriangle[j].n < 0)u = -u;
				if(n2*planesTriangle[j].n < 0)v = -v;
				if(n3*planesTriangle[j].n < 0)w = -w;
				
				if(u<0 && v < 0 && w < 0)
				{
					u = -u;
					v = -v;
					w = -w;
				}
				
				//len = fabs(u) + fabs(v) + fabs(w);
				len = MMSSVector3d::VectorLength(planesTriangle[j].n);
				u = u / len;
				v = v / len;
				w = w / len;

				len = u + v + w;

				if(u < 0 || u > 1 || v < 0 || v > 1 || w < 0 || w > 1)continue;
				else
				{
					sourceMesh[i] = u*targetMeshPoints[triangles[j].p3_index] +
									v*targetMeshPoints[triangles[j].p1_index] +
									w*targetMeshPoints[triangles[j].p2_index];
					break;
				}
				
			}
		}
	}
	#pragma endregion

	free(planesTriangle);
}

/*
Get interpolated positions on target mesh.
*/
MMSSMorphing::Vector* MMSSMorphing::GetSourceVerticesOnTargetMesh()
{
	return sourceMesh;
}

/*
transform points to unit sphere
*/
MMSSMorphing::Vector* MMSSMorphing::TransformToUnitSphere(Vector* meshPoints,GLuint pointsCount,Edge* edges,GLuint edgesCount, GLuint* fixedIndices, GLuint fixedIndicesCount)
{
	Vector* initMeshPoints = (Vector*)malloc(pointsCount*sizeof(Vector));

	//place for fixing indices
	unsigned int fixedCount = fixedIndicesCount + 4;//+4 is place for tetrahedra fixing
	unsigned int* fixed = (unsigned int*)malloc(fixedCount*sizeof(unsigned int));
	for(unsigned int i = 0; i < fixedIndicesCount; i++)
	{
		fixed[i+4] = fixedIndices[i];
	}

	//save meshPoints positions for restarting
	for(unsigned int i = 0; i < pointsCount; i++)
	{
		initMeshPoints[i] = meshPoints[i];
	}
	
	GLfloat m[16];
	GLdouble t[16];

	int iteration = 0;

	do
	{
		//generate random transformation of tetrahedra
		glGetFloatv(GL_MODELVIEW_MATRIX,m);//get orig transform
		glLoadIdentity();
		glRotatef((float)(rand()%360),(float)(rand()%1000)/1000,(float)(rand()%1000)/1000,(float)(rand()%1000)/1000);
		glGetDoublev(GL_MODELVIEW_MATRIX,t);
		Vector a = MMSSVector3d::Transform(tetrahedraP1,t);
		Vector b = MMSSVector3d::Transform(tetrahedraP2,t);
		Vector c = MMSSVector3d::Transform(tetrahedraP3,t);
		Vector d = MMSSVector3d::Transform(tetrahedraP4,t);
	
		glLoadMatrixf(m);//load back original matrix
	
		for(unsigned int i = 0; i < pointsCount; i++)
		{
			meshPoints[i] = MMSSVector3d::Normalize(meshPoints[i]);
		}

		//fixing nearest points from tetrahedra
		
		float lengthA = FLT_MAX;
		float lengthB = FLT_MAX;
		float lengthC = FLT_MAX;
		float lengthD = FLT_MAX;
		float l;

		for(unsigned int i = 0; i < pointsCount; i++)
		{
			l = MMSSVector3d::VectorLength(meshPoints[i]-a); 
			if(l<lengthA)
			{
				lengthA=l;
				fixed[0] = i;
			}
		}

		
		for(unsigned int i = 0; i < pointsCount; i++)
		{
			l = MMSSVector3d::VectorLength(meshPoints[i]-b); 
			if(l<lengthB && i != fixed[0])
			{
				lengthB=l;
				fixed[1] = i;
				continue;
			}
		}
		
		for(unsigned int i = 0; i < pointsCount; i++)
		{
			l = MMSSVector3d::VectorLength(meshPoints[i]-c); 
			if(l<lengthC && i != fixed[0] && i != fixed[1])
			{
				lengthC = l;
				fixed[2] = i;
				continue;
			}
		}

		for(unsigned int i = 0; i < pointsCount; i++)
		{
			l = MMSSVector3d::VectorLength(meshPoints[i]-d); 
			if(l<lengthD && i != fixed[0] && i != fixed[1] && i != fixed[2])
			{
				lengthD = l;
				fixed[3] = i;
				continue;
			}
		}

		meshPoints = Relaxation(meshPoints,pointsCount,edges,edgesCount,fixed,fixedCount);

		//check if relaxation collapse
		//find nearest point to diametric points of tetrahedra and check distance, if less than half of length of edge of tetrahedra, its OK
		Vector zero;
		zero.x = 0;
		zero.y = 0;
		zero.z = 0;

		a = zero - a;
		b = zero - b;
		c = zero - c;
		d = zero - d;
		
		lengthA = FLT_MAX;
		lengthB = FLT_MAX;
		lengthC = FLT_MAX;
		lengthD = FLT_MAX;

		for(unsigned int i = 0; i < pointsCount; i++)
		{
			l = MMSSVector3d::VectorLength(meshPoints[i]-a); 
			if(l<lengthA)
			{
				lengthA=l;
			}

			l = MMSSVector3d::VectorLength(meshPoints[i]-b); 
			if(l<lengthB)
			{
				lengthB=l;
			}

			l = MMSSVector3d::VectorLength(meshPoints[i]-c); 
			if(l<lengthC)
			{
				lengthC = l;
			}

			l = MMSSVector3d::VectorLength(meshPoints[i]-d); 
			if(l<lengthD)
			{
				lengthD = l;
			}
		}

		iteration++;

		if(lengthA > 0.5f || lengthB > 0.5f || lengthC > 0.5f || lengthD > 0.5f )
		{
			//load meshPoints for restarting
			for(unsigned int i = 0; i < pointsCount; i++)
			{
				meshPoints[i] = initMeshPoints[i];
			}

			if(iteration > MAX_RELAXATION_ITERATION)break;

			continue;
		}
		else break;

	}
	while(true);

	free(initMeshPoints);
	free(fixed);

	return meshPoints;
}

/*
return sum of diferences before and after relaxation
*/
MMSSMorphing::Vector* MMSSMorphing::Relaxation(Vector* meshPoints,GLuint pointsCount,Edge* edges,GLuint edgesCount, GLuint* fixedIndices, GLuint fixedIndicesSize)
{
	//relaxation
	Vector* acumPos = (Vector*)malloc(pointsCount*sizeof(Vector));
	Vector temp;
	int* counting = (int*)malloc(pointsCount*sizeof(int));
	
	float diference = FLT_MAX;
	int iteration = 0;

	while(diference > maxDiference && iteration < MAX_RELAXATION_ITERATION)
	{
		diference = 0;

		for(unsigned int i = 0; i < pointsCount; i++)
		{
			counting[i] = 0;
			acumPos[i].x = 0;
			acumPos[i].y = 0;
			acumPos[i].z = 0;
		}
		
		for(unsigned int i = 0; i < edgesCount; i++)
		{
			acumPos[edges[i].p1_index] = acumPos[edges[i].p1_index] + meshPoints[edges[i].p2_index];
			counting[edges[i].p1_index]++;
			acumPos[edges[i].p2_index] = acumPos[edges[i].p2_index] + meshPoints[edges[i].p1_index];
			counting[edges[i].p2_index]++;
		}
		
		for(unsigned int i = 0; i < pointsCount; i++)
		{
			if(IsFixed(i,fixedIndices,fixedIndicesSize))
			{
				temp = MMSSVector3d::Normalize(meshPoints[i]);
			}
			else
			{
				temp.x = acumPos[i].x/counting[i];
				temp.y = acumPos[i].y/counting[i];
				temp.z = acumPos[i].z/counting[i];
		
				temp= MMSSVector3d::Normalize(temp);
			}
			
			diference += MMSSVector3d::VectorLength(meshPoints[i]-temp);
			meshPoints[i] = temp;
		}

		iteration++;
		
	}

	free(acumPos);
	free(counting);
	return meshPoints;
}

/*
Check if index is in fixed indices.
*/
bool MMSSMorphing::IsFixed(GLuint index,GLuint* fixedIndices, GLuint fixedIndicesSize)
{
	for(unsigned int i = 0; i < fixedIndicesSize; i++)
	{
		if(index==fixedIndices[i])return true;
	}

	return false;
}

/*
Get circumsphere of model, which points is in param.
Result of algorithm center and rafius will be in center and radius params.
*/
void MMSSMorphing::GetCircumSphere(Vector* meshPoints,GLuint pointsCount,Vector* outCenter, GLfloat* outRadius)
{
	//init center of object
	Vector center;
	center.x = 0;
	center.y = 0;
	center.z = 0;

	//count circumsphere
	int indexXMin = 0;
	int indexXMax = 0;
	int indexYMin = 0;
	int indexYMax = 0;
	int indexZMin = 0;
	int indexZMax = 0;
	float xMin = meshPoints[0].x;
	float xMax = meshPoints[0].x;
	float yMin = meshPoints[0].y;
	float yMax = meshPoints[0].y;
	float zMin = meshPoints[0].z;
	float zMax = meshPoints[0].z;

	//go through vertices and find minmax box
	for(unsigned int i = 1; i < pointsCount; i++)
	{
		if(meshPoints[i].x < xMin)
		{
			xMin = meshPoints[i].x;
			indexXMin = i;
		}
		else if(meshPoints[i].x > xMax)
		{
			xMax = meshPoints[i].x;
			indexXMax = i;
		}

		if(meshPoints[i].y < yMin)
		{
			yMin = meshPoints[i].y;
			indexYMin = i;
		}
		else if(meshPoints[i].y > yMax)
		{
			yMax = meshPoints[i].y;
			indexYMax = i;
		}

		if(meshPoints[i].z < zMin)
		{
			zMin = meshPoints[i].z;
			indexZMin = i;
		}
		else if(meshPoints[i].z > zMax)
		{
			zMax = meshPoints[i].z;
			indexZMax = i;
		}
	}

	//count init center
	float radius = xMax-xMin;
	center.x = meshPoints[indexXMin].x + 0.5f * (meshPoints[indexXMax].x - meshPoints[indexXMin].x);
	center.y = meshPoints[indexYMin].y + 0.5f * (meshPoints[indexYMax].y - meshPoints[indexYMin].y);
	center.z = meshPoints[indexZMin].z + 0.5f * (meshPoints[indexZMax].z - meshPoints[indexZMin].z);
	
	if((yMax-yMin)/2>radius)
	{
		center = meshPoints[indexYMin] + 0.5f * (meshPoints[indexYMax] - meshPoints[indexYMin]);
		radius = (yMax-yMin)/2;
	}
	if((zMax-zMin)/2>radius)
	{
		center = meshPoints[indexZMin] + 0.5f * (meshPoints[indexZMax] - meshPoints[indexZMin]);
		radius = (zMax-zMin)/2;
	}

	//go through points if point is out of act sphere -> actualize center and diameter of sphere
	float r;
	for(unsigned int i = 0; i < pointsCount; i++)
	{
		r = MMSSVector3d::VectorLength(meshPoints[i]-center); 
		if(r > radius)
		{
			center = ((radius+r)/2)*MMSSVector3d::Normalize(center-meshPoints[i]) + meshPoints[i];
			radius = (radius + r)/2;
		}
	}

	*outCenter = center;
	*outRadius = radius;
}