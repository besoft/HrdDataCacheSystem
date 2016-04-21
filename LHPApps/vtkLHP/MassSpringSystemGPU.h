
#pragma once

#include <ctime>
#include <cmath>
#include "gl_init.h"
#include "IMassSpringSystem.h"

#define DEFAULT_MSS_MIN_STEP 0.04f

/*
Implement system of springs.
Using inovative method with geometry shader for lower memory requirements.
*/
class MassSpringSystemGPU:public IMassSpringSystem
{
public:
	/*
	Constructor
	- vertices is array of vertices positions
	- springs is array of springs
	- count contains count of springs and vertices
	*/
	MassSpringSystemGPU(MMSSVector3d* vertices, Spring* springs, Counts count);//default constructor
	
private:

public:

	/*
	Destructor. Release all alocated memory.
	*/
	~MassSpringSystemGPU(void);
	
	/*
	Move points to new positions.
	- 'indices' means how vertices will be moved (index equal index from vertex array in constructor)
	- 'positions' is one dimensional array of position in indices order(x,y,z,x,y,z...)
	- 'size' is size of both arrays
	*/
	virtual void MovePoints(unsigned int* indices,MMSSVector3d* positions, unsigned int size);
	
	/*
	Set fixed points. Fixed points are points, which will not be actualized (none force will be applicate).
	- all points, which indices are set in parameter 'indices' will be set as fixed points, other points will be free.
	- 'positions' is array of 'size' elements -> it means fixed positions of points, which indices is set in param 'indices'
	*/
	virtual void SetFixedPoints(unsigned int* indices,MMSSVector3d* positions,unsigned int size);
	
	/*
	Count vertices position in next step
	- parameter 'time' means time diference between last state of MSS and new required state
	*/
	virtual void NextStep(float time);
	
	/*
	Get actual positions of all points.
	- parameter 'positions' is array of 'size' points, where positions will be stored.
	*/
	virtual void GetVertices(MMSSVector3d* positions, unsigned int size);
	
	/*
	Method for setting parameters of mass spring system
	- all parameters must be greater than 0
	*/
	virtual void SetCoefs(float damping, float weight);
	
	/*
	Method for getting some inner counters as iterations which were done and time spended in counting iterations
	*/
	virtual void GetTimeIteration(int* iteration, float* time);

	float minStep;//minimal step of simulation
	int actTexture;//identificator of texture with actual positions
	
	MMSSVector3d extForce;

	//GLuint vertexPositionTextur;

private:
	//struct for spring saved as point in vbo
	struct SpringGPU{
		MMSSVector3d pos;
		MMSSVector3d nor;
	};

	//struct for defining screen aligned quad
	struct Vertex_Quad{
		float x, y, z;
		float nx, ny, nz;
	 };
	
	float dampConst;//damping of springs
	float weight;//weight of points

	Counts count;
	
	//identificators of textures and FBO and PBO
	GLuint* vertexPositionTextures;
	GLuint fboPos;
	GLuint acumulatedForceTexture;
	GLuint pbo;
	
	//shader program for rendering new positions of points and uniforms
	GLuint prg_render_pos;
	GLuint posTextureUniform;
	GLuint prevPosTextureUniform;
	GLuint acumulatedForceTextureUniform;
	GLuint dtUniform;
	GLuint dampConstUniform;
	GLuint weightUniform;
	
	//shader program for counting forces in springs and mapping forces to points with uniforms
	GLuint prg_render_force;
	GLuint posTextureUniformForce;
	GLuint externForceUniformForce;
	GLuint appToEndUniformForce;
	
	// VBO buffers of points for rendering
	GLuint buffer_springs;
	GLuint buffer_screen_aligned_quad;

	GLfloat* readedPixels;

	//indices of textures with actual target render texture for rendering new positions, actual positions and previous posiitions
	int renderTexture;
	int posTexture;
	int prevPosTexture;

	Spring* springs;
	
	float timeReserve;

	//textures parameters
	int textureWidth;
	int textureHeight;
	float halfPixelWidth;
	float halfPixelHeight;
	int size;
	float halfPixelWidthScreen;
	float halfPixelHeightScreen;

	//for time counting
	bool timePicked;
	clock_t init;

	//for saving consumed time and iteration count
	float realTimebuffer;
	int iterationCount;

};