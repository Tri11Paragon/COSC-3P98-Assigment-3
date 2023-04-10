#ifdef __cplusplus
#include <string>
std::string shader_geom = R"("
#version 460

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec4 pos_[];

out vec2 uv_;
out float index;

uniform mat4 pvm;
uniform vec4 up;
uniform vec4 right;

void transform(vec3 pos){
    // passthough index
    index = pos_[0].w;
    gl_Position = pvm * vec4(pos, 1.0);
}

void main() {
    const float quad_size = 0.5;
    vec3 pos = pos_[0].xyz;

    // using the up and right vectors to generate the verticies for this particle ensures that the particle always faces the camera
    transform(pos + up.xyz * quad_size + right.xyz * quad_size);
    uv_ = vec2(0, 0);
    EmitVertex();

    transform(pos + up.xyz * quad_size - right.xyz * quad_size);
    uv_ = vec2(0, 1);
    EmitVertex();

    transform(pos - up.xyz * quad_size + right.xyz * quad_size);
    uv_ = vec2(1, 0);
    EmitVertex();

    transform(pos - up.xyz * quad_size - right.xyz * quad_size);
    uv_ = vec2(1, 1);
    EmitVertex();

    EndPrimitive();
}

")";
#endif