#pragma once

typedef struct {
    float x;
    float y;
} Vec2;

float vec2_distance(Vec2 a,Vec2 b);
float vec2_distance_sq(Vec2 a, Vec2 b);
float vec2_length(Vec2 a);
Vec2 vec2_sub(Vec2 a,Vec2 b);
Vec2 vec2_add(Vec2 a,Vec2 b);
Vec2 vec2_scale(Vec2 a,float scaler);
Vec2 vec2_normalize(Vec2 a);

