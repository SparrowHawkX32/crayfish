#ifndef CRAYFISH_UI_H
#define CRAYFISH_UI_H

#include "render.h"
#include <raylib.h>

#define PADDING 5
#define DEFAULT_LEVEL_PATH "../levels"
#define DEFAULT_ATLAS_PATH "../textures"
#define VISIBLE_TILES 8
#define SCROLL_RATE 6
#define TILES_PER_ROW 4

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
    String atlas_dd_entries;
    int atlas_dd_active;
    int atlas_dd_edit;
    char atlas_path[128];
    int atlas_path_edit;
    FilePathList atlases;
    char render_width[10];
    int render_width_edit;
    char render_height[10];
    int render_height_edit;
    char render_dist[10];
    int render_dist_edit;
    char fov[10];
    int fov_edit;
  } menu;
  struct {
    char lvl_name[128];
    int lvl_name_edit;
    char lvl_width[10];
    int lvl_width_edit;
    char lvl_height[10];
    int lvl_height_edit;
    char spawn_x[10];
    int spawn_x_edit;
    char spawn_y[10];
    int spawn_y_edit;
    char spawn_rot[10];
    int spawn_rot_edit;
    char floor_color[10];
    int floor_color_edit;
    char ceil_color[10];
    int ceil_color_edit;
    char air_color[10];
    int air_color_edit;
    char atlas_name[128];
    int atlas_name_edit;
    Level* lvl;
    Texture atlas;
    unsigned char selected_tile;
    float zoom;
    Vector2 map_offset;
  } editor;
} UIState;


void populate_levels(UIState* state);

void populate_atlases(UIState* state);

void try_change_atlas(UIState* state, RenderContext* ctx);

void init_ui(UIState* state, RenderContext* ctx);

void draw_ui(UIState* state, RenderContext* ctx);

void unload_ui(UIState* state);

void init_editor(UIState* state, const RenderContext* ctx, Level* lvl);

void draw_editor(UIState* state);

void unload_editor(UIState* state);

#endif
