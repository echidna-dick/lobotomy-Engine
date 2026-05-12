#ifndef MINIMAP_H
#define MINIMAP_H

#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C"  {
#endif

void put_pixel(uint32_t *buf, int buf_w, int buf_h,
                              int x, int y, uint32_t color);

void fill_rect(uint32_t *buf, int buf_w, int buf_h,
                      int x, int y, int w, int h, uint32_t color);

void draw_line(uint32_t *buf, int buf_w, int buf_h,
                      int x0, int y0, int x1, int y1, uint32_t color);

#ifdef __cplusplus
}
#endif

#endif