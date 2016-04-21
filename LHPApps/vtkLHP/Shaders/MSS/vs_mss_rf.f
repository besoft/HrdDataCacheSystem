//////////////////////////////////////////////////////////////////////  
// This vertex shader only transfer important vals to fragment shader 
//////////////////////////////////////////////////////////////////////

uniform sampler2D posTexture;
varying vec3 force;
varying vec4 secondEnd;
varying float isFixed;

uniform float appToEnd;

const vec2 leftDownCorner = vec2(-1,-1);

void main(void)
{
   gl_PointSize = 1;
   gl_Position = vec4((gl_Vertex.xy)*2 + leftDownCorner,1,gl_Vertex.w);
   secondEnd = vec4((gl_Normal.xy)*2 + leftDownCorner,1,gl_Vertex.w);
   
   isFixed = gl_Vertex.z;
   vec3 dir = (texture2D(posTexture,gl_Normal.xy).rgb - texture2D(posTexture,gl_Vertex.xy).rgb);
   
   float l = length(dir);
   
   force = (1/gl_Normal.z * (l - gl_Normal.z) / l) * (dir * (1/l));
}