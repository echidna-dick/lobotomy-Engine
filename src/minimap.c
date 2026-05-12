#include "minimap.h"

/* === Minimap drawing helpers ===
 * Bounds-checked pixel writers — these are what makes the minimap
 * crash-proof. Out-of-bounds writes are silently skipped instead of
 * corrupting memory. */

void put_pixel(uint32_t *buf, int buf_w, int buf_h,
                              int x, int y, uint32_t color)
{
    if (x < 0 || y < 0 || x >= buf_w || y >= buf_h) return;
    buf[y * buf_w + x] = color;
}

void fill_rect(uint32_t *buf, int buf_w, int buf_h,
                      int x, int y, int w, int h, uint32_t color)
{
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++)
            put_pixel(buf, buf_w, buf_h, xx, yy, color);
}

/* Bresenham's line — walks pixel-by-pixel from (x0,y0) to (x1,y1) */
void draw_line(uint32_t *buf, int buf_w, int buf_h,
                      int x0, int y0, int x1, int y1, uint32_t color)
{
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (1) {
        put_pixel(buf, buf_w, buf_h, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}