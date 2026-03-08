#include "SDL3/SDL_events.h"
#include "SDL3/SDL_mouse.h"
#include <SDL3/SDL.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "boolen_functions.h"
#include "dynamic_fill_array.h"
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

static inline float clamp_float(float value, float min, float max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

void simulate_drop(DynamicFillArray *array, float delta_time) {
  printf("%f\n",delta_time);
  if (!array)
    return;

  for (int i = 0; i < array->object_count - 1; i++) {
    SDFObject *obj_a = array->objects[i];
    if (!obj_a)
      continue;

    for (int j = i + 1; j < array->object_count; j++) {
      SDFObject *obj_b = array->objects[j];
      if (!obj_b)
        continue;

      if (obj_a->type == SDF_TYPE_CIRCLE && obj_b->type == SDF_TYPE_CIRCLE) {
        Circle *obj_a_data = (Circle *)obj_a->data;
        Circle *obj_b_data = (Circle *)obj_b->data;

        float dist_between_surfaces =
            vec2_distance(obj_a_data->center, obj_b_data->center) -
            (obj_a_data->radius + obj_b_data->radius + K / 2.0f);

        Vec2 dir_from_a_to_b =
            vec2_normalize(vec2_sub(obj_b_data->center, obj_a_data->center));

        if (dist_between_surfaces <= 0.0f) {
          Vec2 vel = vec2_scale(dir_from_a_to_b,
                                -6.0f * dist_between_surfaces * delta_time);
          if (obj_a_data->radius >= obj_b_data->radius) {
            obj_b_data->center = vec2_sub(obj_b_data->center, vel);
          } else {
            obj_a_data->center = vec2_add(obj_b_data->center, vel);
          }
        }

        if (vec2_distance(obj_a_data->center, obj_b_data->center) <=
            fmaxf(obj_a_data->radius, obj_b_data->radius)) {

          // Merge and remove the smaller
          if (obj_a_data->radius < obj_b_data->radius) {
            obj_b_data->radius = sqrtf(obj_b_data->radius * obj_b_data->radius +
                                       obj_a_data->radius * obj_a_data->radius);

            dynamic_fill_array_remove_index(array, i); // safe removal
            break; // obj_a deleted, stop inner loop
          } else {
            obj_a_data->radius = sqrtf(obj_a_data->radius * obj_a_data->radius +
                                       obj_b_data->radius * obj_b_data->radius);

            dynamic_fill_array_remove_index(array, j); // safe removal
            continue; // obj_b deleted, check next j
          }
        }
      }
    }
  }
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
  DynamicFillArray drop_array = new_dynamic_fill_array(MAX_OBJECTS);

  Vec2 mouse_pos = {0.0f, 0.0f};
  float mouse_scroll = 0.0f;
  float new_drop_size = 10.0f;
  float k = K;

  Uint64 last_counter = SDL_GetPerformanceCounter();

  while (running) {
    SDL_Event e;
    mouse_scroll = 0.0f;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT)
        running = false;
      if (e.type == SDL_EVENT_MOUSE_WHEEL)
        mouse_scroll = e.wheel.y;
      if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        SDFObject *new_drop = create_circle(mouse_pos,new_drop_size);
        if (new_drop != NULL) {
          dynamic_fill_array_insert(&drop_array, new_drop);
        }
      }
    }

    Uint64 current_counter = SDL_GetPerformanceCounter();
    Uint64 freq = SDL_GetPerformanceFrequency();
    float delta_time = (float)(current_counter - last_counter) / (float)freq;
    last_counter = current_counter;

    SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);

    simulate_drop(&drop_array, delta_time);
    new_drop_size += mouse_scroll;

    /* draw pixels */
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        float min_dist = FLT_MAX;

        for (int i = 0; i < drop_array.object_count; i++) {
          SDFObject *obj = drop_array.objects[i];
          if (!obj)
            continue;

          if (obj->type == SDF_TYPE_CIRCLE) {
            Circle *c = (Circle *)obj->data;
            float maxInfluence = c->radius + k;
            if (vec2_distance_sq((Vec2){(float)x, (float)y}, c->center) >
                maxInfluence * maxInfluence) {
              continue;
            }
          }

          float d = obj->sdf(obj, (Vec2){(float)x, (float)y});
          if (d <= 100.0f)
            min_dist = smooth_union(min_dist, d, k);
        }

        uint8_t r = 0, g = 0, b = 0;
        if (min_dist <= 0.0f) {
          r = clamp_int(100 - (int)fabsf(min_dist), 0, 100);
          g = clamp_int(100 - (int)fabsf(min_dist), 0, 100);
          b = 255;
        }

        buffer[y * WIDTH + x] = make_pixel(r, g, b, 255);
      }
    }

    SDL_UpdateTexture(texture, NULL, buffer, WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
  }

  dynamic_fill_array_free(&drop_array);

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
