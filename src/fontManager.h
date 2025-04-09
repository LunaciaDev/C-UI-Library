#ifndef __FONT_MANAGER_H__
#define __FONT_MANAGER_H__

#include <stdint.h>
#include <raylib.h>

void FontManager_Init(void);
Font FontManager_GetFontByID(uint8_t id);
void FontManager_LoadFont(const char* fontFilePath, uint8_t fontSize);

#endif
