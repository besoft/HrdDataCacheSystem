/*=========================================================================
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: MMSSMorphing.h,v $ 
  Language: C++ 
  Date: $Date: 2011-11-11 09:09:08 $ 
  Version: $Revision: 1.1.2.2 $ 
  Authors: Ivo Zelený
  ========================================================================== 
  Copyright (c) 2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/
#pragma once

#include "MMSSVector3d.h"


//max iteration if not stopped
const int MAX_RELAXATION_ITERATION = 10000;
const int FEATURE_ALIGN_ITERATION = 10;
const int FEATURE_ALIGN_ROTATION_ITERATION = 36;

//using namespace std;

class MMSSMorphing
{
public:
	typedef MMSSVector3d Vector;

	struct Edge{
		GLuint p1_index, p2_index;	//points of triangle.
	};

	struct Triangle{
		GLuint p1_index, p2_index, p3_index;
	};

	struct Plane{
		Vector n;
		float d;
	};	

public:
	MMSSMorphing();//default constructor
	
	~MMSSMorphing(void);
	
	/*
	Define source points and edges.
	*/
	void SetSourceMesh(Vector* sourceMeshPoints,GLuint pointsCount,Edge* edges,GLuint edgesCount);
	
	/*
	Define target mesh points and edges.
	*/
	void SetTargetMesh(Vector* targetMeshPoints,GLuint pointsCount,Triangle* triangles,GLuint trianglesCount);
	
	/*
	Get interpolated positions on target mesh.
	*/
	Vector* GetSourceVerticesOnTargetMesh();
	
	/*
	Set areas on source mesh.
	*/
	void SetSourceMeshAreas(GLuint* originArea, GLuint originAreaCount, GLuint* insertArea, GLuint insertAreaCount);
	
	/*
	Set areas on target mesh.
	*/
	void SetTargetMeshAreas(GLuint* originArea, GLuint originAreaCount, GLuint* insertArea, GLuint insertAreaCount);

private:
	
	/*
	transform points to unit sphere
	*/
	Vector* TransformToUnitSphere(Vector* meshPoints,GLuint pointsCount,Edge* edges,GLuint edgesCount, GLuint* fixedIndices, GLuint fixedIndicesCount);
	
	/*
	Provides relaxation of points on sphere.
	*/
	Vector* Relaxation(Vector* meshPoints,GLuint pointsCount,Edge* edges,GLuint edgesCount, GLuint* fixedIndices, GLuint fixedIndicesSize);
	
	/*
	Check if index is in fixed indices.
	*/
	bool IsFixed(GLuint index,GLuint* fixedIndices, GLuint fixedIndicesSize);
	
	/*
	Get circumsphere of model, which points is in param.
	Result of algorithm center and rafius will be in center and radius params.
	*/
	void GetCircumSphere(Vector* meshPoints,GLuint pointsCount,Vector* center, GLfloat* radius);

	GLfloat maxDiference;
	Vector* sourceMesh;
	GLuint sourceMeshCount;
	Vector* originalTargetMesh;
	Vector* targetMesh;
	Vector tetrahedraP1,tetrahedraP2,tetrahedraP3,tetrahedraP4;
	
	//origin and insertio areas
	GLuint* sourceOriginArea;
	GLuint sourceOriginAreaCount;
	GLuint* sourceInsertArea;
	GLuint sourceInsertAreaCount;
	GLuint* targetOriginArea;
	GLuint targetOriginAreaCount;
	GLuint* targetInsertArea;
	GLuint targetInsertAreaCount;

	Edge* sourceMeshEdges;
	GLuint sourceMeshEdgesCount;

	bool setTetrahedra;
};


