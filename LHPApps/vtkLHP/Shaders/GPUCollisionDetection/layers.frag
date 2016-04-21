#version 330
#extension GL_EXT_gpu_shader4: enable

//uniform usampler1D texture; // will be 2D texture
uniform usampler1DArray texture;
const uint textures = 8u;

layout(location = 0) out uvec4 fragColor[textures]; // array

void main() {
    // cycle over all layers and output to corresponding fragData
    for(uint t = 0u; t < textures; t++) {
        //uvec4 mask = texture1D(texture, gl_FragCoord.z);
        fragColor[t] = texture1DArray(texture, vec2(gl_FragCoord.z, t));
    }
}
