#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <stdint.h>
#include "clay.h"

void Renderer_Init(uint32_t width, uint32_t height);
void Renderer_Render(Clay_RenderCommandArray renderCommands);

#endif
