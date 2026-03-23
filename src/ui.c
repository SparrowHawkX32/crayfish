#include "ui.h"
#include "level.h"
#include "util.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "render.h"
#include <raylib.h>
#include <stdio.h>


void init_ui(UIState *state) {
  int text_size = GetMonitorWidth(GetCurrentMonitor()) > 1920 ? 32 : 24;
  GuiSetStyle(DEFAULT, TEXT_SIZE, text_size);
  //GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

  // Scan + populate levels
  strncpy(state->menu.lvl_path, DEFAULT_LEVEL_PATH, sizeof(state->menu.lvl_path));
  state->menu.levels = LoadDirectoryFilesEx(state->menu.lvl_path, ".lvl", true);
  string_init(&state->menu.lvl_dd_entries);
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


void draw_ui(UIState* state, RenderContext* ctx) {
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
              .x = GetScreenWidth() * 0.5f - 8 * text_size,
              .y = GetScreenHeight() * 0.5f - text_size * 1.5f,
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
    int panel_width = fmaxf(100, GetScreenWidth() * 0.3f);
    Rectangle lvl_panel = {
          .x = PADDING,
          .y = PADDING,
          .width = panel_width + 2 * PADDING,
          .height = 5 * (text_size + PADDING)};
    Rectangle lvl_dropdown = {
          .x = lvl_panel.x + PADDING,
          .y = lvl_panel.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle lvl_load = {
          .x = lvl_dropdown.x,
          .y = lvl_dropdown.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle lvl_path = {
          .x = lvl_dropdown.x,
          .y = lvl_load.y + text_size + PADDING,
          .width = panel_width,
          .height = text_size};
    Rectangle lvl_scan = {
          .x = lvl_dropdown.x,
          .y = lvl_path.y + text_size + PADDING,
          .width = (panel_width - PADDING) * 0.5f,
          .height = text_size};
    Rectangle lvl_default = {
          .x = lvl_scan.x + lvl_scan.width + PADDING,
          .y = lvl_path.y + text_size + PADDING,
          .width = lvl_scan.width,
          .height = text_size};

    if (state->menu.lvl_dd_edit) GuiLock();

    GuiPanel(lvl_panel, "Level");

    if (GuiButton(lvl_load, "Load")) {
      Level* new_lvl = load_level_file(state->menu.levels.paths[state->menu.lvl_dd_active]);

      if (new_lvl != NULL) {
        change_level(ctx, new_lvl);
      }
    }

    state->menu.lvl_path_edit ^= GuiTextBox(lvl_path,
        state->menu.lvl_path, 128, state->menu.lvl_path_edit);

    if (GuiButton(lvl_scan, "Scan")) {
      if (state->menu.levels.paths != NULL) {
        UnloadDirectoryFiles(state->menu.levels);
      }
      state->menu.levels = LoadDirectoryFilesEx(state->menu.lvl_path, ".lvl", true);
      printf("Found %d levels in %s - %d\n", state->menu.levels.count, state->menu.lvl_path, state->menu.levels.capacity);
    }

    if (GuiButton(lvl_default, "Default")) {
      strncpy(state->menu.lvl_path, DEFAULT_LEVEL_PATH, sizeof(state->menu.lvl_path));
    }

    // DRAW LAST IN LEVEL PANEL
    state->menu.lvl_dd_edit ^= GuiDropdownBox(lvl_dropdown, state->menu.lvl_dd_entries.data, 
          &state->menu.lvl_dd_active, state->menu.lvl_dd_edit);

    GuiPanel((Rectangle){
          .x = panel_width + 4 * PADDING,
          .y = PADDING,
          .width = panel_width + 2 * PADDING,
          .height = text_size}, 
        "Camera");

    bool close_menu = IsKeyPressed(KEY_ESCAPE) || GuiButton(
            (Rectangle){
              .x = GetScreenWidth() - 100 - PADDING,
              .y = GetScreenHeight() - text_size - 3 * PADDING,
              .width = 100,
              .height = text_size + 2 * PADDING},
            "Back");
    if (close_menu) { 
      state->page = HUD;
      state->menu.lvl_dd_edit = false;
    }
  }
  else if (state->page == EDITOR) {
  }

  GuiUnlock();
}


void unload_ui(UIState *state) {
  if (state->menu.levels.paths != NULL) {
    UnloadDirectoryFiles(state->menu.levels);
  }
  free(state->menu.lvl_dd_entries.data);
}
