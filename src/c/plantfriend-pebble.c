#include <pebble.h>
#include "plantfriend-pebble.h"
#include "app_message_manager.h"
#define MAX_MENU_DEPTH 5

typedef struct {
  Window *window;
  MenuLayer *menu_layer;
  TextLayer *text_layer;
  Menu menu;
  bool loaded;
  MessageCurrentMenu menu_type;
} MenuWindow;
static MenuWindow *s_menu_stack[MAX_MENU_DEPTH];
static int s_menu_depth = 0;
static MenuWindow *current_menu(void);
static void pop_menu(void *context);

static Menu parse_menu(const char *data);
static void render_menu(void);

static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);
static void prv_window_appear(Window *window);
static uint16_t menu_get_num_rows(
  MenuLayer *menu_layer,
  uint16_t section_index,
  void *context
);
static void menu_draw_row(
  GContext *ctx,
  const Layer *cell_layer,
  MenuIndex *cell_index,
  void *context
);
static void menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context);
static void show_message(MessageCurrentMenu type, const char *message, bool auto_hide);
static void hide_message();
static MenuIcon get_icon(uint8_t icon);

static MenuWindow *create_menu_window(void) {
  MenuWindow *ctx = malloc(sizeof(MenuWindow));
  ctx->menu = (Menu){0};
  ctx->loaded = false;

  ctx->window = window_create();
  window_set_user_data(ctx->window, ctx);
  window_set_window_handlers(
    ctx->window,
    (WindowHandlers) {
      .load = prv_window_load,
      .unload = prv_window_unload,
      .appear = prv_window_appear,
    }
  );
  return ctx;
}

static void message_received(MessageCommand command, MessageCurrentMenu menu_type, Tuple *data) {
  switch (command) {
    case CMD_MENU:
      MenuWindow *menu = current_menu();
      menu->menu = parse_menu(data->value->cstring);
      menu->menu_type = menu_type;
      render_menu();
      break;

    case CMD_SUCCESS:
      show_message(menu_type, "🙂 Done!", false);
      app_timer_register( 1000, pop_menu, NULL);
      break;

    case CMD_ERROR:
      show_message(menu_type, data->value->cstring, false);
      break;

    default:
      break;
  }
}

static MenuWindow *current_menu(void) {
  if (s_menu_depth == 0) {
    return NULL;
  }

  return s_menu_stack[s_menu_depth - 1];
}

static const char *parse_field(char *dest, size_t dest_size, const char *p) {
  size_t i = 0;
  while (*p && *p != '|' && i < dest_size - 1) {
    dest[i++] = *p++;
  }

  dest[i] = '\0';
  while (*p && *p != '|') {
    p++;
  }

  return p;
}


static Menu parse_menu(const char *data) {
  Menu menu = {0};
  const char *p = data;

  while (*p && menu.count < MAX_MENU_ITEMS) {
    MenuItem *item = &menu.items[menu.count];

    // id
    p = parse_field(item->id, sizeof(item->id), p);
    if (*p != '|') {
      break;
    }
    p++;

    // title
    p = parse_field(item->title, sizeof(item->title), p);
    if (*p != '|') {
      break;
    }
    p++;

    // subtitle
    p = parse_field(item->subtitle, sizeof(item->subtitle), p);
    if (*p != '|') {
      break;
    }
    p++;

    // icon
    item->icon = get_icon(atoi(p));

    // Skip to next line
    while (*p && *p != '\n') {
      p++;
    }

    if (*p == '\n') {
      p++;
    }

    menu.count++;
  }

  return menu;
}

static void render_menu(void) {
  MenuWindow *menu = current_menu();

  if (menu->menu.count == 0) {
    text_layer_set_text(menu->text_layer, "Nothing here.");
    layer_set_hidden(text_layer_get_layer(menu->text_layer), false);
    layer_set_hidden(menu_layer_get_layer(menu->menu_layer), true);
    menu->loaded = true;
    return;
  }

  if (!menu->loaded) {
    current_menu()->loaded = true;
    layer_set_hidden(text_layer_get_layer(current_menu()->text_layer), true);
    layer_set_hidden(menu_layer_get_layer(current_menu()->menu_layer), false);
  }

  menu_layer_reload_data(menu->menu_layer);
  menu_layer_set_selected_index(
    menu->menu_layer,
    (MenuIndex) {
      .section = 0,
      .row = 0
    },
    MenuRowAlignCenter,
    false
  );
}

static const char *menu_title(MessageCurrentMenu type) {
  switch (type) {
    case MENU_MAIN:             return "Plant friend";
    case MENU_HOUSEHOLDS:       return "Households";
    case MENU_TASKS:            return "Tasks";
    case MENU_ACTIONS:          return "Actions";
    case MENU_HANDLE_TASK:      return "Handle task";
    case MENU_HANDLE_ACTION:    return "Modify action";
    default:                    return "";
  }
}

static MenuIcon get_icon(uint8_t icon) {
  switch (icon) {
    case 1:
      return (MenuIcon) {
        .normal = s_icon_tasks,
        .selected = s_icon_tasks_selected
      };
    case 2:
      return (MenuIcon) {
        .normal = s_icon_actions,
        .selected = s_icon_actions_selected
      };
    case 3:
      return (MenuIcon) {
        .normal = s_icon_households,
        .selected = s_icon_households_selected
      };
    case 4:
      return (MenuIcon) {
        .normal = s_icon_water,
        .selected = s_icon_water_selected
      };
    case 5:
      return (MenuIcon) {
        .normal = s_icon_fertilise,
        .selected = s_icon_fertilise_selected
      };
    case 6:
      return (MenuIcon) {
        .normal = s_icon_clean,
        .selected = s_icon_clean_selected
      };
    case 7:
      return (MenuIcon) {
        .normal = s_icon_mist,
        .selected = s_icon_mist_selected
      };
    case 8:
      return (MenuIcon) {
        .normal = s_icon_next,
        .selected = s_icon_next_selected
      };
    case 9:
      return (MenuIcon) {
        .normal = s_icon_prev,
        .selected = s_icon_prev_selected
      };
    case 10:
      return (MenuIcon) {
        .normal = s_icon_done,
        .selected = s_icon_done_selected
      };
    case 11:
      return (MenuIcon) {
        .normal = s_icon_snooze,
        .selected = s_icon_snooze_selected
      };
    case 12:
      return (MenuIcon) {
        .normal = s_icon_undo,
        .selected = s_icon_undo_selected
      };
        
    default:
      return (MenuIcon) {
        .normal = NULL,
        .selected = NULL
      };
  }
}

static int16_t menu_get_header_height(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header(
  GContext *ctx,
  const Layer *cell_layer,
  uint16_t section_index,
  void *context
) {
  MenuWindow *menu = context;
  menu_cell_basic_header_draw(ctx, cell_layer, menu_title(menu->menu_type));
}

static uint16_t menu_get_num_rows(
  MenuLayer *menu_layer,
  uint16_t section_index,
  void *context
) {
  MenuWindow *ctx = context;
  return ctx->menu.count;
}

static int16_t menu_get_cell_height(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  MenuWindow *menu_window = context;
  MenuItem *item = &menu_window->menu.items[cell_index->row];
  
  return (strlen(item->subtitle) > 0) ? 36 : 30;
}

static void menu_draw_row(
  GContext *ctx,
  const Layer *cell_layer,
  MenuIndex *cell_index,
  void *context
) {
  MenuWindow *menu_window = context;
  MenuItem *item = &menu_window->menu.items[cell_index->row];
  GRect bounds = layer_get_bounds(cell_layer);

  bool is_highlighted = menu_cell_layer_is_highlighted(cell_layer);
  GBitmap *icon = is_highlighted ? item->icon.selected : item->icon.normal;
  GColor text_color = is_highlighted ? GColorWhite : GColorBlack;

  int16_t x_offset = 6;
  if (icon) {
    gpath_draw_outline;
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, icon, GRect(6, (bounds.size.h - 28) / 2, 28, 28));
    x_offset = 40;
  }

  bool has_subtitle = (strlen(item->subtitle) > 0);
  int16_t title_y = has_subtitle ? -2 : ((bounds.size.h - 18) / 2) - 2;
  graphics_context_set_text_color(ctx, text_color);

  graphics_draw_text(
    ctx,
    item->title,
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
    GRect(x_offset, title_y, bounds.size.w - x_offset - 4, 20),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  if (has_subtitle) {
    graphics_draw_text(
      ctx,
      item->subtitle,
      fonts_get_system_font(FONT_KEY_GOTHIC_14),
      GRect(x_offset, title_y + 17, bounds.size.w - x_offset - 4, 16),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentLeft,
      NULL
    );
  }
}

static void menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  MenuWindow *menu_window = context;
  MenuItem *item = &menu_window->menu.items[cell_index->row];
  if ((menu_window->menu_type != MENU_HANDLE_TASK &&
     menu_window->menu_type != MENU_HANDLE_ACTION &&
     menu_window->menu_type != MENU_HOUSEHOLDS) &&
    strcmp(item->id, "next") != 0 &&
    strcmp(item->id, "prev") != 0
  ) {
    MenuWindow *child = create_menu_window();
    s_menu_stack[s_menu_depth++] = child;
    window_stack_push(child->window, true);
  }
  app_message_send_string(CMD_MENU_SELECT, menu_window->menu_type, item->id);
}

static MenuWindow *find_menu(MessageCurrentMenu type) {
  for (int i = 0; i < s_menu_depth; i++) {
    if (s_menu_stack[i]->menu_type == type) {
      return s_menu_stack[i];
    }
  }

  return NULL;
}

static void pop_menu(void *context) {
  if (s_menu_depth > 1) {
    window_stack_pop(true);
  }
}

static void show_message(MessageCurrentMenu menu_type, const char *message, bool auto_hide) {
  MenuWindow *menu = find_menu(menu_type);
  if (!menu) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not find menu %d", menu_type);
    return;
  }

  text_layer_set_text(menu->text_layer, message);
  layer_set_hidden(text_layer_get_layer(menu->text_layer), false);
  layer_set_hidden(menu_layer_get_layer(menu->menu_layer), true);

  if (auto_hide) {
    app_timer_register(
      2000,
      hide_message,
      menu
    );
  }
}

static void hide_message(void *context) {
  MenuWindow *menu = context;
  layer_set_hidden(text_layer_get_layer(menu->text_layer), true);
  layer_set_hidden(menu_layer_get_layer(menu->menu_layer), false);
}

static void prv_window_load(Window *window) {
  MenuWindow *ctx = window_get_user_data(window);
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  ctx->text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 80));
  text_layer_set_font(ctx->text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));

  text_layer_set_text(ctx->text_layer, "Loading...");
  text_layer_set_text_alignment(ctx->text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(ctx->text_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(ctx->text_layer));

  ctx->menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(
    ctx->menu_layer,
    ctx,
    (MenuLayerCallbacks) {
      .get_num_rows = menu_get_num_rows,
      .draw_row = menu_draw_row,
      .select_click = menu_select,
      .get_header_height = menu_get_header_height,
      .draw_header = menu_draw_header,
      .get_cell_height = menu_get_cell_height,
    }
  );
  menu_layer_set_click_config_onto_window(ctx->menu_layer, window);

  layer_set_hidden(menu_layer_get_layer(ctx->menu_layer), true);
  layer_add_child(window_layer, menu_layer_get_layer(ctx->menu_layer));
}

static void prv_window_appear(Window *window) {
  MenuWindow *ctx = window_get_user_data(window);
  if (!ctx->loaded) {
    return;
  }

  if (
    ctx->menu_type == MENU_MAIN ||
    ctx->menu_type == MENU_TASKS ||
    ctx->menu_type == MENU_ACTIONS
  ) {
    ctx->loaded = false;
    text_layer_set_text(ctx->text_layer, "Loading...");
    layer_set_hidden(text_layer_get_layer(ctx->text_layer), false);
    layer_set_hidden(menu_layer_get_layer(ctx->menu_layer), true);
    app_message_send_string(CMD_MENU_REFRESH, ctx->menu_type, "");
  }
}

static void prv_window_unload(Window *window) {
  MenuWindow *ctx = window_get_user_data(window);
  menu_layer_destroy(ctx->menu_layer);
  text_layer_destroy(ctx->text_layer);
  window_destroy(ctx->window);

  for (int i = 0; i < s_menu_depth; i++) {
    if (s_menu_stack[i] == ctx) {
      while (i < s_menu_depth - 1) {
        s_menu_stack[i] = s_menu_stack[i + 1];
        i++;
      }
      s_menu_depth--;
      break;
    }
  }

  free(ctx);
}

static void prv_init(void) {
  MenuWindow *menu = create_menu_window();
  s_menu_stack[s_menu_depth++] = menu;
  window_stack_push(menu->window, true);
  app_message_manager_init(message_received);

  s_icon_actions = gbitmap_create_with_resource(RESOURCE_ID_ICON_ACTIONS);
  s_icon_actions_selected = gbitmap_create_with_resource(RESOURCE_ID_ICON_ACTIONS_SELECTED);
  s_icon_households = gbitmap_create_with_resource(RESOURCE_ID_ICON_HOUSEHOLDS);
  s_icon_households_selected = gbitmap_create_with_resource(RESOURCE_ID_ICON_HOUSEHOLDS_SELECTED);
  s_icon_tasks = gbitmap_create_with_resource(RESOURCE_ID_ICON_TASKS);
  s_icon_tasks_selected = gbitmap_create_with_resource(RESOURCE_ID_ICON_TASKS_SELECTED);

  s_icon_water = gbitmap_create_with_resource(RESOURCE_ID_ICON_WATER);
  s_icon_water_selected = gbitmap_create_with_resource(RESOURCE_ID_ICON_WATER_SELECTED);
  s_icon_fertilise = gbitmap_create_with_resource(RESOURCE_ID_ICON_FERTILISE);
  s_icon_fertilise_selected = gbitmap_create_with_resource(RESOURCE_ID_ICON_FERTILISE_SELECTED);
  s_icon_clean = gbitmap_create_with_resource(RESOURCE_ID_ICON_CLEAN);
  s_icon_clean_selected = gbitmap_create_with_resource(RESOURCE_ID_ICON_CLEAN_SELECTED);
  s_icon_mist = gbitmap_create_with_resource(RESOURCE_ID_ICON_MIST);
  s_icon_mist_selected = gbitmap_create_with_resource(RESOURCE_ID_ICON_MIST_SELECTED);

  s_icon_next = gbitmap_create_with_resource(RESOURCE_ID_ICON_NEXT);
  s_icon_next_selected = gbitmap_create_with_resource(RESOURCE_ID_ICON_NEXT_SELECTED);
  s_icon_prev = gbitmap_create_with_resource(RESOURCE_ID_ICON_PREVIOUS);
  s_icon_prev_selected = gbitmap_create_with_resource(RESOURCE_ID_ICON_PREVIOUS_SELECTED);

  s_icon_done = gbitmap_create_with_resource(RESOURCE_ID_ICON_DONE);
  s_icon_done_selected = gbitmap_create_with_resource(RESOURCE_ID_ICON_DONE_SELECTED);
  s_icon_snooze = gbitmap_create_with_resource(RESOURCE_ID_ICON_SNOOZE);
  s_icon_snooze_selected = gbitmap_create_with_resource(RESOURCE_ID_ICON_SNOOZE_SELECTED);
  s_icon_undo = gbitmap_create_with_resource(RESOURCE_ID_ICON_UNDO);
  s_icon_undo_selected = gbitmap_create_with_resource(RESOURCE_ID_ICON_UNDO_SELECTED);
}

static void prv_deinit(void) {
  gbitmap_destroy(s_icon_actions);
  gbitmap_destroy(s_icon_actions_selected);
  gbitmap_destroy(s_icon_households);
  gbitmap_destroy(s_icon_households_selected);
  gbitmap_destroy(s_icon_tasks);
  gbitmap_destroy(s_icon_tasks_selected);
  gbitmap_destroy(s_icon_water);
  gbitmap_destroy(s_icon_water_selected);
  gbitmap_destroy(s_icon_fertilise);
  gbitmap_destroy(s_icon_fertilise_selected);
  gbitmap_destroy(s_icon_clean);
  gbitmap_destroy(s_icon_clean_selected);
  gbitmap_destroy(s_icon_mist);
  gbitmap_destroy(s_icon_mist_selected);
  gbitmap_destroy(s_icon_next);
  gbitmap_destroy(s_icon_next_selected);
  gbitmap_destroy(s_icon_prev);
  gbitmap_destroy(s_icon_prev_selected);
  gbitmap_destroy(s_icon_done);
  gbitmap_destroy(s_icon_done_selected);
  gbitmap_destroy(s_icon_snooze);
  gbitmap_destroy(s_icon_snooze_selected);
  gbitmap_destroy(s_icon_undo);
  gbitmap_destroy(s_icon_undo_selected);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
