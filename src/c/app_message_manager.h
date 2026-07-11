#pragma once

#include <pebble.h>

// Commands
typedef enum {
  CMD_MENU = 0,
  CMD_MENU_SELECT,
  CMD_SUCCESS,
  CMD_ERROR,
  CMD_BACK,
  CMD_MENU_REFRESH,
} MessageCommand;

typedef enum {
  MENU_NONE = -1,
  MENU_MAIN = 0,
  MENU_TASKS,
  MENU_ACTIONS,
  MENU_HOUSEHOLDS,
  MENU_HANDLE_TASK,
  MENU_HANDLE_ACTION,
  MENU_HANDLE_HOUSEHOLD,
} MessageCurrentMenu;

// Callback when a message is received.
typedef void (*AppMessageCallback)(
    MessageCommand command,
    MessageCurrentMenu menu,
    Tuple *data);

void app_message_manager_init(AppMessageCallback callback);
void app_message_manager_deinit(void);

AppMessageResult app_message_send_string(
    MessageCommand command,
    MessageCurrentMenu current_menu,
    const char *string);

AppMessageResult app_message_send_uint8(
    MessageCommand command,
    MessageCurrentMenu current_menu,
    uint8_t value);

AppMessageResult app_message_send_int32(
    MessageCommand command,
    MessageCurrentMenu current_menu,
    int32_t value);
