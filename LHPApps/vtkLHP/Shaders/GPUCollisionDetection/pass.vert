#version 330 compatibility
#extension GL_EXT_gpu_shader4: enable
//#extension GL_ARB_gpu_shader_fp64: enable
//#extension GL_ARB_vertex_attrib_64bit: enable

//in dvec3 vertex;
in float shift;
in float xCoord;

flat out vec3 texCoord;
flat out uint bitposition;

void main(void) {
    gl_Position = vec4(2.0*xCoord - 1.0, 0.0, 0.0, 1.0);
    bitposition = 1u << uint(shift);
    vec4 v = gl_ModelViewMatrix * gl_Vertex;//vec4(vertex, 1.0);
    texCoord = 0.5*(v.xyz + 1.0);
}

/*void main(void) {
    gl_Position = vec4(2.0*gl_Color.r - 1.0, 0.0, 0.0, 1.0);
    bitposition = 1u << uint(gl_MultiTexCoord0.x);
    vec4 v = gl_ModelViewMatrix * gl_Vertex;
    texCoord = 0.5*(v.xyz + 1.0);
}*/
