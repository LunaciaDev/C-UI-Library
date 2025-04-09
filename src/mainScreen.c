#include "mainScreen.h"
#include "clay.h"
#include "imageManager.h"

Clay_Dimensions pfp;

void init(void) {
    pfp = ImageManager_LoadImage("image/pfp.png", "pfp");
}

void act(float dt) {
    return;
}

const Clay_Color COLOR_LIGHT = (Clay_Color){224, 215, 210, 255};
const Clay_Color COLOR_RED = (Clay_Color){168, 66, 28, 255};
const Clay_Color COLOR_ORANGE = (Clay_Color){225, 138, 50, 255};

// Layout config is just a struct that can be declared statically, or inline
Clay_ElementDeclaration sidebarItemConfig = (Clay_ElementDeclaration){
    .layout =
        {.sizing =
             {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(100)}},
    .backgroundColor = COLOR_ORANGE,
    .cornerRadius = CLAY_CORNER_RADIUS(10),
    .border = {COLOR_RED, CLAY_BORDER_ALL(15)}
};

// Re-useable components are just normal functions
void SidebarItemComponent(void) {
    CLAY(sidebarItemConfig) {
        // children go here...
    }
}

Clay_RenderCommandArray layout(void) {
    Clay_BeginLayout();

    CLAY({
        .id = CLAY_ID("OuterContainer"),
        .layout =
            {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                     .padding = CLAY_PADDING_ALL(16),
                     .childGap = 16},
        .backgroundColor = {250, 250, 255, 255}
    }) {
        CLAY({
            .id = CLAY_ID("SideBar"),
            .layout =
                {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                         .sizing =
                     {.width = CLAY_SIZING_FIXED(300),
                      .height = CLAY_SIZING_GROW(0)},
                         .padding = CLAY_PADDING_ALL(16),
                         .childGap = 16},
            .backgroundColor = COLOR_LIGHT
        }) {
            CLAY({
                .id = CLAY_ID("ProfilePictureOuter"),
                .layout =
                    {.sizing = {.width = CLAY_SIZING_GROW(0)},
                             .padding = CLAY_PADDING_ALL(16),
                             .childGap = 16,
                             .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}},
                .backgroundColor = COLOR_RED
            }) {
                CLAY_TEXT(
                    CLAY_STRING("Clay - UI Library"),
                    CLAY_TEXT_CONFIG({
                        .fontSize = 16, .textColor = {255, 255, 255, 255}
                })
                );
            }

            // Standard C code like loops etc work inside components
            for (int i = 0; i < 5; i++) {
                SidebarItemComponent();
            }

            CLAY(
                {.id = CLAY_ID("MainContent"),
                 .layout =
                     {.sizing =
                          {.width = CLAY_SIZING_GROW(0),
                           .height = CLAY_SIZING_GROW(0)}},
                 .backgroundColor = COLOR_LIGHT}
            ) {}
        }
    }

    return Clay_EndLayout();
}

struct Screen mainScreen = {.init = init, .act = act, .layout = layout};
