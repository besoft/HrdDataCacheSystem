
#include "Library.h"

using namespace std;

//show info about shader bailding
void show_info_log(const char* fileName,GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog)
{
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = (char*)malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
	/*control if message contain word success yes->everything is OK*/
	
	if(strstr(log,"error")!=0)
	{
		char str[1000];
		sprintf_s(str,"%s %s",fileName,log);
#ifdef _WIN32
		MessageBox(NULL,str,"ERROR",MB_OK|MB_ICONSTOP);				
#else
		#pragma comment "MessageBox to be placed here."
#endif
	}
	free(log);
}

/** Reads the content of the shader file. Returns "", if the file does not exist. */
GLchar* readShaderFile(const char* filename, GLint& read)
{
	ifstream ifs(filename, ifstream::in);	
	ifs.seekg(0, ios_base::end);
	int size = (int)ifs.tellg() + 1;
	ifs.seekg(0);
	
	GLchar* file = (GLchar*)malloc(size*sizeof(GLchar));
	GLint index = 0;	
	while(ifs.good())
	{
		file[index] = (GLchar)ifs.get();
		index++;
	}
	file[index > 0 ? index - 1 : 0] = 0;
	ifs.close();

	read = index;
	return file;
}

//create shader program from vertex shader program and fragment shader
GLuint createShaderProgram(const char* vs_fileName, const char* fs_fileName)
{
	GLint read;

	//read vertex shader
	GLchar* file = readShaderFile(vs_fileName, read);

	//compile vertex shader
	GLuint vs_obj = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs_obj, 1,(const GLchar**)&file, &read);	
	glCompileShader(vs_obj);
	show_info_log(vs_fileName,vs_obj,glGetObjectParameterivARB, glGetInfoLogARB);
	free(file);
	
	//read frament shader	
	file = readShaderFile(fs_fileName, read);
	
	//compile fragment shader
	GLuint ps_obj = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(ps_obj, 1,(const GLchar**)&file, &read);	
	glCompileShader(ps_obj);
	show_info_log(fs_fileName,ps_obj,glGetObjectParameterivARB, glGetInfoLogARB);
	free(file);
	
	//create program	
	GLuint prgobj = glCreateProgramObjectARB();
	glAttachObjectARB(prgobj, vs_obj);
	glAttachObjectARB(prgobj, ps_obj);
	glLinkProgram(prgobj);

	GLint linked;
	glGetProgramiv(prgobj, GL_OBJECT_LINK_STATUS_ARB, &linked);

	if(!linked)
	{
		//if not linked show error
		show_info_log(vs_fileName,prgobj,glGetProgramiv, glGetProgramInfoLog);
		show_info_log(fs_fileName,prgobj,glGetProgramiv, glGetProgramInfoLog);
	}
	
	return prgobj;	
}

//create shader program from vertex shader program, fragment shader, geometry shader
GLuint createShaderProgram(const char* vs_fileName, const char* fs_fileName, const char* gs_fileName, GLint input_gs_primitive, GLint output_gs_primitive)
{
	GLint read;

	//read vertex shader		
	GLchar* file = readShaderFile(vs_fileName, read);

	//compile vertex shader
	GLuint vs_obj = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs_obj, 1,(const GLchar**)&file, &read);	
	glCompileShader(vs_obj);
	show_info_log(vs_fileName,vs_obj,glGetObjectParameterivARB, glGetInfoLogARB);
	free(file);
	
	//read frament shader
	file = readShaderFile(fs_fileName, read);
	
	//compile fragment shader
	GLuint ps_obj = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(ps_obj, 1,(const GLchar**)&file, &read);	
	glCompileShader(ps_obj);
	show_info_log(fs_fileName,ps_obj,glGetObjectParameterivARB, glGetInfoLogARB);
	free(file);
		
	//read geometry shader
	file = readShaderFile(gs_fileName, read);
	
	//compile geometry shader
	GLuint gs_obj = glCreateShader(GL_GEOMETRY_SHADER_EXT);
	glShaderSource(gs_obj, 1,(const GLchar**)&file, &read);	
	glCompileShader(gs_obj);
	show_info_log(gs_fileName,gs_obj,glGetObjectParameterivARB, glGetInfoLogARB);
	free(file);
	

	//create program
	GLuint prgobj = glCreateProgramObjectARB();
	glAttachObjectARB(prgobj, vs_obj);
	glAttachObjectARB(prgobj, ps_obj);
	glAttachObjectARB(prgobj, gs_obj);

	//set input output primitives
	glProgramParameteriEXT(prgobj,GL_GEOMETRY_INPUT_TYPE_EXT,input_gs_primitive);
	glProgramParameteriEXT(prgobj,GL_GEOMETRY_OUTPUT_TYPE_EXT,output_gs_primitive);

	//set max vertices out from GS
	//int temp;
	//glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,&temp);
	glProgramParameteriEXT(prgobj,GL_GEOMETRY_VERTICES_OUT_EXT,2);

	glLinkProgram(prgobj);

	GLint linked;
	glGetProgramiv(prgobj, GL_OBJECT_LINK_STATUS_ARB, &linked);

	if(!linked)
	{
		//if not linked show error
		show_info_log(vs_fileName,prgobj,glGetProgramiv, glGetProgramInfoLog);
		show_info_log(fs_fileName,prgobj,glGetProgramiv, glGetProgramInfoLog);
		show_info_log(gs_fileName,prgobj,glGetProgramiv, glGetProgramInfoLog);

	}	
	return prgobj;
}

void freeShaders(GLuint shader_program_id)
{
	int size;
	GLuint* shaders = (GLuint*)malloc(MAX_ATTACHED_SHADERS_TO_PROGRAM*sizeof(GLuint));
	glGetAttachedShaders(shader_program_id, MAX_ATTACHED_SHADERS_TO_PROGRAM, &size, shaders);
	for(int i = 0; i < size; i++)
	{
		glDetachShader(shader_program_id,shaders[i]);
		glDeleteShader(shaders[i]);
	}
	glDeleteProgram(shader_program_id);
	free(shaders);
}

//implemented function for changing view matrix (look from eye to center and upVector define rotation of camera)
void glLookAt( MMSSVector3d eyePosition, MMSSVector3d center, MMSSVector3d upVector )
{
   MMSSVector3d forward, forwardB, side, up;
   float matrix2[16];
   
   forwardB = center - eyePosition;
   
   //normalize
   forward = MMSSVector3d::Normalize(forwardB);

   side = MMSSVector3d::VectorCross(forward, upVector);
   side = MMSSVector3d::Normalize(side);
   
   //Recompute up as: up = side x forward
   up = MMSSVector3d::VectorCross(side, forward);
   
   //M = ( s [ 0 ] s [ 1 ] s [ 2 ] 0 u [ 0 ] u [ 1 ] u [ 2 ] 0 -f [ 0 ] -f [ 1 ] -f [ 2 ] 0 0 0 0 1 )
   //------------------
   matrix2[0] = side.x;
   matrix2[4] = side.y;
   matrix2[8] = side.z;
   matrix2[12] = 0.0;
   //------------------
   matrix2[1] = up.x;
   matrix2[5] = up.y;
   matrix2[9] = up.z;
   matrix2[13] = 0.0;
   //------------------
   matrix2[2] = -forward.x;
   matrix2[6] = -forward.y;
   matrix2[10] = -forward.z;
   matrix2[14] = 0.0;
   //------------------
   matrix2[3] = matrix2[7] = matrix2[11] = 0.0;
   matrix2[15] = 1.0;
   //------------------
   
   glLoadIdentity();
   glMultMatrixf(matrix2);
   glTranslatef(-eyePosition.x, -eyePosition.y, -eyePosition.z);
   
}

/*
rotation around userlike axis
- normalizedDirection is normalized direction vector of axis
- phi is angle in radians
- m is array of 16 elements for matrix stored in rows
*/
void rotationAroundAxis(Vector n, double phi, double* m)
{
	double c = cos(phi);
	double s = sin(phi);
	double oneSubC = 1 - c;

	double nXnY = n.x*n.y*oneSubC;
	double nXnZ = n.x*n.z*oneSubC;
	double nYnZ = n.y*n.z*oneSubC;

	//-------------------------
	m[0] = c + n.x*n.x*oneSubC;
	m[1] = nXnY - n.z*s;
	m[2] = nXnZ + n.y*s;
	m[3] = 0;
	//-------------------------
	m[4] = nXnY + n.z*s;
	m[5] = c + n.y*n.y*oneSubC;
	m[6] = nYnZ - n.x*s;
	m[7] = 0;
	//-------------------------
	m[8] = nXnZ - n.y*s;
	m[9] = nYnZ + n.x*s;
	m[10] = c + n.z*n.z*oneSubC;
	m[11] = 0;
	//-------------------------
	m[12] = m[13] = m[14] = 0;
	m[15] = 1;
}


int findNearestPoint(MMSSVector3d* points, int pointsCount, MMSSVector3d testedPoint)
{
	int index_from = 0;
	float len_from = FLT_MAX;
	float len;
	
	for(int i = 0;i < pointsCount; i++)
	{
		len = MMSSVector3d::VectorLength(points[i],testedPoint);
		if(len < len_from)
		{
			len_from = len;
			index_from = i;
		}
	}

	return index_from;
}
