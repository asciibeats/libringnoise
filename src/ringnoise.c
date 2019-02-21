#include "ringnoise.h"
#include <math.h>

#define NORMAL 0.7071068f

static const float pi2 = 6.283185307179586476925286766559f;
static const int hmask = 255;
static const int gmask4 = 7;

static const int hash[] = {
  151,160,137, 91, 90, 15,131, 13,201, 95, 96, 53,194,233,  7,225,
  140, 36,103, 30, 69,142,  8, 99, 37,240, 21, 10, 23,190,  6,148,
  247,120,234, 75,  0, 26,197, 62, 94,252,219,203,117, 35, 11, 32,
  57,177, 33, 88,237,149, 56, 87,174, 20,125,136,171,168, 68,175,
  74,165, 71,134,139, 48, 27,166, 77,146,158,231, 83,111,229,122,
  60,211,133,230,220,105, 92, 41, 55, 46,245, 40,244,102,143, 54,
  65, 25, 63,161,  1,216, 80, 73,209, 76,132,187,208, 89, 18,169,
  200,196,135,130,116,188,159, 86,164,100,109,198,173,186,  3, 64,
  52,217,226,250,124,123,  5,202, 38,147,118,126,255, 82, 85,212,
  207,206, 59,227, 47, 16, 58, 17,182,189, 28, 42,223,183,170,213,
  119,248,152,  2, 44,154,163, 70,221,153,101,155,167, 43,172,  9,
  129, 22, 39,253, 19, 98,108,110, 79,113,224,232,178,185,112,104,
  218,246, 97,228,251, 34,242,193,238,210,144, 12,191,179,162,241,
  81, 51,145,235,249, 14,239,107, 49,192,214, 31,181,199,106,157,
  184, 84,204,176,115,121, 50, 45,127,  4,150,254,138,236,205, 93,
  222,114, 67, 29, 24, 72,243,141,128,195, 78, 66,215, 61,156,180,

  151,160,137, 91, 90, 15,131, 13,201, 95, 96, 53,194,233,  7,225,
  140, 36,103, 30, 69,142,  8, 99, 37,240, 21, 10, 23,190,  6,148,
  247,120,234, 75,  0, 26,197, 62, 94,252,219,203,117, 35, 11, 32,
  57,177, 33, 88,237,149, 56, 87,174, 20,125,136,171,168, 68,175,
  74,165, 71,134,139, 48, 27,166, 77,146,158,231, 83,111,229,122,
  60,211,133,230,220,105, 92, 41, 55, 46,245, 40,244,102,143, 54,
  65, 25, 63,161,  1,216, 80, 73,209, 76,132,187,208, 89, 18,169,
  200,196,135,130,116,188,159, 86,164,100,109,198,173,186,  3, 64,
  52,217,226,250,124,123,  5,202, 38,147,118,126,255, 82, 85,212,
  207,206, 59,227, 47, 16, 58, 17,182,189, 28, 42,223,183,170,213,
  119,248,152,  2, 44,154,163, 70,221,153,101,155,167, 43,172,  9,
  129, 22, 39,253, 19, 98,108,110, 79,113,224,232,178,185,112,104,
  218,246, 97,228,251, 34,242,193,238,210,144, 12,191,179,162,241,
  81, 51,145,235,249, 14,239,107, 49,192,214, 31,181,199,106,157,
  184, 84,204,176,115,121, 50, 45,127,  4,150,254,138,236,205, 93,
  222,114, 67, 29, 24, 72,243,141,128,195, 78, 66,215, 61,156,180
};

static const struct vector4 grads4[] = {
  {NORMAL, NORMAL, 0.0f, 0.0f},
  {-NORMAL, -NORMAL, 0.0f, 0.0f},
  {0.0f, NORMAL, NORMAL, 0.0f},
  {0.0f, -NORMAL, -NORMAL, 0.0f},
  {0.0f, 0.0f, NORMAL, NORMAL},
  {0.0f, 0.0f, -NORMAL, -NORMAL},
  {NORMAL, 0.0f, 0.0f, NORMAL},
  {-NORMAL, 0.0f, 0.0f, -NORMAL}
};

static inline float dot4(struct vector4 grad, float x, float y, float z, float w)
{
  return grad.x * x + grad.y * y + grad.z * z + grad.w * w;
}

static inline float smooth(float t)
{
  return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static inline float derivative(float t)
{
  return 30.0f * t * t * (t * (t - 2.0f) + 1.0f);
}

static inline float lerp(float a, float b, float t)
{
  return a + (b - a) * t;
}

float rn_perlin4(struct vector4 point, float frequency)
{
  point.x *= frequency;
  point.y *= frequency;
  point.z *= frequency;
  point.w *= frequency;

  int ix0 = (int)floorf(point.x);
  int iy0 = (int)floorf(point.y);
  int iz0 = (int)floorf(point.z);
  int iw0 = (int)floorf(point.w);

  float tx0 = point.x - ix0;
  float ty0 = point.y - iy0;
  float tz0 = point.z - iz0;
  float tw0 = point.w - iw0;

  float tx1 = tx0 - 1.0f;
  float ty1 = ty0 - 1.0f;
  float tz1 = tz0 - 1.0f;
  float tw1 = tw0 - 1.0f;

  ix0 &= hmask;
  iy0 &= hmask;
  iz0 &= hmask;
  iw0 &= hmask;

  int ix1 = ix0 + 1;
  int iy1 = iy0 + 1;
  int iz1 = iz0 + 1;
  int iw1 = iw0 + 1;

  int h0 = hash[ix0];
  int h1 = hash[ix1];

  int h00 = hash[h0 + iy0];
  int h10 = hash[h1 + iy0];
  int h01 = hash[h0 + iy1];
  int h11 = hash[h1 + iy1];

  int h000 = hash[h00 + iz0];
  int h100 = hash[h10 + iz0];
  int h010 = hash[h01 + iz0];
  int h110 = hash[h11 + iz0];
  int h001 = hash[h00 + iz1];
  int h101 = hash[h10 + iz1];
  int h011 = hash[h01 + iz1];
  int h111 = hash[h11 + iz1];

  struct vector4 g0000 = grads4[hash[h000 + iw0] & gmask4];
  struct vector4 g1000 = grads4[hash[h100 + iw0] & gmask4];
  struct vector4 g0100 = grads4[hash[h010 + iw0] & gmask4];
  struct vector4 g1100 = grads4[hash[h110 + iw0] & gmask4];
  struct vector4 g0010 = grads4[hash[h001 + iw0] & gmask4];
  struct vector4 g1010 = grads4[hash[h101 + iw0] & gmask4];
  struct vector4 g0110 = grads4[hash[h011 + iw0] & gmask4];
  struct vector4 g1110 = grads4[hash[h111 + iw0] & gmask4];
  struct vector4 g0001 = grads4[hash[h000 + iw1] & gmask4];
  struct vector4 g1001 = grads4[hash[h100 + iw1] & gmask4];
  struct vector4 g0101 = grads4[hash[h010 + iw1] & gmask4];
  struct vector4 g1101 = grads4[hash[h110 + iw1] & gmask4];
  struct vector4 g0011 = grads4[hash[h001 + iw1] & gmask4];
  struct vector4 g1011 = grads4[hash[h101 + iw1] & gmask4];
  struct vector4 g0111 = grads4[hash[h011 + iw1] & gmask4];
  struct vector4 g1111 = grads4[hash[h111 + iw1] & gmask4];

  float v0000 = dot4(g0000, tx0, ty0, tz0, tw0);
  float v1000 = dot4(g1000, tx1, ty0, tz0, tw0);
  float v0100 = dot4(g0100, tx0, ty1, tz0, tw0);
  float v1100 = dot4(g1100, tx1, ty1, tz0, tw0);
  float v0010 = dot4(g0010, tx0, ty0, tz1, tw0);
  float v1010 = dot4(g1010, tx1, ty0, tz1, tw0);
  float v0110 = dot4(g0110, tx0, ty1, tz1, tw0);
  float v1110 = dot4(g1110, tx1, ty1, tz1, tw0);
  float v0001 = dot4(g0001, tx0, ty0, tz0, tw1);
  float v1001 = dot4(g1001, tx1, ty0, tz0, tw1);
  float v0101 = dot4(g0101, tx0, ty1, tz0, tw1);
  float v1101 = dot4(g1101, tx1, ty1, tz0, tw1);
  float v0011 = dot4(g0011, tx0, ty0, tz1, tw1);
  float v1011 = dot4(g1011, tx1, ty0, tz1, tw1);
  float v0111 = dot4(g0111, tx0, ty1, tz1, tw1);
  float v1111 = dot4(g1111, tx1, ty1, tz1, tw1);

  float tx = smooth(tx0);
  float ty = smooth(ty0);
  float tz = smooth(tz0);
  float tw = smooth(tw0);

  return lerp(lerp(lerp(lerp(v0000, v1000, tx), lerp(v0100, v1100, tx), ty), lerp(lerp(v0010, v1010, tx), lerp(v0110, v1110, tx), ty), tz), lerp(lerp(lerp(v0001, v1001, tx), lerp(v0101, v1101, tx), ty), lerp(lerp(v0011, v1011, tx), lerp(v0111, v1111, tx), ty), tz), tw) + 0.5f;
}

float rn_perlinsum4(struct vector4 point, float frequency, int octaves, float lacunarity, float persistence)
{
  float sum = rn_perlin4(point, frequency);
  float amp = 1.0f;
  float range = 1.0f;

  for (int o = 1; o < octaves; o++)
  {
    frequency *= lacunarity;
    amp *= persistence;
    range += amp;
    sum += rn_perlin4(point, frequency) * amp;
  }

  return sum / range;
}

float rn_perlinring2(float x, float y, float offset, float frequency)
{
  x = x * pi2;
  y = y * pi2;

  struct vector4 point;

  point.x = sin(x) + offset;
  point.y = cos(x) + offset;
  point.z = sin(y) + offset;
  point.w = cos(y) + offset;

  return rn_perlin4(point, frequency);
}

float rn_perlinringsum2(float x, float y, float offset, float frequency, int octaves, float lacunarity, float persistence)
{
  x = x * pi2;
  y = y * pi2;

  struct vector4 point;

  point.x = sin(x) + offset;
  point.y = cos(x) + offset;
  point.z = sin(y) + offset;
  point.w = cos(y) + offset;

  return rn_perlinsum4(point, frequency, octaves, lacunarity, persistence);
}
