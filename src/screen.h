#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "clay.h"

struct Screen {
    void (*init)(void);
    void (*act)(float dt);
    Clay_RenderCommandArray (*layout)(void);
};

#endif
