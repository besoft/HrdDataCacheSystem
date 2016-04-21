#version 330
#extension GL_EXT_gpu_shader4: enable

//uniform usampler2D texture; // will be texture array
uniform usampler2DArray texture;
uniform usampler2DArray texture2;
//uniform usampler1D depthmask; // will be 2D texture
uniform usampler1DArray depthmask;
const uint textures = 8u;

flat in vec3 texCoord;
flat in uint bitposition;

layout(location = 0) out uvec4 fragColor[2];

void main(void) {

    if(texCoord.x < 0.0 || texCoord.x > 1.0) discard;
    if(texCoord.y < 0.0 || texCoord.y > 1.0) discard;
    if(texCoord.z < 0.0 || texCoord.z > 1.0) discard;

    uvec4 itest = uvec4(0u);
    uvec4 utest = uvec4(0u);
    for(uint t = 0u; t < textures; t++) {
        uvec4 i = texture2DArray(texture, vec3(texCoord.xy, t));
        uvec4 u = texture2DArray(texture2, vec3(texCoord.xy, t));
        uvec4 dm = texture1DArray(depthmask, vec2(texCoord.z, t));
        itest = itest | (i & dm);
        utest = utest | (u & dm);
    }

    fragColor[0] = uvec4(0u, 0u, 0u, 0u);
    fragColor[1] = uvec4(0u, 0u, 0u, 0u);
    if(itest != uvec4(0u)) {
        fragColor[0] = uvec4(bitposition, 0u, 0u, 0u);
    }
    if(utest != uvec4(0u)) {
        fragColor[1] = uvec4(bitposition, 0u, 0u, 0u);
    }
}
