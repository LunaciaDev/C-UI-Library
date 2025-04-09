#include "renderer.h"
#include "fontManager.h"
#include "imageManager.h"
#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

//---------------------------------------------------------
// HELPER FUNCTIONS
//---------------------------------------------------------

// the returning string must be free'd by the caller
static char *_Clay_StringSlice_To_CString(Clay_StringSlice strSlice) {
    char *cstring = malloc(strSlice.length + 1);
    memcpy(cstring, strSlice.chars, strSlice.length);
    cstring[strSlice.length] = '\0';

    return cstring;
}

static Clay_Dimensions _Vector2_To_Clay_Dimensions(Vector2 vector2) {
    return (Clay_Dimensions){vector2.x, vector2.y};
}

static float _Normalize_Corners(Clay_CornerRadius cornerRadius) {
    return fmaxf(
        fmaxf(cornerRadius.bottomLeft, cornerRadius.bottomRight),
        fmaxf(cornerRadius.topLeft, cornerRadius.topRight)
    );
}

static float _Normalize_Thickness(Clay_BorderWidth borderThickness) {
    return fmaxf(
        fmaxf(borderThickness.top, borderThickness.bottom),
        fmaxf(borderThickness.left, borderThickness.right)
    );
}

Color _Clay_To_Raylib_Color(Clay_Color color) {
    return (Color){color.r, color.g, color.b, color.a};
}

static void _Get_CornerCenters(
    Clay_BoundingBox boundingBox, float cornerRadius, Vector2 *cornerCenters
) {
    cornerCenters[0] =
        (Vector2){.x = boundingBox.x + boundingBox.width - cornerRadius,
                  .y = boundingBox.y + boundingBox.height - cornerRadius};
    cornerCenters[1] =
        (Vector2){.x = boundingBox.x + cornerRadius,
                  .y = boundingBox.y + boundingBox.height - cornerRadius};
    cornerCenters[2] = (Vector2){.x = boundingBox.x + cornerRadius,
                                 .y = boundingBox.y + cornerRadius};
    cornerCenters[3] =
        (Vector2){.x = boundingBox.x + boundingBox.width - cornerRadius,
                  .y = boundingBox.y + cornerRadius};
}

static void _Get_RectEdges(
    Clay_BoundingBox boundingBox, float cornerRadius, float edgeThickness,
    Rectangle *rects
) {
    rects[0] = (Rectangle){.x = boundingBox.x + cornerRadius,
                           .y = boundingBox.y,
                           .width = boundingBox.width - 2 * cornerRadius,
                           .height = edgeThickness};
    rects[1] =
        (Rectangle){.x = boundingBox.x + boundingBox.width - edgeThickness,
                    .y = boundingBox.y + cornerRadius,
                    .width = edgeThickness,
                    .height = boundingBox.height - 2 * cornerRadius};
    rects[2] =
        (Rectangle){.x = boundingBox.x + cornerRadius,
                    .y = boundingBox.y + boundingBox.height - edgeThickness,
                    .width = boundingBox.width - 2 * cornerRadius,
                    .height = edgeThickness};
    rects[3] = (Rectangle){.x = boundingBox.x,
                           .y = boundingBox.y + cornerRadius,
                           .width = edgeThickness,
                           .height = boundingBox.height - 2 * cornerRadius};
    rects[4] = (Rectangle){.x = boundingBox.x + cornerRadius,
                           .y = boundingBox.y + cornerRadius,
                           .width = boundingBox.width - 2 * cornerRadius,
                           .height = boundingBox.height - 2 * cornerRadius};
}

//---------------------------------------------------------
// CLAY INITIALIZATION
//---------------------------------------------------------

static void _Error_Handler(Clay_ErrorData errorText) {}

static Clay_Dimensions _Measure_Text(
    Clay_StringSlice strSlice, Clay_TextElementConfig *config, void *usrData
) {
    char *cStrSlice = _Clay_StringSlice_To_CString(strSlice);
    Vector2 measurement = MeasureTextEx(
        FontManager_GetFontByID(config->fontId), cStrSlice, config->fontSize,
        config->letterSpacing
    );
    free(cStrSlice);
    return _Vector2_To_Clay_Dimensions(measurement);
}

void Renderer_Init(uint32_t width, uint32_t height) {
    uint32_t capacity = Clay_MinMemorySize();
    Clay_Arena arena =
        Clay_CreateArenaWithCapacityAndMemory(capacity, malloc(capacity));
    Clay_Initialize(
        arena, (Clay_Dimensions){width, height},
        (Clay_ErrorHandler){_Error_Handler, NULL}
    );
    Clay_SetMeasureTextFunction(_Measure_Text, NULL);
}

//---------------------------------------------------------
// RENDERING IMPLEMENTATION
//---------------------------------------------------------

static void _Render_Rect(Clay_RenderCommand *renderCommand) {
    Clay_RectangleRenderData renderData = renderCommand->renderData.rectangle;
    float cornerRadius = _Normalize_Corners(renderData.cornerRadius);
    Color backgroundColor = _Clay_To_Raylib_Color(renderData.backgroundColor);

    if (cornerRadius == 0) {
        DrawRectangle(
            renderCommand->boundingBox.x, renderCommand->boundingBox.y,
            renderCommand->boundingBox.width, renderCommand->boundingBox.height,
            backgroundColor
        );
        return;
    }

    // rounded corners - Raylib only support drawing rounded corner as a
    // function of roundness against width/height (ln 961, rshapes.c). In this
    // usecase the radius is known beforehand.

    // Vector2 of corners center points, from bottom-right going clockwise
    Vector2 cornerCenters[4];

    // Filling in all remaining rectangular areas:
    Rectangle rects[5];

    _Get_CornerCenters(
        renderCommand->boundingBox, cornerRadius, &cornerCenters[0]
    );
    _Get_RectEdges(
        renderCommand->boundingBox, cornerRadius, cornerRadius, &rects[0]
    );

    // Drawing time:
    int angle = 0;
    for (int i = 0; i < 4; i++) {
        DrawRing(
            cornerCenters[i], 0, cornerRadius, angle, angle + 90, 0,
            backgroundColor
        );
        angle += 90;
    }

    for (int i = 0; i < 5; i++) {
        DrawRectangleRec(rects[i], backgroundColor);
    }
}

static void _Render_Border_Edges(
    Clay_BorderWidth borderWidth, bool *enabledCorners, Rectangle *rects,
    Color color
) {
    if (borderWidth.bottom != 0) {
        enabledCorners[0] = true;
        enabledCorners[1] = true;
        DrawRectangleRec(rects[2], color);
    }

    if (borderWidth.left != 0) {
        enabledCorners[1] = true;
        enabledCorners[2] = true;
        DrawRectangleRec(rects[3], color);
    }

    if (borderWidth.top != 0) {
        enabledCorners[2] = true;
        enabledCorners[3] = true;
        DrawRectangleRec(rects[0], color);
    }

    if (borderWidth.right != 0) {
        enabledCorners[3] = true;
        enabledCorners[0] = true;
        DrawRectangleRec(rects[1], color);
    }
}

static void _Render_Border_No_Rounding(
    Clay_BorderWidth borderWidth, Rectangle boundingBox, Rectangle *rects,
    float cornerRadius, Color color
) {
    bool enabledCorners[4] = {false};

    Rectangle corners[4] = {
        (Rectangle){boundingBox.x + boundingBox.width - cornerRadius,
                    boundingBox.y + boundingBox.height - cornerRadius,
                    cornerRadius, cornerRadius                                                                 },
        (Rectangle){                                   boundingBox.x,
                    boundingBox.y + boundingBox.height - cornerRadius,
                    cornerRadius, cornerRadius                                                                 },
        (Rectangle){                                   boundingBox.x, boundingBox.y, cornerRadius, cornerRadius},
        (Rectangle){boundingBox.x + boundingBox.width - cornerRadius,
                    boundingBox.y, cornerRadius, cornerRadius                                                  }
    };

    _Render_Border_Edges(borderWidth, &enabledCorners[0], rects, color);

    for (int i = 0; i < 4; i++) {
        if (!enabledCorners[i])
            continue;

        DrawRectangleRec(corners[i], color);
    }
}

static void _Render_Border_Rounded(
    Clay_BorderWidth borderWidth, Clay_BoundingBox boundingBox,
    Rectangle *rects, Vector2 *cornerCenters, float cornerRadius,
    float outerBorderThickness, Color color
) {
    bool enabledCorners[4] = {false};
    int angle = -90;

    _Render_Border_Edges(borderWidth, &enabledCorners[0], rects, color);

    for (int i = 0; i < 4; i++) {
        angle += 90;

        if (!enabledCorners[i])
            continue;

        DrawRing(
            cornerCenters[i], cornerRadius - outerBorderThickness, cornerRadius,
            angle, angle + 90, 0, color
        );
    }
}

static void _Render_Border(Clay_RenderCommand *renderCommand) {
    Clay_BorderRenderData renderData = renderCommand->renderData.border;
    Clay_BoundingBox boundingBox = renderCommand->boundingBox;
    float borderThickness = _Normalize_Thickness(renderData.width);
    float cornerRadius = _Normalize_Corners(renderData.cornerRadius);
    Color backgroundColor = _Clay_To_Raylib_Color(renderData.color);

    Rectangle edges[5];
    Vector2 cornerCenters[4];

    float outerBorderThickness = fminf(cornerRadius, borderThickness);

    _Get_RectEdges(boundingBox, cornerRadius, outerBorderThickness, &edges[0]);
    _Get_CornerCenters(boundingBox, cornerRadius, &cornerCenters[0]);

    // draw the outer border first, should be fine when radius is 0... I hope.
    _Render_Border_Rounded(
        renderData.width, boundingBox, &edges[0], &cornerCenters[0],
        cornerRadius, outerBorderThickness, backgroundColor
    );

    // Get remaining border width to fill in.
    // this will be full border thickness if radius is 0
    float remainingBorder = borderThickness - outerBorderThickness;
    Rectangle innerRect = edges[4];

    _Get_RectEdges(
        (Clay_BoundingBox){edges[4].x, edges[4].y, edges[4].width,
                           edges[4].height},
        remainingBorder, remainingBorder, &edges[0]
    );

    _Render_Border_No_Rounding(
        renderData.width, innerRect, &edges[0], remainingBorder, backgroundColor
    );
}

static void _Render_Text(Clay_RenderCommand *renderCommand) {
    Clay_TextRenderData renderData = renderCommand->renderData.text;
    char *cStr = _Clay_StringSlice_To_CString(renderData.stringContents);
    Color textColor = _Clay_To_Raylib_Color(renderData.textColor);

    DrawTextEx(
        FontManager_GetFontByID(renderData.fontId), cStr,
        (Vector2){renderCommand->boundingBox.x, renderCommand->boundingBox.y},
        renderData.fontSize, renderData.letterSpacing, textColor
    );
}

/*
    When rendering an image, we do not know during the layout phase whether
   such image will be sized to by Clay. It could shrink, grow, whatever it
   want to do. Rasterizing an image into a texture every frame is costly
   (RAM -> VRAM is "slow"), especially if the UI is built by drawing textures
   instead of primitives.

   Caching open up the classic problem of cache invalidation. One probably
   can invalidate cache if the image size change?

   So an image is IDed by the Manager, then every frame, check bounding box
   width/height compared against the cached texture. If no change, we reuse
   the texture, otherwise copy, transform, rasterize the image into a new
   texture to be cached.

   But the bounding box are given as a set of float, so this approach hit a
   wall due to exact floating point comparison.

   We can quantize the float down to integer to do comparison. And turns
   out, Raylib renders on integer, not float, so this perfectly match render
   result!

   In this implementation, imageData voidPtr is pointing to a string lateral
   having the name of the image. Corner radius is ignored as Raylib do not
   support that kind of cropping.

   TODO: supporting atlas. Not going to be anytime soon as need to batch draw by
   Atlas, which I have no control of.
*/
static void _Render_Image(Clay_RenderCommand *renderCommand) {
    Clay_ImageRenderData renderData = renderCommand->renderData.image;
    Clay_BoundingBox boundingBox = renderCommand->boundingBox;

    Texture2D image = ImageManager_GetTexture(
        renderData.imageData, boundingBox.width, boundingBox.height
    );

    DrawTexture(image, boundingBox.x, boundingBox.y, WHITE);
}

void Renderer_Render(Clay_RenderCommandArray renderCommands) {
    for (int i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommand *renderCommand = renderCommands.internalArray + i;

        switch (renderCommand->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
            case CLAY_RENDER_COMMAND_TYPE_NONE:
                // noops
                break;

            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
                _Render_Rect(renderCommand);
                break;

            case CLAY_RENDER_COMMAND_TYPE_BORDER:
                _Render_Border(renderCommand);
                break;

            case CLAY_RENDER_COMMAND_TYPE_TEXT:
                _Render_Text(renderCommand);
                break;

            case CLAY_RENDER_COMMAND_TYPE_IMAGE:
                _Render_Image(renderCommand);
                break;

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
                BeginScissorMode(
                    renderCommand->boundingBox.x, renderCommand->boundingBox.y,
                    renderCommand->boundingBox.width,
                    renderCommand->boundingBox.height
                );
                break;

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
                EndScissorMode();
                break;
        }
    }
}
