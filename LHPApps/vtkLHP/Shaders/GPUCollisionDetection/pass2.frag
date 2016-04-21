#version 330
#extension GL_EXT_gpu_shader4: enable

//uniform usampler2D texture; // will be texture array
uniform usampler2DArray texture;
uniform usampler2DArray texture2;
//uniform usampler1D depthmask; // will be 2D texture
uniform usampler1DArray depthmask;
const uint textures = 8u;
const float RES = 1024.0;

flat in vec3 texCoord;
flat in uint bitposition;

layout(location = 0) out uvec4 fragColor[2];

void main(void) {

    if(texCoord.x < 0.0 || texCoord.x > 1.0) discard;
    if(texCoord.y < 0.0 || texCoord.y > 1.0) discard;
    if(texCoord.z < 0.0 || texCoord.z > 1.0) discard;

    vec3 texCoordNorm = texCoord*RES;
    vec3 texCoordCeil = floor(texCoordNorm);
    vec3 coord1 = (texCoordCeil + 0.5)/RES;
    vec3 coord2 = coord1;
    coord2.x += (texCoordNorm.x - texCoordCeil.x) < 0.5 ? -1/RES : 1/RES;
    coord2.y += (texCoordNorm.y - texCoordCeil.y) < 0.5 ? -1/RES : 1/RES;
    coord2.z += (texCoordNorm.z - texCoordCeil.z) < 0.5 ? -1/RES : 1/RES;

    fragColor[0] = uvec4(0u, 0u, 0u, 0u);
    fragColor[1] = uvec4(0u, 0u, 0u, 0u);

    if(coord2.x < 0.0 || coord2.x > 1.0 ||
       coord2.y < 0.0 || coord2.y > 1.0 ||
       coord2.z < 0.0 || coord2.z > 1.0) {
        fragColor[1] = uvec4(bitposition, 0u, 0u, 0u);
        return;
    }

    //uvec4 itest = uvec4(0u);
    uvec4 itest1 = uvec4(0u);
    uvec4 itest2 = uvec4(0u);
    uvec4 itest3 = uvec4(0u);
    uvec4 itest4 = uvec4(0u);
    uvec4 itest5 = uvec4(0u);
    uvec4 itest6 = uvec4(0u);
    uvec4 itest7 = uvec4(0u);
    uvec4 itest8 = uvec4(0u);

    uvec4 utest = uvec4(0u);
    for(uint t = 0u; t < textures; t++) {
        //uvec4 i = texture2DArray(texture, vec3(texCoord.xy, t));
        uvec4 i1 = texture2DArray(texture, vec3(coord1.xy, t));
        uvec4 i2 = texture2DArray(texture, vec3(coord2.xy, t));
        uvec4 i3 = texture2DArray(texture, vec3(coord1.x, coord2.y, t));
        uvec4 i4 = texture2DArray(texture, vec3(coord2.x, coord1.y, t));

        uvec4 u = texture2DArray(texture2, vec3(texCoord.xy, t));

        //uvec4 dm = texture1DArray(depthmask, vec2(texCoord.z, t));
        uvec4 dm1 = texture1DArray(depthmask, vec2(coord1.z, t));
        uvec4 dm2 = texture1DArray(depthmask, vec2(coord2.z, t));

        //itest = itest | (i & dm);
        itest1 = itest1 | (i1 & dm1);
        itest2 = itest2 | (i2 & dm1);
        itest3 = itest3 | (i3 & dm1);
        itest4 = itest4 | (i4 & dm1);
        itest5 = itest5 | (i1 & dm2);
        itest6 = itest6 | (i2 & dm2);
        itest7 = itest7 | (i3 & dm2);
        itest8 = itest8 | (i4 & dm2);

        utest = utest | (u & dm1);
    }

    //if(itest != uvec4(0u)) {
    //    fragColor[0] = uvec4(bitposition, 0u, 0u, 0u);
    //}
    if(itest1 != uvec4(0u) && itest2 != uvec4(0u) && itest3 != uvec4(0u) && itest4 != uvec4(0u) &&
       itest5 != uvec4(0u) && itest6 != uvec4(0u) && itest7 != uvec4(0u) && itest8 != uvec4(0u)) {
        fragColor[0] = uvec4(bitposition, 0u, 0u, 0u);
    }
    else if(itest1 == uvec4(0u) && itest2 == uvec4(0u) && itest3 == uvec4(0u) && itest4 == uvec4(0u) &&
            itest5 == uvec4(0u) && itest6 == uvec4(0u) && itest7 == uvec4(0u) && itest8 == uvec4(0u)) {
        fragColor[0] = uvec4(0u, 0u, 0u, 0u);
    }
    else {
        fragColor[1] = uvec4(bitposition, 0u, 0u, 0u);
    }

    if(utest != uvec4(0u)) {
        fragColor[1] = uvec4(bitposition, 0u, 0u, 0u);
    }
}
