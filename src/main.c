#include <raylib.h>
#include <stdint.h>

#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "renderer.h"
#include "mainScreen.h"
#include "fontManager.h"
#include "imageManager.h"

int main(int argc, char **argv) {
    const uint32_t width = 1280;
    const uint32_t height = 720;

    Renderer_Init(width, height);
    InitWindow(width, height, "New Window");
    FontManager_Init();
    ImageManager_Init();
    SetTargetFPS(60);
    Clay_SetDebugModeEnabled(true);

    struct Screen screen = mainScreen;

    mainScreen.init();

    while (!WindowShouldClose()) {
        screen.act(GetFrameTime());

        BeginDrawing();
        ClearBackground(WHITE);
        Renderer_Render(screen.layout());
        EndDrawing();
    }
}
