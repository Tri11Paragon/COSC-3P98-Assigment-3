/*
 * Created by Brett on 30/03/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <locale.h>
#include <high_perf/gl_util.h>
#include <modes/high_perf.h>
#include <util.h>
#include <camera.h>
#include "blt/std/memory.h"
#include <shaders/vertex.vert>
#include <shaders/fragment.frag>
#include <shaders/physics.comp>
#include <stb_image.h>
#include <stb_image_resize.h>

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

// -------{VBO}-------
GLuint particleTranslationsBuffer;
GLuint particleOffsetsBuffer;
GLuint verticesVBO;
GLuint uvsVBO;
GLuint indicesEBO;

// -------{VAO}-------
GLuint particleVAO;

// -------{Textures}-------
GLuint textureArrayID;

const unsigned int TEXTURE_COUNT = 10;
const unsigned int TEXTURE_WIDTH = 512;
const unsigned int TEXTURE_HEIGHT = 512;

// -------{Particles}-------
const unsigned int particle_count = 128 * 10000;
const unsigned int offset_count = 8192;

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
compute_shader* physics_shader;

void updateView() {
    viewMatrix = createViewMatrix();
}

bool execute = false;

void beginExecution() {
    execute = true;
}

void runPhysicsShader(){
    if (!execute)
        return;
    
    physics_shader->bind();
    glUniform1f(0, (float)((double) getDelta() / 1000000000.0));
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleTranslationsBuffer);
    physics_shader->execute(particle_count / 128, 1, 1);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void render() {
    updateView();
    perspectiveMatrix = blt::perspective(FOV, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f);
    auto pvm = perspectiveMatrix * viewMatrix;
    
    runPhysicsShader();
    
    instance_shader->bind();
    instance_shader->setMatrix("pvm", pvm);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
    
    glBindVertexArray(particleVAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particle_count);
    glBindVertexArray(0);
    
}

void init() {
    BLT_DEBUG("High performance subsystem init");
    setlocale(LC_NUMERIC, "");
    BLT_INFO("Using %'d particle count", particle_count);
    BLT_INFO("Loading %d texture(s) of size (%d, %d)", TEXTURE_COUNT, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    BLT_TRACE("Checking system constants");
    
    // number of work groups allowed (min: 65535)
    GLint workGroupCountX;
    GLint workGroupCountY;
    GLint workGroupCountZ;
    // max local size of the work groups (min: 1024, 1024, 64)
    GLint workGroupSize;
    // max number of work group invocations (min: 1024)
    GLint workGroupInvocations;
    
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupCountX);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupCountY);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGroupCountZ);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSize);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 0, &workGroupInvocations);
    
    BLT_INFO("This system's OpenGL supports (%d, %d, %d) work groups", workGroupCountX, workGroupCountY, workGroupCountZ);
    BLT_INFO("\tLocal group max total size: %d", workGroupSize);
    BLT_INFO("\tMax work group invocations: %d", workGroupInvocations);
    blt::scoped_buffer<particle_record> translations{particle_count};
    blt::scoped_buffer<vec4> offsets{offset_count};
    blt::random<float> dir{-1, 1};
    blt::random<float> lifetime{0, 25};
    
    BLT_TRACE("Creating particles");
    for (int i = 0; i < particle_count; i++)
        translations[i] = particle_record{vec4{0, 1, 0, (float)(i % 10)}, vec4{0, 1, 0, lifetime.get()}};
    
    for (int i = 0; i < offset_count; i++) {
        blt::vec2 v {dir.get(), dir.get()};
        v = v.normalize();
        offsets[i] = vec4{v[0], 0, v[1], 0};
    }
    
    // ----------------------------------
    //       Create OpenGL Objects
    // ----------------------------------
    BLT_TRACE("Creating OpenGL objects");
    // create our VAO
    glGenVertexArrays(1, &particleVAO);
    // create our VBOs
    glGenBuffers(1, &particleTranslationsBuffer);
    glGenBuffers(1, &particleOffsetsBuffer);
    glGenBuffers(1, &verticesVBO);
    glGenBuffers(1, &uvsVBO);
    glGenBuffers(1, &indicesEBO);
    // create our texture
    glGenTextures(1, &textureArrayID);
    
    // ----------------------------------
    //    Upload/Assign OpenGL Objects
    // ----------------------------------
    
    BLT_TRACE("Uploading VBO data and assigning to VAO");
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
    glBindBuffer(GL_ARRAY_BUFFER, particleTranslationsBuffer);
    glBufferData(GL_ARRAY_BUFFER, translations_size, translations.buffer, GL_DYNAMIC_DRAW); // allocate some memory on the GPU
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(particle_record), (void*) 0);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(particle_record), (void*) offsetof(particle_record, dir));
    // tells opengl that we want to present this data per 1 instance instead of per vertex.
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    
    // allow the particle buffer to be used in the computer shader!
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleTranslationsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleTranslationsBuffer);
    
    // generating random numbers on the GPU is hard, we can use enough precomputed random offsets to simulate real time randomness
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleOffsetsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, offset_count * sizeof(vec4), offsets.buffer, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleOffsetsBuffer);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * 6, indices, GL_STATIC_DRAW);
    
    
    // ----------------------------------
    //             Texturing
    // ----------------------------------
    BLT_TRACE("Creating array texture");
    // based on my final project's texture array implementation
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
    // allocate immutable storage for our textures
    // we can change what is stored inside the texture, but we cannot change its size
    // which is why we need to be specific here about the type of data we will be storing.
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, TEXTURE_COUNT);
    
    std::string texture_locations[TEXTURE_COUNT] = {
            "wayland.png",
            "SPONGEBOB_YOUTUBE.jpg",
            "1618325873904.png",
            "1665624414712991.jpg",
            "stonks.png",
            "yak.png",
            "penguin.jpg",
            "fFTkb.png",
            "depression.png",
            "1665624414712991.jpg"
    };
    
    constexpr int channel_count = 4;
    
    int level = 0;
    stbi_set_flip_vertically_on_load(true);
    for (const std::string& texture_loc : texture_locations){
        // load the texture
        int width, height, channels;
        auto* data = stbi_load(
                (std::string("resources/") += texture_loc).c_str(), &width, &height,
                &channels, channel_count
        );
        auto* resized_data = data;
        
        // resize if necessary
        if (width != TEXTURE_WIDTH || height != TEXTURE_HEIGHT){
            // needs to be malloc since stbi_image_free is just free()
            auto output_data = (unsigned char*) malloc(
                    TEXTURE_WIDTH * TEXTURE_HEIGHT * channel_count
            );
            if (stbir_resize_uint8(
                    // input
                    data, width, height, 0,
                    // output
                    output_data, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0,
                    // channels
                    channel_count
            )) {
                BLT_WARN("Error resizing block texture image!");
            }
            stbi_image_free(data);
            resized_data = output_data;
        }
        
        // upload image to the gpu
        glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY, 0, 0, 0, level++, TEXTURE_WIDTH, TEXTURE_HEIGHT, 1,
                GL_RGBA, GL_UNSIGNED_BYTE, resized_data
        );
        
        stbi_image_free(resized_data);
    }
    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // Anisotropy helps preserve textures at oblique angles
    float a = 0;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &a);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, a);
    // mipmaps reduce resolution of textures as the distance to them increases
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    
    BLT_TRACE("Loading shaders");
    
    instance_shader = new shader(shader_vert, shader_frag, "", true);
    physics_shader = new compute_shader(shader_physics);
    
    BLT_DEBUG("High performance subsystem init complete!");
}

void cleanup() {
    // cleanup opengl resources
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleTranslationsBuffer);
    glDeleteBuffers(1, &verticesVBO);
    glDeleteBuffers(1, &uvsVBO);
    glDeleteBuffers(1, &indicesEBO);
    
    delete(instance_shader);
}
