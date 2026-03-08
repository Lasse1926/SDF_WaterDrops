#include "sdf.h"
#include "vec_lib.h"
#include <stdlib.h>

float sdf_circle(SDFObject *obj, Vec2 point){
  Circle *c = (Circle*)obj->data;
  return (vec2_distance(c->center,point) - c->radius); 
}
SDFObject *create_circle(Vec2 center, float radius)
{
    Circle *c = malloc(sizeof(Circle));
    if (!c) return NULL;
    c->center = center;
    c->radius = radius;

    SDFObject *obj = malloc(sizeof(SDFObject));
    if (!obj) {
        free(c);
        return NULL;
    }

    obj->sdf = sdf_circle;
    obj->data = c;
    obj->type = SDF_TYPE_CIRCLE;

    return obj;
}
void sdf_object_free(SDFObject *obj) {
    if (!obj) return;

    // Free the shape-specific data if it exists
    if (obj->data) {
        free(obj->data);
        obj->data = NULL;
    }

    // Free the SDFObject itself
    free(obj);
}
