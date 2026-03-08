#include "vec_lib.h"
#include <math.h>

float vec2_distance(Vec2 a,Vec2 b){
   return vec2_length(vec2_sub(b,a));
}

float vec2_distance_sq(Vec2 a, Vec2 b){
    Vec2 d = vec2_sub(b, a);
    return d.x*d.x + d.y*d.y;
}

float vec2_length(Vec2 a){
  return hypotf(a.x, a.y);
}

Vec2 vec2_sub(Vec2 a,Vec2 b){
  return (Vec2){.x = a.x - b.x, .y = a.y - b.y};
}

Vec2 vec2_add(Vec2 a,Vec2 b){
  return (Vec2){.x = a.x + b.x, .y = a.y + b.y};
}

Vec2 vec2_scale(Vec2 a,float scaler){
  return (Vec2){a.x*scaler,a.y*scaler};
}

Vec2 vec2_normalize(Vec2 a){
  float length = vec2_length(a);
  return (Vec2){a.x/length,a.y/length};
}
