// Copyright 2026 meatcar (@meatcar)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "config.h"
#include "keycodes.h"

#define PROGMEM
#define MATRIX_ROWS 1
#define MATRIX_COLS 6
#define LAYOUT(a, b, c, d, e, f) {{e, a, b, c, d, f}}
#define pgm_read_word(p) (*(p))
#define TAP_CODE_DELAY 0
#define TAPPING_TERM 200
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

typedef uint32_t layer_state_t;
typedef struct { uint8_t row, col; } keypos_t;
typedef struct { keypos_t key; bool pressed; uint16_t time; uint8_t type; } keyevent_t;
typedef struct { keyevent_t event; uint16_t keycode; } keyrecord_t;
typedef struct { uint8_t buttons; int8_t x, y, v, h; } report_mouse_t;
enum { TICK_EVENT, KEY_EVENT, COMBO_EVENT };
#define MAKE_KEYPOS(r, c) ((keypos_t){r, c})
#define MAKE_COMBOEVENT(p) ((keyevent_t){.pressed = p, .type = COMBO_EVENT})
#define IS_NOEVENT(e) ((e).type == TICK_EVENT)
#define IS_KEYEVENT(e) ((e).type == KEY_EVENT)

#include "process_combo.h"
#include "via.h"
#include "ploopyco.h"

extern layer_state_t layer_state, default_layer_state;
extern const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS];
extern combo_t key_combos[];
uint16_t timer_read(void);
uint16_t timer_elapsed(uint16_t start);
uint32_t timer_read32(void);
uint32_t timer_elapsed32(uint32_t start);
void layer_invert(uint8_t layer);
void layer_clear(void);
void default_layer_set(layer_state_t state);
void pointing_device_set_cpi(uint16_t cpi);
bool eeconfig_is_user_datablock_valid(void);
uint32_t eeconfig_read_user_datablock(void *data, uint32_t offset, uint32_t length);
uint32_t eeconfig_update_user_datablock(const void *data, uint32_t offset, uint32_t length);
void eeconfig_update_kb(uint32_t value);
void clear_weak_mods(void);
uint16_t combo_count(void);
combo_t *combo_get(uint16_t index);
uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key);
void process_record(keyrecord_t *record);
bool pre_process_record_user(uint16_t keycode, keyrecord_t *record);
bool process_record_user(uint16_t keycode, keyrecord_t *record);
report_mouse_t pointing_device_task_user(report_mouse_t report);
void keyboard_post_init_user(void);
void eeconfig_init_user(void);
void housekeeping_task_user(void);
layer_state_t layer_state_set_user(layer_state_t state);
void via_custom_value_command_kb(uint8_t *data, uint8_t length);
