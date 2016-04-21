////////////////////////////////////////////////////////////////  
// This fragment shader contains:
// Phong light model, shadow counting with shadow map 
// (using precounted depth map saved in ShadowMap)
////////////////////////////////////////////////////////////////

varying vec3 force_fs;
varying float isFixed_fs;

uniform vec3 extForce;
uniform float appToEnd;

void main()
{	
	if(isFixed_fs > 0)discard;
	gl_FragColor = vec4(force_fs.rgb+extForce,1);
}