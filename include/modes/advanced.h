/*
 * Created by Brett on 30/03/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef ASSIGN3_ADVANCED_H
#define ASSIGN3_ADVANCED_H

#include <glad/gl.h>
#include <camera.h>
#include <particle_system.h>

extern particle_system* fountain;
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
extern camera cam;

void window_resize(int width, int height);

void updateView();

void render();

void init();

void cleanup();

#endif //ASSIGN3_ADVANCED_H
