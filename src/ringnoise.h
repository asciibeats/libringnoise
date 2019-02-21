#ifndef RINGNOISE_H
#define RINGNOISE_H

#ifdef __cplusplus
extern "C" {
#endif

struct vector4 {
  float x, y, z, w;
};

extern float rn_perlin4(struct vector4 point, float frequency);
extern float rn_perlinsum4(struct vector4 point, float frequency, int octaves, float lacunarity, float persistence);
extern float rn_perlinring2(float x, float y, float offset, float frequency);
extern float rn_perlinringsum2(float x, float y, float offset, float frequency, int octaves, float lacunarity, float persistence);

#ifdef __cplusplus
}
#endif

#endif
