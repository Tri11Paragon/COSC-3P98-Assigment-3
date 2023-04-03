#ifdef __cplusplus
#include <string>
std::string shader_vert = R"("
#version 460

//layout (location = 0) in vec3 vertex;
//layout (location = 1) in vec2 uv;
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 dir;

out vec4 pos_;

uniform mat4 pvm;

void main() {
    // passthrough point position to geometry shader
    pos_ = pos;
}

")";
#endif