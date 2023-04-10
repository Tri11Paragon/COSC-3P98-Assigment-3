/*
 * Created by Brett on 30/03/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <clocale>
#include <high_perf/gl_util.h>
#include <modes/high_perf.h>
#include <util.h>
#include <camera.h>
#include "blt/std/memory.h"
#include <shaders/vertex.vert>
#include <shaders/fragment.frag>
#include <shaders/geometry.geom>
#include <blt/profiling/profiler.h>
#include <shaders/physics.comp>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <filesystem>

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

// -------{VAO}-------
GLuint particleVAO;

// -------{Textures}-------
GLuint textureArrayID;

// must make sure the texture list contains this number of textures otherwise weird errors will occur!
// !*might not crash*!
const unsigned int TEXTURE_COUNT = 10;
const unsigned int TEXTURE_WIDTH = 512;
const unsigned int TEXTURE_HEIGHT = 512;

// -------{Particles}-------
const unsigned int particle_count = 128 * 500; // must be a multiple of group size divisor!
const unsigned int offset_count = 8192;
const float particle_lifetime = 25.0f;

// generally alignment to multiples of 4 floats helps performance, plus we can use that extra space for info we need.
typedef struct {
    // x y z (texture index)
    vec4 pos;
    // dx dy dz (unused)
    vec4 dir;
} particle_record;

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
    physics_shader->setFloat("deltaSeconds", (float)((double) getDelta() / 1000000000.0));
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleTranslationsBuffer);
    physics_shader->execute(particle_count / 128, 1, 1);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void render() {
    updateView();
    perspectiveMatrix = blt::perspective(FOV, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f);
    auto pvm = perspectiveMatrix * viewMatrix;
    
    auto inverseView = viewMatrix.transpose();
    
    // we need the up and right vectors for fast geometry shader billboard
    // thankfully they are easy to extract from our view matrix.
    blt::vec4 up {inverseView.m01(), inverseView.m11(), inverseView.m21()};
    blt::vec4 right {inverseView.m00(), inverseView.m10(), inverseView.m20()};
    
    BLT_START_INTERVAL("Particles", "Compute Shader");
    runPhysicsShader();
    BLT_END_INTERVAL("Particles", "Compute Shader");
    
    BLT_START_INTERVAL("Particles", "Render");
    instance_shader->bind();
    instance_shader->setVec4("up", up);
    instance_shader->setVec4("right", right);
    instance_shader->setMatrix("pvm", pvm);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
    
    glBindVertexArray(particleVAO);
    //glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particle_count);
    glDrawArrays(GL_POINTS, 0, particle_count);
    glBindVertexArray(0);
    BLT_END_INTERVAL("Particles", "Render");
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
    blt::random<float> lifetime{0, particle_lifetime};
    
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
    // create our texture
    glGenTextures(1, &textureArrayID);
    
    // ----------------------------------
    //    Upload/Assign OpenGL Objects
    // ----------------------------------
    
    BLT_TRACE("Uploading VBO data and assigning to VAO");
    glBindVertexArray(particleVAO);
    
    int translations_size = sizeof(particle_record) * particle_count;
    glBindBuffer(GL_ARRAY_BUFFER, particleTranslationsBuffer);
    glBufferData(GL_ARRAY_BUFFER, translations_size, translations.buffer, GL_DYNAMIC_DRAW); // allocate some memory on the GPU
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(particle_record), (void*) 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(particle_record), (void*) offsetof(particle_record, dir));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    // allow the particle buffer to be used in the computer shader!
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleTranslationsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleTranslationsBuffer);
    
    // generating random numbers on the GPU is hard, we can use enough precomputed random offsets to simulate real time randomness
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleOffsetsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, offset_count * sizeof(vec4), offsets.buffer, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleOffsetsBuffer);
    
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

//    std::vector<std::string> texture_locations {};
//    texture_locations.reserve(64);
//    for (int i = 0; i < 64; i++)
//        texture_locations.push_back(std::string("thinmatrix/fire_") += std::to_string(i) += ".png");
    
    constexpr int channel_count = 4;
    
    int level = 0;
    stbi_set_flip_vertically_on_load(false);
    for (const std::string& texture_loc : texture_locations){
        auto resource_location = (std::string("resources/") += texture_loc);
        
        if (!std::filesystem::exists(resource_location)) {
            BLT_FATAL("Unable to load file %s", resource_location.c_str());
            std::abort();
        }
        
        // load the texture
        int width, height, channels;
        auto* data = stbi_load(
                resource_location.c_str(), &width, &height,
                &channels, channel_count
        );
        auto* resized_data = data;
        
        BLT_TRACE("Loading image %s width %d, %d width/height", resource_location.c_str(), width, height);
        
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
    
    instance_shader = new shader(shader_vert, shader_frag, shader_geom, true);
    physics_shader = new compute_shader(shader_physics);
    
    BLT_DEBUG("High performance subsystem init complete!");
}

void cleanup() {
    BLT_PRINT_PROFILE("Particles", blt::logging::NONE, true);
    // cleanup opengl resources
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleTranslationsBuffer);
    glDeleteBuffers(1, &particleOffsetsBuffer);
    
    delete(instance_shader);
}
