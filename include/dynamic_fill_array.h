#pragma once

#include "sdf.h"

typedef struct {
    int max_objects;
    int object_count;
    SDFObject **objects;
    int *freeIndexes;
    int freeCount;
} DynamicFillArray;

DynamicFillArray new_dynamic_fill_array(int size);
void dynamic_fill_array_remove_index(DynamicFillArray *array,int index);
void dynamic_fill_array_insert(DynamicFillArray *array,SDFObject *new_object);
void dynamic_fill_array_free(DynamicFillArray *array);
