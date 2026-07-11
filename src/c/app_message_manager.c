#include <pebble.h>
#include "app_message_manager.h"

static AppMessageCallback s_callback = NULL;

static void inbox_received(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received");
  Tuple *cmd = dict_find(iter, MESSAGE_KEY_cmd);

  if (!cmd || !s_callback) {
    return;
  }

  Tuple *menu = dict_find(iter, MESSAGE_KEY_cur_menu);
  Tuple *data = dict_find(iter, MESSAGE_KEY_data);
  s_callback(
    (MessageCommand)cmd->value->uint8,
    (MessageCurrentMenu)menu->value->uint8,
    data
  );
}

static void inbox_dropped(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_WARNING,
          "Inbox dropped: %d",
          reason);
}

static void outbox_sent(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message sent");
}

static void outbox_failed(DictionaryIterator *iter,
                          AppMessageResult reason,
                          void *context) {
  APP_LOG(APP_LOG_LEVEL_WARNING,
          "Outbox failed: %d",
          reason);
}

void app_message_manager_init(AppMessageCallback callback) {
  s_callback = callback;

  app_message_register_inbox_received(inbox_received);
  app_message_register_inbox_dropped(inbox_dropped);
  app_message_register_outbox_sent(outbox_sent);
  app_message_register_outbox_failed(outbox_failed);

  app_message_open(512, 512);
}

void app_message_manager_deinit(void) {
}

AppMessageResult app_message_send_string(
    MessageCommand command,
    MessageCurrentMenu current_menu,
    const char *string) {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);

  if (result != APP_MSG_OK)
    return result;

  dict_write_uint8(iter, MESSAGE_KEY_cmd, command);
  dict_write_cstring(iter, MESSAGE_KEY_data, string);
  dict_write_uint8(iter, MESSAGE_KEY_cur_menu, current_menu);

  dict_write_end(iter);

  return app_message_outbox_send();
}

AppMessageResult app_message_send_uint8(
    MessageCommand command,
    MessageCurrentMenu current_menu,
    uint8_t value) {

  DictionaryIterator *iter;

  AppMessageResult result = app_message_outbox_begin(&iter);

  if (result != APP_MSG_OK)
    return result;

  dict_write_uint8(iter, MESSAGE_KEY_cmd, command);
  dict_write_uint8(iter, MESSAGE_KEY_data, value);
  dict_write_uint8(iter, MESSAGE_KEY_cur_menu, current_menu);

  dict_write_end(iter);

  return app_message_outbox_send();
}

AppMessageResult app_message_send_int32(
    MessageCommand command,
    MessageCurrentMenu current_menu,
    int32_t value) {

  DictionaryIterator *iter;

  AppMessageResult result = app_message_outbox_begin(&iter);

  if (result != APP_MSG_OK)
    return result;

  dict_write_uint8(iter, MESSAGE_KEY_cmd, command);
  dict_write_int32(iter, MESSAGE_KEY_data, value);
  dict_write_uint8(iter, MESSAGE_KEY_cur_menu, current_menu);

  dict_write_end(iter);

  return app_message_outbox_send();
}
