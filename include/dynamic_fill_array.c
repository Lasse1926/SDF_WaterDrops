#include "dynamic_fill_array.h"
#include "sdf.h"
#include <stdio.h>
#include <stdlib.h>

// Create a new DynamicFillArray
DynamicFillArray new_dynamic_fill_array(int size) {
  DynamicFillArray array;

  array.max_objects = size;
  array.object_count = 0;
  array.freeCount = 0;

  // Allocate memory for object pointers
  array.objects = (SDFObject **)malloc(sizeof(SDFObject *) * size);
  if (!array.objects) {
    fprintf(stderr, "Failed to allocate memory for objects array\n");
    exit(1);
  }

  // Initialize all object pointers to NULL
  for (int i = 0; i < size; i++)
    array.objects[i] = NULL;

  // Allocate memory for freeIndexes stack
  array.freeIndexes = (int *)malloc(sizeof(int) * size);
  if (!array.freeIndexes) {
    fprintf(stderr, "Failed to allocate memory for freeIndexes array\n");
    free(array.objects);
    exit(1);
  }

  return array;
}

// Insert a new object, reusing free slots if available
void dynamic_fill_array_insert(DynamicFillArray *array, SDFObject *new_object) {
  if (!array || !new_object)
    return;

  int insert_index;

  if (array->freeCount > 0) {
    // Reuse a freed slot
    insert_index = array->freeIndexes[--array->freeCount];
  } else if (array->object_count < array->max_objects) {
    // Append at the end
    insert_index = array->object_count++;
  } else {
    fprintf(stderr, "DynamicFillArray is full, cannot insert new object\n");
    return;
  }

  array->objects[insert_index] = new_object;

  // Ensure object_count covers the highest used index
  if (insert_index >= array->object_count)
    array->object_count = insert_index + 1;
}

void dynamic_fill_array_remove_index(DynamicFillArray *array, int index) {
  if (!array)
    return;
  if (index < 0 || index >= array->max_objects)
    return;
  if (array->objects[index] == NULL)
    return;

  array->objects[index] = NULL;
  array->freeIndexes[array->freeCount++] = index;

  // If we removed the last used object, shrink object_count
  if (index == array->object_count - 1) {
    while (array->object_count > 0 &&
           array->objects[array->object_count - 1] == NULL) {
      array->object_count--;
    }
  }
}

// Free all objects and the array itself
void dynamic_fill_array_free(DynamicFillArray *array) {
  if (!array)
    return;

  // Free all allocated SDFObjects
  for (int i = 0; i < array->max_objects; i++) {
    if (array->objects[i] != NULL) {
      sdf_object_free(array->objects[i]);
      array->objects[i] = NULL;
    }
  }

  // Free internal arrays
  free(array->objects);
  array->objects = NULL;

  free(array->freeIndexes);
  array->freeIndexes = NULL;

  // Optional: reset counts
  array->object_count = 0;
  array->freeCount = 0;
  array->max_objects = 0;
}
