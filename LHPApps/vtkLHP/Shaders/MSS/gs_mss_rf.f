//////////////////////////////////////////////////////////////////////  
// This geometry shader duply vertex to other end of spring 
//////////////////////////////////////////////////////////////////////

#version 120 
#extension GL_EXT_geometry_shader4 : enable

varying in vec3 force[1];
varying in vec4 secondEnd[1];
varying in float isFixed[1];

varying out vec3 force_fs;
varying out float isFixed_fs;

const float coef1[4] = float[4](1,-1,1,-1);
const float coef2[4] = float[4](1,1,-1,-1);

void main(void)
{
  int index = int(isFixed[0]);
  gl_Position = gl_PositionIn[0];
  force_fs = force[0];
  isFixed_fs = coef1[index];
  EmitVertex();
  EndPrimitive();
  
  gl_Position = secondEnd[0];
  force_fs = -force[0];
  isFixed_fs = coef2[index];
  EmitVertex();
  EndPrimitive();
  
}