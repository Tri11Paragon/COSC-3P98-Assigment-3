/*
 * Created by Brett on 31/03/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef ASSIGN3_GL_UTIL_H
#define ASSIGN3_GL_UTIL_H

#include <glad/gl.h>
#include <blt/math/math.h>
#include <blt/std/string.h>
#include <string>
#include <unordered_map>

/**
 * This part was made for this assignment and will likely be used in future projects
 */
class compute_shader {
    private:
        GLuint shaderID = 0;
        GLuint programID = 0;
    public:
        explicit compute_shader(const std::string& shader_source, bool loadAsString = true);
        inline void bind() const {
            glUseProgram(programID);
        }
        inline void execute(int x, int y, int z) const {
            bind();
            glDispatchCompute(x, y, z);
        }
        ~compute_shader();
};

/**
 * Note: This is taken from my final project,
 * https://github.com/Tri11Paragon/COSC-3P98-Final-Project/blob/main/include/render/gl.h
 */

class shader {
    private:
        struct IntDefaultedToMinusOne {
            GLint i = -1;
        };
        // we can have shaders of many types in OpenGL
        GLuint programID = 0;
        // but we will only make use of these two for now
        GLuint vertexShaderID = 0;
        GLuint fragmentShaderID = 0;
        // while these will remain unused. (Webgl2 apparently doesn't support them despite being based on GL4.3? that's a TODO!)
        GLuint geometryShaderID = 0;
        // this would be very useful however it is highly unlikely webgl will support it
        // im leaving some of this stuff in here because I might expand the native application to use some of it.
        // im trying to keep the web and native versions the same though
        GLuint tessellationShaderID = 0;
        std::unordered_map<std::string, IntDefaultedToMinusOne> uniformVars;
        
        static unsigned int createShader(const std::string& source, int type);
        
        inline GLint getUniformLocation(const std::string &name) {
            if (uniformVars[name].i != -1)
                return uniformVars[name].i;
            // caching the result is a lot faster since it won't change after the shader is created.
            // TODO: look into this: https://webglfundamentals.org/webgl/lessons/webgl-qna-how-can-i-get-all-the-uniforms-and-uniformblocks.html
            int loc = glGetUniformLocation(programID, name.c_str());
            uniformVars[name].i = loc;
            return loc;
        }
    
    public:
        /**
         * Creates a shader
         * @param vertex vertex shader source or file
         * @param fragment fragment shader source or file
         * @param geometry geometry shader source or file (optional)
         * @param load_as_string load the shader as a string (true) or use the string to load the shader as a file (false)
         */
        shader(const std::string &vertex, const std::string &fragment, const std::string &geometry = "", bool load_as_string = true);
        
        shader(shader&& move) noexcept;
        
        // used to set the location of VAOs to the in variables in opengl shaders.
        void bindAttribute(int attribute, const std::string &name) const;
        
        // used to set location of shared UBOs like the perspective and view matrix
        void setUniformBlockLocation(const std::string &name, int location) const;
        
        // set various data-types.
        inline void setBool(const std::string &name, bool value) {
            glUniform1i(getUniformLocation(name), (int) value);
        }
        
        inline void setInt(const std::string &name, int value) {
            glUniform1i(getUniformLocation(name), value);
        }
        
        inline void setFloat(const std::string &name, float value) {
            glUniform1f(getUniformLocation(name), value);
        }
        
        inline void setMatrix(const std::string &name, blt::mat4x4 &matrix) {
            glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, matrix.ptr());
        }
        
        inline void setVec3(const std::string &name, const blt::vec3 &vec) {
            glUniform3f(getUniformLocation(name), vec.x(), vec.y(), vec.z());
        }
        
        inline void setVec4(const std::string &name, const blt::vec4 &vec) {
            // TODO: edit BLT to include a w component
            glUniform4f(getUniformLocation(name), vec.x(), vec.y(), vec.z(), vec[3]);
        }
        
        inline void setVec2(const std::string &name, float x, float y) {
            glUniform2f(getUniformLocation(name), x, y);
        }
        
        inline void setVec3(const std::string &name, float x, float y, float z) {
            glUniform3f(getUniformLocation(name), x, y, z);
        }
        
        inline void setVec4(const std::string &name, float x, float y, float z, float w) {
            glUniform4f(getUniformLocation(name), x, y, z, w);
        }
        
        inline void bind() const {
            glUseProgram(programID);
        }
        
        ~shader();
};

#endif //ASSIGN3_GL_UTIL_H
