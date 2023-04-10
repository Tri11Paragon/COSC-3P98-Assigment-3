/*
 * Created by Brett Terpstra 6920201 on 27/03/23.
 * Copyright (c) Brett Terpstra 2023 All Rights Reserved
 */

#ifndef ASSIGN3_TEXTURE_H
#define ASSIGN3_TEXTURE_H

#include <glad/gl.h>

#include <GL/glut.h>
#include <GL/freeglut.h>
#include <string>
#include <iostream>

class texture {
    private:
        unsigned int textureID = 0;
    public:
        texture(unsigned char* data, int width, int height, int channels) {
            glGenTextures(1, &textureID);
            bind();
            
            glTexImage2D(
                    GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, channels == 4 ? GL_RGBA : GL_RGB,
                    GL_UNSIGNED_BYTE, data
            );
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // nearest preserves the pixely look
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // Anisotropy helps preserve textures at oblique angles
            float a = 0;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &a);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, a);
            
            unbind();
            // stbi mallocs (it is a c lib)!
            free(data);
        }
        
        inline void bind() const {
            glBindTexture(GL_TEXTURE_2D, textureID);
        }
        
        static inline void unbind() {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        
        ~texture(){
            glDeleteTextures(1, &textureID);
        }
};

texture* loadTexture(const std::string& path);

#endif //ASSIGN3_TEXTURE_H
