#ifdef __cplusplus
#include <string>
std::string shader_sort = R"("
#version 460

layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

void main() {

}

")";
#endif