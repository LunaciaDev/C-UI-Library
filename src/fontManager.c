#include "fontManager.h"
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define FONTMANAGER_MAX_LOADED_FONTS 32

struct FontArray {
    Font fonts[FONTMANAGER_MAX_LOADED_FONTS];
    size_t loadedFontsCount;
};

struct FontArray fontArray;

void FontManager_LoadFont(const char* fontFilePath, uint8_t fontSize) {
    if (fontArray.loadedFontsCount == FONTMANAGER_MAX_LOADED_FONTS) {
        fprintf(stderr, "FONT: Cannot load font - exceeded capacity limit.\n");
        return;
    }

    Font loadedFont = LoadFontEx(fontFilePath, fontSize, NULL, 0);

    if (loadedFont.baseSize != fontSize) {
        fprintf(stderr, "FONT: Cannot load font - Base size mismatch. Does the font file exists?\n");
        return;
    }

    fontArray.fonts[fontArray.loadedFontsCount++] = loadedFont;
}

void FontManager_Init(void) {
    fontArray.loadedFontsCount = 0;

    FontManager_LoadFont("fonts/OpenSans-Regular.ttf", 16);
}

Font FontManager_GetFontByID(uint8_t id) {
    return fontArray.fonts[id];
}
