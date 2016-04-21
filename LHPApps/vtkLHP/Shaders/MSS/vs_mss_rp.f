// This vertex shader only transfer important vals to fragment shader 
void main(void)
{
   gl_Position = gl_Vertex;
   gl_TexCoord[0] = vec4(gl_Normal.xy,0,0);
}

