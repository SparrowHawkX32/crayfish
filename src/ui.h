#ifndef CRAYFISH_UI_H
#define CRAYFISH_UI_H

#include "render.h"
#include <raylib.h>

#define PADDING 5
#define DEFAULT_LEVEL_PATH "../levels"

typedef struct {
  enum {
    HUD,
    MENU,
    EDITOR
  } page;
  struct {
    String lvl_dd_entries;
    int lvl_dd_active;
    int lvl_dd_edit;
    char lvl_path[128];
    int lvl_path_edit;
    FilePathList levels;
  } menu;
} UIState;


void init_ui(UIState* state);

void unload_ui(UIState* state);

void draw_ui(UIState* state, RenderContext* ctx);


#endif
