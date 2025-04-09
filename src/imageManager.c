#include "imageManager.h"
#include "clay.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define IMAGEMANAGER_MAX_LOADED_IMAGE 128

struct ImageData {
    Image image;
    Texture2D cachedTexture;
    uint32_t quantizedWidth;
    uint32_t quantizedHeight;
    uint32_t hash;
};

struct ImageArray {
    struct ImageData images[IMAGEMANAGER_MAX_LOADED_IMAGE];
    size_t endPtr;
};

struct ImageArray imageArray;

// djb2 string hashing algorithm, using xor instead of addition.
// uint32_t overflow are well-defined as result of modulus of 2^32
uint32_t _Hash_String(const char *str) {
    uint32_t hash = 5381;
    char c;

    while ((c = *str++) != '\0') {
        hash = ((hash << 5) + hash) ^ c;
    }

    return hash;
}

void ImageManager_Init(void) {
    imageArray.endPtr = 0;
}

Clay_Dimensions
ImageManager_LoadImage(const char *filePath, const char *imageName) {
    if (imageArray.endPtr == IMAGEMANAGER_MAX_LOADED_IMAGE) {
        fprintf(
            stderr, "IMAGE: Failed to load image - out of allocated space.\n"
        );
        return (Clay_Dimensions){0};
    }

    Image image = LoadImage(filePath);

    if (!IsImageReady(image)) {
        fprintf(
            stderr, "IMAGE: invalid image - is %s a valid image?.\n", filePath
        );
        return (Clay_Dimensions){0};
    }

    imageArray.images[imageArray.endPtr++] =
        (struct ImageData){.image = image,
                           .hash = _Hash_String(imageName),
                           .quantizedWidth = 0,
                           .quantizedHeight = 0,
                           .cachedTexture = (Texture2D){.id = 0}};

    return (Clay_Dimensions){image.width, image.height};
}

Texture2D
ImageManager_GetTexture(const char *imageName, float width, float height) {
    uint32_t hash = _Hash_String(imageName);

    for (int i = 0; i < imageArray.endPtr; i++) {
        if (imageArray.images[i].hash == hash) {
            struct ImageData *imageData = &imageArray.images[i];

            // Raylib resizing work with integer, so truncating would match the
            // actual texture better. Can't do anything about that.
            uint32_t w = (uint32_t)width;
            uint32_t h = (uint32_t)height;

            if (w == imageData->quantizedWidth &&
                h == imageData->quantizedHeight) {
                return imageData->cachedTexture;
            }

            // cache invalidation
            imageData->quantizedHeight = h;
            imageData->quantizedWidth = w;
            // safe, as uninitialized texture has id = 0
            UnloadTexture(imageData->cachedTexture);

            Image temp = ImageCopy(imageData->image);
            ImageResize(&temp, width, height);

            imageData->cachedTexture = LoadTextureFromImage(temp);

            UnloadImage(temp);

            return imageData->cachedTexture;
        }
    }

    fprintf(stderr, "IMAGE: image %s not found.\n", imageName);
    return (Texture){0};
}
