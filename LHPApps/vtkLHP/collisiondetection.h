#ifndef COLLISIONDETECTION_H
#define COLLISIONDETECTION_H

#pragma once

#include "GPUOGL_CD.h"
#include <glew.h>
#include <GL/gl.h>
#include <nvMath.h>

#include <vtkPolyData.h>

//#define SHADER_PATH "C:/Users/cmolikl/Projects/Private/MAF/GPUCollisionDetection/"
#define SHADER_PATH "Shaders/GPUCollisionDetection/"

#define RES 1024
#define BITDEPTH 32
#define VIRTUALBITDEPTH (RES / 4)
#define TEXTURES VIRTUALBITDEPTH / BITDEPTH

using nv::vec3f;

typedef struct {
    GLuint vbo;
    GLuint ebo;
    GLuint numberOfPoints;
    GLuint numberOfIds;
    vec3f min;
    vec3f max;
} gpuModel;

class CollisionDetection : public GPUOGL_CD {
public:
	int inside;
private:
    gpuModel shiftBuffer;
    gpuModel xcoordBuffer;
    unsigned int numberOfPoints;

    GLenum drawBuffers[8];
    GLuint maskid;
    GLuint depthmaskid;
    GLuint resultsid;
	GLuint undecidedid;
    GLuint texid;
	GLuint tex2id;
    GLuint fboid;
	GLuint fbo2id;
    GLuint resultsfboid;
    GLuint progid;
    GLuint saqid;
	GLuint phongid;
	GLuint showid;
private:
    void drawBuffer(gpuModel model, GLenum primitive);
    void testBuffer(gpuModel model, GLenum primitive, GLuint program);
    void modelViewMatrix(gpuModel model, bool test);
public:
    CollisionDetection();
    ~CollisionDetection();
    void init();
	void destroy();
    bool extensionsAvailable();
    gpuModel generateBuffers(vtkPolyData* data, bool deformable);
	void updateBuffers(gpuModel* model, double* data);
    void generateIdBuffers(int numberOfPoints);
    void display(gpuModel model1, gpuModel model2);
    void prepareVoxelization();
	void prepareBoundaryVoxelization();
    void voxelize(gpuModel model);
    void prepareTest();
    char test(gpuModel model, unsigned int* buffer, unsigned int* buffer2);
};

#endif // COLLISIONDETECTION_H
