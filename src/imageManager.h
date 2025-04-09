#ifndef __IMAGE_MANAGER_H__
#define __IMAGE_MANAGER_H__

#include <raylib.h>
#include "clay.h"

void ImageManager_Init(void);
Clay_Dimensions ImageManager_LoadImage(const char* filePath, const char* imageName);
Texture2D ImageManager_GetTexture(const char* imageName, float width, float height);

#endif
