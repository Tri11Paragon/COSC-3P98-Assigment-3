/*
 * Created by Brett on 30/03/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <modes/basic.h>

#ifndef EXTRAS
    texture* world_floor;
    texture* particle_tex[10];
    
    void window_resize(int width, int height) {
        glViewport(0, 0, width, height);
    }
    
    void updateView(){
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(FOV, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 500.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        const auto& pos = cam.getPosition();
        const auto& rot = cam.getRotation();
        // pitch
        glRotatef(rot.y(), 1, 0, 0);
        // yaw
        glRotatef(rot.z(), 0, 1, 0);
        glTranslatef(-pos.x(), -pos.y(), -pos.z());
    }
    
    void render() {
        glActiveTexture(GL_TEXTURE0);
        world_floor->bind();
        renderPlane(0, 0, 0, 100, 90, 0, 0);
        renderCube(0, 1, 0, 1, 0, 0, 0);
        
        fountain->update(cam, -50, -50, 50, 50);
        fountain->render(cam, particle_tex);
        
        glFlush();
    }
    
    void init() {
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        glEnable(GL_TEXTURE_2D);
        glShadeModel(GL_SMOOTH);
        
        // I make no political statement with this image.
        world_floor = loadTexture("resources/x4twijcwsyb51.jpg");
        //particle_tex = loadTexture("resources/SPONGEBOB_YOUTUBE.jpg");
        particle_tex[0] = loadTexture("resources/1618325873904.png");
        particle_tex[1] = loadTexture("resources/1665624414712991.jpg");
        particle_tex[2] = loadTexture("resources/SPONGEBOB_YOUTUBE.jpg");
        particle_tex[3] = loadTexture("resources/stonks.png");
        // I do not use wayland.
        particle_tex[4] = loadTexture("resources/wayland.png");
        particle_tex[5] = loadTexture("resources/yak.png");
        particle_tex[6] = loadTexture("resources/penguin.jpg");
        particle_tex[7] = loadTexture("resources/fFTkb.png");
        particle_tex[8] = loadTexture("resources/depression.png");
        particle_tex[9] = loadTexture("resources/1665624414712991.jpg");
    }
    
    void cleanup() {
        delete world_floor;
    }
#endif