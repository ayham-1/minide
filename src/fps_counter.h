#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

#include "text_renderer.h"

void fps_counter_init(size_t width, size_t height);
void fps_counter_render();
void fps_counter_update(size_t frames);

#endif
