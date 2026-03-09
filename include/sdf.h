#pragma once
#include "vec_lib.h"

typedef enum {
  SDF_TYPE_CIRCLE,
} SDFType;

typedef struct SDFObject {
    float (*sdf)(struct SDFObject *self,Vec2 point); // function pointer
    void *data; // pointer to shape-specific data
    SDFType type;
} SDFObject;

typedef struct {
  Vec2 center;
  float radius;
  Vec2 drop_dist;
} Circle;

SDFObject *create_circle(Vec2 center, float radius);
float sdf_circle(SDFObject *obj,Vec2 point);
void sdf_object_free(SDFObject *obj);
