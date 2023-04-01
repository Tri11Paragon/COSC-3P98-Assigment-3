#ifdef __cplusplus
#include <string>
std::string shader_frag = R"("
#version 460

in vec2 uv_;

out vec4 out_color;

void main() {
    out_color = vec4(uv_, 0.0, 1.0);
}

")";
#endif