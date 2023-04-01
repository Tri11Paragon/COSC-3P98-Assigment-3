/*
 * Created by Brett on 30/03/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <high_perf/gl_util.h>
#include <modes/high_perf.h>
#include <util.h>
#include <camera.h>
#include "blt/std/memory.h"
#include <shaders/vertex.vert>
#include <shaders/fragment.frag>

//static inline float degreesToRadian(float deg) {
//    return deg * (float)M_PI / 180.0f;
//}

blt::mat4x4 createViewMatrix(){
    auto position = cam.getPosition();
    auto rotation = cam.getRotation();

    blt::mat4x4 viewMatrix;
    
    viewMatrix.rotateX(rotation.y() * TO_RAD);
    viewMatrix.rotateY(rotation.z() * TO_RAD);
    viewMatrix.translate(-position);
    
    return viewMatrix;
}

void window_resize(int width, int height) {

}

GLuint particleTranslationsVBO;
GLuint verticesVBO;
GLuint uvsVBO;
GLuint indicesEBO;
GLuint particleVAO;

const unsigned int particle_count = 25000000;

// generally alignment to multiples of 4 floats helps performance, plus we can use that extra space for info we need.
typedef struct {
    // x y z (texture index)
    vec4 pos;
    // dx dy dz (unused)
    vec4 dir;
} particle_record;

const float vertices[] = {
        0.5f,  0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
};
const float uvs[] = {
        0, 0,
        0, 1,
        1, 1,
        1, 0
};
const unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
};

blt::mat4x4 perspectiveMatrix;
blt::mat4x4 viewMatrix;

shader* instance_shader;

void updateView() {
    viewMatrix = createViewMatrix();
}

void render() {
    updateView();
    perspectiveMatrix = blt::perspective(FOV, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f);
    auto pvm = perspectiveMatrix * viewMatrix;
    
    instance_shader->bind();
    instance_shader->setMatrix("pvm", pvm);
    
    glBindVertexArray(particleVAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particle_count);
    glBindVertexArray(0);
    
}

void init() {
    blt::scoped_buffer<particle_record> translations{particle_count};
    blt::random<float> pos{-50.0, 50.0};
    
    for (int i = 0; i < particle_count; i++)
        translations[i] = particle_record{vec4{pos.get(), pos.get() / 2, pos.get(), (float)(i % 10)}, vec4{0, 0, 0, 0}};
    
    // ----------------------------------
    //       Create OpenGL Objects
    // ----------------------------------
    // create our VAO
    glGenVertexArrays(1, &particleVAO);
    // create our VBOs
    glGenBuffers(1, &particleTranslationsVBO);
    glGenBuffers(1, &verticesVBO);
    glGenBuffers(1, &uvsVBO);
    glGenBuffers(1, &indicesEBO);
    
    glBindVertexArray(particleVAO);
    
    // bind and upload vertices data to the GPU
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, vertices, GL_STATIC_DRAW);
    // tell OpenGL how to handle the vertex data when rendering the VAO, the vertices will be bound to slot 0.
    // (we will tell OpenGL what variable uses slot 0 in the shader!)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*) 0);
    // tell OpenGL we will be using the first VAO slot, prevents us from having to call it before rendering
    glEnableVertexAttribArray(0);
    
    
    glBindBuffer(GL_ARRAY_BUFFER, uvsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, uvs, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*) 0);
    glEnableVertexAttribArray(1);
    
    int translations_size = sizeof(particle_record) * particle_count;
    glBindBuffer(GL_ARRAY_BUFFER, particleTranslationsVBO);
    glBufferData(GL_ARRAY_BUFFER, translations_size, translations.buffer, GL_DYNAMIC_DRAW); // allocate some memory on the GPU
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(particle_record), (void*) 0);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(particle_record), (void*) offsetof(particle_record, dir));
    // tells opengl that we want to present this data per 1 instance instead of per vertex.
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * 6, indices, GL_STATIC_DRAW);
    
    
    instance_shader = new shader(shader_vert, shader_frag, "", true);
}

void cleanup() {
    // cleanup opengl resources
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleTranslationsVBO);
    glDeleteBuffers(1, &verticesVBO);
    glDeleteBuffers(1, &uvsVBO);
    glDeleteBuffers(1, &indicesEBO);
    
    delete(instance_shader);
}
