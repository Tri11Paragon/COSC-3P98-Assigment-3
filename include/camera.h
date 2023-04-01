/*
 * Created by Brett Terpstra 6920201 on 20/03/23.
 * Copyright (c) Brett Terpstra 2023 All Rights Reserved
 */

#ifndef ASSIGN3_CAMERA_H
#define ASSIGN3_CAMERA_H

#include <GL/glut.h>
#include <GL/freeglut.h>
#include <cmath>
#include <blt/math/vectors.h>
#include <util.h>
#include "blt/std/logging.h"

class camera {
    private:
        bool keyMap[512] {};
        bool keyState[512] {};
        bool specialState[512] {};
        
        blt::vec3 position {0, 3, 0};
        blt::vec3 rotation;
        
        bool mouseGrabbed = false;
        
        int lastX = 0, lastY = 0, curX = 0, curY = 0;
        int dx = 0, dy = 0;
        
        const float MAX_SPEED = 100;
        const float DEFAULT_SPEED = 50;
        const float MIN_SPEED = 1;
        const float ROTATION_SPEED = 3;
        
        float cur_speed = DEFAULT_SPEED;
        float currentSpeedX = 0;
        float currentSpeedY = 0;
        float currentSpeedZ = 0;
        
        void handleMouseMotion(int x, int y){
            lastX = curX;
            lastY = curY;
            curX = x;
            curY = y;
            dx = curX - lastX;
            dy = curY - lastY;
        }
    public:
        camera() = default;
        
        inline void keyPress(unsigned char key, int x, int y) {
            keyState[key] = keyMap[key] = true;
        }
        
        inline void specialPress(int key){
            specialState[key] = true;
        }
        inline void specialRelease(int key){
            specialState[key] = false;
        }
        
        inline void keyRelease(unsigned char key, int x, int y) {
            keyMap[key] = false;
            if (key == 27) {
                mouseGrabbed = !mouseGrabbed;
                if (mouseGrabbed)
                    glutSetCursor(GLUT_CURSOR_NONE);
                else
                    glutSetCursor(GLUT_CURSOR_INHERIT);
            }
        }
        
        inline bool isKeyPressed(unsigned char key){
            return keyMap[key];
        }
        
        inline bool getKeyState(unsigned char key) {
            return keyState[key];
        }
        
        inline void mouseMotion(int x, int y){
            handleMouseMotion(x,y);
        }
        
        inline void mousePassiveMotion(int x, int y){
            handleMouseMotion(x,y);
        }
        
        void update(int window_width, int window_height, float delta){
            if (mouseGrabbed) {
                // center the cursor
                glutWarpPointer(window_width / 2, window_height / 2);
                // apply rotation only if the mouse is grabbed
                rotation[1] += (-(float) dy * delta * ROTATION_SPEED);
                rotation[2] += (-(float) dx * delta * ROTATION_SPEED);
            }
            
            const float horzSpeed = 25;
            const float vertSpeed = 25;
            
            if (specialState[GLUT_KEY_UP])
                rotation[1] += (-(float) vertSpeed * delta * ROTATION_SPEED);
            else if (specialState[GLUT_KEY_DOWN])
                rotation[1] += ((float) vertSpeed * delta * ROTATION_SPEED);
    
            if (specialState[GLUT_KEY_LEFT])
                rotation[2] += (-(float) horzSpeed * delta * ROTATION_SPEED);
            else if (specialState[GLUT_KEY_RIGHT])
                rotation[2] += ((float) horzSpeed * delta * ROTATION_SPEED);
            
            if (specialState[GLUT_KEY_F1])
                cur_speed = DEFAULT_SPEED;
            if (specialState[GLUT_KEY_F2])
                cur_speed = MIN_SPEED;
            if (specialState[GLUT_KEY_F3])
                cur_speed = MAX_SPEED;
            
            if (rotation[2] > 360)
                rotation[2] = 0;
            if (rotation[2] < 0)
                rotation[2] = 360;
            if (rotation[1] > 90)
                rotation[1] = 90;
            if (rotation[1] < -90)
                rotation[1] = -90;
    
            if (keyMap['w'])
                currentSpeedX = -cur_speed;
            else if (keyMap['s'])
                currentSpeedX = cur_speed;
            else
                currentSpeedX = 0;
            
            if (keyMap['a'])
                currentSpeedZ = cur_speed;
            else if (keyMap['d'])
                currentSpeedZ = -cur_speed;
            else
                currentSpeedZ = 0;
            
            if (keyMap['q'])
                currentSpeedY = -cur_speed;
            else if (keyMap['e'])
                currentSpeedY = cur_speed;
            else
                currentSpeedY = 0;
            
            float yaw = rotation[2] * TO_RAD;
    
            // https://www.desmos.com/calculator/gccgtjqpcy
            // visual description of this equation, Y is Z here and R is the yaw
            float p_dx = -currentSpeedX * sinf(yaw) + -currentSpeedZ * cosf(yaw);
            float p_dy = currentSpeedY;
            float p_dz = currentSpeedX * cosf(yaw) + -currentSpeedZ * sinf(yaw);
    
            position[0] += (float)(p_dx * delta);
            position[1] += (float)(p_dy * delta);
            position[2] += (float)(p_dz * delta);
        }
        
        void inputUpdate(){
            // would've made sense to split this into another class at this point
            for (bool& b : keyState)
                b = false;
        }
        
        const blt::vec3& getPosition(){
            return position;
        }
        
        const blt::vec3& getRotation(){
            return rotation;
        }
};

#endif //ASSIGN3_CAMERA_H
