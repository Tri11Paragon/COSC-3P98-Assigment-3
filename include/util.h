/*
 * Created by Brett Terpstra 6920201 on 20/03/23.
 * Copyright (c) Brett Terpstra 2023 All Rights Reserved
 */

#ifndef ASSIGN3_UTIL_H
#define ASSIGN3_UTIL_H

#include <chrono>
#include <config.h>
#include <string>
#include <blt/math/vectors.h>
#include <type_traits>

const std::string WINDOW_TITLE = "Assignment 3: They said WHAT?!";

static constexpr int TARGET_FPS = 60;
static constexpr float FOV = 90;
static constexpr float EPSILON = 1.0f;
static constexpr float GRAVITY = 9.8f;
static constexpr float TO_RAD = 3.14159265359 / 180.0;

static inline auto getCurrentTimeNanoseconds() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

long getDelta();

// blt:vec isn't a POD
typedef struct {
    float x, y, z;
} vec;

typedef struct {
    float x, y, z, w;
} vec4;

inline vec operator+(const vec& l, const vec& r) {
    return {l.x + r.x, l.y + r.y, l.z + r.z};
}

// template voodoo magic to only allow floats / doubles / other arithmetic types
// keeps the multiplication of a double type in double precision and downcast to float AFTER calc
template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
inline vec operator*(const vec& v, T s) {
    return {(float) (v.x * s), (float) (v.y * s), (float) (v.z * s)};
}

inline vec operator-(const vec& l, const vec& r) {
    return {l.x - r.x, l.y - r.y, l.z - r.z};
}

inline vec operator-(const vec& l) {
    return {-l.x, -l.y, -l.z};
}

inline vec conv(const blt::vec3& v) {
    return vec{v.x(), v.y(), v.z()};
}

#endif //ASSIGN3_UTIL_H
