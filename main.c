#include "SDL3/SDL_mouse.h"
#include <SDL3/SDL.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "boolen_functions.h"
#include "sdf.h"
#include "vec_lib.h"

#define WIDTH 800
#define HEIGHT 600
#define MAX_OBJECTS 100
#define K 20.0f

static inline uint32_t make_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) |
         (uint32_t)b;
}
static inline int clamp_int(int value, int min, int max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

static inline int clamp_float(float value, float min, float max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

void simulate_drop(SDFObject *objects[], int num_objects, float delta_time) {
  for (int i = 0; i < num_objects - 1; i++) {
    if (objects[i] == NULL)
      continue; // skip deleted spots
    for (int j = i + 1; j < num_objects; j++) {
      if (objects[j] == NULL)
        continue; // skip deleted spots

      SDFObject *obj_a = objects[i];
      SDFObject *obj_b = objects[j];

      if (obj_a->type == SDF_TYPE_CIRCLE && obj_b->type == SDF_TYPE_CIRCLE) {
        Circle *obj_a_data = (Circle *)obj_a->data;
        Circle *obj_b_data = (Circle *)obj_b->data;

        float dist_dbetween_surfaces =
            vec2_distance(obj_a_data->center, obj_b_data->center) -
            (obj_a_data->radius + obj_b_data->radius + K / 2.0);
        Vec2 dir_from_a_to_b =
            vec2_normalize(vec2_sub(obj_b_data->center, obj_a_data->center));

        if (dist_dbetween_surfaces <= 0.0) {
          Vec2 vel =
              vec2_scale(dir_from_a_to_b, -6.0 * dist_dbetween_surfaces * delta_time);
          if (obj_a_data->radius >= obj_b_data->radius) {
            obj_b_data->center = vec2_sub(obj_b_data->center, vel);
          } else {
            obj_a_data->center = vec2_add(obj_b_data->center, vel);
          }
        }

        if (vec2_distance(obj_a_data->center, obj_b_data->center) <=
            fmaxf(obj_a_data->radius, obj_b_data->radius)) {

          // mark the smaller for deletion
          if (obj_a_data->radius < obj_b_data->radius) {
            obj_b_data->radius = sqrtf(obj_b_data->radius * obj_b_data->radius +
                                       obj_a_data->radius * obj_a_data->radius);
            objects[i] = NULL;
            break; // obj_a is deleted, stop inner loop
          } else {
            obj_a_data->radius = sqrtf(obj_a_data->radius * obj_a_data->radius +
                                       obj_b_data->radius * obj_b_data->radius);
            objects[j] = NULL;
            continue; // obj_b deleted, check next j
          }
        }
      }
    }
  }
}

void clean_object_array(SDFObject *objects[], int *num_objcts) {
  int write_index = 0;

  for (int read_index = 0; read_index < *num_objcts; read_index++) {
    if (objects[read_index] != NULL) {
      objects[write_index++] = objects[read_index];
    }
  }
  *num_objcts = write_index;
}

int main(void) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL init failed: %s", SDL_GetError());
    return 1;
  }

  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;

  if (!SDL_CreateWindowAndRenderer("SDL3 Pixel Buffer", WIDTH, HEIGHT, 0,
                                   &window, &renderer)) {
    SDL_Log("CreateWindowAndRenderer failed: %s", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

  if (!texture) {
    SDL_Log("CreateTexture failed: %s", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  static uint32_t buffer[WIDTH * HEIGHT];

  bool running = true;
  SDFObject *objects[MAX_OBJECTS];
  int object_count = 0;

  objects[object_count++] = create_circle((Vec2){0, 0}, 20);

  objects[object_count++] = create_circle((Vec2){50, 50}, 20);

  objects[object_count++] = create_circle((Vec2){400, 300}, 20);
  objects[object_count++] = create_circle((Vec2){600, 120}, 10);
  objects[object_count++] = create_circle((Vec2){500, 350}, 40);
  objects[object_count++] = create_circle((Vec2){450, 500}, 24);
  objects[object_count++] = create_circle((Vec2){300, 600}, 20);
  objects[object_count++] = create_circle((Vec2){240, 550}, 50);

  Vec2 mouse_pos = (Vec2){0.0, 0.0};
  float mouse_scroll = 0.0;
  float k = K;

  Uint64 last_counter = SDL_GetPerformanceCounter();

  while (running) {
    SDL_Event e;
    mouse_scroll = 0.0;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        running = false;
      }
      if (e.type == SDL_EVENT_MOUSE_WHEEL) {
        mouse_scroll = e.wheel.y;
      }
    }
    Uint64 current_counter = SDL_GetPerformanceCounter();
    Uint64 freq = SDL_GetPerformanceFrequency();

    float delta_time = (float)(current_counter - last_counter) / (float)freq;

    last_counter = current_counter;

    SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);

    simulate_drop(objects, object_count,delta_time);
    clean_object_array(objects, &object_count);

    Circle *c = (Circle *)objects[0]->data;
    c->center = mouse_pos;
    c->radius += mouse_scroll;

    /* draw pixels */
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        float min_dist = FLT_MAX;
        for (int i = 0; i < object_count; i++) {
          if (objects[i]->type == SDF_TYPE_CIRCLE) {
            Circle *obj = (Circle *)objects[i]->data;
            float maxInfluence = obj->radius + k;
            float maxInfluenceSq = maxInfluence * maxInfluence;

            if (vec2_distance_sq((Vec2){x, y}, obj->center) > maxInfluenceSq) {
              continue;
            }
          }
          float d = objects[i]->sdf(objects[i], (Vec2){x, y});
          if (d <= 100.0) {
            min_dist = smooth_union(min_dist, d, k);
          }
        }

        uint8_t red = (min_dist <= 0.0f) ? 255 : 0;
        buffer[y * WIDTH + x] = make_pixel(red, 0, 0, 255);
      }
    }
    SDL_UpdateTexture(texture, NULL, buffer, WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
  }

  for (int i = 0; i < object_count; i++) {
    free(objects[i]->data);
    free(objects[i]);
    objects[i] = NULL;
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
