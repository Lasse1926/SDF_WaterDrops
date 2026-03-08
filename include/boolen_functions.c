#include <math.h>

float smooth_union(float d1, float d2, float k)
{
    float h = fmaxf(0.0f, fminf(1.0f, 0.5f + 0.5f*(d2 - d1)/k));
    return (1.0f - h)*d2 + h*d1 - k*h*(1.0f - h);
}

float smooth_subtract(float d1, float d2, float k)
{
    float h = fmaxf(0.0f, fminf(1.0f, 0.5f - 0.5f*(d2 + d1)/k));
    return (1.0f - h)*d2 + h*(-d1) + k*h*(1.0f - h);
}

float smooth_intersect(float d1, float d2, float k)
{
    float h = fmaxf(0.0f, fminf(1.0f, 0.5f - 0.5f*(d2 - d1)/k));
    return (1.0f - h)*d2 + h*d1 + k*h*(1.0f - h);
}
