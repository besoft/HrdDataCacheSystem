#version 150 compatibility

//layout(location = 0) in vec3 gl_Vertex;
//layout(location = 1) in uint
//out vec3 texCoord;

void main(void) {
    gl_Position = gl_ModelViewMatrix * gl_Vertex;
    //texCoord = gl_MultiTexCoord0.xyz;
}
