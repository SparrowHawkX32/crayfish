#include "ui.h"
#include "level.h"
#include "util.h"
#include <float.h>
#include <limits.h>
#include <math.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "render.h"
#include <raylib.h>
#include <stdio.h>


void populate_levels(UIState *state) {
  state->menu.levels = LoadDirectoryFilesEx(state->menu.lvl_path, ".lvl", true);
  String path = {0};
  string_init(&path);

  for (int i = 0; i < state->menu.levels.count; i++) {
    if (i) string_append(&state->menu.lvl_dd_entries, ";");

    string_zero(&path);
    string_append(&path, state->menu.levels.paths[i]);
    char* token = NULL;
    char* next_token = path.data;
    while (next_token != NULL){
      token = strsep(&next_token, "/");
    }
    string_append(&state->menu.lvl_dd_entries, token);
  }

  free(path.data);
}


void populate_atlases(UIState *state) {
  state->menu.atlases = LoadDirectoryFilesEx(state->menu.atlas_path, ".png", true);
  String path = {0};
  string_init(&path);

  for (int i = 0; i < state->menu.atlases.count; i++) {
    if (i) string_append(&state->menu.atlas_dd_entries, ";");

    string_zero(&path);
    string_append(&path, state->menu.atlases.paths[i]);
    char* token = NULL;
    char* next_token = path.data;
    while (next_token != NULL){
      token = strsep(&next_token, "/");
    }
    string_append(&state->menu.atlas_dd_entries, token);
  }

  free(path.data);
}


void try_change_atlas(UIState *state, RenderContext *ctx) {
  if (!ctx->level) return;
  if (ctx->level->atlas_name[0] == '\0') {
    load_placeholder_atlas(ctx);
    return;
  }

  int atlas_count = 0;
  const char** atlas = TextSplit(state->menu.atlas_dd_entries.data, ';', &atlas_count);
  for (int i = 0; i < state->menu.atlases.count; i++) {
    int name_len = strlen(*atlas);
    if (strlen(ctx->level->atlas_name) <= name_len &&
        strncmp(*atlas, ctx->level->atlas_name, name_len - 4) == 0) {
      change_atlas(ctx, state->menu.atlases.paths[i]);
      state->menu.atlas_dd_active = i;
      break;
    }
    atlas++;
  }
}


void init_ui(UIState* state, RenderContext* ctx) {
  int text_size = GetMonitorWidth(GetCurrentMonitor()) > 1920 ? 32 : 24;
  GuiSetStyle(DEFAULT, TEXT_SIZE, text_size);
  GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

  // Scan + populate levels + atlases
  strncpy(state->menu.lvl_path, DEFAULT_LEVEL_PATH, sizeof(state->menu.lvl_path));
  strncpy(state->menu.atlas_path, DEFAULT_ATLAS_PATH, sizeof(state->menu.atlas_path));
  string_init(&state->menu.lvl_dd_entries);
  string_init(&state->menu.atlas_dd_entries);
  populate_levels(state);
  populate_atlases(state);

  snprintf(state->menu.render_width, 10, "%d", ctx->render_width);
  snprintf(state->menu.render_height, 10, "%d", ctx->render_height);
  snprintf(state->menu.render_dist, 10, "%.2f", ctx->render_dist);
  snprintf(state->menu.fov, 10, "%.2f", ctx->fov);
}


void draw_ui(UIState* state, RenderContext* ctx) {
  int width = GetScreenWidth();
  int height = GetScreenHeight();
  int text_size = GuiGetStyle(DEFAULT, TEXT_SIZE);

  if (state->page == HUD) {
      char hud_text[32] = "";

      sprintf(hud_text, "X: %.2f, Y: %.2f", ctx->cam_pos.x, ctx->cam_pos.y); GuiLabel((Rectangle) {
            .x = PADDING,
            .y = PADDING,
            .width = 200,
            .height = text_size},
          hud_text);

      sprintf(hud_text, "FPS: %.0f", 1 / GetFrameTime());
      GuiLabel((Rectangle){
            .x = PADDING,
            .y = PADDING + text_size,
            .width = 200,
            .height = text_size},
          hud_text);

      if (ctx->level == NULL) {
        GuiDrawText("No level selected\n\n[Esc] to open menu", (Rectangle) {
              .x = width * 0.5f - 8 * text_size,
              .y = height * 0.5f - text_size * 1.5f,
              .width = 16 * text_size,
              .height = 3 * text_size
            },
            TEXT_ALIGN_CENTER, RAYWHITE);
      }

      if (IsKeyPressed(KEY_ESCAPE)) {
        state->page = MENU;
      }
  }
  else if (state->page == MENU) {
    // Calculate sizes
    int panel_width = fmaxf(100, (width - 4 * PADDING) / 3.0f - 2 * PADDING);
    Rectangle lvl_panel = {
          .x = PADDING,
          .y = PADDING,
          .width = panel_width + 2 * PADDING,
          .height = 6 * (text_size + PADDING)};
    Rectangle lvl_dropdown = {
          .x = lvl_panel.x + PADDING,
          .y = lvl_panel.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle lvl_load = {
          .x = lvl_dropdown.x,
          .y = lvl_dropdown.y + text_size + PADDING,
          .width = (panel_width - PADDING) * 0.5f,
          .height = text_size};
    Rectangle lvl_edit = {
          .x = lvl_load.x + lvl_load.width + PADDING,
          .y = lvl_load.y,
          .width = (panel_width - PADDING) * 0.5f,
          .height = text_size};
    Rectangle lvl_new = {
          .x = lvl_load.x,
          .y = lvl_load.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle lvl_path = {
          .x = lvl_dropdown.x,
          .y = lvl_new.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle lvl_scan = {
          .x = lvl_dropdown.x,
          .y = lvl_path.y + text_size + PADDING,
          .width = lvl_load.width,
          .height = text_size};
    Rectangle lvl_default = {
          .x = lvl_edit.x,
          .y = lvl_scan.y,
          .width = lvl_load.width,
          .height = text_size};
    Rectangle atlas_panel = {
          .x = lvl_panel.x + lvl_panel.width + PADDING,
          .y = PADDING,
          .width = lvl_panel.width,
          .height = 5 * (text_size + PADDING)};
    Rectangle atlas_dropdown = {
          .x = atlas_panel.x + PADDING,
          .y = atlas_panel.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle atlas_load = {
          .x = atlas_dropdown.x,
          .y = atlas_dropdown.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle atlas_path = {
          .x = atlas_dropdown.x,
          .y = atlas_load.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle atlas_scan = {
          .x = atlas_dropdown.x,
          .y = atlas_path.y + text_size + PADDING,
          .width = (panel_width - PADDING) * 0.5f,
          .height = text_size};
    Rectangle atlas_default = {
          .x = atlas_scan.x + lvl_scan.width + PADDING,
          .y = atlas_scan.y,
          .width = atlas_scan.width,
          .height = text_size};
    Rectangle render_panel = {
          .x = atlas_panel.x + lvl_panel.width + PADDING,
          .y = PADDING,
          .width = lvl_panel.width,
          .height = 4 * (text_size + PADDING)};
    Rectangle render_width = {
          .x = render_panel.x + PADDING,
          .y = render_panel.y + text_size + PADDING,
          .width = (panel_width - PADDING) * 0.5f,
          .height = text_size};
    Rectangle render_height = {
          .x = render_width.x + render_width.width + PADDING,
          .y = render_width.y,
          .width = render_width.width,
          .height = text_size};
    Rectangle render_dist = {
          .x = render_panel.x + PADDING,
          .y = render_width.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle fov = {
          .x = render_panel.x + PADDING,
          .y = render_dist.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};


    if (state->menu.lvl_dd_edit || state->menu.atlas_dd_edit) GuiLock();

    // LEVEL PANEL
    GuiPanel(lvl_panel, "Level");

    if (GuiButton(lvl_load, "Load")) {
      Level* new_lvl = load_level_file(state->menu.levels.paths[state->menu.lvl_dd_active]);

      if (new_lvl != NULL) {
        change_level(ctx, new_lvl);
        try_change_atlas(state, ctx);
      }
    }

    if (GuiButton(lvl_edit, "Edit")) {
      Level* new_lvl = load_level_file(state->menu.levels.paths[state->menu.lvl_dd_active]);

      if (new_lvl != NULL) {
        change_level(ctx, new_lvl);
        try_change_atlas(state, ctx);

        init_editor(state, ctx, ctx->level);
        state->page = EDITOR;
      }
      
    }

    if (GuiButton(lvl_new, "New")) {
      Level* new_lvl = malloc(sizeof(Level));
      sprintf(new_lvl->name, "new_level");
      sprintf(new_lvl->atlas_name, "");
      new_lvl->width = 5;
      new_lvl->height = 5;
      new_lvl->map = malloc(5 * 5);
      new_lvl->spawn_pos = (Vector2){0};
      new_lvl->spawn_rot = 0;
      new_lvl->floor_color = BLACK;
      new_lvl->ceil_color = WHITE;
      new_lvl->air_color = WHITE;
      memset(new_lvl->map, 0, 5 * 5);

      init_editor(state, ctx, new_lvl);
      state->page = EDITOR;
    }

    state->menu.lvl_path_edit ^= GuiTextBox(lvl_path,
        state->menu.lvl_path, 128, state->menu.lvl_path_edit);

    if (GuiButton(lvl_scan, "Scan")) {
      if (state->menu.levels.paths != NULL) {
        UnloadDirectoryFiles(state->menu.levels);
        string_zero(&state->menu.lvl_dd_entries);
      }
      populate_levels(state);
      printf("Found %d levels in %s\n", state->menu.levels.count, state->menu.lvl_path);
    }

    if (GuiButton(lvl_default, "Default")) {
      strncpy(state->menu.lvl_path, DEFAULT_LEVEL_PATH, sizeof(state->menu.lvl_path));
    }

    // Draw last in level panel
    state->menu.lvl_dd_edit ^= GuiDropdownBox(lvl_dropdown, state->menu.lvl_dd_entries.data, 
          &state->menu.lvl_dd_active, state->menu.lvl_dd_edit);

    // ATLAS PANEL
    GuiPanel(atlas_panel, "Textures");

    if (GuiButton(atlas_load, "Load")) {
      change_atlas(ctx, state->menu.atlases.paths[state->menu.atlas_dd_active]);
    }

    state->menu.atlas_path_edit ^= GuiTextBox(atlas_path,
        state->menu.atlas_path, 128, state->menu.atlas_path_edit);

    if (GuiButton(atlas_scan, "Scan")) {
      if (state->menu.atlases.paths != NULL) {
        UnloadDirectoryFiles(state->menu.atlases);
        string_zero(&state->menu.atlas_dd_entries);
      }
      populate_atlases(state);
      printf("Found %d atlases in %s\n", state->menu.atlases.count, state->menu.atlas_path);
    }

    if (GuiButton(atlas_default, "Default")) {
      strncpy(state->menu.atlas_path, DEFAULT_ATLAS_PATH, sizeof(state->menu.atlas_path));
    }

    // Draw last in atlas panel
    state->menu.atlas_dd_edit ^= GuiDropdownBox(atlas_dropdown, state->menu.atlas_dd_entries.data, 
          &state->menu.atlas_dd_active, state->menu.atlas_dd_edit);

    // RENDER PANEL
    GuiPanel(render_panel, "Render");
    
    int prev_edit;
    prev_edit = state->menu.render_width_edit;
    state->menu.render_width_edit ^= GuiTextBox(render_width, state->menu.render_width, 10, state->menu.render_width_edit);
    if (prev_edit && state->menu.render_width_edit != prev_edit) {
      if (parse_uint(state->menu.render_width, &ctx->render_width, 1, UINT_MAX, ctx->render_width)) {
        UnloadRenderTexture(ctx->bg);
        ctx->bg = LoadRenderTexture(ctx->render_width, ctx->render_height);
        UnloadRenderTexture(ctx->target);
        ctx->target = LoadRenderTexture(ctx->render_width, ctx->render_height);
        generate_bg(ctx);
      }
    }

    prev_edit = state->menu.render_height_edit;
    state->menu.render_height_edit ^= GuiTextBox(render_height, state->menu.render_height, 10, state->menu.render_height_edit);
    if (prev_edit && state->menu.render_height_edit != prev_edit) {
      if (parse_uint(state->menu.render_height, &ctx->render_height, 1, UINT_MAX, ctx->render_height)) {
        UnloadRenderTexture(ctx->bg);
        ctx->bg = LoadRenderTexture(ctx->render_width, ctx->render_height);
        UnloadRenderTexture(ctx->target);
        ctx->target = LoadRenderTexture(ctx->render_width, ctx->render_height);
        generate_bg(ctx);
      }
    }
    
    prev_edit = state->menu.render_dist_edit;
    state->menu.render_dist_edit ^= GuiTextBox(render_dist, state->menu.render_dist, 10, state->menu.render_dist_edit);
    if (prev_edit && state->menu.render_dist_edit != prev_edit) {
      parse_float(state->menu.render_dist, &ctx->render_dist, 1, UINT_MAX, ctx->render_dist);
    }
    
    prev_edit = state->menu.fov_edit;
    state->menu.fov_edit ^= GuiTextBox(fov, state->menu.fov, 10, state->menu.fov_edit);
    if (prev_edit && state->menu.fov_edit != prev_edit) {
      parse_float(state->menu.fov, &ctx->fov, 0.01f, UINT_MAX, ctx->fov);
    }

    bool close_menu = IsKeyPressed(KEY_ESCAPE) || GuiButton(
            (Rectangle){
              .x = width - 100 - PADDING,
              .y = height - text_size - 3 * PADDING,
              .width = 100,
              .height = text_size + 2 * PADDING},
            "Back");
    if (close_menu) { 
      state->page = HUD;
      state->menu.lvl_dd_edit = false;
    }
  }
  else if (state->page == EDITOR) {
    draw_editor(state);

    int panel_width = fmaxf(100, width * 0.25f);
    const int tile_size = (panel_width - PADDING * (TILES_PER_ROW + 1)) / TILES_PER_ROW;

    Rectangle palette = {
          .x = 0,
          .y = 0,
          .width = panel_width,
          .height = height};
    Rectangle options_panel = {
          .x = width - (panel_width + 2 * PADDING),
          .y = 0,
          .width = panel_width + 2 * PADDING,
          .height = height};
    Rectangle lvl_name = {
          .x = options_panel.x + PADDING,
          .y = options_panel.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle lvl_width = {
          .x = options_panel.x + PADDING,
          .y = lvl_name.y + text_size + PADDING,
          .width = (panel_width - PADDING) * 0.5f,
          .height = text_size};
    Rectangle lvl_height = {
          .x = lvl_width.x + lvl_width.width + PADDING,
          .y = lvl_width.y,
          .width = (panel_width - PADDING) * 0.5f,
          .height = text_size};
    Rectangle spawn_x = {
          .x = options_panel.x + PADDING,
          .y = lvl_width.y + text_size + PADDING,
          .width = (panel_width - PADDING) * 0.5f,
          .height = text_size};
    Rectangle spawn_y = {
          .x = spawn_x.x + spawn_x.width + PADDING,
          .y = spawn_x.y,
          .width = (panel_width - PADDING) * 0.5f,
          .height = text_size};
    Rectangle spawn_rot = {
          .x = options_panel.x + PADDING,
          .y = spawn_x.y + text_size + PADDING,
          .width = (panel_width - PADDING) * 0.5f,
          .height = text_size};
    Rectangle floor_color = {
          .x = options_panel.x + PADDING,
          .y = spawn_rot.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle ceil_color = {
          .x = options_panel.x + PADDING,
          .y = floor_color.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle air_color = {
          .x = options_panel.x + PADDING,
          .y = ceil_color.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle atlas_name = {
          .x = options_panel.x + PADDING,
          .y = air_color.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle lvl_exit = {
          .x = options_panel.x + PADDING,
          .y = height - text_size - PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle lvl_save = {
          .x = lvl_name.x,
          .y = lvl_exit.y - text_size - PADDING,
          .width = panel_width,
          .height = text_size};

    // PALETTE
    GuiPanel(palette, "Palette");

    if (GuiButton((Rectangle){PADDING, text_size, tile_size, tile_size}, "")) 
    {
      state->editor.selected_tile = 0;
    }
    DrawRectangle(PADDING, text_size, tile_size, tile_size, BLACK);

    for (int i = 1; i <= state->editor.atlas.width / state->editor.atlas.height; i++) {
      int row_idx = i % TILES_PER_ROW;
      int col_idx = i / TILES_PER_ROW;
      int tile_x = row_idx * (PADDING + tile_size) + PADDING;
      int tile_y = col_idx * (PADDING + tile_size) + text_size + PADDING;
      if (GuiButton((Rectangle){tile_x, tile_y, tile_size, tile_size}, "")) 
      {
        state->editor.selected_tile = i;
      };
      DrawTexturePro(state->editor.atlas, 
        (Rectangle){
          .x = state->editor.atlas.height * (i - 1),
          .y = 0,
          .width = state->editor.atlas.height,
          .height = state->editor.atlas.height}, 
        (Rectangle){
          .x = tile_x,
          .y = tile_y,
          .width = tile_size,
          .height = tile_size},
        (Vector2){0}, 0, WHITE);
    }

    // OPTIONS
    GuiPanel(options_panel, "Options");
    
    int prev_edit = state->editor.lvl_name_edit;
    state->editor.lvl_name_edit ^= GuiTextBox(lvl_name, state->editor.lvl_name, 128, state->editor.lvl_name_edit);
    if (prev_edit && state->editor.lvl_name_edit != prev_edit) {
      verify_filename(state->editor.lvl_name, state->editor.lvl->name, false);
    }

    prev_edit = state->editor.lvl_width_edit;
    state->editor.lvl_width_edit ^= GuiTextBox(lvl_width, state->editor.lvl_width, 10, state->editor.lvl_width_edit);
    if (prev_edit && state->editor.lvl_width_edit != prev_edit) {
      unsigned int result = 0;
      if (parse_uint(state->editor.lvl_width, &result, 0, UINT_MAX, state->editor.lvl->width)) {
        resize_level(state->editor.lvl, result, state->editor.lvl->height);
      }
    }

    prev_edit = state->editor.lvl_height_edit;
    state->editor.lvl_height_edit ^= GuiTextBox(lvl_height, state->editor.lvl_height, 10, state->editor.lvl_height_edit);
    if (prev_edit && state->editor.lvl_height_edit != prev_edit) {
      unsigned int result = 0;
      if (parse_uint(state->editor.lvl_height, &result, 0, UINT_MAX, state->editor.lvl->height)) {
        resize_level(state->editor.lvl, state->editor.lvl->width, result);
      }
    }

    prev_edit = state->editor.spawn_x_edit;
    state->editor.spawn_x_edit ^= GuiTextBox(spawn_x, state->editor.spawn_x, 10, state->editor.spawn_x_edit);
    if (prev_edit && state->editor.spawn_x_edit != prev_edit) {
      parse_float(state->editor.spawn_x, &state->editor.lvl->spawn_pos.x, 0, state->editor.lvl->width, state->editor.lvl->spawn_pos.x);
    }

    prev_edit = state->editor.spawn_y_edit;
    state->editor.spawn_y_edit ^= GuiTextBox(spawn_y, state->editor.spawn_y, 10, state->editor.spawn_y_edit);
    if (prev_edit && state->editor.spawn_y_edit != prev_edit) {
      parse_float(state->editor.spawn_y, &state->editor.lvl->spawn_pos.y, 0, state->editor.lvl->height, state->editor.lvl->spawn_pos.y);
    }

    prev_edit = state->editor.spawn_rot_edit;
    state->editor.spawn_rot_edit ^= GuiTextBox(spawn_rot, state->editor.spawn_rot, 10, state->editor.spawn_rot_edit);
    if (prev_edit && state->editor.spawn_rot_edit != prev_edit) {
      parse_float(state->editor.spawn_rot, &state->editor.lvl->spawn_rot, 0, 360, state->editor.lvl->spawn_rot);
    }

    prev_edit = state->editor.floor_color_edit;
    state->editor.floor_color_edit ^= GuiTextBox(floor_color, state->editor.floor_color, 10, state->editor.floor_color_edit);
    if (prev_edit && state->editor.floor_color_edit != prev_edit) {
      parse_color(state->editor.floor_color, &state->editor.lvl->floor_color, state->editor.lvl->floor_color);
    }

    prev_edit = state->editor.ceil_color_edit;
    state->editor.ceil_color_edit ^= GuiTextBox(ceil_color, state->editor.ceil_color, 10, state->editor.ceil_color_edit);
    if (prev_edit && state->editor.ceil_color_edit != prev_edit) {
      parse_color(state->editor.ceil_color, &state->editor.lvl->ceil_color, state->editor.lvl->ceil_color);
    }

    prev_edit = state->editor.air_color_edit;
    state->editor.air_color_edit ^= GuiTextBox(air_color, state->editor.air_color, 10, state->editor.air_color_edit);
    if (prev_edit && state->editor.air_color_edit != prev_edit) {
      parse_color(state->editor.air_color, &state->editor.lvl->air_color, state->editor.lvl->air_color);
    }

    prev_edit = state->editor.atlas_name_edit;
    state->editor.atlas_name_edit ^= GuiTextBox(atlas_name, state->editor.atlas_name, 128, state->editor.atlas_name_edit);
    if (prev_edit && state->editor.atlas_name_edit != prev_edit) {
      verify_filename(state->editor.atlas_name, state->editor.lvl->atlas_name, true);
    }

    if (GuiButton(lvl_save, "Save")) {
      save_level_file(state->editor.lvl, state->menu.lvl_path);
    }

    if (GuiButton(lvl_exit, "Exit")) {
      unload_editor(state);
      state->page = MENU;

      if (state->editor.lvl == ctx->level) {
        Level* new_lvl = load_level_file(state->menu.levels.paths[state->menu.lvl_dd_active]);
        if (new_lvl != NULL) change_level(ctx, new_lvl);
      }
      else {
        unload_level(state->editor.lvl);
      }

      if (state->menu.levels.paths != NULL) {
        UnloadDirectoryFiles(state->menu.levels);
        string_zero(&state->menu.lvl_dd_entries);
      }
      populate_levels(state);
      try_change_atlas(state, ctx);
    }
  }

  GuiUnlock();
}


void unload_ui(UIState *state) {
  if (state->menu.levels.paths != NULL) {
    UnloadDirectoryFiles(state->menu.levels);
  }
  if (state->menu.atlases.paths != NULL) {
    UnloadDirectoryFiles(state->menu.atlases);
  }
  free(state->menu.lvl_dd_entries.data);
  free(state->menu.atlas_dd_entries.data);
}


void init_editor(UIState *state, const RenderContext *ctx, Level *lvl) {
  state->editor.atlas = LoadTextureFromImage(ctx->atlas);
  state->editor.zoom = 1;
  state->editor.lvl = lvl;
  strcpy(state->editor.lvl_name, lvl->name);
  snprintf(state->editor.lvl_width, 10, "%d", lvl->width);
  snprintf(state->editor.lvl_height, 10, "%d", lvl->height);
  snprintf(state->editor.spawn_x, 10, "%.2f", lvl->spawn_pos.x);
  snprintf(state->editor.spawn_y, 10, "%.2f", lvl->spawn_pos.y);
  snprintf(state->editor.spawn_rot, 10, "%.2f", lvl->spawn_rot);
  snprintf(state->editor.floor_color, 10, "%08X", ColorToInt(lvl->floor_color));
  snprintf(state->editor.ceil_color, 10, "%08X", ColorToInt(lvl->ceil_color));
  snprintf(state->editor.air_color, 10, "%08X", ColorToInt(lvl->air_color));
  strcpy(state->editor.atlas_name, lvl->atlas_name);
}


void draw_editor(UIState *state) {
  int width = GetScreenWidth();
  int height = GetScreenHeight();
  float height_ratio = height / (width * 0.5f);
  state->editor.zoom = fmaxf(state->editor.zoom + GetMouseWheelMove() * 0.2f, 0.02f);
  double inverse_zoom = 1 / state->editor.zoom;
  int tile_size = width * 0.5f / VISIBLE_TILES;
  int tile_size_scaled = tile_size * inverse_zoom;

  Vector2 offset_delta = {0};
  if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    offset_delta = Vector2Scale(GetMouseDelta(), -1);
  }
  else {
    offset_delta = Vector2Scale(
      (Vector2){
        IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT),
        IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)},
       SCROLL_RATE);
  }

  state->editor.map_offset = Vector2Add(state->editor.map_offset, Vector2Scale(offset_delta, 1.0f / tile_size_scaled));
  state->editor.map_offset.x = Wrap(state->editor.map_offset.x, 0, state->editor.lvl->width);
  state->editor.map_offset.y = Wrap(state->editor.map_offset.y, 0, state->editor.lvl->height);

  for (int grid_y = 0; grid_y < floorf(VISIBLE_TILES * state->editor.zoom * height_ratio) + 1; grid_y++) {
    for (int grid_x = 0; grid_x < VISIBLE_TILES * state->editor.zoom + 1; grid_x++) {
      int map_idx = (int)Wrap(grid_y + state->editor.map_offset.y, 0, state->editor.lvl->height) * state->editor.lvl->width + 
        (int)Wrap(grid_x + state->editor.map_offset.x, 0, state->editor.lvl->width);
      int tile_x = width * 0.25f + grid_x * tile_size_scaled;
      int tile_y = grid_y * tile_size_scaled;
      int right_bound = tile_x + tile_size_scaled;
      if (right_bound > width * 0.75f) right_bound = width * 0.75;
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
          GetMouseX() >= tile_x && GetMouseX() < right_bound &&
          GetMouseY() >= tile_y && GetMouseY() < tile_y + tile_size_scaled) {
        if (state->editor.lvl->map[map_idx] != state->editor.selected_tile) {
          state->editor.lvl->map[map_idx] = state->editor.selected_tile;
        }
      }

      if (state->editor.lvl->map[map_idx] == 0) continue;

      DrawTexturePro(state->editor.atlas, 
          (Rectangle){
            .x = state->editor.atlas.height * (state->editor.lvl->map[map_idx] - 1), 
            .y = 0,
            .width = state->editor.atlas.height,
            .height = state->editor.atlas.height}, 
          (Rectangle){
            .x = tile_x,
            .y = tile_y,
            .width = tile_size_scaled,
            .height = tile_size_scaled},
          (Vector2){0}, 0, WHITE);
    }
    if (((int)state->editor.map_offset.y + grid_y) % state->editor.lvl->height == 0) {
      float line_y = grid_y * tile_size_scaled;
      DrawLine(0, line_y, width, line_y, LIGHTGRAY);
    }
  }
  for (int x = 0; x <= ceilf((float)VISIBLE_TILES * state->editor.zoom / state->editor.lvl->width); x++) {
    float line_x = 0.25f * width + (x * state->editor.lvl->width - (int)state->editor.map_offset.x) * tile_size_scaled;
    DrawLine(line_x, 0, line_x, height, LIGHTGRAY);
    
    for (int y = 0; y <= ceilf((float)VISIBLE_TILES * state->editor.zoom); y++) {
      DrawCircleLines(
        line_x - (state->editor.lvl->width - state->editor.lvl->spawn_pos.x) * tile_size_scaled, 
        (y * state->editor.lvl->height - (int)state->editor.map_offset.y - (state->editor.lvl->spawn_pos.y)) * tile_size_scaled,
        tile_size_scaled * 0.2f,
        RED);
    }
  }
}


void unload_editor(UIState *state) {
  UnloadTexture(state->editor.atlas);
}
