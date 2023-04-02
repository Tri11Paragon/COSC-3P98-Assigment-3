#ifdef __cplusplus
#include <string>
std::string shader_frag = R"("
#version 460
precision mediump float;

in vec2 uv_;
in float index;

out vec4 out_color;

uniform mediump sampler2DArray texture_array;

void main() {
    //out_color = vec4(uv_, 0.0, 1.0);
    out_color = texture(texture_array, vec3(uv_, index));

    if (out_color.a < 0.1)
        discard;
}

")";
#endif