#undef UNICODE//unset unicode coding(because there is problem with names of window and windowClass)

#pragma once

#include "gl_init.h"
#include "MMSSVector3d.h"
#include "MMSSMorphing.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <vector>

const int MAX_ATTACHED_SHADERS_TO_PROGRAM = 100;

using namespace std;

/*
Show info dialog with error during compilation shader.
- fileName is name of file with shader
- object is actual compilated object
- last two params are functions for getting info about state of compilation
*/
void show_info_log(const char* fileName,GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);

/*
Create shader program from vertex shader and fragment shader, which are saved in files.
File names are in parameters.
*/
GLuint createShaderProgram(const char* vs_fileName, const char* fs_fileName);

/*
Create shader program from vertex shader and fragment shader and geometry shader, which are saved in files.
File names are in parameters. Other two parameters is specification of input and output primitive of geometry shader.
*/
GLuint createShaderProgram(const char* vs_fileName, const char* fs_fileName, const char* gs_fileName, GLint input_gs_primitive, GLint output_gs_primitive);

/*
Free shaders programs from GPU memory.
- identificator of shader program is shader_program_id
*/
void freeShaders(GLuint shader_program_id);

/*
Get transformation as glut look at.
Camera will view from eyePosition to center.
*/
void glLookAt( Vector eyePosition, Vector center, Vector upVector );

/*
rotation around userlike axis
- normalizedDirection is normalized direction vector of axis
- phi is angle in radians
- m is array of 16 elements for matrix stored in rows
*/
void rotationAroundAxis(Vector normalizedDirection, double phi, double* m);

int findNearestPoint(Vector* points, int pointsCount, Vector testedPoint);