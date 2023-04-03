#ifdef __cplusplus
#include <string>
std::string shader_geom = R"("
#version 460

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec4 pos_[];

out vec2 uv_;
out float index;

const vec3 vertices[] = {
    vec3(0.5f,  0.5f, 0.0f),
    vec3(0.5f, -0.5f, 0.0f),
    vec3(-0.5f, -0.5f, 0.0f),
    vec3(-0.5f,  0.5f, 0.0f)
};

uniform mat4 pvm;
uniform vec4 up;
uniform vec4 right;

void emitTransformed(vec3 pos){
    // passthough index
    index = pos_[0].w;
    //gl_Position = pvm * vec4(pos_[0].xyz + vertices[0], 1.0);
    gl_Position = pvm * vec4(pos, 1.0);
}

void main() {
    const float quad_size = 0.5;
    vec3 pos = pos_[0].xyz;

    emitTransformed(pos + up.xyz * quad_size + right.xyz * quad_size);
    uv_ = vec2(0, 0);
    EmitVertex();

    emitTransformed(pos + up.xyz * quad_size - right.xyz * quad_size);
    uv_ = vec2(0, 1);
    EmitVertex();

    emitTransformed(pos - up.xyz * quad_size + right.xyz * quad_size);
    uv_ = vec2(1, 0);
    EmitVertex();

    emitTransformed(pos - up.xyz * quad_size - right.xyz * quad_size);
    uv_ = vec2(1, 1);
    EmitVertex();

    EndPrimitive();
}

")";
#endif