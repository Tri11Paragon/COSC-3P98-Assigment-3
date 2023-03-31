/*
 * Created by Brett on 30/03/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef ASSIGN3_BASIC_H
#define ASSIGN3_BASIC_H

#include <render.h>
#include <particle_system.h>

extern texture* world_floor;
extern texture* particle_tex[10];
extern particle_system* fountain;
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
extern camera cam;

void window_resize(int width, int height);

void updateView();

void render();

void init();

void cleanup();


#endif //ASSIGN3_BASIC_H
