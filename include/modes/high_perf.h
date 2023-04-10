/*
 * Created by Brett on 30/03/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef ASSIGN3_HIGH_PERF_H
#define ASSIGN3_HIGH_PERF_H

#include <glad/gl.h>
#include <camera.h>
#include <particle_system.h>

#ifdef EXTRAS

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
extern particle_system* fountain;
extern camera cam;
extern const unsigned int particle_count;

void window_resize(int width, int height);

void updateView();

void render();

void init();

void cleanup();

void runPhysicsShader();

void beginExecution();

#endif

#endif //ASSIGN3_HIGH_PERF_H
