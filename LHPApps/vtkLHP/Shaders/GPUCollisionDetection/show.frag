#version 330
#extension GL_EXT_gpu_shader4: enable

uniform usampler1D result;
uniform usampler1D undecided;

flat in float x;
flat in uint bitposition;

layout(location = 0) out vec4 fragColor;

void main(void) {
    uvec4 i = texture1D(result, x);
    uvec4 u = texture1D(undecided, x);
    uint test = i.r & bitposition;
    if(test == bitposition) {
        fragColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
    else {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
    test = u.r & bitposition;
    if(test == bitposition) {
        fragColor = vec4(1.0, 1.0, 0.0, 1.0);
    }
    //else {
    //    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    //}
}
