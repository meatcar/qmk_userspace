// Copyright 2026 meatcar (@meatcar)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qmk_stub.h"
#include <assert.h>
#include <string.h>

uint32_t now;
uint16_t sensor_cpi;
uint32_t saved_keyboard;
unsigned storage_writes;
uint8_t storage[EECONFIG_USER_DATA_SIZE];
bool storage_valid;
layer_state_t layer_state, default_layer_state = 1;
keyboard_config_t keyboard_config;
uint16_t dpi_array[] = {600, 900, 1200, 1600, 2400};
uint16_t sent_keycodes[128];
bool sent_presses[128];
unsigned sent_count;
bool stock_scroll;

extern combo_t key_combos[] __attribute__((weak));

__attribute__((weak)) bool pre_process_record_user(uint16_t k, keyrecord_t *r) { (void)k; (void)r; return true; }
__attribute__((weak)) bool process_record_user(uint16_t k, keyrecord_t *r) { (void)k; (void)r; return true; }
__attribute__((weak)) report_mouse_t pointing_device_task_user(report_mouse_t r) { return r; }
__attribute__((weak)) void keyboard_post_init_user(void) {}
__attribute__((weak)) void eeconfig_init_user(void) {}
__attribute__((weak)) void housekeeping_task_user(void) {}
__attribute__((weak)) layer_state_t layer_state_set_user(layer_state_t s) { return s; }
__attribute__((weak)) void via_custom_value_command_kb(uint8_t *d, uint8_t n) { if (n) d[0] = id_unhandled; }
uint16_t combo_count(void) { return key_combos ? 2 : 0; }
combo_t *combo_get(uint16_t i) { return &key_combos[i]; }

uint16_t timer_read(void) { return now; }
uint16_t timer_elapsed(uint16_t start) { return (uint16_t)(now - start); }
uint32_t timer_read32(void) { return now; }
uint32_t timer_elapsed32(uint32_t start) { return now - start; }
void layer_invert(uint8_t layer) { layer_state = layer_state_set_user(layer_state ^ (1u << layer)); }
void layer_clear(void) { layer_state = layer_state_set_user(0); }
void default_layer_set(layer_state_t state) { default_layer_state = state; }
void pointing_device_set_cpi(uint16_t cpi) { sensor_cpi = cpi; }
void eeconfig_update_kb(uint32_t value) { saved_keyboard = value; }
bool eeconfig_is_user_datablock_valid(void) { return storage_valid; }
uint32_t eeconfig_read_user_datablock(void *data, uint32_t offset, uint32_t length) {
    assert(offset + length <= sizeof(storage));
    if (!storage_valid) return 0;
    memcpy(data, storage + offset, length);
    return length;
}
uint32_t eeconfig_update_user_datablock(const void *data, uint32_t offset, uint32_t length) {
    assert(offset + length <= sizeof(storage));
    memcpy(storage + offset, data, length);
    storage_valid = true;
    storage_writes++;
    return length;
}
void cycle_dpi(void) {
    keyboard_config.dpi_config = (keyboard_config.dpi_config + 1) % 5;
    eeconfig_update_kb(keyboard_config.raw);
    pointing_device_set_cpi(dpi_array[keyboard_config.dpi_config]);
}
void clear_weak_mods(void) {}
uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key) { return keymaps[layer][key.row][key.col]; }

void process_record(keyrecord_t *record) {
    uint16_t code = record->keycode ? record->keycode : keymap_key_to_keycode(layer_state & 2 ? 1 : 0, record->event.key);
    if (!process_record_user(code, record)) return;
    if (code == DRAG_SCROLL) {
        if (record->event.pressed) stock_scroll = !stock_scroll;
    } else if (code == DPI_CONFIG) {
        if (record->event.pressed) cycle_dpi();
    } else {
        assert(sent_count < ARRAY_SIZE(sent_keycodes));
        sent_keycodes[sent_count] = code;
        sent_presses[sent_count++] = record->event.pressed;
    }
}
