/*
 * Created by Brett Terpstra 6920201 on 20/03/23.
 * Copyright (c) Brett Terpstra 2023 All Rights Reserved
 */

#ifndef ASSIGN3_RENDER_H
#define ASSIGN3_RENDER_H

#include <glad/gl.h>

#include <GL/glut.h>
#include <GL/freeglut.h>

#include <util.h>
#include <blt/math/vectors.h>
#include <blt/std/random.h>
#include <camera.h>
#include "blt/std/logging.h"

static void applyTranslation(
        float x, float y, float z, float size = 1, float angx = 0, float angy = 0, float angz = 0
) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(x, y, z);
    if (size != 1)
        glScalef(size, size, size);
    if (angx != 0)
        glRotatef(angx, 1, 0, 0);
    if (angy != 0)
        glRotatef(angy, 0, 1, 0);
    if (angz != 0)
        glRotatef(angz, 0, 0, 1);
}

static void renderCube(float x, float y, float z, float size, float angx, float angy, float angz) {
    applyTranslation(x, y, z, size, angx, angy, angz);
    
    const float s = 0.5f;
    // modified from http://www.cosc.brocku.ca/Offerings/3P98/course/OpenGL/3P98Examples/OpenGLExamples/orient.c
    float p[][3] = {{s,  s,  s},
                    {s,  -s, s},
                    {-s, -s, s},
                    {-s, s,  s},
                    {s,  s,  -s},
                    {s,  -s, -s},
                    {-s, -s, -s},
                    {-s, s,  -s}};
    
    int e[][4] = {{0, 3, 2, 1},
                  {3, 7, 6, 2},
                  {7, 4, 5, 6},
                  {4, 0, 1, 5},
                  {0, 4, 7, 3},
                  {1, 2, 6, 5}};
    
    glBegin(GL_QUADS);
    for (auto& i : e) {
        glTexCoord2f(1, 1);
        glVertex3fv(p[i[0]]);
        glTexCoord2f(1, 0);
        glVertex3fv(p[i[1]]);
        glTexCoord2f(0, 0);
        glVertex3fv(p[i[2]]);
        glTexCoord2f(0, 1);
        glVertex3fv(p[i[3]]);
    }
    glEnd();
    
    glPopMatrix();
}

static void renderPlane(float x, float y, float z, float size, float angx, float angy, float angz) {
    glDisable(GL_CULL_FACE);
    
    applyTranslation(x, y, z, size, angx, angy, angz);
    const float s = 0.5f;
    
    glBegin(GL_QUADS);
    glTexCoord2f(1, 1);
    glVertex3f(s, s, 0);
    glTexCoord2f(1, 0);
    glVertex3f(s, -s, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-s, -s, 0);
    glTexCoord2f(0, 1);
    glVertex3f(-s, s, 0);
    glEnd();
    
    glPopMatrix();
    glEnable(GL_CULL_FACE);
}


#endif //ASSIGN3_RENDER_H
