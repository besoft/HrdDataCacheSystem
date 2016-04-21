// This fragment shader contains:
// Actualizing vertex position of mss based on prev position, 
// act position and forces
uniform sampler2D posTexture;
uniform sampler2D prevPosTexture;
uniform sampler2D acumulatedForceTexture;
uniform float weight;
uniform float dampConst;

uniform float dt;

void main(void)
{
   
   vec3 pos = texture2D(posTexture,gl_TexCoord[0].st).rgb;
   vec3 prevpos = texture2D(prevPosTexture,gl_TexCoord[0].st).rgb;
   
   vec3 acumForce = texture2D(acumulatedForceTexture,gl_TexCoord[0].st).rgb;
   
   vec3 ftot = acumForce - dampConst*(pos - prevpos)/dt;
   vec3 newpos = (ftot *dt *dt)/weight + 2 * pos - prevpos;
   gl_FragColor = vec4(newpos,1);
}
