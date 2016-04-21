#include "collisiondetection.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define NV_REPORT_COMPILE_ERRORS
#include <nvShaderUtils.h>

#include "MeshSurface.h"
#include <vtkPolyData.h>
#include <vtkGenericCell.h>
#include <string>

#include "mafDbg.h"

CollisionDetection::CollisionDetection(): GPUOGL_CD(512, 512) {
}

CollisionDetection::~CollisionDetection() {
}

gpuModel CollisionDetection::generateBuffers(vtkPolyData* data, bool deformable) {

	gpuModel model;
	double bounds[6];
	data->GetBounds(bounds);
	for(int i = 0; i < 3; i++) {
		model.min[i] = (float) bounds[2*i];
		model.max[i] = (float) bounds[2*i+1];
	}

	model.numberOfPoints = data->GetNumberOfPoints();
	int numberOfCoords = 3*model.numberOfPoints;
	int numberOfCells = data->GetNumberOfCells();
	model.numberOfIds = 3*numberOfCells;

	double* points = NULL;
	//if(!mesh->GetDeformable()) {
	points = new double[numberOfCoords];
	for(unsigned int i = 0; i < model.numberOfPoints; i++) {
		data->GetPoint(i, &points[3*i]);
	}
	//}

	GLuint* ids = new GLuint[model.numberOfIds];	
	for(int i = 0; i < numberOfCells; i++) {
		vtkIdType nPts, *pPts;
		data->GetCellPoints(i, nPts, pPts);
		_ASSERT(nPts == 3);		//only triangles are supported

		for(int j = 0; j < 3; j++) {
			ids[3*i + j] = (GLuint) pPts[j];
		}
	}

	glGenBuffers(1, &model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
	//if(!mesh->GetDeformable()) {
	glBufferData(GL_ARRAY_BUFFER, numberOfCoords * sizeof(double), points, GL_STATIC_DRAW);
	//}
	//else {
	//	glBufferData(GL_ARRAY_BUFFER, numberOfCoords * sizeof(double), points, GL_DYNAMIC_COPY); //or GL_DYNAMIC_DRAW?
	//}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &model.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.numberOfIds * sizeof(GLuint), ids, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete[] points;
	delete[] ids;

	return model;
}

void CollisionDetection::updateBuffers(gpuModel* model, double* data) {
	model->max.x = model->min.x = data[0];
	model->max.y = model->min.y = data[1];
	model->max.z = model->min.z = data[2];
	for(int i = 3; i < (int)model->numberOfPoints*3; i+=3) {
		if(model->max.x < data[i]) model->max.x = (float) data[i];
		if(model->max.y < data[i+1]) model->max.y = (float) data[i+1];
		if(model->max.z < data[i+2]) model->max.z = (float) data[i+2];
		if(model->min.x > data[i]) model->min.x = (float) data[i];
		if(model->min.y > data[i+1]) model->min.y = (float) data[i+1];
		if(model->min.z > data[i+2]) model->min.z = (float) data[i+2];
	}
	glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
	glBufferData(GL_ARRAY_BUFFER, 3*model->numberOfPoints * sizeof(double), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CollisionDetection::generateIdBuffers(int numberOfPoints) {

	this->numberOfPoints = numberOfPoints;
	shiftBuffer.numberOfPoints = numberOfPoints;
	shiftBuffer.numberOfIds = numberOfPoints;
	xcoordBuffer.numberOfPoints = numberOfPoints;
	xcoordBuffer.numberOfIds = numberOfPoints;

	GLfloat* shifts = new GLfloat[shiftBuffer.numberOfIds];
	GLfloat* xCoords = new GLfloat[xcoordBuffer.numberOfIds];
	unsigned int d = numberOfPoints / 32;
	if(numberOfPoints % 32 != 0) d++;
	GLfloat step = (GLfloat) 1.0/d;
	GLfloat shift = 0;
	GLfloat xCoord = (GLfloat) 0.5*step;
	for(unsigned int i = 0; i < xcoordBuffer.numberOfIds; i++) {
		xCoords[i] = xCoord;
		shifts[i] = shift;
		shift++;
		if(shift == 32) {
			xCoord += step;
			shift = 0;
		}
		//printf("x: %f, s: %u\n", xCoords[i], shifts[i]);
	}

	glGenBuffers(1, &shiftBuffer.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, shiftBuffer.vbo);
	glBufferData(GL_ARRAY_BUFFER, shiftBuffer.numberOfIds * sizeof(GLfloat), shifts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &xcoordBuffer.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, xcoordBuffer.vbo);
	glBufferData(GL_ARRAY_BUFFER, xcoordBuffer.numberOfIds * sizeof(GLfloat), xCoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] shifts;
	delete[] xCoords;
}

void CollisionDetection::drawBuffer(gpuModel model, GLenum primitive) {
	// bind buffers
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ebo);
	glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, 0);


	// Draw the elements
	glDrawElements(
		primitive,          // mode
		model.numberOfIds,  // count
		GL_UNSIGNED_INT,    // type
		0                   // element array buffer offset
		);

	// unbind buffers
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);
}

void CollisionDetection::testBuffer(gpuModel model, GLenum primitive, GLuint program) {
	// bind buffers

	//GLuint vertex = glGetAttribLocation(program, "vertex");
	GLuint shift = glGetAttribLocation(program, "shift");
	GLuint xCoord = glGetAttribLocation(program, "xCoord");

	glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
	//glEnableClientState(GL_VERTEX_ARRAY);
	//glVertexPointer(3, GL_FLOAT, 0, 0);
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, shiftBuffer.vbo);
	//glEnableClientState(GL_COLOR_ARRAY);
	//glVertexPointer(1, GL_UNSIGNED_INT, 0, 0);
	glEnableVertexAttribArray(shift);
	glVertexAttribPointer(shift, 1, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, xcoordBuffer.vbo);
	//glEnableClientState(GL_COLOR_ARRAY);
	//glColorPointer(1, GL_FLOAT, 0, 0);
	glEnableVertexAttribArray(xCoord);
	glVertexAttribPointer(xCoord, 1, GL_FLOAT, GL_FALSE, 0, 0);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ebo);

	glDrawArrays(GL_POINTS, 0, model.numberOfPoints);
	// Draw the elements
	//glDrawElements(
	//    primitive,          // mode
	//    model.numberOfIds,  // count
	//    GL_UNSIGNED_INT,    // type
	//    0                   // element array buffer offset
	//);

	// unbind buffers
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_COLOR_ARRAY);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(shift);
	glDisableVertexAttribArray(xCoord);
}

void CollisionDetection::modelViewMatrix(gpuModel model, bool test) {
	vec3f center;
	vec3f factor = 2.0f / (model.max - model.min);
	//float scale = 2.0f / nv::length(sceneMax - sceneMin);

	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	//if(test) {
	center = (model.max + model.min) * 0.5f;
	glLoadIdentity();
	glScalef(factor.x, factor.y, factor.z);
	//}
	//else {
	//    center = (sceneMax + sceneMin) * 0.5f;
	//    glScalef(scale, scale, scale);
	//}
	glTranslatef(-center.x, -center.y, -center.z);
}

bool CollisionDetection::extensionsAvailable() {

	glewInit();

	if (!glewIsSupported(
		"GL_VERSION_2_1 "
		"GL_EXT_texture_array "
		"GL_EXT_gpu_shader4 "
		))
	{
		printf("Unable to load extension(s), this app requires:\n"
			" OpenGL version 2.1\n"
			" GL_EXT_texture_array\n"
			" GL_EXT_gpu_shader4\n\n"
			"Exiting...\n");
		return false;
	}

	glClearColor(0.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	swapBuffers();

	return true;
}

/*gpuModel CollisionDetection::initData(vtkPolyData* data) {//1, vtkSmartPointer<vtkPolyData> data2) {

gpuModel model;
double bounds[6];
data->GetBounds(bounds);
//data2->GetBounds(bounds2);
model.numberOfIds = data->GetNumberOfPoints();

model = generateBuffers(data);

//gpuModel2 = generateBuffers(data2);
//generateIdBuffers();

for(int i = 0; i < 3; i++) {
//sceneMin[i] = bounds1[2*i] < bounds2[2*i] ? bounds1[2*i] : bounds2[2*i];
//sceneMax[i] = bounds1[2*i+1] > bounds2[2*i+1] ? bounds1[2*i+1] : bounds2[2*i+1];
model.min[i] = bounds[2*i];
model.max[i] = bounds[2*i+1];
}
}*/

void CollisionDetection::init() {


	unsigned int mask2[TEXTURES][RES][4];
	unsigned int mask4[TEXTURES][RES][4];

	unsigned int value2[TEXTURES][4];
	unsigned int* value3 = &value2[0][0];
	unsigned int value4[TEXTURES][4];
	unsigned int* value5 = &value4[0][0];
	for(int res = 0; res < TEXTURES*4; res++) {
		value3[res] = 0u;
		value5[res] = 0u;
	}
	int index = 0;
	value3[index] = 1u;
	value5[index] = 1u;

	for(int res = 0; res < RES; res++) {
		for(int i = 0; i < TEXTURES; i++) {
			for(int j = 0; j < 4; j++) {
				mask2[i][res][j] = value2[i][j];
				mask4[i][res][j] = value4[i][j];
				//printf("%u, ", mask4[i][res][j]);
			}
		}
		//printf("\n");
		if(value3[index] == 0xffffffff) {
			value5[index] = 0u;
			index++;
			if(index < TEXTURES*4) value5[index] = 1u;
		}
		else {
			value5[index] = value5[index] << 1;
		}
		if(index < TEXTURES*4) value3[index] = (value3[index] << 1) + 1u;
	}

	//printf("\n");

	/*unsigned int* mask3 = &mask4[0][0][0];
	int band = 0;
	int block = 0;
	for(int res = 0; res < RES*TEXTURES*4; res++) {
	printf("%u ", mask3[res]);
	band++;
	if(band == 4) {
	printf("\n");
	band = 0;
	block++;
	}
	if(block == RES) {
	printf("\n");
	block = 0;
	}
	}*/


	// create 1d textures
	glGenTextures(1, &maskid);
	glBindTexture(GL_TEXTURE_1D_ARRAY_EXT, maskid);
	glTexParameteri(GL_TEXTURE_1D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_1D_ARRAY_EXT, 0, GL_RGBA32UI_EXT, RES, TEXTURES, 0, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT, mask2);
	//for (int t = 0; t < TEXTURES; t++) {
	//    glTexSubImage2D(GL_TEXTURE_1D_ARRAY_EXT, 0, 0, t, RES, 1, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT, &mask2[t][0][0]);
	//}

	glGenTextures(1, &depthmaskid);
	glBindTexture(GL_TEXTURE_1D_ARRAY_EXT, depthmaskid);
	glTexParameteri(GL_TEXTURE_1D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_1D_ARRAY_EXT, 0, GL_RGBA32UI_EXT, RES, TEXTURES, 0, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT, mask4);
	//for (int t = 0; t < TEXTURES; t++) {
	//    glTexSubImage2D(GL_TEXTURE_1D_ARRAY_EXT, 0, 0, t, RES, 1, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT, &mask4[t][0][0]);
	//}

	// create texture for storing results of vertices tests
	GLuint textureSize = numberOfPoints / 32;
	if(numberOfPoints % 32 != 0) {
		textureSize++;
	}

	glGenTextures(1, &resultsid);
	glBindTexture(GL_TEXTURE_1D, resultsid);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32UI_EXT, textureSize, 0, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT, 0);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32UI_EXT, textureSize, 0, GL_RED_INTEGER_EXT, GL_UNSIGNED_INT, 0);

	glGenTextures(1, &undecidedid);
	glBindTexture(GL_TEXTURE_1D, undecidedid);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32UI_EXT, textureSize, 0, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT, 0);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32UI_EXT, textureSize, 0, GL_RED_INTEGER_EXT, GL_UNSIGNED_INT, 0);

	// create 2d texture
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, texid);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_RGBA32UI_EXT, RES, RES, TEXTURES, 0, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT, 0);

	glGenTextures(1, &tex2id);
	glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, tex2id);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_RGBA32UI_EXT, RES, RES, TEXTURES, 0, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT, 0);

	//for(int t = 0; t < TEXTURES; t++) {
	for(int t = 0; t < 8; t++) {
		drawBuffers[t] = GL_COLOR_ATTACHMENT0_EXT + t;
	}

	//unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_1D_ARRAY_EXT, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);

	glGenFramebuffersEXT(1, &fboid);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboid);
	//glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, texid, 0);
	for(int t = 0; t < TEXTURES; t++) {
		glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + t, texid, 0, t);
	}

	glGenFramebuffersEXT(1, &fbo2id);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo2id);
	//glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, texid, 0);
	for(int t = 0; t < TEXTURES; t++) {
		glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + t, tex2id, 0, t);
	}

	glGenFramebuffersEXT(1, &resultsfboid);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, resultsfboid);
	glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, resultsid, 0);
	glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, undecidedid, 0);

	//unbind fbo
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

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
	str = str +  SHADER_PATH;  

	printf("The needed number of textures is %u.\n", TEXTURES);
	printf("Compiling vertex shader.\n");
	int vsid1 = nv::CompileGLSLShaderFromFile(GL_VERTEX_SHADER, (str + "layers.vert").c_str());
	printf("Compiling fragment shader.\n");
	int fsid1 = nv::CompileGLSLShaderFromFile(GL_FRAGMENT_SHADER, (str + "layers.frag").c_str());
	printf("Linking program.\n");
	progid = nv::LinkGLSLProgram(vsid1, fsid1);

	int vsid2 = nv::CompileGLSLShaderFromFile(GL_VERTEX_SHADER, (str + "pass.vert").c_str());
	int fsid2 = nv::CompileGLSLShaderFromFile(GL_FRAGMENT_SHADER, (str + "pass2.frag").c_str());
	saqid = nv::LinkGLSLProgram(vsid2, fsid2);

	int vsid3 = nv::CompileGLSLShaderFromFile(GL_VERTEX_SHADER, (str + "show.vert").c_str());
	int fsid3 = nv::CompileGLSLShaderFromFile(GL_FRAGMENT_SHADER, (str + "show.frag").c_str());
	showid = nv::LinkGLSLProgram(vsid3, fsid3);

	int vsid4 = nv::CompileGLSLShaderFromFile(GL_VERTEX_SHADER, (str + "phong.vert").c_str());
	int fsid4 = nv::CompileGLSLShaderFromFile(GL_FRAGMENT_SHADER, (str + "phong.frag").c_str());
	phongid = nv::LinkGLSLProgram(vsid4, fsid4);
}

void CollisionDetection::destroy() {
	glDeleteBuffers(1, &shiftBuffer.vbo);
	glDeleteBuffers(1, &xcoordBuffer.vbo);
	glDeleteProgram(progid);
	glDeleteProgram(saqid);
	glDeleteProgram(phongid);
	glDeleteProgram(showid);
	glDeleteTextures(1, &maskid);
	glDeleteTextures(1, &depthmaskid);
	glDeleteTextures(1, &resultsid);
	glDeleteTextures(1, &undecidedid);
	glDeleteTextures(1, &texid);
	glDeleteTextures(1, &tex2id);
	glDeleteFramebuffersEXT(1, &fboid);
	glDeleteFramebuffersEXT(1, &fbo2id);
	glDeleteFramebuffersEXT(1, &resultsfboid);
}

void CollisionDetection::prepareVoxelization() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// render object
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboid);
	glDrawBuffers(TEXTURES, drawBuffers);
	int fboStatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(fboStatus != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("FBO status is %d.\n", fboStatus);
	}
	glUseProgram(progid);
	int location = glGetUniformLocation(progid, "texture");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D_ARRAY_EXT, maskid);
	glUniform1i(location, 0);
	glClearColorIuiEXT(0u, 0u, 0u, 0u);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_LOGIC_OP);
	glBlendEquation(GL_LOGIC_OP);
	glLogicOp(GL_XOR);
	glViewport(0, 0, RES, RES);

}

void CollisionDetection::prepareBoundaryVoxelization() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// render object
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo2id);
	glDrawBuffers(TEXTURES, drawBuffers);
	int fboStatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(fboStatus != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("FBO status is %d.\n", fboStatus);
	}
	glUseProgram(progid);
	int location = glGetUniformLocation(progid, "texture");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D_ARRAY_EXT, depthmaskid);
	glUniform1i(location, 0);
	glClearColorIuiEXT(0u, 0u, 0u, 0u);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_LOGIC_OP);
	glBlendEquation(GL_LOGIC_OP);
	glLogicOp(GL_OR);
	glViewport(0, 0, RES, RES);

}

void CollisionDetection::voxelize(gpuModel model) {

	glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	modelViewMatrix(model, true);
	drawBuffer(model, GL_TRIANGLES);

}

void CollisionDetection::prepareTest() {

	GLuint textureSize = numberOfPoints / 32;
	if(numberOfPoints % 32 != 0) {
		textureSize++;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, resultsfboid);
	glDrawBuffers(2, drawBuffers);
	int fboStatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(fboStatus != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("FBO status is %d.\n", fboStatus);
	}
	glUseProgram(saqid);
	GLuint location = glGetUniformLocation(saqid, "texture");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, texid);
	glUniform1i(location, 0);
	location = glGetUniformLocation(saqid, "texture2");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, tex2id);
	glUniform1i(location, 1);
	location = glGetUniformLocation(saqid, "depthmask");
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_1D_ARRAY_EXT, depthmaskid);
	glUniform1i(location, 2);
	glClearColorIuiEXT(0u, 0u, 0u, 0u);
	//glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_OR);
	glViewport(0, 0, textureSize, 1);
}

/*void CollisionDetection::prepareBoundaryTest() {

GLuint textureSize = numberOfPoints / 32;
if(numberOfPoints % 32 != 0) {
textureSize++;
}

glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, resultsfboid);
glDrawBuffers(1, drawBuffers);
glUseProgram(saqid2);
GLuint location = glGetUniformLocation(saqid2, "texture");
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, texid);
glUniform1i(location, 0);
location = glGetUniformLocation(saqid2, "depthmask");
glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_1D_ARRAY_EXT, depthmaskid);
glUniform1i(location, 1);
glClearColorIuiEXT(0u, 0u, 0u, 0u);
//glClear(GL_COLOR_BUFFER_BIT);
glEnable(GL_BLEND);
glEnable(GL_COLOR_LOGIC_OP);
glLogicOp(GL_OR);
glViewport(0, 0, textureSize, 1);
}*/

char CollisionDetection::test(gpuModel model, unsigned int* buffer, unsigned int* buffer2) {

	GLuint textureSize = model.numberOfPoints / 32;
	if(model.numberOfPoints % 32 != 0) {
		textureSize++;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	testBuffer(model, GL_POINTS, saqid);
	//glPopMatrix();

	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glReadPixels(0, 0, textureSize, 1, GL_RED_INTEGER_EXT, GL_UNSIGNED_INT, buffer);
	glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
	glReadPixels(0, 0, textureSize, 1, GL_RED_INTEGER_EXT, GL_UNSIGNED_INT, buffer2);
	//glDrawBuffers(2, drawBuffers);

	//int counter = 0;
	bool intersect = false;
	bool undecided = false;
	inside = 0;
	for(unsigned int i = 0; i < textureSize; i++) {
		if(buffer2[i] != 0u) {
			undecided = true;
		}
		else if(buffer[i] != 0u) {
			intersect = true;
			/*for(int j = 0; j < 32; j++) {
			unsigned int mask = 0x1 << j;
			unsigned int test = buffer[i] & mask;
			if(test == mask) {
			inside++;
			}
			//counter++;
			}*/
		}
	}

	if(undecided) return 'u';
	if(inside) return 'i';
	return 'o';
}

/*bool CollisionDetection::boundaryTest(gpuModel model, unsigned int* buffer) {

GLuint textureSize = model.numberOfPoints / 32;
if(model.numberOfPoints % 32 != 0) {
textureSize++;
}

//unsigned int* hlp = new unsigned int[textureSize];

glClear(GL_COLOR_BUFFER_BIT);
testBuffer(model, GL_POINTS, saqid);
//glPopMatrix();

glReadPixels(0, 0, textureSize, 1, GL_RED_INTEGER_EXT, GL_UNSIGNED_INT, buffer);

//int counter = 0;
bool undecided = false;
inside = 0;
for(unsigned int i = 0; i < textureSize; i++) {
if(buffer[i] != 0u) {
undecided = true;
//buffer[i] = buffer[i] | hlp[i];
//for(int j = 0; j < 32; j++) {
//	unsigned int mask = 0x1 << j;
//	unsigned int test = buffer[i] & mask;
//	if(test == mask) {
//		inside++;
//	}
//	//counter++;
//}
}
}

//delete[] hlp;

if(undecided) return true;
return false;
}*/

void CollisionDetection::display(gpuModel model1, gpuModel model2) {

	glRotatef(5.0, 0.0, 1.0, 0.0);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glUseProgram(phongid);
	glClearColor(0.0, 0.0, 1.0, 0.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_COLOR_LOGIC_OP);
	glViewport(0, 0, width, height);

	glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	modelViewMatrix(model1, false);
	glColor3f(0.3, 0.3, 0.3);
	drawBuffer(model1, GL_TRIANGLES);
	glColor3f(0.0, 0.7, 0.0);
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	drawBuffer(model2, GL_TRIANGLES);
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );


	glDisable(GL_DEPTH_TEST);
	glUseProgram(showid);
	GLuint location = glGetUniformLocation(showid, "result");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, resultsid);
	glUniform1i(location, 0);
	glPointSize(2.0);

	testBuffer(model2, GL_POINTS, showid);
	glPointSize(1.0);
	//glPopMatrix();
	glDisable(GL_DEPTH_TEST);

	swapBuffers();
}
