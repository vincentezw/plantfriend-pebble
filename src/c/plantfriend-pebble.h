#pragma once

#define MAX_MENU_ITEMS 7

typedef struct {
  GBitmap *normal;
  GBitmap *selected;
} MenuIcon;

typedef struct {
  char id[32];
  char title[48];
  char subtitle[64];
  MenuIcon icon;
} MenuItem;

typedef struct {
  MenuItem items[MAX_MENU_ITEMS];
  int count;
} Menu;

static GBitmap *s_icon_actions;
static GBitmap *s_icon_actions_selected;
static GBitmap *s_icon_households;
static GBitmap *s_icon_households_selected;
static GBitmap *s_icon_tasks;
static GBitmap *s_icon_tasks_selected;
static GBitmap *s_icon_water;
static GBitmap *s_icon_water_selected;
static GBitmap *s_icon_fertilise;
static GBitmap *s_icon_fertilise_selected;
static GBitmap *s_icon_clean;
static GBitmap *s_icon_clean_selected;
static GBitmap *s_icon_mist;
static GBitmap *s_icon_mist_selected;
static GBitmap *s_icon_next;
static GBitmap *s_icon_next_selected;
static GBitmap *s_icon_prev;
static GBitmap *s_icon_prev_selected;
static GBitmap *s_icon_done;
static GBitmap *s_icon_done_selected;
static GBitmap *s_icon_snooze;
static GBitmap *s_icon_snooze_selected;
static GBitmap *s_icon_undo;
static GBitmap *s_icon_undo_selected;
