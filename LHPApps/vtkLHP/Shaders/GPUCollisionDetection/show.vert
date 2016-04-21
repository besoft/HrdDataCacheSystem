#version 330 compatibility
#extension GL_EXT_gpu_shader4: enable
//#extension GL_ARB_gpu_shader_fp64: enable
//#extension GL_ARB_vertex_attrib_64bit: enable

//in dvec3 vertex;
in float shift;
in float xCoord;

flat out float x;
flat out uint bitposition;

void main(void) {
    gl_Position = gl_ModelViewMatrix * gl_Vertex;//vec4(vertex, 1.0);
    bitposition = 1u << uint(shift);
    x = xCoord;
}
