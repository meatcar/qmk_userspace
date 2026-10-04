// Copyright 2026 meatcar (@meatcar)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qmk_stub.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

extern uint32_t now;
extern bool stock_scroll;
extern unsigned sent_count;
extern uint16_t sensor_cpi;
extern unsigned storage_writes;
extern uint32_t saved_keyboard;
extern uint16_t sent_keycodes[];
extern bool sent_presses[];
extern uint8_t storage[];
extern bool storage_valid;

static void button(uint8_t col, bool pressed) {
    keyrecord_t record = {.event = {.key = {0, col}, .pressed = pressed, .time = timer_read(), .type = KEY_EVENT}};
    uint16_t code = keymap_key_to_keycode(layer_state & 2 ? 1 : 0, record.event.key);
    if (pre_process_record_user(code, &record) && process_combo(code, &record)) process_record(&record);
}

static report_mouse_t motion(int8_t x, int8_t y) {
    report_mouse_t report = pointing_device_task_user((report_mouse_t){.x = x, .y = y, .buttons = 1});
    if (stock_scroll) {
        report.h = report.x / 8;
        report.v = report.y / 8;
        report.x = report.y = 0;
    }
    return report;
}

static void advance(uint32_t ms) {
    now += ms;
    combo_task();
    housekeeping_task_user();
}

static void test_scroll_edges(void) {
    button(3, true);
    advance(199);
    button(3, false);
    report_mouse_t report = motion(16, -24);
    assert(report.x == 0 && report.h == 2 && report.v == -3);
    button(3, true);
    advance(10);
    button(3, false);
    report = motion(16, -24);
    assert(report.x == 16 && report.y == -24 && report.h == 0);

    button(3, true);
    advance(200);
    button(3, false);
    assert(motion(16, 0).x == 16);

    button(3, true);
    report = motion(1, 0);
    assert(report.x == 0 && report.h == 0);
    button(3, false);
    assert(motion(16, 0).x == 16);

    button(3, true);
    report = motion(3, -5);
    assert(report.h == 0 && report.v == 0);
    report = motion(5, -3);
    assert(report.h == 1 && report.v == -1);
    button(3, false);

    button(3, true);
    button(3, false);
    button(3, true);
    report = motion(8, 16);
    assert(report.h == 1 && report.v == 2);
    button(3, false);
    assert(motion(16, 0).x == 16);

    button(3, true);
    advance(65536 + 10);
    button(3, false);
    assert(motion(16, 0).x == 16);
    assert(!stock_scroll);
}

static void test_chords(void) {
    sent_count = 0;
    button(1, true);
    advance(40);
    button(4, true);
    advance(199);
    assert(layer_state == 0 && sent_count == 0);
    advance(1);
    assert(layer_state == 2 && sent_count == 0);
    advance(1000);
    assert(layer_state == 2);
    button(1, false);
    button(4, false);
    assert(sent_count == 0);

    button(3, true);
    report_mouse_t report = motion(40, -24);
    assert(report.h == 0 && report.v == -3 && report.x == 0 && report.y == 0);
    button(3, false);
    report = motion(40, -24);
    assert(report.x == 40 && report.y == -24);

    button(4, true);
    advance(50);
    button(1, true);
    advance(200);
    assert(layer_state == 0);
    button(4, false);
    button(1, false);

    keyboard_config.dpi_config = 4;
    button(2, true);
    advance(20);
    button(4, true);
    advance(200);
    assert(keyboard_config.dpi_config == 0 && sensor_cpi == 600 && sent_count == 0);
    advance(1000);
    assert(keyboard_config.dpi_config == 0);
    button(2, false);
    button(4, false);

    button(1, true);
    advance(51);
    button(4, true);
    advance(250);
    assert(layer_state == 0 && sent_count == 2);
    button(1, false);
    button(4, false);
    assert(sent_count == 4 && sent_keycodes[0] == MS_BTN4 && sent_presses[0]);
    assert(sent_keycodes[1] == MS_BTN2 && sent_presses[1]);
    assert(!sent_presses[2] && !sent_presses[3]);

    sent_count = 0;
    button(1, true);
    advance(20);
    button(4, true);
    advance(100);
    button(1, false);
    advance(500);
    assert(layer_state == 0 && sent_count == 0);
    button(4, false);

    button(1, true);
    advance(10);
    button(4, true);
    advance(5);
    button(0, true);
    assert(layer_state == 0);
    advance(194);
    assert(layer_state == 0);
    advance(1);
    assert(layer_state == 2);
    button(0, false);
    button(4, false);
    button(1, false);
    layer_invert(1);
}

static void set_value(uint8_t id, uint16_t value, bool word) {
    uint8_t packet[32] = {id_custom_set_value, 0, id};
    packet[3] = word ? value >> 8 : value;
    packet[4] = value;
    via_custom_value_command_kb(packet, sizeof(packet));
    assert(packet[0] == id_custom_set_value);
}

static uint16_t get_value(uint8_t id, bool word) {
    uint8_t packet[32] = {id_custom_get_value, 0, id};
    via_custom_value_command_kb(packet, sizeof(packet));
    assert(packet[0] == id_custom_get_value);
    return word ? ((uint16_t)packet[3] << 8) | packet[4] : packet[3];
}

static void test_via_settings(void) {
    unsigned writes = storage_writes;
    set_value(1, 1, false);
    set_value(3, 17, false);
    assert(sensor_cpi == 1700 && get_value(3, false) == 17);
    set_value(7, 3, false);
    set_value(8, 5, false);
    set_value(9, 1, false);
    set_value(11, 350, true);
    set_value(12, 80, false);
    set_value(13, 700, true);
    assert(get_value(11, true) == 350 && get_value(13, true) == 700);
    assert(storage_writes == writes);

    button(3, true);
    report_mouse_t report = motion(11, -17);
    assert(report.h == -3 && report.v == -3);
    report = motion(1, -3);
    assert(report.h == -1 && report.v == -1);
    button(3, false);

    uint8_t save[32] = {id_custom_save, 0};
    via_custom_value_command_kb(save, sizeof(save));
    assert(save[0] == id_custom_save && storage_writes == writes + 1);
    assert(saved_keyboard == keyboard_config.raw);
    set_value(3, 42, false);
    assert(sensor_cpi == 4200);
    set_value(1, 4, false);
    layer_invert(1);
    keyboard_config.raw = saved_keyboard;
    keyboard_post_init_user();
    assert(sensor_cpi == 1700 && get_value(7, false) == 3 && get_value(11, true) == 350);
    assert(layer_state == 0 && default_layer_state == 1);
    report = motion(11, -17);
    assert(report.x == 11 && report.y == -17);

    button(3, true);
    advance(349);
    button(3, false);
    assert(motion(6, 10).x == 0);
    button(3, true);
    button(3, false);
    assert(motion(6, 10).x == 6);
    button(3, true);
    advance(350);
    button(3, false);
    assert(motion(6, 10).x == 6);

    button(4, true);
    advance(79);
    button(1, true);
    advance(699);
    assert(layer_state == 0);
    advance(1);
    assert(layer_state == 2);
    button(4, false);
    button(1, false);
    button(3, true);
    report = motion(63, -10);
    assert(report.h == 0 && report.v == -2);
    button(3, false);
    keyboard_post_init_user();
}

static void test_invalid_settings(void) {
    const uint8_t invalid[][5] = {
        {7, 0, 1, 5}, {7, 0, 2, 0}, {7, 0, 7, 0}, {7, 0, 8, 65},
        {7, 0, 9, 2}, {7, 0, 11, 0, 49}, {7, 0, 12, 251}, {7, 0, 13, 7, 209},
        {7, 0, 99}, {7, 1, 1}, {0xFE, 0, 1}
    };
    unsigned writes = storage_writes;
    for (size_t i = 0; i < ARRAY_SIZE(invalid); i++) {
        uint8_t packet[32] = {0};
        memcpy(packet, invalid[i], 5);
        via_custom_value_command_kb(packet, sizeof(packet));
        assert(packet[0] == id_unhandled);
    }
    assert(get_value(1, false) == 1 && get_value(2, false) == 6);
    assert(get_value(7, false) == 3 && get_value(8, false) == 5);
    assert(get_value(9, false) == 1 && get_value(11, true) == 350 && get_value(13, true) == 700);
    assert(storage_writes == writes);

    via_custom_value_command_kb(NULL, 0);
    uint8_t short_packet[] = {7, 0, 11, 0};
    via_custom_value_command_kb(short_packet, sizeof(short_packet));
    assert(short_packet[0] == id_unhandled && get_value(11, true) == 350);
    memset(storage, 0, EECONFIG_USER_DATA_SIZE);
    keyboard_post_init_user();
    assert(get_value(3, false) == 9 && get_value(7, false) == 8 && get_value(11, true) == 200);
    assert(sensor_cpi == 900 && storage_writes == writes + 1);
    storage_valid = false;
    keyboard_post_init_user();
    assert(storage_writes == writes + 2 && get_value(13, true) == 200);
}

int main(void) {
    now = 100;
    eeconfig_init_user();
    keyboard_post_init_user();
    button(3, true);
    report_mouse_t report = motion(24, -40);
    assert(report.x == 0 && report.y == 0 && report.h == 3 && report.v == -5 && report.buttons == 1);
    now += 80;
    button(3, false);
    report = motion(16, 32);
    assert(report.x == 16 && report.y == 32 && report.h == 0 && report.v == 0);
    test_scroll_edges();
    test_chords();
    test_via_settings();
    test_invalid_settings();
    puts("Adept scroll, chord, and VIA persistence tests passed");
}
