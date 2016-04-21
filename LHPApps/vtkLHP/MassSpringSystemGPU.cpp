/*=========================================================================
  Program: Multimod Application Framework RELOADED 
  Module: $RCSfile: MassSpringSystemGPU.cpp,v $ 
  Language: C++ 
  Date: $Date: 2012-10-23 19:12:42 $ 
  Version: $Revision: 1.1.2.3 $ 
  Authors: Ivo Zelený, Jan Rus
  ========================================================================== 
  Copyright (c) 2011 University of West Bohemia (www.zcu.cz)
  See the COPYINGS file for license details 
  =========================================================================
*/

////////////////////////////////////////////////////////////////  
// This file contains:
// MassSpringSystemGPU class, render points and springs
// This is the GPU counting way to count particle system
////////////////////////////////////////////////////////////////

#include "MassSpringSystemGPU.h"
#include "Library.h"
#include <stdio.h>
#include "mafDefines.h"
#include "vtkMAFVisualDebugger.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkIdList.h"

#pragma warning(push)
#pragma warning(disable: 4996)
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkGlyph3D.h"
#include "vtkTubeFilter.h"
#include "vtkGraphicsFactory.h"
#include "mafDbg.h"

vtkRenderWindow *renWin;

/*
Create and initialize new vertices and springs for spring system
- vertices are all coordinates in one dimensional field x,y,z,x,y,z...
- springs contain all springs
- count contain count of vertices and springs
*/
MassSpringSystemGPU::MassSpringSystemGPU(MMSSVector3d* vertices, Spring* springs, Counts count)
{
	//init vals
	minStep = DEFAULT_MSS_MIN_STEP;
	dampConst = 1;
	weight = 1;
	timeReserve = 0;
	realTimebuffer = 0;
	iterationCount = 0;

	extForce.x = 0;
	extForce.y = 0;
	extForce.z = 0;

	this->count = count;

	this->timePicked = true;
	
	int w = (int)sqrt((float)count.verticesCount);
	int len = 256;
	while(len < w)len*=2;

	int rows = count.verticesCount / len + 1;
	float filledRowsRatio = ((float)rows)/len;

	textureWidth = len;
	textureHeight = len;
	halfPixelWidth = 1.0f/(2*textureWidth);
	halfPixelHeight = 1.0f/(2*textureHeight);
	size = textureWidth * textureHeight * 3;
	halfPixelWidthScreen = halfPixelWidth * 2;
	halfPixelHeightScreen = halfPixelHeight * 2;

	GLfloat* verts = (GLfloat*)malloc(size*sizeof(GLfloat));
	//set vertices positions to texture
	int index = 0;
	
	for(unsigned int i = 0; i < count.verticesCount; i++)
	{
		verts[index] = vertices[i].x;
		index++;
		verts[index] = vertices[i].y;
		index++;
		verts[index] = vertices[i].z;
		index++;
	}
	
	
	for(int i = index; i < size; i++)
	{
		verts[i] = 0;
	}
	
	renWin = vtkRenderWindow::New();
	//renWin->GetGenericContext();
	//renWin->OffScreenRenderingOn();
	//renWin->SetOffScreenRendering(1);
	renWin->Render();
	renWin->MakeCurrent();
	
	GLenum error = glewInit();

	if( error != GLEW_OK ) 
	{
		mafLogMessage("Glew init failed.");
		return;
	}

	vertexPositionTextures = (GLuint*)malloc(3*sizeof(GLuint));

	//gen 3 position texture for previous position, actual position -> new position
	glGenTextures(3, vertexPositionTextures);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, vertexPositionTextures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, verts);
	
	glBindTexture(GL_TEXTURE_2D, vertexPositionTextures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, verts);
	
	glBindTexture(GL_TEXTURE_2D, vertexPositionTextures[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, verts);
	
	//set texture for rendering acumulated forces to point
	glGenTextures(1, &acumulatedForceTexture);
	glBindTexture(GL_TEXTURE_2D, acumulatedForceTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, verts);

	//init all stuff for fast readback-pbo and memory
	glGenBuffers(1, &pbo);
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, size * sizeof(GLfloat),NULL, GL_STREAM_READ_ARB);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
	

	// create a renderbuffer object to store depth info
	glGenFramebuffers(1, &fboPos);
	glBindFramebuffer(GL_FRAMEBUFFER, fboPos);
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, vertexPositionTextures[1], 0);//attachment fbo
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
	
	glReadPixels(0, 0, textureWidth, textureHeight, GL_RGB,GL_FLOAT, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	actTexture = 1;
	
	renderTexture = 2;
	posTexture = 1;
	prevPosTexture = 0;

	free(verts);
	
#ifdef _WIN32
	char szPath[MAX_PATH];
	GetModuleFileNameA(NULL, szPath, MAX_PATH);

	std::string CurrentPath = szPath;
	std::string s1 = CurrentPath.substr(0,CurrentPath.rfind('/'));
	std::string s2 = CurrentPath.substr(0,CurrentPath.rfind('\\'));

	if (s1.size() > s2.size())
		CurrentPath = s2;
	else
		CurrentPath = s1;

	if (CurrentPath.size() > 0)
		CurrentPath += '/';
#else  
	wxString CurrentPath = wxGetCwd();
	CurrentPath += '/';
#endif //_WIN32

	std::string str = CurrentPath  ;
	str =str +  "Shaders/";  

	prg_render_pos = createShaderProgram((str + "MSS/vs_mss_rp.f").c_str(), (str + "MSS/fs_mss_rp.f").c_str());
	
	posTextureUniform = glGetUniformLocationARB(prg_render_pos,"posTexture");
	prevPosTextureUniform = glGetUniformLocationARB(prg_render_pos,"prevPosTexture");
	acumulatedForceTextureUniform = glGetUniformLocationARB(prg_render_pos,"acumulatedForceTexture");
	dtUniform = glGetUniformLocation(prg_render_pos, "dt");
	weightUniform = glGetUniformLocation(prg_render_pos, "weight");
	dampConstUniform = glGetUniformLocation(prg_render_pos, "dampConst");
	
	prg_render_force = createShaderProgram((str + "MSS/vs_mss_rf.f").c_str(), (str + "MSS/fs_mss_rf.f").c_str(), (str + "MSS/gs_mss_rf.f").c_str(),GL_POINTS,GL_POINTS);
	posTextureUniformForce = glGetUniformLocationARB(prg_render_force,"posTexture");
	externForceUniformForce = glGetUniformLocationARB(prg_render_force,"extForce");
	appToEndUniformForce = glGetUniformLocationARB(prg_render_force,"appToEnd");
	
	//create point represents springs
	SpringGPU* springsGPU = (SpringGPU*)malloc(count.springsCount*sizeof(SpringGPU));
	this->springs = (Spring*)malloc(count.springsCount*sizeof(SpringGPU));
	
	MMSSVector3d v,n;
	int vertexIndex;
	
	for(unsigned int i = 0; i < count.springsCount; i++)
	{
		//first index
		//convert index of point to its texture coord in position texture
		vertexIndex = springs[i].p1_index;
		v.x = (vertexIndex % textureWidth)*2*halfPixelWidth + halfPixelWidth;
		v.y = (vertexIndex / textureWidth)*2*halfPixelHeight + halfPixelHeight;
		v.z = 3;//set 3 means both ends of spring isn't fixed
		springsGPU[i].pos = v;
		
		vertexIndex = springs[i].p2_index;
		//convert index of spring to texture coord in force texture
		n.x = (vertexIndex % textureWidth)*2*halfPixelWidth + halfPixelWidth;
		n.y = (vertexIndex / textureWidth)*2*halfPixelHeight + halfPixelHeight;
		n.z = springs[i].initLen;
		springsGPU[i].nor = n;
		
		this->springs[i] = springs[i];
	}

	//generate buffer with points
	glGenBuffers(1, & buffer_springs );
	glBindBuffer( GL_ARRAY_BUFFER , buffer_springs );
	glBufferData( GL_ARRAY_BUFFER , count.springsCount*sizeof(SpringGPU) ,NULL , GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER , 0, count.springsCount*sizeof(SpringGPU) , &springsGPU[0] );
	glBindBuffer( GL_ARRAY_BUFFER , 0);

	free(springsGPU);//remove springs from CPU memory

	readedPixels = (GLfloat*)malloc(size*sizeof(GLfloat));

	Vertex_Quad* quad = (Vertex_Quad*)malloc(4*sizeof(Vertex_Quad));
	//prepare screen aligned quad
	quad[0].x = -1;
	quad[0].y = -1;
	quad[0].z = 1;
	quad[0].nx = 0;
	quad[0].ny = 0;
	quad[1].x = 1;
	quad[1].y = -1;
	quad[1].z = 1;
	quad[1].nx = 1;
	quad[1].ny = 0;
	quad[2].x = 1;
	quad[2].y = -1 + filledRowsRatio*2;
	quad[2].z = 1;
	quad[2].nx = 1;
	quad[2].ny = filledRowsRatio;
	quad[3].x = -1;
	quad[3].y = -1 + filledRowsRatio*2;
	quad[3].z = 1;
	quad[3].nx = 0;
	quad[3].ny = filledRowsRatio;

	glGenBuffers(1, &buffer_screen_aligned_quad);
	glBindBuffer( GL_ARRAY_BUFFER , buffer_screen_aligned_quad );
	glBufferData( GL_ARRAY_BUFFER , 4*sizeof(Vertex_Quad) ,NULL , GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER , 0, 4*sizeof(Vertex_Quad) , &quad[0] );
	glBindBuffer( GL_ARRAY_BUFFER , 0);
	free(quad);
}

/*
release alocated memory
*/
MassSpringSystemGPU::~MassSpringSystemGPU(void)
{
	glDeleteTextures( 3, vertexPositionTextures);
	glDeleteTextures( 1, &acumulatedForceTexture);
	
	//clear shaders
	freeShaders(prg_render_pos);
	freeShaders(prg_render_force);
	
	glDeleteFramebuffers(1, &fboPos);
	glDeleteBuffers(1,&buffer_springs);
	glDeleteBuffers(1,&buffer_screen_aligned_quad);
	glDeleteBuffers(1, &pbo);
	
	free(springs);
	free(vertexPositionTextures);
	free(readedPixels);

	renWin->Delete();
}

/*
Set fixed points, which will not be actualized, none force will be applicate.
All points, which indices in parameter will be set as fixed points, other points will be free.
Actualize whole array of point and spring -> better option for lot of points.
*/
void MassSpringSystemGPU::SetFixedPoints(GLuint* indices,MMSSVector3d* positions,GLuint size)
{
	renWin->MakeCurrent();

	//create point represents springs
	SpringGPU* springsGPU = (SpringGPU*)malloc(count.springsCount*sizeof(SpringGPU));
	
	MMSSVector3d v,n;
	int vertexIndex,fixedIndex;

	for(unsigned int i = 0; i < count.springsCount; i++)
	{
		//first index
		//convert index of point to its texture coord in position texture
		vertexIndex = springs[i].p1_index;
		v.x = (vertexIndex % textureWidth)*2*halfPixelWidth + halfPixelWidth;
		v.y = (vertexIndex / textureWidth)*2*halfPixelHeight + halfPixelHeight;
		v.z = 3;//set 3 means both ends of spring isn't fixed
		springsGPU[i].pos = v;
		
		vertexIndex = springs[i].p2_index;
		//convert index of spring to texture coord in force texture
		n.x = (vertexIndex % textureWidth)*2*halfPixelWidth + halfPixelWidth;
		n.y = (vertexIndex / textureWidth)*2*halfPixelHeight + halfPixelHeight;
		n.z = springs[i].initLen;
		springsGPU[i].nor = n;
	}

	for(unsigned int index = 0; index < size; index++)
	{
		fixedIndex = indices[index];
		for(unsigned int i = 0; i < count.springsCount; i++)
		{
			//first index
			if(fixedIndex == springs[i].p1_index)springsGPU[i].pos.z=springsGPU[i].pos.z-1;
			else if(fixedIndex == springs[i].p2_index)springsGPU[i].pos.z=springsGPU[i].pos.z-2; 
		}
	}

	glBindBuffer( GL_ARRAY_BUFFER , buffer_springs );
	glBufferSubData(GL_ARRAY_BUFFER , 0, count.springsCount*sizeof(SpringGPU) , &springsGPU[0] );
	glBindBuffer( GL_ARRAY_BUFFER , 0);

	free(springsGPU);

	//in the end move points because we need set prev positions and act positions to same values
	//if not and prev positions is diferent -> moving equation will be continue
	MovePoints(indices,positions,size);

}

/*
move points to new positions
-indices means how vertices will be moved (index equal index from vertex array in constructor)
-positions is one dimensional array of position in indices order(x,y,z,x,y,z...)
*/
void  MassSpringSystemGPU::MovePoints(GLuint* indices,MMSSVector3d* positions, GLuint size)
{
	renWin->MakeCurrent();
	//get active FBO
	GLint activeFBO;
	glGetIntegerv (GL_FRAMEBUFFER_BINDING_EXT, &activeFBO);

	//get act viewport
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glBindFramebuffer(GL_FRAMEBUFFER, fboPos);
	glViewport(0,0,textureWidth,textureHeight);
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, vertexPositionTextures[posTexture], 0);

	
	GLfloat* pos = (GLfloat*)malloc(4*sizeof(GLfloat));

	for(unsigned int i = 0; i < size; i++)
	{
		//set target frame buffer to previous position texture for set point position in this (if not point will have big energy)
		glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, vertexPositionTextures[prevPosTexture], 0);
		pos[0] = positions[indices[i]].x;
		pos[1] = positions[indices[i]].y;
		pos[2] = positions[indices[i]].z;

		//set actual pos in window, place for setting pixel
		glWindowPos2i(indices[i]%textureWidth,indices[i]/textureWidth);

		glDrawPixels(1,1,GL_RGB,GL_FLOAT,pos);

		//set target frame buffer to position texture for set point position in this
		glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, vertexPositionTextures[posTexture], 0);
		pos[0] = positions[indices[i]].x;
		pos[1] = positions[indices[i]].y;
		pos[2] = positions[indices[i]].z;

		//set actual pos in window, place for setting pixel
		glWindowPos2i(indices[i]%textureWidth,indices[i]/textureWidth);

		glDrawPixels(1,1,GL_RGB,GL_FLOAT,pos);
	}
	
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
	glReadPixels(0, 0, textureWidth, textureHeight, GL_RGB,GL_FLOAT, 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, activeFBO);
	glViewport(0,0,viewport[2],viewport[3]);

	free(pos);
}

/*
count vertices position in next step
- time means time diference between last time calling and now calling
*/
void  MassSpringSystemGPU::NextStep(float time)
{
	renWin->MakeCurrent();
	glFinish();
	if(this->timePicked)
	{
		init = clock();
		this->timePicked = false;
	}
	
	float actTime = this->minStep - this->timeReserve;
	
	//get active gpu program
	GLint activeProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM,&activeProgram);

	//get active FBO
	GLint activeFBO;
	glGetIntegerv (GL_FRAMEBUFFER_BINDING_EXT, &activeFBO);

	//get act viewport
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glBindFramebuffer(GL_FRAMEBUFFER, fboPos);
	glViewport(0,0,textureWidth,textureHeight);

	glUseProgramObjectARB(prg_render_pos);
	glUniform1f(dtUniform,minStep);
	glUniform1f(weightUniform,weight);
	glUniform1f(dampConstUniform,dampConst);

	glUseProgramObjectARB(prg_render_force);
	glUniform1i(posTextureUniformForce,posTexture);
	glUniform3f(externForceUniformForce,extForce.x,extForce.y,extForce.z);
	
	//render as point (one pixel -> one texel -> set force in texture)
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, vertexPositionTextures[0]);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, vertexPositionTextures[1]);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, vertexPositionTextures[2]);
		
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, acumulatedForceTexture);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);// Black Background -> zero forces everywhere

	glEnable( GL_TEXTURE_2D );

	//step with minStep 
	while(actTime < time)
	{
		
		//---------------------------render accumulated forces---------------------------------
		
		glUseProgramObjectARB(prg_render_force);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, acumulatedForceTexture, 0);
		
		glClear(GL_COLOR_BUFFER_BIT);
		
		//set blending for acumulate force in acumulated position texture
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_ONE);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

		//connect buffer for springs rendering
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glBindBuffer( GL_ARRAY_BUFFER , buffer_springs );
		glVertexPointer(3, GL_FLOAT, sizeof(SpringGPU),(char *)NULL);   //The starting point of the VBO, for the vertices
		glNormalPointer(GL_FLOAT, sizeof(SpringGPU), (char*)NULL+ sizeof(MMSSVector3d));
		
		//draw springs
		glDrawArrays(GL_POINTS, 0, count.springsCount);
		
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisable(GL_BLEND);
		
		//----------------------render positions------------------------------------
		
		glUseProgramObjectARB(prg_render_pos);
		glUniform1i(posTextureUniform,posTexture);	
		glUniform1i(prevPosTextureUniform,prevPosTexture);
		glUniform1i(acumulatedForceTextureUniform,3);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, vertexPositionTextures[renderTexture], 0);
		
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

		//place quad over screen(define place where count will be aplicated)
		//connect buffer for screen aligned quad
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glBindBuffer( GL_ARRAY_BUFFER , buffer_screen_aligned_quad );
		glVertexPointer(3,GL_FLOAT, sizeof(Vertex_Quad),(char *)NULL);   //The starting point of the VBO, for the vertices
		glNormalPointer(GL_FLOAT, sizeof(Vertex_Quad), (char*)NULL+ 3*sizeof(GLfloat));
		//glTexCoordPointer(2,GL_FLOAT, sizeof(Vertex_Quad), (char*)(NULL+ 3*sizeof(GLfloat)));
		
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		

		//cycle texture
		renderTexture = (renderTexture+1)%3;
		posTexture = (posTexture+1)%3;
		prevPosTexture = (prevPosTexture+1)%3;
		
		actTime += minStep;
		iterationCount++;
		
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glFinish();
	if(!timePicked)
	{
		realTimebuffer += ((float)(clock()-init))/((float)CLOCKS_PER_SEC);
		timePicked = true;
	}
	
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
	glReadPixels(0, 0, textureWidth, textureHeight, GL_RGB,GL_FLOAT, 0);

	//if there is some time reserve save it
	timeReserve = time - (actTime - minStep);

	glBindFramebuffer(GL_FRAMEBUFFER, activeFBO);
	glViewport(0,0,viewport[2],viewport[3]);
	glUseProgramObjectARB(activeProgram);
	
	glDisable( GL_TEXTURE_2D );

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, vertexPositionTextures[posTexture]);
}

/*
Method for setting parameters of mass spring system
all parameters must be greater than 0
*/
void MassSpringSystemGPU::SetCoefs(float damping, float weight)
{
	if(damping <= 0 || weight <= 0)return;
	this->dampConst = damping;
	this->weight = weight;
}

/*
Method for getting some inner counters as iterations which were done and time spended in counting iterations
*/
void MassSpringSystemGPU::GetTimeIteration(int* iteration, float* time)
{
	*iteration = this->iterationCount;
	*time =	this->realTimebuffer;
}

/*
Method for get all positions of vertices.
'Positions' is array of 'size' points, where positions will be stored.
Using asynchronous reading from texture and PBOs objects.
*/
void MassSpringSystemGPU::GetVertices(MMSSVector3d* positions,unsigned int size)
{
	renWin->MakeCurrent();
	GLfloat * buf = NULL;
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
	
	buf = (GLfloat*)glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);

	//memcpy(positions,buf,count.verticesCount*sizeof(GLfloat));
	
	int index = 0;
	for(unsigned int i = 0; i < size; i++)
	{
		positions[i].x = buf[index];
		index++;
		positions[i].y = buf[index];
		index++;
		positions[i].z = buf[index];
		index++;
	}
	
	//glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
	glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
	
	
}