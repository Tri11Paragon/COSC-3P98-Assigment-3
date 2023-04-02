#ifdef __cplusplus
#include <string>
std::string shader_vert = R"("
#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 pos;
layout (location = 3) in vec4 dir;

out vec2 uv_;
out float index;

uniform mat4 pvm;

void main() {
    // passthough the UV (OpenGL interpolates this per fragment)
    uv_ = uv;
    index = pos.w;
    // offset the vertex by the particle's position
    gl_Position = pvm * vec4(vertex + pos.xyz, 1.0);
}

")";
#endif